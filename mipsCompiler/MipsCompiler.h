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
#include "MipsAssembler.h"
#include "operationsCompiler.h"

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

std::string compile_expr(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

void compile_instructions(BreakScope* breakScope, const std::vector<Token*>& tokens, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

bool sort_ast(std::vector<Token*>* tokens, Scope* scope);

#endif //I2C2_MIPSCOMPILER_H
