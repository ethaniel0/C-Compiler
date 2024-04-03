//
// Created by Ethan Horowitz on 4/2/24.
//

#include "operationsCompiler.h"

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
};

//region binary ops

// parses an add operation. Returns the register of the final result
std::string comp_add(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addOp = (BinaryOpToken*) token;
    std::string left = compile_op("", addOp->left, mipsBuilder, varTracker);

    // use addi
    if (addOp->right->type == TYPE_VALUE){
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result, 0);
        auto imm = (int16_t) stoi(addOp->right->lexeme);
        uint8_t reg_a = varTracker->getReg(left, addOp->left->track);
        mipsBuilder->addInstruction(new InstrAddi(reg_result, reg_a, imm), "");

        if (addOp->left->val_type != TokenValue::IDENTIFIER)
            varTracker->removeVar(left);
        return result;
    }

    // use normal add
    std::string right = compile_op("", addOp->right, mipsBuilder, varTracker);
    uint8_t reg_a = varTracker->getReg(left, addOp->left->track);
    uint8_t reg_b = varTracker->getReg(right, addOp->right->track);
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
    uint8_t reg_result = varTracker->getReg(result, 0);
    mipsBuilder->addInstruction(new InstrAdd(reg_result, reg_a, reg_b), "");

    if (left_type == TokenValue::FLOAT || right_type == TokenValue::FLOAT)
        varTracker->set_var_type(result, TokenValue::FLOAT);
    else if (left_type == TokenValue::INT || right_type == TokenValue::INT)
        varTracker->set_var_type(result, TokenValue::INT);
    else
        varTracker->set_var_type(result, left_type);

    return result;
}

std::string comp_add_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addEqOp = (BinaryOpToken*) token;
    std::string left = compile_op("", addEqOp->left, mipsBuilder, varTracker);

    TokenValue left_type = addEqOp->left->val_type;
    TokenValue right_type = addEqOp->right->val_type;
    if (right_type != left_type){
        if (right_type == TokenValue::FLOAT){
            // shift right by 16 to truncate
            std::string temp = varTracker->add_temp_variable();
            uint8_t reg_temp = varTracker->getReg(temp, 0);
//            mipsBuilder->addInstruction(new InstrSra())
        }
    }

    // use addi
    if (addEqOp->right->type == TYPE_VALUE){
        uint8_t reg_a = varTracker->getReg(left, addEqOp->left->track);
        auto imm = (int16_t) stoi(addEqOp->right->lexeme);
        mipsBuilder->addInstruction(new InstrAddi(reg_a, reg_a, imm), "");
        return left;
    }

    // use normal add
    std::string right = compile_op("", addEqOp->right, mipsBuilder, varTracker);
    uint8_t reg_a = varTracker->getReg(left, addEqOp->left->track);
    uint8_t reg_b = varTracker->getReg(right, addEqOp->right->track);
    if (addEqOp->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (addEqOp->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    mipsBuilder->addInstruction(new InstrAdd(reg_a, reg_a, reg_b), "");
    return left;
}

RTypeVals comp_bin_op(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    std::string left = compile_op("", op->left, mipsBuilder, varTracker);
    std::string right = compile_op("", op->right, mipsBuilder, varTracker);

    uint8_t reg_a = varTracker->getReg(left, op->left->track);
    uint8_t reg_b = varTracker->getReg(right, op->right->track);
    if (op->left->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(left);
    if (op->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result, 0);

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
    std::string right = compile_op("", op->right, mipsBuilder, varTracker);

    uint8_t reg_a = varTracker->getReg(left, op->left->track);
    uint8_t reg_b = varTracker->getReg(right, op->right->track);
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
    mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_false), "");
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
    mipsBuilder->addInstruction(new InstrBne(vals.rs, vals.rt, label_true), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 0), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(vals.rd, 0, 1), label_true);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    return vals.resultTag;
}

