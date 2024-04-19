//
// Created by Ethan Horowitz on 4/2/24.
//

#include "operationsCompiler.h"
#include "MipsCompiler.h"

#ifndef SP
#define SP 29
#endif

#ifndef HP
#define HP 28
#endif


#ifndef RSTATUS
#define RSTATUS 30
#endif

struct RTypeVals{
    uint8_t rd;
    uint8_t rs;
    uint8_t rt;
    std::string resultTag;
    TokenValue resultType;
};

int32_t parse_number(Token* token){
    if (token->val_type == NUMBER_INT){
        return stoi(token->lexeme);
    }
    if (token->val_type == NUMBER_FLOAT){
        float val = stof(token->lexeme);
        int int_part = (int) val;
        int float_part = (int) ((val - int_part) * 65536);
        return (int_part << 16) | float_part;
    }
    throw std::runtime_error("Expected number at " + token->toString());
}

MatchTypeResults match_num_types(std::string& var1, std::string& var2, VariableTracker* varTracker, MipsBuilder* mipsBuilder){
    TokenValue type1 = varTracker->get_var_type(var1);
    int refs1 = varTracker->get_var_type_refs(var1);
    TokenValue type2 = varTracker->get_var_type(var2);
    int refs2 = varTracker->get_var_type_refs(var2);

    if (refs1 > 0 || type1 != FLOAT) type1 = TokenValue::INT;
    if (refs2 > 0|| type2 != FLOAT) type2 = TokenValue::INT;

    if (type1 != type2 && (type1 == TokenValue::FLOAT || type2 == TokenValue::FLOAT)){
        std::string non_float_var;
        if (type1 == TokenValue::FLOAT) non_float_var = var2;
        else non_float_var = var1;

        std::string intermediate = varTracker->add_temp_variable();
        uint8_t reg_intermediate = varTracker->getReg(intermediate);
        varTracker->set_var_type(intermediate, TokenValue::FLOAT);

        // shift left by 16 to make it a floating point operation
        uint8_t reg2 = varTracker->getReg(non_float_var);
        mipsBuilder->addInstruction(new InstrSll(reg_intermediate, reg2, 16), "");

        if (type1 == TokenValue::FLOAT){
            return {
                var1,
                intermediate
            };
        }
        return {
            intermediate,
            var2
        };

    }
    return {
        var1,
        var2
    };
}

std::string force_type(std::string& varHost, std::string& varFollow, VariableTracker* tracker, MipsBuilder* mipsBuilder){
    TokenValue typeHost = tracker->get_var_type(varHost);
    int refsHost = tracker->get_var_type_refs(varHost);
    TokenValue typeFollow = tracker->get_var_type(varFollow);
    int refsFollow = tracker->get_var_type_refs(varFollow);

    if (refsHost > 0 || typeHost != FLOAT) typeHost = TokenValue::INT;
    if (refsFollow > 0 || typeFollow != FLOAT) typeFollow = TokenValue::INT;

    if (typeHost != typeFollow){
        std::string intermediate = tracker->add_temp_variable();
        uint8_t reg_intermediate = tracker->getReg(intermediate);

        if (typeHost == TokenValue::FLOAT && refsHost == 0){
            uint8_t reg = tracker->getReg(varFollow);
            mipsBuilder->addInstruction(new InstrSll(reg_intermediate, reg, 16), "");
        }
        else {
            uint8_t reg = tracker->getReg(varFollow);
            mipsBuilder->addInstruction(new InstrSra(reg_intermediate, reg, 16), "");
        }

        return intermediate;
    }
    return varFollow;
}

TokenValue get_result_type(std::string& var1, std::string& var2, VariableTracker* varTracker){
    TokenValue type1 = varTracker->get_var_type(var1);
    TokenValue type2 = varTracker->get_var_type(var2);
    if (type1 == TokenValue::FLOAT || type2 == TokenValue::FLOAT) return TokenValue::FLOAT;
    return TokenValue::INT;
}

//region binary ops

