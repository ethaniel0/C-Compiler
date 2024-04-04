//
// Created by Ethan Horowitz on 3/16/24.
//

#include "compilationTests.h"

#include <utility>

void run_compilation_tests(){
    RUN_TEST_GROUP("compilation");
}

TEST(compilation, one_add){
    char code[] = "1 + 2";

    std::vector<Token> tokens = tokenize(code);
    std::vector<Token*> token_ptrs = toTokenRefs(tokens);
    TokenIterator tokens_iter(token_ptrs);
    Scope scope(nullptr);
    std::vector<Token*> ast = parse(tokens_iter, &scope);

    auto* expr = (BinaryOpToken*) ast[0];

    ASSERT_EQ(expr->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(expr->val_type, TokenValue::ADD, %d)

    MipsBuilder builder;
    VariableTracker tracker(&builder);
    BreakScope breakScope;
    std::string result = compile_expr(&breakScope, expr, &builder, &tracker);

    builder.linkLabels();

    std::vector<Instruction*> instructions = builder.getInstructions();
    ASSERT_EQ(instructions.size(), 2, %d)
    Instruction* i1 = instructions[0];
    Instruction* i2 = instructions[1];
    EXPECT_EQ(i1->type, InstructionType::I_ADDI, %d)
    EXPECT_EQ(i2->type, InstructionType::I_ADDI, %d)
}

TEST(compilation, one_def){
    char code[] = "int a = 1;";

    std::vector<Token> tokens = tokenize(code);
    std::vector<Token*> token_ptrs = toTokenRefs(tokens);
    TokenIterator tokens_iter(token_ptrs);
    Scope scope(nullptr);
    std::vector<Token*> ast = parse(tokens_iter, &scope);

    auto* def = (DefinitionToken*) ast[0];

    MipsBuilder builder;
    VariableTracker tracker(&builder);
    BreakScope breakScope;
    std::string result = compile_expr(&breakScope, def, &builder, &tracker);

    uint8_t result_reg = tracker.getReg(result, -1);

    builder.linkLabels();

    std::vector<Instruction*> instructions = builder.getInstructions();

    MipsRunner runner(1024, instructions.data(), instructions.size());
    runner.run(5);
    uint32_t reg_result = runner.get_reg(result_reg);
    ASSERT_EQ(reg_result, 1, %d)
}

TEST(compilation, def_with_expression){
    char code[] = "int a = 1 + 2 * 8;";

    std::vector<Token> tokens = tokenize(code);
    std::vector<Token*> token_ptrs = toTokenRefs(tokens);
    TokenIterator tokens_iter(token_ptrs);
    Scope scope(nullptr);
    std::vector<Token*> ast = parse(tokens_iter, &scope);

    auto* def = (DefinitionToken*) ast[0];

    auto *expr = (BinaryOpToken*) def->value;
    ASSERT_EQ(expr->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(expr->val_type, TokenValue::ADD, %d)

    ASSERT_EQ(expr->left->val_type, TokenValue::NUMBER, %d)
    ASSERT_EQ(expr->right->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(expr->right->val_type, TokenValue::MULT, %d)
    auto* mult = (BinaryOpToken*) expr->right;
    ASSERT_EQ(mult->left->val_type, TokenValue::NUMBER, %d)
    ASSERT_EQ(mult->right->val_type, TokenValue::NUMBER, %d)

    MipsBuilder builder;
    VariableTracker tracker(&builder);
    BreakScope breakScope;
    std::string result = compile_expr(&breakScope, def, &builder, &tracker);

    uint8_t result_reg = tracker.getReg(result, -1);

    builder.linkLabels();

    std::vector<Instruction*> instructions = builder.getInstructions();

    MipsRunner runner(1024, instructions.data(), instructions.size());
    runner.run(8);
    uint32_t reg_result = runner.get_reg(result_reg);
    ASSERT_EQ(reg_result, 17, %d)
}

TEST(compilation, two_dependent_defs){
    char code[] = "int a = 1; "
                  "int b = a + 2;";

    std::vector<Token> tokens = tokenize(code);
    std::vector<Token*> token_ptrs = toTokenRefs(tokens);
    TokenIterator tokens_iter(token_ptrs);
    Scope scope(nullptr);
    std::vector<Token*> ast = parse(tokens_iter, &scope);
    ASSERT_EQ(ast.size(), 2, %d)

    MipsBuilder builder;
    VariableTracker tracker(&builder);
    BreakScope breakScope;
    compile_expr(&breakScope, ast[0], &builder, &tracker);
    compile_expr(&breakScope, ast[1], &builder, &tracker);


    uint8_t result_reg1 = tracker.getReg("a", -1);
    uint8_t result_reg2 = tracker.getReg("b", -1);

    builder.linkLabels();

    std::vector<Instruction*> instructions = builder.getInstructions();

    MipsRunner runner(1024, instructions.data(), instructions.size());
    runner.run(8);
    uint32_t reg_result1 = runner.get_reg(result_reg1);
    uint32_t reg_result2 = runner.get_reg(result_reg2);
    ASSERT_EQ(reg_result1, 1, %d)
    ASSERT_EQ(reg_result2, 3, %d)
}

int test_with_regs(std::string source, int cycles, std::map<std::string, int32_t> expectedVarMap, bool print_instructions=false, bool use_mem=false){
    std::vector<Token> tokens = tokenize(std::move(source));

    std::vector<Token*> token_ptrs = toTokenRefs(tokens);
    TokenIterator tokens_iter(token_ptrs);
    Scope scope(nullptr);
    std::vector<Token*> ast = parse(tokens_iter, &scope);

    sort_ast(&ast, &scope);

    MipsBuilder builder;
    builder.addInstruction(new InstrAddi(29, 29, 2047), "");
    VariableTracker tracker(&builder);
    BreakScope breakScope;
    compile_instructions(&breakScope, ast, &builder, &tracker);

    builder.simplify();

    builder.linkLabels();

    if (print_instructions){
        printf("%s", builder.export_str().c_str());
    }

    std::vector<Instruction*> instructions = builder.getInstructions();

    MipsRunner runner(2048, instructions.data(), instructions.size());
    int num_cycles = runner.run(cycles);

    for (const auto& pair : expectedVarMap){
        uint32_t result;
        if (use_mem){
            uint32_t mem_addr = tracker.get_mem_addr(pair.first);
            result = runner.get_mem(mem_addr);
        } else {
            uint8_t result_reg = tracker.getReg(pair.first, -1, false);
            result = runner.get_reg(result_reg);
        }
        EXPECT_EQ(result, pair.second, %d)
    }
    return num_cycles;
}

TEST(compilation, bit_ops){
    char code[] = "int a = 1; "
                  "int b = 2;"
                  "a = a & b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);


    char code2[] = "int a = 1; "
                  "int b = 2;"
                  "a = a | b;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 3},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 1; "
                  "int b = 2;"
                  "a = a ^ b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 3},
            {"b", 2}
    };
    test_with_regs(code3, 10, valMap3);

    char code4[] = "int a = 1; "
                  "int b = 2;"
                  "a = a << 2;";
    std::map<std::string, int32_t> valMap4 = {
            {"a", 4},
            {"b", 2}
    };
    test_with_regs(code4, 10, valMap4);

    char code5[] = "int a = 4; "
                  "int b = 2;"
                  "a = a >> 2;";
    std::map<std::string, int32_t> valMap5 = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code5, 10, valMap5);
}

TEST(compilation, bit_op_eq){
    char code[] = "int a = 1; "
                  "int b = 2;"
                  "a &= b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                  "int b = 2;"
                  "a |= b;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 3},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 1; "
                  "int b = 2;"
                  "a ^= b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 3},
            {"b", 2}
    };
    test_with_regs(code3, 10, valMap3);

    char code4[] = "int a = 1; "
                  "int b = 2;"
                  "a <<= 2;";
    std::map<std::string, int32_t> valMap4 = {
            {"a", 4},
            {"b", 2}
    };
    test_with_regs(code4, 10, valMap4);

    char code5[] = "int a = 4; "
                  "int b = 2;"
                  "a >>= 2;";
    std::map<std::string, int32_t> valMap5 = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code5, 10, valMap5);

}

