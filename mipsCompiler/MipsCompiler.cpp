//
// Created by Ethan Horowitz on 3/15/24.
//

#include "MipsCompiler.h"

#define SP 29
#define RSTATUS 30
#define RET_ADDR 31

// parses an add operation. Returns the register of the final result
std::string comp_add(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addOp = (BinaryOpToken*) token;
    std::string left = compile_op("", addOp->left, mipsBuilder, varTracker);

    // use addi
    if (addOp->right->type == TYPE_VALUE){
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result);
        auto imm = (int16_t) stoi(addOp->right->lexeme);
        uint8_t reg_a = varTracker->getReg(left);
        mipsBuilder->addInstruction(new InstrAddi(reg_result, reg_a, imm), "");

        if (addOp->left->val_type != TokenValue::IDENTIFIER)
            varTracker->removeVar(left);
        return result;
    }

    // use normal add
    std::string right = compile_op("", addOp->right, mipsBuilder, varTracker);
    uint8_t reg_a = varTracker->getReg(left);
    uint8_t reg_b = varTracker->getReg(right);
    if (addOp->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (addOp->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);
    mipsBuilder->addInstruction(new InstrAdd(reg_result, reg_a, reg_b), "");
    return result;
}

std::string comp_add_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addEqOp = (BinaryOpToken*) token;
    std::string left = compile_op("", addEqOp->left, mipsBuilder, varTracker);
    uint8_t reg_a = varTracker->getReg(left);

    // use addi
    if (addEqOp->right->type == TYPE_VALUE){
        auto imm = (int16_t) stoi(addEqOp->right->lexeme);
        mipsBuilder->addInstruction(new InstrAddi(reg_a, reg_a, imm), "");
        return left;
    }

    // use normal add
    std::string right = compile_op("", addEqOp->right, mipsBuilder, varTracker);
    uint8_t reg_b = varTracker->getReg(right);
    if (addEqOp->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (addEqOp->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    mipsBuilder->addInstruction(new InstrAdd(reg_a, reg_a, reg_b), "");
    return left;
}

struct RTypeVals{
    uint8_t rd;
    uint8_t rs;
    uint8_t rt;
    std::string resultTag;
};

//region binary ops

RTypeVals comp_bin_op(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    std::string left = compile_op("", op->left, mipsBuilder, varTracker);
    std::string right = compile_op("", op->right, mipsBuilder, varTracker);

    uint8_t reg_a = varTracker->getReg(left);
    uint8_t reg_b = varTracker->getReg(right);
    if (op->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (op->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result);

    return {
        reg_result,
        reg_a,
        reg_b,
        result
    };
}

RTypeVals comp_bin_op_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    std::string left = compile_op("", op->left, mipsBuilder, varTracker);
    uint8_t reg_a = varTracker->getReg(left);

    std::string right = compile_op("", op->right, mipsBuilder, varTracker);
    uint8_t reg_b = varTracker->getReg(right);
    if (op->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (op->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    return {
        reg_a,
        reg_a,
        reg_b,
        left
    };
}

std::string comp_minus_or_minus_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrSub(vals.rd, vals.rs, vals.rt), "");
    return vals.resultTag;
}

std::string comp_mult_or_mult_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrMul(vals.rd, vals.rs, vals.rt), "");
    return vals.resultTag;
}

std::string comp_div_or_div_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrDiv(vals.rd, vals.rs, vals.rt), "");
    return vals.resultTag;
}

std::string comp_bin_and_or_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrAnd(vals.rd, vals.rs, vals.rt), "");
    return vals.resultTag;
}

std::string comp_bin_or_or_eq(bool is_eq, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = is_eq ?
                     comp_bin_op_eq(token, mipsBuilder, varTracker) :
                     comp_bin_op(token, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrOr(vals.rd, vals.rs, vals.rt), "");
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
    return vals.resultTag;
}

std::string comp_lt(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker);
        mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, break_to), "");
        return vals.resultTag;
    }

    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker);
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
    std::string label_true = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, label_true), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), label_true);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    return vals.resultTag;
}