// parses an add operation. Returns the register of the final result
std::string comp_add(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addOp = (BinaryOpToken*) token;
    std::string left = compile_op("", addOp->left, mipsBuilder, varTracker);

    // use addi
    if (addOp->right->type == TYPE_VALUE){
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result);
        int32_t imm = parse_number(addOp->right);
        if(varTracker->get_var_type(left) == TokenValue::FLOAT || addOp->right->val_type == NUMBER_FLOAT){
            varTracker->set_var_type(result, TokenValue::FLOAT);
        }
        else {
            varTracker->set_var_type(result, TokenValue::INT);
        }

        if (imm > 65536){
            std::string value = compile_op("", addOp->right, mipsBuilder, varTracker);
            uint8_t reg_value = varTracker->getReg(value);
            uint8_t reg_a = varTracker->getReg(left);
            mipsBuilder->addInstruction(new InstrAdd(reg_result, reg_a, reg_value), "");

            varTracker->removeVar(value);
        }
        else {
            uint8_t reg_a = varTracker->getReg(left);
            mipsBuilder->addInstruction(new InstrAddi(reg_result, reg_a, (int16_t) imm), "");
        }

        if (addOp->left->val_type != TokenValue::IDENTIFIER)
            varTracker->removeVar(left);
        return result;
    }

    // use normal add
    std::string right = compile_op("", addOp->right, mipsBuilder, varTracker);

    MatchTypeResults results = match_num_types(left, right, varTracker, mipsBuilder);
    left = results.left;
    right = results.right;

    uint8_t reg_a = varTracker->getReg(left);
    uint8_t reg_b = varTracker->getReg(right);
    if (addOp->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (addOp->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    TokenValue left_type = addOp->left->val_type;
    TokenValue right_type = addOp->right->val_type;

    if (left_type == TokenValue::STRUCT || right_type == TokenValue::STRUCT){
        throw std::runtime_error("Cannot add structs at " + addOp->toString());
    }

    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);
    mipsBuilder->addInstruction(new InstrAdd(reg_result, reg_a, reg_b), "");

    if (left_type == TokenValue::FLOAT || right_type == TokenValue::FLOAT)
        varTracker->set_var_type(result, TokenValue::FLOAT);
    else varTracker->set_var_type(result, TokenValue::INT);

    return result;
}

std::string comp_add_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addEqOp = (BinaryOpToken*) token;
    std::string left = compile_op("", addEqOp->left, mipsBuilder, varTracker);

    // use addi
    if (addEqOp->right->type == TYPE_VALUE){
        uint8_t reg_a = varTracker->getReg(left);
        auto imm = parse_number(addEqOp->right);
        if (imm >= 65536 || imm < -65536){
            std::string right = compile_op("", addEqOp->right, mipsBuilder, varTracker);
            uint8_t reg_b = varTracker->getReg(right);
            mipsBuilder->addInstruction(new InstrAdd(reg_a, reg_a, reg_b), "");
        }
        else {
            mipsBuilder->addInstruction(new InstrAddi(reg_a, reg_a, (int16_t) imm), "");
        }

        return left;
    }

    // use normal add
    std::string right = compile_op("", addEqOp->right, mipsBuilder, varTracker);

    MatchTypeResults results = match_num_types(left, right, varTracker, mipsBuilder);
    left = results.left;
    right = results.right;

    uint8_t reg_a = varTracker->getReg(left);
    uint8_t reg_b = varTracker->getReg(right);
    if (addEqOp->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (addEqOp->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    mipsBuilder->addInstruction(new InstrAdd(reg_a, reg_a, reg_b), "");
    return left;
}

RTypeVals comp_bin_op(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker, bool match_types=false){
    auto* op = (BinaryOpToken*) token;
    std::string left = compile_op("", op->left, mipsBuilder, varTracker);
    std::string right = compile_op("", op->right, mipsBuilder, varTracker);

    if (match_types){
        MatchTypeResults results = match_num_types(left, right, varTracker, mipsBuilder);
        left = results.left;
        right = results.right;
    }

    uint8_t reg_a = varTracker->getReg(left);
    uint8_t reg_b = varTracker->getReg(right);
    varTracker->removeIfTemp(left);
    varTracker->removeIfTemp(right);

    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);

    TokenValue resultType = varTracker->get_var_type(left);
    if (match_types) resultType = get_result_type(left, right, varTracker);

    return {
            reg_result,
            reg_a,
            reg_b,
            result,
            resultType
    };
}

RTypeVals comp_bin_op_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker, bool match_types=false){
    auto* op = (BinaryOpToken*) token;
    std::string left = compile_op("", op->left, mipsBuilder, varTracker);
    std::string right = compile_op("", op->right, mipsBuilder, varTracker);

    if (match_types) {
        MatchTypeResults results = match_num_types(left, right, varTracker, mipsBuilder);
        left = results.left;
        right = results.right;
    }

    uint8_t reg_a = varTracker->getReg(left);
    uint8_t reg_b = varTracker->getReg(right);
    varTracker->removeIfTemp(left);
    varTracker->removeIfTemp(right);

    TokenValue resultType = varTracker->get_var_type(left);
    if (match_types) resultType = get_result_type(left, right, varTracker);

    return {
            reg_a,
            reg_a,
            reg_b,
            left,
            resultType
    };
}