TEST(compilation, op_eqs){
    char code[] = "int a = 1; "
                  "int b = 2;"
                  "a += b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 3},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                  "int b = 2;"
                  "a -= b;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", -1},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 13; "
                  "int b = 3;"
                  "a *= b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 39},
            {"b", 3}
    };
    test_with_regs(code3, 10, valMap3);

    char code4[] = "int a = 13; "
                  "int b = 3;"
                  "a /= b;";
    std::map<std::string, int32_t> valMap4 = {
            {"a", 4},
            {"b", 3}
    };
    test_with_regs(code4, 10, valMap4);
}

TEST(compilation, simple_if_with_take_and_no_take){
    char code[] = "int a = 1; "
                  "int b = a + 2;"
                  "if (1){"
                  " b = 15;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 15}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                  "int b = a + 2;"
                  "if (0){"
                  " b = 15;"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 3}
    };
    test_with_regs(code2, 10, valMap2);
}

TEST(compilation, if_with_expr_lt){
    char code[] = "int a = 1; "
                  "int b = a + 2;"
                  "if (a < b){"
                  " b = 15;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 15}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                  "int b = a - 2;"
                  "if (a < b){"
                  " b = 15;"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", -1}
    };
    test_with_regs(code2, 10, valMap2);

}

TEST(compilation, if_with_expr_gt){
    char code[] = "int a = 1; "
                   "int b = a + 2;"
                   "if (a > b){"
                   " b = 15;"
                   "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 3}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                  "int b = a - 2;"
                  "if (a > b){"
                  " b = 15;"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 15}
    };
    test_with_regs(code2, 10, valMap2);

}