std::string comp_lte(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){


    // a <= b is the same as a < b or a = b

    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker);
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
        return vals.resultTag;
    }
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker);
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
    return vals.resultTag;
}

std::string comp_gt(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // a > b is the same as !(a <= b) = !(a < b) && !(a == b)

    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker);
        // break if not < and not =
        /*
         (pseudo) bgt $reg_a, reg_b, label
         blt $reg_a, $reg_b, label_no_jump
         bne $reg_a, $reg_b, label
         label_no_jump: nop
         */
        std::string label_no_jump = mipsBuilder->genUnnamedLabel();
        mipsBuilder->addInstruction(new InstrBlt(vals.rd, vals.rt, label_no_jump), "");
        mipsBuilder->addInstruction(new InstrBne(vals.rd, vals.rt, break_to), "");
        mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_no_jump);
        return vals.resultTag;
    }
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker);
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
    std::string label_true = mipsBuilder->genUnnamedLabel();
    std::string label_false = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, label_false), "");
    mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_true), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), label_false);
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), label_true);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    return vals.resultTag;
}

std::string comp_gte(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // a >= b is the same as !(a < b)

    if (!break_to.empty()){
        RTypeVals vals = comp_bin_op_eq(token, mipsBuilder, varTracker);
        // break if equal, or break if not a < b
        // (pseudo) bgte $reg_a, reg_b, label
        // bne $reg_a, $reg_b, label_stage_2
        // j label
        // label_stage_2: blt $reg_a, $reg_b, label_no_jump
        // j label
        // label_no_jump: nop
        std::string label_stage_2 = mipsBuilder->genUnnamedLabel();
        std::string label_no_jump = mipsBuilder->genUnnamedLabel();
        mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_stage_2), "");
        mipsBuilder->addInstruction(new InstrJ(break_to), "");
        mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, label_no_jump), label_stage_2);
        mipsBuilder->addInstruction(new InstrJ(break_to), "");
        mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_no_jump);
        return vals.resultTag;
    }
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker);
    /*
     if not breaking, then use blt to set register to 1 if true, 0 if false
     (pseudo) gte $reg, $r_a, $r_b =
     blt $r_a, $r_b, label_false
     addi $reg, $0, 1
     j label_end
     label_false: addi $reg, $0, 0
     label_end: noop, this can be filtered out later in an optimizing step
    */
    std::string label_false = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, label_false), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), label_false);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    return vals.resultTag;
}

std::string comp_eq_eq(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker);

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
    mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, label_false), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), label_false);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    return vals.resultTag;
}

std::string comp_not_eq(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    RTypeVals vals = comp_bin_op(token, mipsBuilder, varTracker);

    if (!break_to.empty()){
        // literally just a bne
        mipsBuilder->addInstruction(new InstrBne(vals.rd, vals.rt, break_to), "");
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
    mipsBuilder->addInstruction(new InstrBlt(vals.rs, vals.rt, label_true), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), label_true);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    return vals.resultTag;
}

std::string comp_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    std::string left;

    if (op->left->val_type != IDENTIFIER){
        left = compile_op("", op->left, mipsBuilder, varTracker);
    }
    else left = op->left->lexeme;


    uint8_t reg_a = varTracker->getReg(left);

    if (op->right->type == TYPE_VALUE){
        auto imm = (int16_t) stoi(op->right->lexeme);
        mipsBuilder->addInstruction(new InstrAddi(reg_a, 0, imm), "");
        return left;
    }

    std::string right = compile_op("", op->right, mipsBuilder, varTracker);
    uint8_t reg_b = varTracker->getReg(right);

    mipsBuilder->addInstruction(new InstrAdd(reg_a, 0, reg_b), "");

    if (op->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    return left;
}

//endregion