std::string comp_minus_or_minus_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker, true) :
                     comp_bin_op(token, mipsBuilder, varTracker, true);
    mipsBuilder->addInstruction(new InstrSub(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, vals.resultType);
    return vals.resultTag;
}

std::string comp_mult_or_mult_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker, true) :
                     comp_bin_op(token, mipsBuilder, varTracker, true);
    mipsBuilder->addInstruction(new InstrMul(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, vals.resultType);
    return vals.resultTag;
}

std::string comp_div_or_div_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker, true) :
                     comp_bin_op(token, mipsBuilder, varTracker, true);
    mipsBuilder->addInstruction(new InstrDiv(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, vals.resultType);
    return vals.resultTag;
}

std::string comp_bin_and_or_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrAnd(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, vals.resultType);
    return vals.resultTag;
}

std::string comp_bin_or_or_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrOr(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, vals.resultType);
    return vals.resultTag;
}

std::string comp_xor_or_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // a ^ b = (a | b) - (a & b)
    // $1 = a | b
    // $reg = a & b
    // $reg = $1 - $reg
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrOr(1, vals.rs, vals.rt), "");
    mipsBuilder->addInstruction(new InstrAnd(vals.rd, vals.rs, vals.rt), "");
    mipsBuilder->addInstruction(new InstrSub(vals.rd, 1, vals.rd), "");
    varTracker->set_var_type(vals.resultTag, vals.resultType);
    return vals.resultTag;
}

std::string comp_sll_or_sll_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    BinaryOpToken* op = (BinaryOpToken*) token;
    std::string left = compile_op("", op->left, mipsBuilder, varTracker);
    Token* right = op->right;
    if (right->val_type != NUMBER_INT) throw std::runtime_error("Shift amount must be a number at " + right->toString());
    int shift = stoi(right->lexeme);
    uint8_t reg_a = varTracker->getReg(left);
    if (is_eq){
        mipsBuilder->addInstruction(new InstrSll(reg_a, reg_a, shift), "");
        return left;
    }
    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);
    mipsBuilder->addInstruction(new InstrSll(reg_result, reg_a, shift), "");
    varTracker->set_var_type(result, varTracker->get_var_type(left));
    return result;
}

std::string comp_sra_or_sra_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    BinaryOpToken* op = (BinaryOpToken*) token;
    std::string left = compile_op("", op->left, mipsBuilder, varTracker);
    Token* right = op->right;
    if (right->val_type != NUMBER_INT) throw std::runtime_error("Shift amount must be a number at " + right->toString());
    int shift = stoi(right->lexeme);
    uint8_t reg_a = varTracker->getReg(left);
    if (is_eq){
        mipsBuilder->addInstruction(new InstrSra(reg_a, reg_a, shift), "");
        return left;
    }
    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);
    mipsBuilder->addInstruction(new InstrSra(reg_result, reg_a, shift), "");
    varTracker->set_var_type(result, varTracker->get_var_type(left));
    return result;
}

std::string comp_lt(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker, true);
        mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, break_to), "");
        varTracker->set_var_type(vals.resultTag, TokenValue::INT);
        return vals.resultTag;
    }

    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker, true);
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     (pseudo) lt $reg, $r_a, $r_b =
     ble $r_a, $r_b, label_true
     addi $reg, $0, 0
     j label_end
     label_true:
     addi $reg, $0, 1
     label_end: noop, this can be filtered out later in an optimizing step
    */
    mipsBuilder->addInstruction(new InstrSlt(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, TokenValue::INT);
    return vals.resultTag;
}

std::string comp_lte(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // a <= b is the same as a < b or a = b

    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker, true);
        // if breaking to something, simply break
        // (pseudo) blte $r_a, $r_b, label
        // blt $r_a, $r_b, label
        // bne $r_a, $r_b, no_jump
        // j label
        // no_jump: nop
        std::string label_no_jump = mipsBuilder->genUnnamedLabel();
        mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, break_to), "");
        mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_no_jump), "");
        mipsBuilder->addInstruction(new InstrJ(break_to), "");
        mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_no_jump);
        varTracker->set_var_type(vals.resultTag, TokenValue::INT);
        return vals.resultTag;
    }
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker, true);
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     (pseudo) lte $reg, $r_a, $r_b =
     blt $r_a, $r_b, label_true
     bne $r_a, $r_b, label_false
     addi $reg, $0, 1
     j label_end
     label_false: addi $reg, $0, 0
     j label_end
     label_true:
     addi $reg, $0, 1
     label_end: noop, this can be filtered out later in an optimizing step
    */
    std::string label_true = mipsBuilder->genUnnamedLabel();
    std::string label_false = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, label_true), "");
    mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_false), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), label_false);
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), label_true);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    varTracker->set_var_type(vals.resultTag, TokenValue::INT);
    return vals.resultTag;
}

