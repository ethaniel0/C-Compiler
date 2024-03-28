//
// Created by Ethan Horowitz on 3/8/24.
//

#ifndef I2C_TOKENIZE_H
#define I2C_TOKENIZE_H

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "tokenTypes.h"

std::vector<Token> tokenize(std::string source);

#endif //I2C_TOKENIZE_H