std::string compile_function_call(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* call = (FunctionCallToken*) token;
    std::vector<std::string> args;
    std::vector<int> arg_regs;
    for (Token* arg : call->arguments){
        std::string name = compile_op("", arg, mipsBuilder, varTracker);
        args.push_back(name);
    }
    if (!args.empty()) mipsBuilder->addInstruction(new InstrAdd(4, 0, varTracker->getReg(args[0])), "");
    if (args.size() >= 2) mipsBuilder->addInstruction(new InstrAdd(5, 0, varTracker->getReg(args[1])), "");
    if (args.size() >= 3) mipsBuilder->addInstruction(new InstrAdd(6, 0, varTracker->getReg(args[2])), "");
    if (args.size() >= 4) mipsBuilder->addInstruction(new InstrAdd(7, 0, varTracker->getReg(args[3])), "");
    if (args.size() > 4){
        // load arguments into stack pointer
        int num_args_left = args.size() - 4;
        mipsBuilder->addInstruction(new InstrSub(SP, SP, num_args_left), "");
        for (int i = 4; i < args.size(); i++){
            mipsBuilder->addInstruction(new InstrLw(SP, varTracker->getReg(args[3]), i-4), "");
        }
    }

    // save current registers in scope
    varTracker->store_current_regs_in_stack();

    // jal
    mipsBuilder->addInstruction(new InstrJal(call->lexeme), "");

    // restore registers
    varTracker->restore_regs_from_stack();
    // save v0 into a register
    if (call->returnType != VOID){
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result);
        mipsBuilder->addInstruction(new InstrAdd(reg_result, 0, 2), "");
        return result;
    }
    return "";
}

std::string compile_op(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (token->type == TokenType::TYPE_OPERATOR){

        if (token->val_type == FUNCTION){
            return compile_function_call(nullptr, token, mipsBuilder, varTracker);
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
            case TokenValue::NOT_EQ:
                return comp_not_eq(break_to, token, mipsBuilder, varTracker);
            case TokenValue::EQ:
                return comp_eq(token, mipsBuilder, varTracker);
            default:
                throw std::runtime_error("Invalid token value for compile_op");
        }
    }
    if (token->type == TokenType::TYPE_VALUE){
        std::string varname = varTracker->add_temp_variable();
        uint8_t reg = varTracker->getReg(varname);
        mipsBuilder->addInstruction(new InstrAddi(reg, 0, stoi(token->lexeme)), "");
        return varname;
    }
    if (token->type == TokenType::TYPE_IDENTIFIER && token->val_type == TokenValue::IDENTIFIER){
        return token->lexeme;
    }
    throw std::runtime_error("Invalid token type for compile_op");
}

std::string compile_value_def(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* def = (DefinitionToken*) token;
    std::string value = compile_op("", def->value, mipsBuilder, varTracker);
    if (def->value->type != TYPE_IDENTIFIER ){
        varTracker->renameVar(value, def->name);
    }
    else {
        uint8_t reg = varTracker->getReg(def->name);
        uint8_t val_reg = varTracker->getReg(value);
        mipsBuilder->addInstruction(new InstrAdd(reg, 0, val_reg), "");
    }

    return def->name;
}

void compile_jump_condition(const std::string& break_to, Token* condition, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (condition->val_type == LT || condition->val_type == LTE ||
        condition->val_type == GT || condition->val_type == GTE ||
        condition->val_type == EQ_EQ || condition->val_type == NOT_EQ){
        compile_op(break_to, condition, mipsBuilder, varTracker);
    }
    else {
        std::string v = compile_op("", condition, mipsBuilder, varTracker);
        uint8_t reg = varTracker->getReg(v);
        mipsBuilder->addInstruction(new InstrBne(reg, 0, break_to), "");
    }
}

void compile_if(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* if_statement = (IfElseToken*) token;
    Token* condition = if_statement->condition;
    std::string label_true = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    compile_jump_condition(label_true, condition, mipsBuilder, varTracker);
    // if elses
    std::vector<std::string> elseIfLabels;
    if (!if_statement->elseIfConditions.empty()){
        for (Token* c : if_statement->elseIfConditions){
            std::string label = mipsBuilder->genUnnamedLabel();
            elseIfLabels.push_back(label);
            compile_jump_condition(label, c, mipsBuilder, varTracker);
        }
    }
    // write else, then jump
    if (if_statement->elseBody != nullptr){
        compile_instructions(breakScope, if_statement->elseBody->expressions, mipsBuilder, varTracker);
    }
    mipsBuilder->addInstruction(new InstrJ(label_end), "");

    // write if elses
    for (int i = 0; i < if_statement->elseIfBodies.size(); i++){
        // noop as start to label
        mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), elseIfLabels[i]);
        compile_instructions(breakScope, if_statement->elseIfBodies[i]->expressions, mipsBuilder, varTracker);
        mipsBuilder->addInstruction(new InstrJ(label_end), "");
    }

    // write if
    // noop as start
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_true);
    compile_instructions(breakScope, if_statement->ifBody->expressions, mipsBuilder, varTracker);

    // noop at end as end label
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end);
}