TEST(compilation, if_with_expr_lte){
    char code[] = "int a = 1; "
                   "int b = a;"
                   "if (a <= b){"
                   " b = 15;"
                   "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 15}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                  "int b = a + 1;"
                  "if (a <= b){"
                  " b = 16;"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 16}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 1; "
                   "int b = a - 1;"
                   "if (a <= b){"
                   " b = 17;"
                   "}";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 1},
            {"b", 0}
    };
    test_with_regs(code3, 10, valMap3);
}

TEST(compilation, if_with_expr_gte){
    char code[] = "int a = 1; "
                   "int b = a;"
                   "if (a >= b){"
                   " b = 15;"
                   "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 15}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                  "int b = a - 1;"
                  "if (a >= b){"
                  " b = 16;"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 16}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 1; "
                   "int b = a + 1;"
                   "if (a >= b){"
                   " b = 17;"
                   "}";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code3, 10, valMap3);
}

TEST(compilation, if_with_not){
    char code[] = "int a = 1; "
                   "int b = a;"
                   "if (!a){"
                   " b = 15;"
                   "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 1}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 0; "
                  "int b = a;"
                  "if (!a){"
                  " b = 16;"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 0},
            {"b", 16}
    };
    test_with_regs(code2, 10, valMap2);

}

TEST(compilation, if_with_neq){
    char code[] = "int a = 1; "
                   "int b = 1;"
                   "if (a != b){"
                   " b = 15;"
                   "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 1}
    };
    test_with_regs(code, 15, valMap);

    char code2[] = "int a = 1; "
                  "int b = 2;"
                  "if (a != b){"
                  " b = 16;"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 16}
    };
    test_with_regs(code2, 15, valMap2);
}

TEST(compilation, set_var_to_lt){
    char code[] = "int a = 8; "
                  "int b = 2;"
                  "a = a < b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 8; "
                  "int b = 2;"
                  "a = b < a;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);
}

TEST(compilation, set_var_to_gt){
    char code[] = "int a = 8; "
                  "int b = 2;"
                  "a = a > b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 8; "
                  "int b = 2;"
                  "a = b > a;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);
}