std::string comp_gt(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // a > b is the same as !(a <= b) = !(a < b) && !(a == b)

    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker, true);
        // break if not < and not =
        /*
         (pseudo) bgt $reg_a, reg_b, label
         blt $reg_a, $reg_b, label_no_jump
         bne $reg_a, $reg_b, label
         label_no_jump: nop
         */
        std::string temp_var = varTracker->add_temp_variable();
        uint8_t reg_temp = varTracker->getReg(temp_var);
        mipsBuilder->addInstruction(new InstrSgt(reg_temp, vals.rs, vals.rt), "");
        mipsBuilder->addInstruction(new InstrBne(reg_temp, 0, break_to), "");
        varTracker->removeVar(temp_var);
        varTracker->set_var_type(vals.resultTag, TokenValue::INT);
        return vals.resultTag;
    }
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker, true);
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     (pseudo) gt $reg, $r_a, $r_b =
     blt $r_a, $r_b, label_false
     bne $r_a, $r_b, label_true
     label_false: addi $reg, $0, 0
     j label_end
     label_true: addi $reg, $0, 1
     label_end: noop, this can be filtered out later in an optimizing step
    */
    mipsBuilder->addInstruction(new InstrSgt(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, TokenValue::INT);
    return vals.resultTag;
}

std::string comp_gte(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // a >= b is the same as !(a < b)

    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker, true);
        // break if equal, or break if not a < b
        // (pseudo) bgte $reg_a, reg_b, label
        // bne $reg_a, $reg_b, label_stage_2
        // j label
        // label_stage_2: blt $reg_a, $reg_b, label_no_jump
        // j label
        // label_no_jump: nop
        std::string temp_var = varTracker->add_temp_variable();
        uint8_t reg_temp = varTracker->getReg(temp_var);
        mipsBuilder->addInstruction(new InstrSge(reg_temp, vals.rs, vals.rt), "");
        mipsBuilder->addInstruction(new InstrBne(reg_temp, 0, break_to), "");
        varTracker->removeVar(temp_var);
        varTracker->set_var_type(vals.resultTag, TokenValue::INT);
        return vals.resultTag;
    }
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker, true);
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     (pseudo) gte $reg, $r_a, $r_b =
     blt $r_a, $r_b, label_false
     addi $reg, $0, 1
     j label_end
     label_false: addi $reg, $0, 0
     label_end: noop, this can be filtered out later in an optimizing step
    */
    mipsBuilder->addInstruction(new InstrSge(vals.rd, vals.rs, vals.rt), "");
    varTracker->set_var_type(vals.resultTag, TokenValue::INT);
    return vals.resultTag;
}

std::string comp_eq_eq(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker, true);

    if (!break_to.empty()){
        // break if equal
        // (pseudo) beq $reg_a, reg_b, label
        // bne $reg_a, $reg_b, label_no_jump
        // j label
        // label_no_jump: nop
        std::string label_no_jump = mipsBuilder->genUnnamedLabel();
        mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_no_jump), "");
        mipsBuilder->addInstruction(new InstrJ(break_to), "");
        mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_no_jump);
        varTracker->set_var_type(vals.resultTag, TokenValue::INT);
        return vals.resultTag;
    }
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     (pseudo) seteq $reg, $reg_a, $reg_b
     bne $r_a, $r_b, label_false
     addi $reg, $0, 1
     j label_end
     label_false: addi $reg, $0, 0
     label_end: noop, this can be filtered out later in an optimizing step
    */
    std::string label_false = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_false), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), label_false);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    varTracker->set_var_type(vals.resultTag, TokenValue::INT);
    return vals.resultTag;
}

std::string comp_not_eq(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker, true);

    if (!break_to.empty()){
        // literally just a bne
        mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, break_to), "");
        varTracker->set_var_type(vals.resultTag, TokenValue::INT);
        return vals.resultTag;
    }
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     bne $reg_a, $reg_b, label_true
     addi $reg, $0, 0
     j label_end
     label_true: addi $reg, $0, 0
     label_end: nop
    */
    std::string label_true = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_true), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), label_true);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    varTracker->set_var_type(vals.resultTag, TokenValue::INT);
    return vals.resultTag;
}

