//
// Created by Ethan Horowitz on 4/2/24.
//

#ifndef I2C2_OPERATIONSCOMPILER_H
#define I2C2_OPERATIONSCOMPILER_H

#include "../parsing/parse.h"
#include "../mips/MipsInstructions.h"
#include "MipsBuilder.h"
#include "VariableTracker.h"
#include "MipsAssembler.h"

std::string compile_op(const std::string& break_to, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker);

#endif //I2C2_OPERATIONSCOMPILER_H
