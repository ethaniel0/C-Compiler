//
// Created by Ethan Horowitz on 4/2/24.
//

#ifndef I2C2_OPERATIONSCOMPILER_H
#define I2C2_OPERATIONSCOMPILER_H

#include <vector>
#include <stdexcept>
#include <string>

#include "../parsing/parse.h"
#include "../mips/MipsInstructions.h"
#include "MipsBuilder.h"
#include "VariableTracker.h"
#include "MipsAssembler.h"

struct MatchTypeResults{
    std::string left;
    std::string right;
};

MatchTypeResults match_num_types(std::string& var1, std::string& var2, VariableTracker* varTracker, MipsBuilder* mipsBuilder);

std::string force_type(std::string& varHost, std::string& varFollow, VariableTracker* tracker, MipsBuilder* mipsBuilder);

std::string compile_op(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

#endif //I2C2_OPERATIONSCOMPILER_H