std::string compile_array_set(Token* token, Token* value, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;

    auto* var_tok = (Token*) addr->left;
    if (var_tok->val_type == NUMBER_INT) throw std::runtime_error("Cannot dereference a number at " + var_tok->toString());
    if (var_tok->val_type == FUNCTION) throw std::runtime_error("Cannot dereference a function at " + var_tok->toString());
    if (var_tok->val_type != IDENTIFIER) throw std::runtime_error("Cannot dereference a non-variable at " + var_tok->toString());

    std::string var = var_tok->lexeme;

    bool use_right_num = false;
    int right_num;

    if (addr->right->val_type == NUMBER_INT){
        use_right_num = true;
        right_num = stoi(addr->right->lexeme);
    }

    std::string value_str = compile_op("", value, mipsBuilder, varTracker);
    std::string index;
    if (!use_right_num) {
        index = compile_op("", addr->right, mipsBuilder, varTracker);
    }
    if (!index.empty() && varTracker->get_var_type(index) != TokenValue::INT)
        throw std::runtime_error("Array index must be an integer at " + addr->right->toString());

    uint8_t mem = varTracker->getReg(var);
    uint8_t value_reg = varTracker->getReg(value_str);

    varTracker->removeIfTemp(value_str);

    if (use_right_num){
        mipsBuilder->addInstruction(new InstrSw(value_reg, mem, (int16_t) right_num), "");
    }
    else {
        uint8_t offset_reg = varTracker->getReg(index);
        mipsBuilder->addInstruction(new InstrAdd(offset_reg, offset_reg, mem), "");
        mipsBuilder->addInstruction(new InstrSw(value_reg, offset_reg, 0), "");

        varTracker->removeIfTemp(index);
    }

    varTracker->removeIfTemp(var);

    return value_str;

}

std::string comp_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    std::string left;

    if (op->left->val_type == DEREF){
        auto* addr = (BinaryOpToken*) op->left;
        std::string result = compile_op("", addr->right, mipsBuilder, varTracker);
        std::string right = compile_op("", op->right, mipsBuilder, varTracker);

        uint8_t reg_a = varTracker->getReg(result);
        uint8_t reg_b = varTracker->getReg(right);
        mipsBuilder->addInstruction(new InstrSw(reg_b, reg_a, 0), "");

        varTracker->removeIfTemp(right);
        return result;
    }

    if (op->left->val_type == ARRAY){
        // array access
        return compile_array_set(op->left, op->right, mipsBuilder, varTracker);
    }

    if (op->left->val_type != IDENTIFIER){
        left = compile_op("", op->left, mipsBuilder, varTracker);
    }
    else left = op->left->lexeme;

    if (op->right->type == TYPE_VALUE){
        uint8_t reg_a = varTracker->getReg(left);
        auto imm = parse_number(op->right);
        if (imm > 65536 || imm <= -65536){
            std::string value = compile_op("", op->right, mipsBuilder, varTracker);
            uint8_t reg_value = varTracker->getReg(value);
            mipsBuilder->addInstruction(new InstrAdd(reg_a, 0, reg_value), "");

            varTracker->removeVar(value);
        }
        else {
            // use addi
            mipsBuilder->addInstruction(new InstrAddi(reg_a, 0, (int16_t) imm), "");
        }
        return left;
    }

    std::string right = compile_op("", op->right, mipsBuilder, varTracker);
    right = force_type(left, right, varTracker, mipsBuilder);
    uint8_t reg_a = varTracker->getReg(left);
    uint8_t reg_b = varTracker->getReg(right);

    mipsBuilder->addInstruction(new InstrAdd(reg_a, 0, reg_b), "");

    varTracker->removeIfTemp(right);

    return left;
}

//endregion

//region unary ops

std::string comp_ref(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;
    auto* value = (Token*) addr->right;
    if (value->val_type == NUMBER_INT) throw std::runtime_error("Cannot take address of a number at " + value->toString());
    if (value->val_type == FUNCTION) throw std::runtime_error("Cannot take address of a function at " + value->toString());
    if (value->val_type != IDENTIFIER) throw std::runtime_error("Cannot take address of a non-variable at " + value->toString());

    std::string var = value->lexeme;
    int mem_loc = varTracker->get_mem_addr(var);
    // positive = stack, negative = global
    if (mem_loc <= 0){
        mem_loc = -mem_loc;
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result);
        mipsBuilder->addInstruction(new InstrAddi(reg_result, 0, (int16_t) mem_loc), "");

        varTracker->set_var_type(result, varTracker->get_var_type(var));
        varTracker->set_var_type_refs(result, varTracker->get_var_type_refs(var) + 1);

        return result;
    }
    // mem is positive, is on stack
    mem_loc -= 1; // stack mem is returned as 1-indexed
    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);
    mipsBuilder->addInstruction(new InstrAddi(reg_result, SP, (int16_t) mem_loc), "");

    varTracker->set_var_type(result, varTracker->get_var_type(var));
    varTracker->set_var_type_refs(result, varTracker->get_var_type_refs(var) + 1);

    return result;
}