std::string compile_array_set(Token* token, Token* value, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;

    auto* var_tok = (Token*) addr->left;
    if (var_tok->val_type == NUMBER) throw std::runtime_error("Cannot dereference a number at " + var_tok->toString());
    if (var_tok->val_type == FUNCTION) throw std::runtime_error("Cannot dereference a function at " + var_tok->toString());
    if (var_tok->val_type != IDENTIFIER) throw std::runtime_error("Cannot dereference a non-variable at " + var_tok->toString());

    std::string var = var_tok->lexeme;

    bool use_right_num = false;
    int right_num;

    if (addr->right->val_type == NUMBER){
        use_right_num = true;
        right_num = stoi(addr->right->lexeme);
    }

    std::string value_str = compile_op("", value, mipsBuilder, varTracker);
    uint8_t value_reg = varTracker->getReg(value_str, value->track);

    std::string index;
    if (!use_right_num) {
        index = compile_op("", addr->right, mipsBuilder, varTracker);
    }

    int mem = varTracker->getReg(var, -1);

    if (use_right_num){
        mipsBuilder->addInstruction(new InstrSw(value_reg, mem, (int16_t) right_num), "");
    }
    else {
        uint8_t offset_reg = varTracker->getReg(index, -1);
        mipsBuilder->addInstruction(new InstrAdd(offset_reg, offset_reg, mem), "");
        mipsBuilder->addInstruction(new InstrLw(value_reg, offset_reg, 0), "");
    }

    return value_str;

}

std::string comp_eq(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    std::string left;

    if (op->left->val_type == DEREF){
        auto* addr = (BinaryOpToken*) op->left;
        std::string result = compile_op("", addr->right, mipsBuilder, varTracker);
        std::string right = compile_op("", op->right, mipsBuilder, varTracker);
        uint8_t reg_a = varTracker->getReg(result, op->left->track);
        uint8_t reg_b = varTracker->getReg(right, op->right->track);
        mipsBuilder->addInstruction(new InstrSw(reg_b, reg_a, 0), "");

        if (op->right->val_type != TokenValue::IDENTIFIER)
            varTracker->removeVar(right);
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
        uint8_t reg_a = varTracker->getReg(left, op->left->track);
        auto imm = (int16_t) stoi(op->right->lexeme);
        mipsBuilder->addInstruction(new InstrAddi(reg_a, 0, imm), "");
        return left;
    }

    std::string right = compile_op("", op->right, mipsBuilder, varTracker);
    uint8_t reg_a = varTracker->getReg(left, op->left->track);
    uint8_t reg_b = varTracker->getReg(right, op->right->track);

    mipsBuilder->addInstruction(new InstrAdd(reg_a, 0, reg_b), "");

    if (op->right->val_type != TokenValue::IDENTIFIER)
        varTracker->removeVar(right);

    return left;
}

//endregion

//region unary ops

std::string comp_ref(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;
    auto* value = (Token*) addr->right;
    if (value->val_type == NUMBER) throw std::runtime_error("Cannot take address of a number at " + value->toString());
    if (value->val_type == FUNCTION) throw std::runtime_error("Cannot take address of a function at " + value->toString());
    if (value->val_type != IDENTIFIER) throw std::runtime_error("Cannot take address of a non-variable at " + value->toString());

    std::string var = value->lexeme;
    int mem_loc = varTracker->get_mem_addr(var);
    // positive = stack, negative = global
    if (mem_loc <= 0){
        mem_loc = -mem_loc;
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result, 0);
        mipsBuilder->addInstruction(new InstrAddi(reg_result, 0, (int16_t) mem_loc), "");
        return result;
    }
    // mem is positive, is on stack
    mem_loc -= 1; // stack mem is returned as 1-indexed
    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result, 0);
    mipsBuilder->addInstruction(new InstrAddi(reg_result, SP, (int16_t) mem_loc), "");
    return result;
}

std::string comp_deref(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;
    auto* value = (Token*) addr->right;
    if (value->val_type == NUMBER) throw std::runtime_error("Cannot dereference a number at " + value->toString());
    if (value->val_type == FUNCTION) throw std::runtime_error("Cannot dereference a function at " + value->toString());
    if (value->val_type != IDENTIFIER) throw std::runtime_error("Cannot dereference a non-variable at " + value->toString());

    std::string var = value->lexeme;

    uint8_t reg = varTracker->getReg(var, value->track);
    std::string temp_var = varTracker->add_temp_variable();
    uint8_t reg_temp = varTracker->getReg(temp_var, 0);
    mipsBuilder->addInstruction(new InstrLw(reg_temp, reg, 0), "");
    return temp_var;
}

std::string comp_not(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* op = (BinaryOpToken*) token;
    Token* value = op->right;
    std::string right_label = compile_op("", value, mipsBuilder, varTracker);
    uint8_t reg = varTracker->getReg(right_label, value->track);

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
    uint8_t result_reg = varTracker->getReg(result_tag, 0);

    mipsBuilder->addInstruction(new InstrBne(reg, 0, label_false), "");
    mipsBuilder->addInstruction(new InstrAddi(result_reg, 0, 1), "");
    mipsBuilder->addInstruction(new InstrJ(label_end), "");
    mipsBuilder->addInstruction(new InstrAddi(result_reg, 0, 0), label_false);
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end); // noop
    return result_tag;
}