TEST(compilation, set_var_to_lte){
    char code[] = "int a = 8; "
                  "int b = 2;"
                  "a = a <= b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 8; "
                  "int b = 2;"
                  "a = b <= a;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 8; "
                   "int b = 8;"
                   "a = a <= b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 1},
            {"b", 8}
    };
    test_with_regs(code3, 10, valMap3);
}

TEST(compilation, set_var_to_gte){
    char code[] = "int a = 8; "
                  "int b = 2;"
                  "a = a >= b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 8; "
                  "int b = 2;"
                  "a = b >= a;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 8; "
                   "int b = 8;"
                   "a = a >= b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 1},
            {"b", 8}
    };
    test_with_regs(code3, 10, valMap3);
}

TEST(compilation, set_var_to_eq_eq){
    char code[] = "int a = 8; "
                  "int b = 2;"
                  "a = a == b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 8; "
                  "int b = 2;"
                  "a = b == a;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 0},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 8; "
                   "int b = 8;"
                   "a = a == b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 1},
            {"b", 8}
    };
    test_with_regs(code3, 10, valMap3);
}

TEST(compilation, set_var_to_neq){
    char code[] = "int a = 8; "
                  "int b = 2;"
                  "a = a != b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 8; "
                  "int b = 2;"
                  "a = b != a;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int a = 8; "
                   "int b = 8;"
                   "a = a != b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 0},
            {"b", 8}
    };
    test_with_regs(code3, 10, valMap3);
}

TEST(compilation, set_var_to_not){
    char code[] = "int a = 8; "
                  "int b = 2;"
                  "a = !(a == b);";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 2}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int b = 0;"
                  "int a = !b;";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 1},
            {"b", 0}
    };
    test_with_regs(code2, 10, valMap2);

    char code3[] = "int b = 8;"
                   "int a = !b;";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 0},
            {"b", 8}
    };
    test_with_regs(code3, 10, valMap3);
}

TEST(compilation, simple_if_else){
    char code[] = "int a = 1; "
                  "int b = a + 2;"
                  "if (1){"
                  " b = 15;"
                  "} else {"
                  "a = 15"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 15}
    };
    test_with_regs(code, 10, valMap);

    char code2[] = "int a = 1; "
                   "int b = a + 2;"
                   "if (0){"
                   " b = 15;"
                   "} else {"
                   " a = 15"
                   "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 15},
            {"b", 3}
    };
    test_with_regs(code2, 10, valMap2);
}

TEST(compilation, simple_if_else_if_else){
    char code[] = "int a = 1; "
                  "int b = 0;"
                  "if (a < 2){"
                  " b = 1;"
                  "} "
                  "else if (a < 6) {"
                  " b = 2"
                  "}"
                  "else {"
                  " b = 3"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 1}
    };
    test_with_regs(code, 20, valMap);

    char code2[] = "int a = 4; "
                  "int b = 0;"
                  "if (a < 2){"
                  " b = 1;"
                  "} "
                  "else if (a < 6) {"
                  " b = 2"
                  "}"
                  "else {"
                  " b = 3"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 4},
            {"b", 2}
    };
    test_with_regs(code2, 20, valMap2);

    char code3[] = "int a = 8; "
                   "int b = 0;"
                   "if (a < 2){"
                   " b = 1;"
                   "} "
                   "else if (a < 6) {"
                   " b = 2"
                   "}"
                   "else {"
                   " b = 3"
                   "}";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 8},
            {"b", 3}
    };
    test_with_regs(code3, 20, valMap3);
}