std::string comp_deref(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;
    auto* value = (Token*) addr->right;
    if (value->val_type == NUMBER_INT) throw std::runtime_error("Cannot dereference a number at " + value->toString());
    if (value->val_type == FUNCTION) throw std::runtime_error("Cannot dereference a function at " + value->toString());
    if (value->val_type != IDENTIFIER) throw std::runtime_error("Cannot dereference a non-variable at " + value->toString());

    std::string var = value->lexeme;

    uint8_t reg = varTracker->getReg(var);
    std::string temp_var = varTracker->add_temp_variable();
    uint8_t reg_temp = varTracker->getReg(temp_var);
    mipsBuilder->addInstruction(new InstrLw(reg_temp, reg, 0), "");

    varTracker->set_var_type(temp_var, varTracker->get_var_type(var));
    int num_refs = varTracker->get_var_type_refs(var);
    if (num_refs == 0) throw std::runtime_error("Cannot dereference a variable with no references at " + value->toString());
    varTracker->set_var_type_refs(temp_var, num_refs - 1);
    return temp_var;
}

std::string comp_not(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    Token* value = op->right;
    std::string right_label = compile_op("", value, mipsBuilder, varTracker);
    uint8_t reg = varTracker->getReg(right_label);

    if (!break_to.empty()){
        // break if not equal
        // (pseudo) bne $reg_a, reg_b, label
        // beq $reg_a, $reg_b, label_no_jump
        // j label
        // label_no_jump: nop
        std::string label_no_jump = mipsBuilder->genUnnamedLabel();
        mipsBuilder->addInstruction(new InstrBne(reg, 0, label_no_jump), "");
        mipsBuilder->addInstruction(new InstrJ(break_to), "");
        mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_no_jump);
        varTracker->set_var_type(right_label, TokenValue::INT);
        return right_label;
    }
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     (pseudo) setne $reg, $reg_a, $reg_b
     beq $r_a, $r_b, label_false
     addi $reg, $0, 1
     j label_end
     label_false: addi $reg, $0, 0
     label_end: noop, this can be filtered out later in an optimizing step
    */
    std::string label_false = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();

    std::string result_tag = varTracker->add_temp_variable();
    uint8_t result_reg = varTracker->getReg(result_tag);

    mipsBuilder->addInstruction(new InstrBne(reg, 0, label_false), "");
    mipsBuilder->addInstruction(new InstrAddi(result_reg, 0, 1), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(result_reg, 0, 0), label_false);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop

    varTracker->set_var_type(result_tag, TokenValue::INT);
    return result_tag;
}

//endregion

void compile_instructions(BreakScope* breakScope, const std::vector<Token*>& tokens, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

std::string compile_inline_function_call(FunctionCallToken* call, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    FunctionToken* func = varTracker->get_inline_function(call->lexeme);

    std::vector<std::string> args;
    std::map<std::string, std::string> saved_vars;

    for (int i = 0; i < call->arguments.size(); i++){
        Token* arg = call->arguments[i];
        DefinitionToken* arg_name_tok = call->arg_names[i];
        std::string result = compile_op("", arg, mipsBuilder, varTracker);
        varTracker->getReg(result);
        std::string arg_name = arg_name_tok->name;

        if (varTracker->var_exists(arg_name)){
            std::string new_name = varTracker->add_temp_variable();
            uint8_t cur_reg = varTracker->getReg(arg_name);
            uint8_t reg_new_name = varTracker->getReg(new_name);
            mipsBuilder->addInstruction(new InstrAdd(reg_new_name, 0, cur_reg), "");
            saved_vars[arg_name] = new_name;
            arg_name = new_name;
        }

        varTracker->set_alias(arg_name, result);
        args.push_back(arg_name);
    }
    // compile rest

    std::string endLabel = mipsBuilder->genUnnamedLabel();

    varTracker->incScope(true);
    varTracker->set_in_inline(true);

    BreakScope breakScope;
    breakScope.returnLabel = endLabel;
    compile_instructions(&breakScope, func->body->expressions, mipsBuilder, varTracker);

    int retvar = -1;
    if (varTracker->var_exists("return")){
        retvar = varTracker->getReg("return");
    }

    varTracker->decScope(true);
    varTracker->set_in_inline(false);

    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), endLabel);

    std::string result;

    // load return value
    if (call->returnType != VOID) {
        if(retvar != -1){
            result = varTracker->gen_temp_var_name();
            varTracker->add_variable(result, retvar);
        }
        else {
            result = varTracker->add_temp_variable();
            uint8_t reg = varTracker->getReg(result);
            mipsBuilder->addInstruction(new InstrAdd(reg, 0, 2), "");
        }
    }

    for (const std::string& arg : args){
        varTracker->removeIfTemp(arg);
        varTracker->remove_alias(arg);
    }
    for (const auto& pair : saved_vars){
        varTracker->renameVar(pair.second, pair.first);
    }

    return result;
}