void compile_break(Token* token, BreakScope* breakScope, MipsBuilder* mipsBuilder){
    if (breakScope->breakLabel.empty()) throw std::runtime_error("Break outside of loop at " + token->toString());
    mipsBuilder->addInstruction(new InstrJ(breakScope->breakLabel), "");
}
void compile_continue(Token* token, BreakScope* breakScope, MipsBuilder* mipsBuilder){
    if (breakScope->continueLabel.empty()) throw std::runtime_error("Continue outside of loop at " + token->toString());
    mipsBuilder->addInstruction(new InstrJ(breakScope->continueLabel), "");
}
void continue_return(Token* token, BreakScope* breakScope, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (breakScope->returnLabel.empty()) throw std::runtime_error("Return outside of function at " + token->toString());
    // get value
    auto* ret = (ReturnToken*) token;
    if (ret->value != nullptr){
        std::string value = compile_op("", ret->value, mipsBuilder, varTracker);
        uint8_t reg = varTracker->getReg(value);
        mipsBuilder->addInstruction(new InstrAdd(2, 0, reg), "");
    }
    mipsBuilder->addInstruction(new InstrJ(breakScope->returnLabel), "");
}

void compile_for(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* for_statement = (ForToken*) token;
    std::string label_loop_condition = mipsBuilder->genUnnamedLabel();
    std::string label_loop = mipsBuilder->genUnnamedLabel();
    std::string label_incr = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();

    /*
     Structure:
     [init]
     loopCondition:
        [condition]
        j loopEnd
     loop:
        [loop]
        [increment]
        j loopCondition
     loopEnd: noop
     */

    // init
    if (for_statement->init != nullptr){
        compile_expr(breakScope, for_statement->init, mipsBuilder, varTracker);
    }
    // loop condition
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_loop_condition);
    if (for_statement->condition != nullptr){
        compile_jump_condition(label_loop, for_statement->condition, mipsBuilder, varTracker);
        mipsBuilder->addInstruction(new InstrJ(label_end), "");
    }

    // configure break scope
    std::string prev_break = breakScope->breakLabel;
    std::string prev_continue = breakScope->continueLabel;

    breakScope->breakLabel = label_end;
    breakScope->continueLabel = label_incr;

    // loop body
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_loop);
    if (for_statement->body != nullptr){
        compile_instructions(breakScope, for_statement->body->expressions, mipsBuilder, varTracker);
    }
    // increment
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_incr);
    if (for_statement->increment != nullptr){
        compile_expr(breakScope, for_statement->increment, mipsBuilder, varTracker);
    }
    // jump back to condition and label end
    mipsBuilder->addInstruction(new InstrJ(label_loop_condition), "");
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end);

    // reset break and continue
    breakScope->breakLabel = prev_break;
    breakScope->continueLabel = prev_continue;
}

void compile_while(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* while_statement = (WhileToken*) token;
    std::string label_loop_condition = mipsBuilder->genUnnamedLabel();
    std::string label_loop = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();

    /*
     Structure:
     loopCondition:
        [condition]
        j loopEnd
     loop:
        [loop]
        j loopCondition
     loopEnd: noop
     */

    // loop condition
    compile_jump_condition(label_loop, while_statement->condition, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrJ(label_end), "");

    // configure break scope
    std::string prev_break = breakScope->breakLabel;
    std::string prev_continue = breakScope->continueLabel;

    breakScope->breakLabel = label_end;
    breakScope->continueLabel = label_loop_condition;

    // loop body
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_loop);
    if (while_statement->body != nullptr){
        compile_instructions(breakScope, while_statement->body->expressions, mipsBuilder, varTracker);
    }
    // jump back to condition and label end
    mipsBuilder->addInstruction(new InstrJ(label_loop_condition), "");
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end);

    // reset break and continue
    breakScope->breakLabel = prev_break;
    breakScope->continueLabel = prev_continue;
}

