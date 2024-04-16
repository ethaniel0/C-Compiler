cmake_minimum_required(VERSION 3.26)
project(i2c2)

set(CMAKE_CXX_STANDARD 17)

add_executable(i2c2
        main.cpp
        parsing/parse.cpp
        parsing/token.cpp
        mips/MipsRunner.cpp
        mipsCompiler/MipsCompiler.cpp
        mipsCompiler/VariableTracker.cpp
        mipsCompiler/MipsBuilder.cpp
        mipsCompiler/MipsAssembler.cpp
        mipsCompiler/MipsAssembler.h
        mipsCompiler/operationsCompiler.cpp
        mipsCompiler/operationsCompiler.h)
add_executable(parser_tests tests/tests.cpp
        tests/testFramework/TestFramework.cpp
        tests/tokenizerTests.cpp
        tests/tokenizerTests.h
        parsing/parse.cpp
        parsing/token.cpp
        tests/parserTests.cpp
        tests/mipsTests.cpp
        mips/MipsRunner.cpp
        mipsCompiler/MipsCompiler.cpp
        mipsCompiler/VariableTracker.cpp
        mipsCompiler/MipsBuilder.cpp
        tests/compilationTests.cpp
        tests/compilationTests.h
        mipsCompiler/MipsAssembler.cpp
        mipsCompiler/operationsCompiler.cpp)