//endregion

std::string compile_function_call(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* call = (FunctionCallToken*) token;
    std::vector<std::string> args;
    std::vector<int> arg_regs;
    for (Token* arg : call->arguments){
        std::string name = compile_op("", arg, mipsBuilder, varTracker);
        varTracker->set_track_number(name, 2);
        args.push_back(name);
    }

    // save current registers in scope

    if (!args.empty()){
        varTracker->reserve_reg(4);
        mipsBuilder->addInstruction(new InstrAdd(4, 0, varTracker->getReg(args[0], -1)), "");
    }
    if (args.size() >= 2){
        varTracker->reserve_reg(5);
        mipsBuilder->addInstruction(new InstrAdd(5, 0, varTracker->getReg(args[1], -1)), "");
    }
    if (args.size() >= 3){
        varTracker->reserve_reg(6);
        mipsBuilder->addInstruction(new InstrAdd(6, 0, varTracker->getReg(args[2], -1)), "");
    }
    if (args.size() >= 4){
        varTracker->reserve_reg(7);
        mipsBuilder->addInstruction(new InstrAdd(7, 0, varTracker->getReg(args[3], -1)), "");
    }

    varTracker->store_current_regs_in_stack();

    if (args.size() > 4){
        // load arguments into stack pointer
        int num_args_left = args.size() - 4;
        mipsBuilder->addInstruction(new InstrAddi(SP, SP, -num_args_left), "");
        for (int i = 4; i < args.size(); i++){
            mipsBuilder->addInstruction(new InstrSw(varTracker->getReg(args[i], 0), SP, i-4), "");
        }
    }

    // jal
    mipsBuilder->addInstruction(new InstrJal(call->lexeme), "");

    // bring stack back from arguments
    if (args.size() > 4){
        int num_args_left = args.size() - 4;
        mipsBuilder->addInstruction(new InstrAddi(SP, SP, num_args_left), "");
    }

    // restore registers
    varTracker->restore_regs_from_stack();

    // restore stack pointer
    if (args.size() > 4){
        int num_args_left = args.size() - 4;
        mipsBuilder->addInstruction(new InstrAddi(SP, SP, num_args_left), "");
    }

    // save v0 into a register
    if (call->returnType != VOID){
        std::string result = varTracker->add_temp_variable();
        uint8_t reg_result = varTracker->getReg(result, 0);
        mipsBuilder->addInstruction(new InstrAdd(reg_result, 0, 2), "");
        return result;
    }
    return "";
}

std::string compile_array_access(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* addr = (BinaryOpToken*) token;

    auto* var_tok = (Token*) addr->left;
    if (var_tok->val_type == NUMBER) throw std::runtime_error("Cannot dereference a number at " + var_tok->toString());
    if (var_tok->val_type == FUNCTION) throw std::runtime_error("Cannot dereference a function at " + var_tok->toString());
    if (var_tok->val_type != IDENTIFIER) throw std::runtime_error("Cannot dereference a non-variable at " + var_tok->toString());

    std::string var = var_tok->lexeme;

    bool use_right_num = false;
    int right_num;

    if (addr->right->val_type == NUMBER){
        use_right_num = true;
        right_num = stoi(addr->right->lexeme);
    }

    std::string index;
    if (!use_right_num) {
        index = compile_op("", addr->right, mipsBuilder, varTracker);
    }

    int mem_reg = varTracker->getReg(var, -1);

    // on heap
    std::string result = varTracker->add_temp_variable();
    uint8_t reg_result = varTracker->getReg(result, 0);

    if (use_right_num){
        mipsBuilder->addInstruction(new InstrLw(reg_result, mem_reg, (int16_t) right_num), "");
    }
    else {
        uint8_t offset_reg = varTracker->getReg(index, -1);
        mipsBuilder->addInstruction(new InstrAdd(offset_reg, offset_reg, mem_reg), "");
        mipsBuilder->addInstruction(new InstrLw(reg_result, offset_reg, 0), "");
    }
    return result;


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
        uint8_t reg = varTracker->getReg(varname, 0);
        mipsBuilder->addInstruction(new InstrAddi(reg, 0, stoi(token->lexeme)), "");
        return varname;
    }
    if (token->type == TokenType::TYPE_IDENTIFIER && token->val_type == TokenValue::IDENTIFIER){
        return token->lexeme;
    }
    throw std::runtime_error("Invalid token type for compile_op");
}