void compile_function(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // $29 is the stack pointer
    // $31 is the return address

    // skip over compile function definition so it's only run when called
    std::string after_function = mipsBuilder->genUnnamedLabel();
    std::string just_jump = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrJ(after_function), "");

    auto* function = (FunctionToken*) token;
    std::string name = function->name;
    /*
        Structure:
        1. save stack pointer in stack
        2. save duplicated registers that are in use
        3. run code
        4. restore duplicated registers
        5. restore stack pointer
        6. return
     */
    varTracker->incScope();
    // save ra
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), name);

    // load first four arguments into registers
    for (int i = 0; i < function->parameters.size() && i < 4; i++){
        uint8_t reg = varTracker->add_variable(function->parameters[i]->name);
        mipsBuilder->addInstruction(new InstrAddi(reg, 4+i, 0), "");
    }

    // if more arguments, load them into stack pointer
    if (function->parameters.size() > 4){
        int num_to_load = function->parameters.size() - 4;
        // load arguments into stack pointer
        for (int i = 4; i < function->parameters.size(); i++){
            uint8_t reg = varTracker->getReg(function->parameters[i]->name);
            mipsBuilder->addInstruction(new InstrLw(reg, varTracker->getReg(function->parameters[i]->name), num_to_load - i), "");
        }
    }

    // adjust breakscope
    std::string func_end = mipsBuilder->genUnnamedLabel();
    std::string prev_return = breakScope->returnLabel;
    breakScope->returnLabel = just_jump;

    // run code
    compile_instructions(breakScope, function->body->expressions, mipsBuilder, varTracker);

    // load ra back
//    int offset = varTracker->get_stack_offset();
//    mipsBuilder->addInstruction(new InstrLw(31, SP, offset-1), func_end);

    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), just_jump);
    varTracker->decScope();
    // jd $ra
    mipsBuilder->addInstruction(new InstrJr(31), "");

    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), after_function);
}

std::string compile_expr(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (token == nullptr) return "";
    if (token->type == TokenType::TYPE_OPERATOR && token->val_type == TokenValue::IDENTIFIER){
        return compile_value_def(token, mipsBuilder, varTracker);
    }
    else if (token->type == TokenType::TYPE_OPERATOR){
        return compile_op("", token, mipsBuilder, varTracker);
    }
    else if (token->type == TokenType::TYPE_KEYWORD){
        if (token->val_type == TokenValue::IF) compile_if(breakScope, token, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::FOR) compile_for(breakScope, token, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::WHILE) compile_while(breakScope, token, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::BREAK) compile_break(token, breakScope, mipsBuilder);
        else if (token->val_type == TokenValue::CONTINUE) compile_continue(token, breakScope, mipsBuilder);
        else if (token->val_type == TokenValue::RETURN) continue_return(token, breakScope, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::FUNCTION) compile_function(breakScope, token, mipsBuilder, varTracker);
    }
    return "";
}

void compile_instructions(BreakScope* breakScope, const std::vector<Token*>& tokens, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // tokens are the heads of parse trees - parse each one individually, then append them together
    // make main function first, so the first instruction starts there
    for (Token* token : tokens){
        compile_expr(breakScope, token, mipsBuilder, varTracker);
    }
}

void sort_ast(std::vector<Token*>* tokens){
    std::vector<Token*> functions;
    for (int i = 0; i < tokens->size(); i++){
        if ((*tokens)[i]->type == TokenType::TYPE_KEYWORD && (*tokens)[i]->val_type == TokenValue::FUNCTION){
            functions.push_back((*tokens)[i]);
            // remove token
            tokens->erase(tokens->begin() + i);
            i--;
        }
    }
    for (Token* function : functions) {
        tokens->push_back(function);
    }
}