TEST(compilation, multiple_if_else_ifs_no_else){
    char code[] = "int a = 1; "
                  "int b = 0;"
                  "if (a < 2){"
                  " b = 1;"
                  "} "
                  "else if (a < 6) {"
                  " b = 2"
                  "}"
                  "else if (a < 8) {"
                  " b = 3"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 1},
            {"b", 1}
    };
    test_with_regs(code, 20, valMap);

    char code2[] = "int a = 4; "
                  "int b = 0;"
                  "if (a < 2){"
                  " b = 1;"
                  "} "
                  "else if (a < 6) {"
                  " b = 2"
                  "}"
                  "else if (a < 8) {"
                  " b = 3"
                  "}";
    std::map<std::string, int32_t> valMap2 = {
            {"a", 4},
            {"b", 2}
    };
    test_with_regs(code2, 20, valMap2);

    char code3[] = "int a = 7; "
                   "int b = 0;"
                   "if (a < 2){"
                   " b = 1;"
                   "} "
                   "else if (a < 6) {"
                   " b = 2"
                   "}"
                   "else if (a < 8) {"
                   " b = 3"
                   "}";
    std::map<std::string, int32_t> valMap3 = {
            {"a", 7},
            {"b", 3}
    };
    test_with_regs(code3, 20, valMap3);

    char code4[] = "int a = 9; "
                   "int b = 0;"
                   "if (a < 2){"
                   " b = 1;"
                   "} "
                   "else if (a < 6) {"
                   " b = 2"
                   "}"
                   "else if (a < 8) {"
                   " b = 3"
                   "}";
    std::map<std::string, int32_t> valMap4 = {
            {"a", 9},
            {"b", 0}
    };
    test_with_regs(code4, 20, valMap4);
}

TEST(compilation, simple_while){
    char code[] = "int a = 0; "
                  "while (a < 5){"
                  " a = a + 2;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 6}
    };
    test_with_regs(code, 120, valMap);

}

TEST(compilation, simple_for){
    char code[] = "int a = 0; "
                  "for (int i = 0; i < 5; i = i + 1){"
                  " a = a + 2;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 10}
    };
    test_with_regs(code, 60, valMap);
}

TEST(compilation, for_with_break){
    char code[] = "int a = 0; "
                  "for (int i = 0; i < 5; i += 1){"
                  " if (i == 3){"
                  "     break;"
                  " } "
                  " a += 2;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 6}
    };
    test_with_regs(code, 60, valMap);
}

TEST(compilation, for_with_continue){
    char code[] = "int a = 0; "
                  "for (int i = 0; i < 5; i += 1){"
                  " if (i == 3){"
                  "     continue;"
                  " } "
                  " a += 2;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 8}
    };
    test_with_regs(code, 100, valMap);
}

TEST(compilation, for_without_init){
    char code[] = "int a = 0; "
                  "int i = 0;"
                  "for (; i < 5; i = i + 1){"
                  " a = a + 2;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 10}
    };
    test_with_regs(code, 60, valMap);
}

TEST(compilation, for_without_incr){
    char code[] = "int a = 0; "
                  "for (int i = 0; i < 5; ){"
                  " a = a + 2;"
                  " i = i + 1;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 10}
    };
    test_with_regs(code, 60, valMap);
}

TEST(compilation, for_without_condition){
    char code[] = "int a = 0; "
                  "int i = 0;"
                  "for (; ; i = i + 1){"
                  " if (i == 5){"
                  "     break;"
                  " } "
                  " a += 2;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 10}
    };
    test_with_regs(code, 60, valMap);
}

TEST(compilation, for_with_no_init_condition_or_incr){
    char code[] = "int a = 0; "
                  "int i = 0;"
                  "for (;;){"
                  " if (i == 5){"
                  "     break;"
                  " } "
                  " a += 2;"
                  " i = i + 1;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 10}
    };
    test_with_regs(code, 60, valMap);
}

TEST(compilation, function_editing_global_variable){
    char code[] = "int a = 0; "
                  "void foo(){"
                  " a = 1;"
                  "}"
                  "foo();"
                  "a = a + 0;";
    std::map<std::string, int32_t> valMap = {
            {"a", 1}
    };
    test_with_regs(code, 10, valMap);
}