std::string compile_function_call(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* call = (FunctionCallToken*) token;

    if (call->is_inline){
        return compile_inline_function_call(call, mipsBuilder, varTracker);
    }

    std::vector<std::string> args;
    std::vector<int> arg_regs;

    for (Token* arg : call->arguments){
        std::string name = compile_op("", arg, mipsBuilder, varTracker);
        args.push_back(name);
    }

    // save current registers in scope

    if (!args.empty()){
        varTracker->reserve_reg(4);
        mipsBuilder->addInstruction(new InstrAdd(4, 0, varTracker->getReg(args[0])), "");
        varTracker->removeIfTemp(args[0]);
    }
    if (args.size() >= 2){
        varTracker->reserve_reg(5);
        mipsBuilder->addInstruction(new InstrAdd(5, 0, varTracker->getReg(args[1])), "");
        varTracker->removeIfTemp(args[1]);
    }
    if (args.size() >= 3){
        varTracker->reserve_reg(6);
        mipsBuilder->addInstruction(new InstrAdd(6, 0, varTracker->getReg(args[2])), "");
        varTracker->removeIfTemp(args[2]);
    }
    if (args.size() >= 4){
        varTracker->reserve_reg(7);
        mipsBuilder->addInstruction(new InstrAdd(7, 0, varTracker->getReg(args[3])), "");
        varTracker->removeIfTemp(args[3]);
    }

    varTracker->store_current_regs_in_stack();

    if (args.size() > 4){
        // load arguments into stack pointer
        int num_args_left = args.size() - 4;
        varTracker->add_stack_offset(num_args_left);
        for (int i = 4; i < args.size(); i++){
            mipsBuilder->addInstruction(new InstrSw(varTracker->getReg(args[i]), SP, i - 4), "");
            varTracker->removeIfTemp(args[i]);
        }
    }

    // jal
    mipsBuilder->addInstruction(new InstrJal(call->lexeme), "");

    // bring stack back from arguments
    if (args.size() > 4){
        int num_args_left = args.size() - 4;
        varTracker->reduce_stack_offset(num_args_left);
    }

    // restore registers
    varTracker->restore_regs_from_stack();

    // save v0 into a register
    if (call->returnType != VOID){
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result);
        mipsBuilder->addInstruction(new InstrAdd(reg_result, 0, 2), "");

        varTracker->set_var_type(result, call->returnType);
        varTracker->set_var_type_refs(result, call->returnTypeRefs);

        return result;
    }
    return "";
}

std::string compile_array_access(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;

    TokenValue val_type = varTracker->get_var_type(addr->left->lexeme);
    int refCount = varTracker->get_var_type_refs(addr->left->lexeme);

    if (refCount == 0) throw std::runtime_error("Variable not array at " + addr->left->toString());

    auto* var_tok = (Token*) addr->left;
    if (var_tok->val_type == NUMBER_INT) throw std::runtime_error("Cannot dereference a number at " + var_tok->toString());
    if (var_tok->val_type == FUNCTION) throw std::runtime_error("Cannot dereference a function at " + var_tok->toString());
    if (var_tok->val_type != IDENTIFIER) throw std::runtime_error("Cannot dereference a non-variable at " + var_tok->toString());

    std::string var = var_tok->lexeme;

    bool use_right_num = false;
    int right_num;

    if (addr->right->val_type == NUMBER_INT){
        use_right_num = true;
        right_num = stoi(addr->right->lexeme);
    }

    std::string index;
    if (!use_right_num) {
        index = compile_op("", addr->right, mipsBuilder, varTracker);
    }

    int mem_reg = varTracker->getReg(var);

    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);

    if (use_right_num){
        mipsBuilder->addInstruction(new InstrLw(reg_result, mem_reg, (int16_t) right_num), "");
    }
    else {
        uint8_t offset_reg = varTracker->getReg(index);
        mipsBuilder->addInstruction(new InstrAdd(offset_reg, offset_reg, mem_reg), "");
        mipsBuilder->addInstruction(new InstrLw(reg_result, offset_reg, 0), "");
    }

    varTracker->set_var_type(result, val_type);
    varTracker->set_var_type_refs(result, refCount - 1);

    return result;
}

