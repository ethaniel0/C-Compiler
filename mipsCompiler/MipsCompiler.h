//
// Created by Ethan Horowitz on 3/15/24.
//

#ifndef I2C2_MIPSCOMPILER_H
#define I2C2_MIPSCOMPILER_H

#include "../parsing/parse.h"
#include "../mips/MipsRunner.h"
#include "../mips/MipsInstructions.h"
#include "MipsBuilder.h"
#include "VariableTracker.h"

/*
BinaryOpToken -> [string of operations]
IfElseToken -> [string of operations for condition],
                bne or ble,
                two branches (else first, then if with label)

FunctionCallToken -> store used registers in stack,
                      store $31 in stack,
                      jal
                      restore used registers
                      store return value in new register

DefinitionToken -> store value in register

FunctionToken -> store $s registers in stack,
                  [string of operations]
                  restore $s registers
                  if main, j to endLabel (end of program)
                  else jr $31

ReturnToken -> [string of operations for return value]
                store return value in $2
                jr $31

ForToken -> [string of operations for condition],
             bne or ble,
             two branches (else first, then if with label)
             [string of operations for loop]
             bne or ble
             two branches (else first, then if with label)

WhileToken -> [string of operations for condition],
                bne or ble,
                two branches (else first, then if with label)
                [string of operations for loop]
                bne or ble
                two branches (else first, then if with label)
 */

struct BreakScope {
    std::string returnLabel;
    std::string breakLabel;
    std::string continueLabel;
    BreakScope(){
        returnLabel = "";
        breakLabel = "";
        continueLabel = "";
    }
};

std::string compile_op(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

std::string compile_expr(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

void compile_instructions(BreakScope* breakScope, const std::vector<Token*>& tokens, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

void sort_ast(std::vector<Token*>* tokens);

#endif //I2C2_MIPSCOMPILER_H