TEST(compilation, function_no_args_with_return){
    char code[] = "int a = 0; "
                  "int foo(){"
                  " return 1;"
                  "}"
                  "a = foo();";
    std::map<std::string, int32_t> valMap = {
            {"a", 1}
    };
    test_with_regs(code, 10, valMap);
}

TEST(compilation, function_with_single_parameter){
    char code[] = "int foo(int b){"
                  " return b + 1;"
                  "}"
                  "int a = foo(2);";
    std::map<std::string, int32_t> valMap = {
            {"a", 3}
    };
    test_with_regs(code, 100, valMap);
}

TEST(compilation, function_with_conflicting_parameter_name){
    char code[] = "int a = 4;"
                  "int foo(int a){"
                  " return a + 1;"
                  "}"
                  "int b = foo(a);"
                  "a += 0;";
    std::map<std::string, int32_t> valMap = {
            {"a", 4},
            {"b", 5}
    };
    test_with_regs(code, 100, valMap);
}

TEST(compilation, function_with_two_variables){
    char code[] = "int foo(int a, int b){"
                  " return a + b;"
                  "}"
                  "int c = foo(2, 3);";
    std::map<std::string, int32_t> valMap = {
            {"c", 5}
    };
    test_with_regs(code, 100, valMap);
}

TEST(compilation, fibonacci_recursion){
    char code[] = "int fib(int n){"
                  " if (n < 2){"
                  "     return n;"
                  " }"
                  " return fib(n - 1) + fib(n - 2);"
                  "}"
                  "int a = fib(6);";
    std::map<std::string, int32_t> valMap = {
            {"a", 8}
    };
    test_with_regs(code, 800, valMap);
}

TEST(compilation, global_pointer_def){
    char code[] = "int a = 6;"
                  "int* b = &a;"
                  "int c = *b;";
    std::map<std::string, int32_t> valMap = {
            {"c", 6}
    };
    test_with_regs(code, 10, valMap);
}

TEST(compilation, global_pointer_from_func){
    char code[] = "int a = 6;"
                  "int* foo(){"
                  " return &a;"
                  "}"
                  "int* b = foo();"
                  "a += 0;"
                  "int c = *b;";
    std::map<std::string, int32_t> valMap = {
            {"a", 6},
            {"c", 6}
    };
    test_with_regs(code, 20, valMap);
}

TEST(compilation, pointer_from_var_on_stack){
    char code[] = "int a = 6;"
                  "int foo(){"
                  " int a = 7;"
                  " int* b = &a;"
                  " return *b;"
                  "}"
                  "int c = foo();";
    std::map<std::string, int32_t> valMap = {
            {"c", 7}
    };
    test_with_regs(code, 30, valMap);
}

TEST(compilation, set_pointer){
    char code[] = "int a = 6;"
                  "int* b = &a;"
                  "*b = 7;"
                  "a += 0";
    std::map<std::string, int32_t> valMap = {
            {"a", 7},
    };
    test_with_regs(code, 10, valMap);
}

TEST(compilation, set_pointer_in_function){
    char code[] = "int a = 6;"
                  "int foo(int* b){"
                  "*b = 7;"
                  "}"
                  "foo(&a);"
                  "a += 0";
    std::map<std::string, int32_t> valMap = {
            {"a", 7},
    };
    test_with_regs(code, 30, valMap);
}

TEST(compilation, function_with_four_variables){
    char code[] = "int foo(int a, int b, int c, int d){"
                  " return a + b + c + d;"
                  "}"
                  "int e = foo(1, 2, 3, 4);";
    std::map<std::string, int32_t> valMap = {
            {"e", 10}
    };
    test_with_regs(code, 30, valMap);
}

TEST(compilation, function_with_five_variables){
    char code[] = "int foo(int a, int b, int c, int d, int e){"
                  " return a + b + c + d + e;"
                  "}"
                  "int f = foo(1, 2, 3, 4, 5);";
    std::map<std::string, int32_t> valMap = {
            {"f", 15}
    };
    int cycles = test_with_regs(code, 40, valMap);
}