std::string compile_op(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (token->type == TokenType::TYPE_OPERATOR){

        if (token->val_type == FUNCTION){
            return compile_function_call(token, mipsBuilder, varTracker);
        }

        auto* op = (BinaryOpToken*) token;
        switch (op->val_type) {
            case TokenValue::ADD:
                return comp_add(token, mipsBuilder, varTracker);
            case TokenValue::ADD_EQ:
                return comp_add_eq(token, mipsBuilder, varTracker);
            case TokenValue::MINUS:
                return comp_minus_or_minus_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::MINUS_EQ:
                return comp_minus_or_minus_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::MULT:
                return comp_mult_or_mult_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::MULT_EQ:
                return comp_mult_or_mult_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::DIV:
                return comp_div_or_div_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::DIV_EQ:
                return comp_div_or_div_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::BIN_AND:
                return comp_bin_and_or_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::BIN_AND_EQ:
                return comp_bin_and_or_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::BIN_OR:
                return comp_bin_or_or_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::BIN_OR_EQ:
                return comp_bin_or_or_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::XOR:
                return comp_xor_or_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::XOR_EQ:
                return comp_xor_or_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::LSHIFT:
                return comp_sll_or_sll_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::LSHIFT_EQ:
                return comp_sll_or_sll_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::RSHIFT:
                return comp_sra_or_sra_eq(false, token, mipsBuilder, varTracker);
            case TokenValue::RSHIFT_EQ:
                return comp_sra_or_sra_eq(true, token, mipsBuilder, varTracker);
            case TokenValue::LT:
                return comp_lt(break_to, token, mipsBuilder, varTracker);
            case TokenValue::LTE:
                return comp_lte(break_to, token, mipsBuilder, varTracker);
            case TokenValue::GT:
                return comp_gt(break_to, token, mipsBuilder, varTracker);
            case TokenValue::GTE:
                return comp_gte(break_to, token, mipsBuilder, varTracker);
            case TokenValue::EQ_EQ:
                return comp_eq_eq(break_to, token, mipsBuilder, varTracker);
            case TokenValue::NOT:
                return comp_not(break_to, token, mipsBuilder, varTracker);
            case TokenValue::NOT_EQ:
                return comp_not_eq(break_to, token, mipsBuilder, varTracker);
            case TokenValue::EQ:
                return comp_eq(token, mipsBuilder, varTracker);
            case TokenValue::REF:
                return comp_ref(token, mipsBuilder, varTracker);
            case TokenValue::DEREF:
                return comp_deref(token, mipsBuilder, varTracker);
            case TokenValue::ARRAY:
                return compile_array_access(token, mipsBuilder, varTracker);
            default:
                throw std::runtime_error("Invalid token value for compile_op");
        }
    }
    if (token->type == TokenType::TYPE_VALUE){
        std::string varname = varTracker->add_temp_variable();
        uint8_t reg = varTracker->getReg(varname);
        varTracker->set_var_type(varname, token->val_type == NUMBER_INT ? TokenValue::INT : TokenValue::FLOAT);
        int32_t value = parse_number(token);
        if (value < 65536 && value > -65536) {
            mipsBuilder->addInstruction(new InstrAddi(reg, 0, value), "");
        }
        else {
            // load upper then lower
            int16_t upper = (int16_t) (value >> 16);
            int32_t lower = (value & 0x0000FFFF);
            mipsBuilder->addInstruction(new InstrAddi(reg, 0, upper), "");
            mipsBuilder->addInstruction(new InstrSll(reg, reg, 16), "");
            if ((int16_t) lower < 0){
                int16_t lower_m1 = (lower >> 1) & 0x7FFF;
                mipsBuilder->addInstruction(new InstrAddi(1, 0, lower_m1), "");
                mipsBuilder->addInstruction(new InstrSll(1, 1, 1), "");
                mipsBuilder->addInstruction(new InstrAddi(1, 1, lower%2), "");
                mipsBuilder->addInstruction(new InstrOr(reg, reg, 1), "");
            }
            else mipsBuilder->addInstruction(new InstrAddi(reg, reg, (int16_t) lower), "");
        }
        return varname;
    }
    if (token->type == TokenType::TYPE_IDENTIFIER && token->val_type == TokenValue::IDENTIFIER){
        return token->lexeme;
    }
    throw std::runtime_error("Invalid token type for compile_op");
}
