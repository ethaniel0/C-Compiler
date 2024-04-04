//
// Created by Ethan Horowitz on 3/31/24.
//

#ifndef I2C2_MIPSASSEMBLER_H
#define I2C2_MIPSASSEMBLER_H
#include "VariableTracker.h"
#include "MipsBuilder.h"


void assembleMips(const std::string& mips, MipsBuilder* builder, VariableTracker* tracker);


#endif //I2C2_MIPSASSEMBLER_H