TEST(compilation, function_with_six_variables){
    char code[] = "int foo(int a, int b, int c, int d, int e, int f){"
                  " return (a + b + c + d + e) * f;"
                  "}"
                  "int g = foo(1, 2, 3, 4, 5, 6);";
    std::map<std::string, int32_t> valMap = {
            {"g", 90}
    };
    test_with_regs(code, 50, valMap);
}

TEST(compilation, function_with_six_variables_called_from_another_function){
    char code[] = "int foo(int a, int b, int c, int d, int e, int f){"
                  " return (a + b + c + d + e) * f;"
                  "}"
                  "int bar(int a, int b, int c, int d, int e, int f){"
                  " return foo(a, b, c, d, e, f);"
                  "}"
                  "int g = bar(1, 2, 3, 4, 5, 6);";
    std::map<std::string, int32_t> valMap = {
            {"g", 90}
    };
    test_with_regs(code, 80, valMap);
}

TEST(compilation, simple_inline_assembly){
    char code[] = "int a = 0;"
                  "__asm__(\"addi (a), (a), 5\");"
                  "a = 5;";
    std::map<std::string, int32_t> valMap = {
            {"a", 5}
    };
    test_with_regs(code, 10, valMap);
}

TEST(compilation, function_body_in_inline_assembly){
    char code[] = "int a = 5;"
                  "int foo(int a){"
                  " __asm__(\"addi $2, $4, 5\");"
                  "}"
                  "a = foo(a);";
    std::map<std::string, int32_t> valMap = {
            {"a", 10}
    };
    test_with_regs(code, 20, valMap);
}

TEST(compilation, defining_a_function_fully_in_assembly){
    char code[] = "int a = 5;"
                  "int foo(int a);"
                  "__asm__("
                  "\"j after\""
                  "\"foo: addi $2, $4, 5\""
                  "\"jr $31\""
                  "\"after: add $0 $0 $0\""
                  ");"
                  "a = foo(a);";
    std::map<std::string, int32_t> valMap = {
            {"a", 10}
    };
    test_with_regs(code, 20, valMap);
}

TEST(compilation, inline_assembly_load_word_and_store_word){
    char code[] = "int a = 5;"
                  "int b = 0;"
                  ""
                  "__asm__("
                  "\"sw (a), 0((b))\""
                  "\"lw (b), 0((b))\""
                  ");";
    std::map<std::string, int32_t> valMap = {
            {"a", 5},
            {"b", 5}
    };
    test_with_regs(code, 20, valMap);
}

TEST(compilation, array_definition_set_and_access){
    char code[] = "int a[5];"
                  "a[0] = 1;"
                  "a[1] = 2;"
                  "a[2] = 3;"
                  "a[3] = 4;"
                  "a[4] = 5;"
                  ""
                  "int b = a[0];"
                  "int c = a[1];"
                  "int d = a[2];"
                  "int e = a[3];"
                  "int f = a[4];";
    std::map<std::string, int32_t> valMap = {
            {"a", 0},
            {"b", 1},
            {"c", 2},
            {"d", 3},
            {"e", 4},
            {"f", 5}
    };
    test_with_regs(code, 20, valMap);
}

TEST(compilation, array_definition_and_access_on_stack){
    char code[] = "int a1 = 0;"
                  "int a2 = 0;"
                  "int a3 = 0;"
                  "void foo(){"
                    " int b[3];"
                    " b[0] = 1;"
                    " b[1] = 2;"
                    " b[2] = 3;"
                    ""
                    " a1 = b[0];"
                    " a2 = b[1];"
                    " a3 = b[2];"
                "}"
                "foo();"
                "a1 += 0;"
                "a2 += 0;"
                "a3 += 0;";

    std::map<std::string, int32_t> valMap = {
            {"a1", 1},
            {"a2", 2},
            {"a3", 3}
    };
    test_with_regs(code, 100, valMap);
}

TEST(compilation, array_definition_and_initialization){
    char code[] = "int a[5] = {1, 2, 3, 4, 5};"
                  ""
                  "int b = a[0];"
                  "int c = a[1];"
                  "int d = a[2];"
                  "int e = a[3];"
                  "int f = a[4];";
    std::map<std::string, int32_t> valMap = {
            {"b", 1},
            {"c", 2},
            {"d", 3},
            {"e", 4},
            {"f", 5}
    };
    test_with_regs(code, 20, valMap);
}

TEST(compilation, array_definition_and_initialization_in_function){
    char code[] =
                  "int b = 0;"
                  "int c = 0;"
                  "int d = 0;"
                  "int e = 0;"
                  "void foo(){"
                  " int a[5] = {1, 2, 3, 4, 5};"
                  ""
                  " b = a[0];"
                  " c = a[1];"
                  " d = a[2];"
                  " e = a[3];"
                  "}"
                  "foo();";
    std::map<std::string, int32_t> valMap = {
            {"b", 1},
            {"c", 2},
            {"d", 3},
            {"e", 4}
    };
    test_with_regs(code, 40, valMap);
}

TEST(compilation, array_access_with_expression){
    char code[] = "int a[5] = {1, 2, 3, 4, 5};"
                  "int x = 4;"
                  "int b = a[x-4];"
                  "int c = a[x * 2 - 7];"
                  "int d = a[x/2];"
                  "int e = a[x - x + 3];"
                  "int f = a[x];";
    std::map<std::string, int32_t> valMap = {
            {"b", 1},
            {"c", 2},
            {"d", 3},
            {"e", 4},
            {"f", 5}
    };
    test_with_regs(code, 90, valMap);
}

TEST(compilation, array_set_with_expression){
    char code[] = "int a[5];"
                  "int x = 4;"
                  "a[x-4] = 1;"
                  "a[x * 2 - 7] = 2;"
                  "a[x/2] = 3;"
                  "a[x - x + 3] = 4;"
                  "a[x] = 5;"
                  ""
                  "int b = a[0];"
                  "int c = a[1];"
                  "int d = a[2];"
                  "int e = a[3];"
                  "int f = a[4];";
    std::map<std::string, int32_t> valMap = {
            {"b", 1},
            {"c", 2},
            {"d", 3},
            {"e", 4},
            {"f", 5}
    };
    test_with_regs(code, 150, valMap);
}

TEST(compilation, array_set_with_expression_on_stack){
    char code[] = "int a1 = 0;"
                  "int a2 = 0;"
                  "int a3 = 0;"
                  "void foo(){"
                  " int b[5];"
                  " int x = 4;"
                  " b[x-4] = 1;"
                  " b[x * 2 - 7] = 2;"
                  " b[x/2] = 3;"
                  " b[x - x + 3] = 4;"
                  " b[x] = 5;"
                  ""
                  " a1 = b[0];"
                  " a2 = b[1];"
                  " a3 = b[2];"
                  "}"
                  "foo();"
                  "a1 += 0;"
                  "a2 += 0;"
                  "a3 += 0;";
    std::map<std::string, int32_t> valMap = {
            {"a1", 1},
            {"a2", 2},
            {"a3", 3}
    };
    test_with_regs(code, 200, valMap);
}

TEST(compilation, array_2d_init){
    char code[] = "int a[2][3] = {{1, 2, 3}, {4, 5, 6}};"
                  ""
                  "int b = a[0];"
                  "int c = a[1];"
                  "int d = a[2];"
                  "int e = a[3];"
                  "int f = a[4];"
                  "int g = a[5];";
    std::map<std::string, int32_t> valMap = {
            {"b", 1},
            {"c", 2},
            {"d", 3},
            {"e", 4},
            {"f", 5},
            {"g", 6}
    };
    test_with_regs(code, 20, valMap);
}

TEST(compilation, with_main_function){
    char code[] = "int a = 5;"
                  "int main(){"
                  " a = 6;"
                  "}";
    std::map<std::string, int32_t> valMap = {
            {"a", 6}
    };
    test_with_regs(code, 10, valMap, false, true);
}