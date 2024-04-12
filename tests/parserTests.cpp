//
// Created by Ethan Horowitz on 3/13/24.
//

#include "parserTests.h"

void run_parser_tests(){
    RUN_TEST_GROUP("parser");
}

TEST(parser, toRefs){
    std::vector<Token*> tokens = tokenize
            ("int a = 4;\n"
             "int main(){\n"
             "    int x = 0;\n"
             "    int y = 5;\n"
             "    int z = x + y;\n"
             "    a += z;\n"
             "\n"
             "    return 0;\n"
             "}\n");
    for (int i = 0; i < tokens.size(); i++){
        ASSERT_EQ(tokens[i]->type, tokens[i]->type, %d)
        ASSERT_EQ(tokens[i]->lexeme.c_str(), tokens[i]->lexeme.c_str(), %s)
    }
}

TEST(parser, simple_expression){
    std::vector<Token*> tokens = tokenize("1 + 2");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 1, %d)
    Token* start = parsed[0];
    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* op = (BinaryOpToken*)start;
    EXPECT_EQ(op->val_type, ADD, %d)

    Token* left = op->left;
    EXPECT_EQ(left->type, TYPE_VALUE, %d)
    EXPECT_EQ(left->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(left->lexeme == "1")

    Token* right = op->right;
    EXPECT_EQ(right->type, TYPE_VALUE, %d)
    EXPECT_EQ(right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(right->lexeme == "2")
}

TEST(parser, complicated_expression){
    std::vector<Token*> tokens = tokenize("1 + 4 * 2 / 9 & 5 - 8");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 1, %d)
    Token* start = parsed[0];

    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* op = (BinaryOpToken*)start;
    ASSERT_EQ(op->val_type, BIN_AND, %d)
    auto* left = (BinaryOpToken*)op->left;
    ASSERT_EQ(left->val_type, ADD, %d)
    auto* right = (BinaryOpToken*)op->right;
    ASSERT_EQ(right->val_type, MINUS, %d)

    auto* right_left = right->left;
    ASSERT_EQ(right_left->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(right_left->lexeme == "5")

    auto* right_right = right->right;
    ASSERT_EQ(right_right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(right_right->lexeme == "8")

    auto* left_left = left->left;
    EXPECT_EQ(left_left->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(left_left->lexeme == "1")

    auto* left_right = (BinaryOpToken*)left->right;
    ASSERT_EQ(left_right->val_type, DIV, %d)

    auto* left_right_left = (BinaryOpToken*) left_right->left;
    ASSERT_EQ(left_right_left->val_type, MULT, %d)

    auto* left_right_left_left = left_right_left->left;
    ASSERT_EQ(left_right_left_left->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(left_right_left_left->lexeme == "4")

    auto* left_right_left_right = left_right_left->right;
    ASSERT_EQ(left_right_left_right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(left_right_left_right->lexeme == "2")

    auto* left_right_right = left_right->right;
    ASSERT_EQ(left_right_right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(left_right_right->lexeme == "9")
}

TEST(parser, invalid_expression_op_missing_right){
    std::vector<Token*> tokens = tokenize("1 + 4 * 2 / 9 & 5 -");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token*> parsed = parse(iterator, &scope);
    } catch (std::runtime_error& e){
        EXPECT_TRUE(strcmp(e.what(), "Operator <OPERATOR MINUS - line 1> expected value on right") == 0)
    }
}

TEST(parser, invalid_expression_op_missing_right_2){
    std::vector<Token*> tokens = tokenize("1 + 4 * 2 - / 9 & 5 -");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token*> parsed = parse(iterator, &scope);
    } catch (std::runtime_error& e){
        EXPECT_TRUE(strcmp(e.what(), "Operator <OPERATOR MINUS - line 1> expected value on right") == 0)
    }
}

TEST(parser, invalid_expression_op_missing_right_3){
    std::vector<Token*> tokens = tokenize("1 + 4 * * 2 - / 9 & 5 -");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token*> parsed = parse(iterator, &scope);
    } catch (std::runtime_error& e){
        std::string result = e.what();
        EXPECT_EQ_SPECIAL(result, "Must use variable after ref or deref on line 1", %s, .c_str(),)
    }
}

TEST(parser, invalid_expression_op_missing_left){
    std::vector<Token*> tokens = tokenize("+ 4 * 2 / 9 & 5 -");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token*> parsed = parse(iterator, &scope);
    } catch (std::runtime_error& e){
        std::string result = e.what();
        EXPECT_EQ_SPECIAL(result, "Unexpected token <OPERATOR ADD + line 1>", %s, .c_str(),)
    }
}

TEST(parser, expression_with_parentheses){
    std::vector<Token*> tokens = tokenize("(1 + 4) * 2 / (9 & 5) - 8");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 1, %d)
    Token* start = parsed[0];

    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* sub = (BinaryOpToken*)start;
    ASSERT_EQ(sub->val_type, MINUS, %d)
    // SUB RIGHT
    auto* eight = (BinaryOpToken*)sub->right;
    ASSERT_EQ(eight->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eight->lexeme == "8")
    // SUB LEFT
    auto* div = (BinaryOpToken*)sub->left;
    ASSERT_EQ(div->val_type, DIV, %d)
    // DIV RIGHT
    auto* andOp = (BinaryOpToken*)div->right;
    ASSERT_EQ(andOp->val_type, BIN_AND, %d)
    // AND RIGHT
    auto* five = andOp->right;
    ASSERT_EQ(five->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(five->lexeme == "5")
    // AND LEFT
    auto* nine = andOp->left;
    ASSERT_EQ(nine->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(nine->lexeme == "9")
    // DIV LEFT
    auto* mult = (BinaryOpToken*)div->left;
    ASSERT_EQ(mult->val_type, MULT, %d)
    // MULT RIGHT
    auto* two = mult->right;
    ASSERT_EQ(two->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(two->lexeme == "2")
    // MULT LEFT
    auto* add = (BinaryOpToken*)mult->left;
    ASSERT_EQ(add->val_type, ADD, %d)
    // ADD RIGHT
    auto* four = add->right;
    ASSERT_EQ(four->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(four->lexeme == "4")
    // ADD LEFT
    auto* one = add->left;
    ASSERT_EQ(one->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(one->lexeme == "1")
}

TEST(parser, nested_parentheses){
    std::vector<Token*> tokens = tokenize("((1 + 4) * 2) / 2");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 1, %d)
    Token* start = parsed[0];

    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* div = (BinaryOpToken*)start;
    ASSERT_EQ(div->val_type, DIV, %d)
    // DIV RIGHT
    auto* two = div->right;
    ASSERT_EQ(two->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(two->lexeme == "2")
    // DIV LEFT
    auto* mult = (BinaryOpToken*)div->left;
    ASSERT_EQ(mult->val_type, MULT, %d)
    // MULT RIGHT
    auto* twoInner = mult->right;
    ASSERT_EQ(twoInner->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(twoInner->lexeme == "2")
    // MULT LEFT
    auto* add = (BinaryOpToken*)mult->left;
    ASSERT_EQ(add->val_type, ADD, %d)
    // ADD RIGHT
    auto* four = add->right;
    ASSERT_EQ(four->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(four->lexeme == "4")
    // ADD LEFT
    auto* one = add->left;
    ASSERT_EQ(one->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(one->lexeme == "1")
}

TEST(parser, simple_assignment){
    std::vector<Token*> tokens = tokenize("int a = 4;");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 1, %d)
    Token* start = parsed[0];

    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* assign = (DefinitionToken*)start;
    ASSERT_EQ(assign->val_type, IDENTIFIER, %d)

    EXPECT_EQ(assign->valueType, INT, %d)

    EXPECT_EQ(assign->value->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(assign->value->lexeme == "4")

    EXPECT_EQ(assign->refCount, 0, %d)

    EXPECT_TRUE(assign->name == "a")
}

TEST(parser, assignment_with_ref){
    std::vector<Token*> tokens = tokenize("int a = 4;\n"
                                             "int* b = &a;");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 2, %d)
    Token* start = parsed[1];

    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* assign = (DefinitionToken*)start;
    ASSERT_EQ(assign->val_type, IDENTIFIER, %d)

    EXPECT_EQ(assign->valueType, INT, %d)
    EXPECT_EQ(assign->refCount, 1, %d)

    EXPECT_EQ(assign->value->val_type, REF, %d)
    EXPECT_TRUE(assign->name == "b")

    auto* ref = (BinaryOpToken*)assign->value;
    EXPECT_EQ(ref->right->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(ref->right->lexeme == "a")
}

TEST(parser, int_with_two_refs){
    std::vector<Token*> tokens = tokenize("int** a = 0;");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 1, %d)
    Token* start = parsed[0];

    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* assign = (DefinitionToken*)start;
    ASSERT_EQ(assign->val_type, IDENTIFIER, %d)
    ASSERT_EQ(assign->valueType, INT, %d)
    ASSERT_EQ(assign->refCount, 2, %d)

}

TEST(parser, invalid_assignment_no_expr){
    std::vector<Token*> tokens = tokenize("int a =");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token*> parsed = parse(iterator, &scope);
    } catch (std::runtime_error& e){
        std::string result = e.what();
        EXPECT_EQ_SPECIAL(result, "Operator <OPERATOR NONE  line 0> expected value on right", %s, .c_str(),)
    }
}

TEST(parser, invalid_assignment_bad_ops){
    std::vector<Token*> tokens = tokenize("int a = = 4;");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token*> parsed = parse(iterator, &scope);
    } catch (std::runtime_error& e) {
        std::string result = e.what();
        EXPECT_EQ_SPECIAL(result, "Operator <OPERATOR NONE  line 1> expected value on right", %s, .c_str(),)
    }
}

TEST(parser, nested_parentheces){
    std::vector<Token*> tokens = tokenize("int a = ((4 + 2) * 3) / 2;");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token*> parsed = parse(iterator, &scope);
    ASSERT_EQ((int)parsed.size(), 1, %d)
    Token* start = parsed[0];

    ASSERT_EQ(start->type, TYPE_OPERATOR, %d)
    auto* assign = (DefinitionToken*)start;
    ASSERT_EQ(assign->val_type, IDENTIFIER, %d)

    EXPECT_EQ(assign->valueType, INT, %d)
    EXPECT_EQ(assign->refCount, 0, %d)

    auto* div = (BinaryOpToken*)assign->value;
    ASSERT_EQ(div->val_type, DIV, %d)

    auto* two = div->right;
    ASSERT_EQ(two->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(two->lexeme == "2")

    auto* mult = (BinaryOpToken*)div->left;
    ASSERT_EQ(mult->val_type, MULT, %d)

    auto* three = mult->right;
    ASSERT_EQ(three->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(three->lexeme == "3")

    auto* add = (BinaryOpToken*)mult->left;
    ASSERT_EQ(add->val_type, ADD, %d)

    auto* twoInner = add->right;
    ASSERT_EQ(twoInner->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(twoInner->lexeme == "2")

    auto* four = add->left;
    ASSERT_EQ(four->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(four->lexeme == "4")
}

TEST(parser, if_statement){
    std::vector<Token*> tokens = tokenize("int x = 1; if(x < 5){\n"
                                             "    x = 5;\n"
                                             "}\n");
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)

    auto* if_statement = (IfElseToken*) parsed[1];

    Token* condition = if_statement->condition;
    ASSERT_EQ(condition->type, TYPE_OPERATOR, %d)
    auto* lessThan = (BinaryOpToken*)condition;
    ASSERT_EQ(lessThan->val_type, LT, %d)
    ASSERT_EQ(lessThan->left->val_type, IDENTIFIER, %d)
    ASSERT_EQ(lessThan->right->val_type, NUMBER_INT, %d)

    GroupToken* ifBody = if_statement->ifBody;
    ASSERT_EQ(ifBody->expressions.size(), 1, %d)
    ASSERT_EQ(ifBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq = (BinaryOpToken*)ifBody->expressions[0];
    ASSERT_EQ(eq->val_type, EQ, %d)
    ASSERT_EQ(eq->left->val_type, IDENTIFIER, %d)
    ASSERT_TRUE(eq->left->lexeme == "x")
    ASSERT_EQ(eq->right->val_type, NUMBER_INT, %d)

    EXPECT_TRUE(if_statement->elseIfBodies.empty())
    EXPECT_EQ(if_statement->elseBody, nullptr, %p)
}

TEST(parser, if_else_statement){
    std::vector<Token*> tokens = tokenize("int x = 1; if(x < 5){\n"
                                             "    x = 5;\n"
                                             "} else {\n"
                                             "    x = 6;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)

    auto* if_statement = (IfElseToken*) parsed[1];

    Token* condition = if_statement->condition;
    ASSERT_EQ(condition->type, TYPE_OPERATOR, %d)
    auto* lessThan = (BinaryOpToken*)condition;
    ASSERT_EQ(lessThan->val_type, LT, %d)
    ASSERT_EQ(lessThan->left->val_type, IDENTIFIER, %d)
    ASSERT_EQ(lessThan->right->val_type, NUMBER_INT, %d)

    GroupToken* ifBody = if_statement->ifBody;
    ASSERT_EQ(ifBody->expressions.size(), 1, %d)
    ASSERT_EQ(ifBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq = (BinaryOpToken*)ifBody->expressions[0];
    ASSERT_EQ(eq->val_type, EQ, %d)
    EXPECT_EQ(eq->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq->left->lexeme == "x")
    EXPECT_EQ(eq->right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eq->right->lexeme == "5")

    GroupToken* elseBody = if_statement->elseBody;
    ASSERT_EQ(elseBody->expressions.size(), 1, %d)
    ASSERT_EQ(elseBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq2 = (BinaryOpToken*)elseBody->expressions[0];
    ASSERT_EQ(eq2->val_type, EQ, %d)
    EXPECT_EQ(eq2->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq2->left->lexeme == "x")
    EXPECT_EQ(eq2->right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eq2->right->lexeme == "6")
}

TEST(parser, if_else_if_statement){
    std::vector<Token*> tokens = tokenize("int x = 1; if(x < 5){\n"
                                             "    x = 5;\n"
                                             "} else if(x < 6){\n"
                                             "    x = 6;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)

    auto* if_statement = (IfElseToken*) parsed[1];

    Token* condition = if_statement->condition;
    ASSERT_EQ(condition->type, TYPE_OPERATOR, %d)
    auto* lessThan = (BinaryOpToken*)condition;
    ASSERT_EQ(lessThan->val_type, LT, %d)
    ASSERT_EQ(lessThan->left->val_type, IDENTIFIER, %d)
    ASSERT_EQ(lessThan->right->val_type, NUMBER_INT, %d)

    GroupToken* ifBody = if_statement->ifBody;
    ASSERT_EQ(ifBody->expressions.size(), 1, %d)
    ASSERT_EQ(ifBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq = (BinaryOpToken*)ifBody->expressions[0];
    ASSERT_EQ(eq->val_type, EQ, %d)
    EXPECT_EQ(eq->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq->left->lexeme == "x")
    EXPECT_EQ(eq->right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eq->right->lexeme == "5")

    GroupToken* elseIfBody = if_statement->elseIfBodies[0];
    ASSERT_EQ(elseIfBody->expressions.size(), 1, %d)
    ASSERT_EQ(elseIfBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq2 = (BinaryOpToken*)elseIfBody->expressions[0];
    ASSERT_EQ(eq2->val_type, EQ, %d)
    EXPECT_EQ(eq2->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq2->left->lexeme == "x")
    EXPECT_EQ(eq2->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(eq2->right->lexeme,  "6", %s, .c_str(),)

    EXPECT_EQ(if_statement->elseBody, nullptr, %p)
}

TEST(parser, if_else_if_else_statement){
    std::vector<Token*> tokens = tokenize("int x = 1; if(x < 5){\n"
                                             "    x = 5;\n"
                                             "} else if(x < 6){\n"
                                             "    x = 6;\n"
                                             "} else {\n"
                                             "    x = 7;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)

    auto* if_statement = (IfElseToken*) parsed[1];

    Token* condition = if_statement->condition;
    ASSERT_EQ(condition->type, TYPE_OPERATOR, %d)
    auto* lessThan = (BinaryOpToken*)condition;
    ASSERT_EQ(lessThan->val_type, LT, %d)
    ASSERT_EQ(lessThan->left->val_type, IDENTIFIER, %d)
    ASSERT_EQ(lessThan->right->val_type, NUMBER_INT, %d)

    GroupToken* ifBody = if_statement->ifBody;
    ASSERT_EQ(ifBody->expressions.size(), 1, %d)
    ASSERT_EQ(ifBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq = (BinaryOpToken*)ifBody->expressions[0];
    ASSERT_EQ(eq->val_type, EQ, %d)
    EXPECT_EQ(eq->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq->left->lexeme == "x")
    EXPECT_EQ(eq->right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eq->right->lexeme == "5")

    GroupToken* elseIfBody = if_statement->elseIfBodies[0];
    ASSERT_EQ(elseIfBody->expressions.size(), 1, %d)
    ASSERT_EQ(elseIfBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq2 = (BinaryOpToken*)elseIfBody->expressions[0];
    ASSERT_EQ(eq2->val_type, EQ, %d)
    EXPECT_EQ(eq2->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq2->left->lexeme == "x")
    EXPECT_EQ(eq2->right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eq2->right->lexeme == "6")

    GroupToken* elseBody = if_statement->elseBody;
    ASSERT_EQ(elseBody->expressions.size(), 1, %d)
    ASSERT_EQ(elseBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq3 = (BinaryOpToken*)elseBody->expressions[0];
    ASSERT_EQ(eq3->val_type, EQ, %d)
    EXPECT_EQ(eq3->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq3->left->lexeme == "x")
    EXPECT_EQ(eq3->right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eq3->right->lexeme == "7")
}

TEST(parser, if_with_nested_parentheces){
    std::vector<Token*> tokens = tokenize("int x = 1; if((x < 5) && (x > 0)){\n"
                                             "    x = 5;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)

    auto* if_statement = (IfElseToken*) parsed[1];

    Token* condition = if_statement->condition;
    ASSERT_EQ(condition->type, TYPE_OPERATOR, %d)
    auto* andOp = (BinaryOpToken*)condition;
    ASSERT_EQ(andOp->val_type, AND, %d)
    ASSERT_EQ(andOp->left->type, TYPE_OPERATOR, %d)
    ASSERT_EQ(andOp->right->type, TYPE_OPERATOR, %d)

    auto* lessThan = (BinaryOpToken*)andOp->left;
    ASSERT_EQ(lessThan->val_type, LT, %d)
    ASSERT_EQ(lessThan->left->val_type, IDENTIFIER, %d)
    ASSERT_EQ(lessThan->right->val_type, NUMBER_INT, %d)

    auto* greaterThan = (BinaryOpToken*)andOp->right;
    ASSERT_EQ(greaterThan->val_type, GT, %d)
    ASSERT_EQ(greaterThan->left->val_type, IDENTIFIER, %d)
    ASSERT_EQ(greaterThan->right->val_type, NUMBER_INT, %d)

    GroupToken* ifBody = if_statement->ifBody;
    ASSERT_EQ(ifBody->expressions.size(), 1, %d)
    ASSERT_EQ(ifBody->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* eq = (BinaryOpToken*)ifBody->expressions[0];
    ASSERT_EQ(eq->val_type, EQ, %d)
    EXPECT_EQ(eq->left->val_type, IDENTIFIER, %d)
    EXPECT_TRUE(eq->left->lexeme == "x")
    EXPECT_EQ(eq->right->val_type, NUMBER_INT, %d)
    EXPECT_TRUE(eq->right->lexeme == "5")

    EXPECT_TRUE(if_statement->elseIfBodies.empty())
    EXPECT_EQ(if_statement->elseBody, nullptr, %p)
}

TEST(parser, function_definition_with_empty_function){
    std::vector<Token*> tokens = tokenize("int main(){\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(function->name, "main", %s, .c_str(),)
    EXPECT_TRUE(function->parameters.empty())
    EXPECT_TRUE(function->body->expressions.empty())
}

TEST(parser, function_definition_with_pointer_empty){
    std::vector<Token*> tokens = tokenize("int* main(){\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 1, %d)
    EXPECT_EQ_SPECIAL(function->name, "main", %s, .c_str(),)
    EXPECT_TRUE(function->parameters.empty())
    EXPECT_TRUE(function->body->expressions.empty())
}

TEST(parser, function_with_argument_that_has_two_refs){
    std::vector<Token*> tokens = tokenize("int* main(int** x){\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 1, %d)
    EXPECT_EQ_SPECIAL(function->name, "main", %s, .c_str(),)
    ASSERT_EQ(function->parameters.size(), 1, %d)
    auto* param = function->parameters[0];
    EXPECT_EQ(param->valueType, INT, %d)
    EXPECT_EQ(param->refCount, 2, %d)
    EXPECT_EQ_SPECIAL(param->name, "x", %s, .c_str(),)
}

TEST(parser, function_definition_with_body){
    std::vector<Token*> tokens = tokenize("int main(){\n"
                                             "    int x = 5;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(function->name, "main", %s, .c_str(),)
    EXPECT_TRUE(function->parameters.empty())
    EXPECT_EQ(function->body->expressions.size(), 1, %d)
    EXPECT_EQ(function->body->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* def = (DefinitionToken*)function->body->expressions[0];
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "x", %s, .c_str(),)
    EXPECT_EQ(def->value->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->value->lexeme, "5", %s, .c_str(),)
}

TEST(parser, function_call_with_no_params){
    std::vector<Token*> tokens = tokenize("int main(){\n"
                                             "    return 0;\n"
                                             "}\n"
                                             "int z = main();\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)
    auto* expr = (DefinitionToken*)parsed[1];
    EXPECT_EQ(expr->valueType, INT, %d)
    EXPECT_EQ(expr->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(expr->name, "z", %s, .c_str(),)
    auto* call = (FunctionCallToken*)expr->value;
    ASSERT_EQ(call->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(call->val_type, TokenValue::FUNCTION, %d)
    EXPECT_EQ(call->function->val_type, TokenValue::IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(call->function->lexeme, "main", %s, .c_str(),)
    EXPECT_TRUE(call->arguments.empty())
}

TEST(parser, function_call){
    std::vector<Token*> tokens = tokenize("int add(int x, int y){\n"
                                             "    return x + y;\n"
                                             "}\n"
                                             "int z = add(2, 3);\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)
    auto* expr = (DefinitionToken*)parsed[1];
    EXPECT_EQ(expr->valueType, INT, %d)
    EXPECT_EQ(expr->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(expr->name, "z", %s, .c_str(),)
    auto* call = (FunctionCallToken*)expr->value;
    ASSERT_EQ(call->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(call->val_type, TokenValue::FUNCTION, %d)
    EXPECT_EQ(call->function->val_type, TokenValue::IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(call->function->lexeme, "add", %s, .c_str(),)
    ASSERT_EQ(call->arguments.size(), 2, %d)
    EXPECT_EQ(call->arguments[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(call->arguments[0]->lexeme, "2", %s, .c_str(),)
    EXPECT_EQ(call->arguments[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(call->arguments[1]->lexeme, "3", %s, .c_str(),)
}

TEST(parser, exprs_in_braces){
    std::vector<Token*> tokens = tokenize("{"
                                             "int x = 0;"
                                             "int y = 4;"
                                             "{"
                                             "    int z = 5;"
                                             "}"
                                             "}");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);

    ASSERT_EQ((int) parsed.size(), 1, %d)
    auto* group = (GroupToken*)parsed[0];
    ASSERT_EQ(group->expressions.size(), 3, %d)
    auto* x = (DefinitionToken*)group->expressions[0];
    EXPECT_EQ(x->valueType, INT, %d)
    EXPECT_EQ(x->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(x->name, "x", %s, .c_str(),)
    auto* y = (DefinitionToken*)group->expressions[1];
    EXPECT_EQ(y->valueType, INT, %d)
    EXPECT_EQ(y->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(y->name, "y", %s, .c_str(),)
    auto* group2 = (GroupToken*)group->expressions[2];
    ASSERT_EQ(group2->expressions.size(), 1, %d)
    auto* z = (DefinitionToken*)group2->expressions[0];
    EXPECT_EQ(z->valueType, INT, %d)
    EXPECT_EQ(z->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(z->name, "z", %s, .c_str(),)
}

TEST(parser, function_call_with_nested_parentheces){
    std::vector<Token*> tokens = tokenize("int add(int x, int y){\n"
                                             "    return x + y;\n"
                                             "}\n"
                                             "int z = add((2 + 3), 3);\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)
    auto* expr = (DefinitionToken*)parsed[1];
    EXPECT_EQ(expr->valueType, INT, %d)
    EXPECT_EQ(expr->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(expr->name, "z", %s, .c_str(),)
    auto* call = (FunctionCallToken*)expr->value;
    ASSERT_EQ(call->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(call->val_type, TokenValue::FUNCTION, %d)
    EXPECT_EQ(call->function->val_type, TokenValue::IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(call->function->lexeme, "add", %s, .c_str(),)
    ASSERT_EQ(call->arguments.size(), 2, %d)
    auto* add = (BinaryOpToken*)call->arguments[0];
    ASSERT_EQ(add->val_type, ADD, %d)
    EXPECT_EQ(add->left->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(add->left->lexeme, "2", %s, .c_str(),)
    EXPECT_EQ(add->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(add->right->lexeme, "3", %s, .c_str(),)
    EXPECT_EQ(call->arguments[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(call->arguments[1]->lexeme, "3", %s, .c_str(),)
}

TEST(parser, function_call_with_parentheces_with_operation){
    std::vector<Token*> tokens = tokenize("int add(int x, int y){\n"
                                             "    return x + y;\n"
                                             "}\n"
                                             "int z = add((2 << 3) & 5, 3);\n");

    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)
    auto* expr = (DefinitionToken*)parsed[1];
    EXPECT_EQ(expr->valueType, INT, %d)
    EXPECT_EQ(expr->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(expr->name, "z", %s, .c_str(),)
    auto* call = (FunctionCallToken*)expr->value;
    ASSERT_EQ(call->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(call->val_type, TokenValue::FUNCTION, %d)
    EXPECT_EQ(call->function->val_type, TokenValue::IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(call->function->lexeme, "add", %s, .c_str(),)
    ASSERT_EQ(call->arguments.size(), 2, %d)
    auto* andOp = (BinaryOpToken*)call->arguments[0];
    ASSERT_EQ(andOp->val_type, BIN_AND, %d)
    auto* shift = (BinaryOpToken*)andOp->left;
    ASSERT_EQ(shift->val_type, LSHIFT, %d)
    EXPECT_EQ(shift->left->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(shift->left->lexeme, "2", %s, .c_str(),)
    EXPECT_EQ(shift->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(shift->right->lexeme, "3", %s, .c_str(),)
    EXPECT_EQ(andOp->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(andOp->right->lexeme, "5", %s, .c_str(),)
    EXPECT_EQ(call->arguments[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(call->arguments[1]->lexeme, "3", %s, .c_str(),)
}

TEST(parser, function_header){
    std::vector<Token*> tokens = tokenize("int thing(int x, int y);");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(function->name, "thing", %s, .c_str(),)
    ASSERT_EQ(function->parameters.size(), 2, %d)
    auto* x = function->parameters[0];
    EXPECT_EQ(x->valueType, INT, %d)
    EXPECT_EQ(x->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(x->name, "x", %s, .c_str(),)
    auto* y = function->parameters[1];
    EXPECT_EQ(y->valueType, INT, %d)
    EXPECT_EQ(y->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(y->name, "y", %s, .c_str(),)
    EXPECT_TRUE(function->body == nullptr)
}

TEST(parser, function_impl_following_function_header){
    std::vector<Token*> tokens = tokenize("int thing(int x, int y);\n"
                                             "int thing(int x, int y){\n"
                                             "    return x + y;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(function->name, "thing", %s, .c_str(),)
    ASSERT_EQ(function->parameters.size(), 2, %d)
    auto* x = function->parameters[0];
    EXPECT_EQ(x->valueType, INT, %d)
    EXPECT_EQ(x->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(x->name, "x", %s, .c_str(),)
    auto* y = function->parameters[1];
    EXPECT_EQ(y->valueType, INT, %d)
    EXPECT_EQ(y->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(y->name, "y", %s, .c_str(),)
    EXPECT_EQ(function->body->expressions.size(), 1, %d)
    EXPECT_EQ(function->body->expressions[0]->type, TYPE_KEYWORD, %d)

    auto* function2 = (FunctionToken*) parsed[1];
    EXPECT_EQ(function2->returnType, INT, %d)
    EXPECT_EQ(function2->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(function2->name, "thing", %s, .c_str(),)
    ASSERT_EQ(function2->parameters.size(), 2, %d)
    auto* x2 = function2->parameters[0];
    EXPECT_EQ(x2->valueType, INT, %d)
    EXPECT_EQ(x2->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(x2->name, "x", %s, .c_str(),)
    auto* y2 = function2->parameters[1];
    EXPECT_EQ(y2->valueType, INT, %d)
    EXPECT_EQ(y2->refCount, 0, %d)
}

TEST(parser, function_with_return){
    std::vector<Token*> tokens = tokenize("int main(){\n"
                                             "    return 5;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(function->name, "main", %s, .c_str(),)
    EXPECT_TRUE(function->parameters.empty())
    EXPECT_EQ(function->body->expressions.size(), 1, %d)
    EXPECT_EQ(function->body->expressions[0]->type, TYPE_KEYWORD, %d)

    auto* ret = (ReturnToken*)function->body->expressions[0];
    EXPECT_EQ(ret->value->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(ret->value->lexeme, "5", %s, .c_str(),)
}

TEST(parser, for_loop_with_empty_body){
    std::vector<Token*> tokens = tokenize("for(int i = 0; i <= 5; i += 1){\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* forLoop = (ForToken*) parsed[0];
    ASSERT_EQ(forLoop->init->val_type, IDENTIFIER, %d)
    auto* def = (DefinitionToken*)forLoop->init;
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "i", %s, .c_str(),)
    EXPECT_EQ(def->value->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->value->lexeme, "0", %s, .c_str(),)

    auto* condition = (BinaryOpToken*)forLoop->condition;
    EXPECT_EQ(condition->val_type, LTE, %d)
    EXPECT_EQ(condition->left->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(condition->left->lexeme, "i", %s, .c_str(),)
    EXPECT_EQ(condition->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(condition->right->lexeme, "5", %s, .c_str(),)

    auto* increment = (BinaryOpToken*)forLoop->increment;
    EXPECT_EQ(increment->val_type, ADD_EQ, %d)
    EXPECT_EQ(increment->left->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(increment->left->lexeme, "i", %s, .c_str(),)
    EXPECT_EQ(increment->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(increment->right->lexeme, "1", %s, .c_str(),)

    EXPECT_TRUE(forLoop->body->expressions.empty())
}

TEST(parser, for_loop_with_simple_body){
    std::vector<Token*> tokens = tokenize("for(int i = 0; i <= 5; i += 1){\n"
                                             "    int x = 5;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* forLoop = (ForToken*) parsed[0];
    ASSERT_EQ(forLoop->init->val_type, IDENTIFIER, %d)
    auto* def = (DefinitionToken*)forLoop->init;
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "i", %s, .c_str(),)
    EXPECT_EQ(def->value->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->value->lexeme, "0", %s, .c_str(),)

    auto* condition = (BinaryOpToken*)forLoop->condition;
    EXPECT_EQ(condition->val_type, LTE, %d)
    EXPECT_EQ(condition->left->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(condition->left->lexeme, "i", %s, .c_str(),)
    EXPECT_EQ(condition->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(condition->right->lexeme, "5", %s, .c_str(),)

    auto* increment = (BinaryOpToken*)forLoop->increment;
    EXPECT_EQ(increment->val_type, ADD_EQ, %d)
    EXPECT_EQ(increment->left->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(increment->left->lexeme, "i", %s, .c_str(),)
    EXPECT_EQ(increment->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(increment->right->lexeme, "1", %s, .c_str(),)

    EXPECT_EQ(forLoop->body->expressions.size(), 1, %d)
    EXPECT_EQ(forLoop->body->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* def2 = (DefinitionToken*)forLoop->body->expressions[0];
    EXPECT_EQ(def2->valueType, INT, %d)
    EXPECT_EQ(def2->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def2->name, "x", %s, .c_str(),)
    EXPECT_EQ(def2->value->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def2->value->lexeme, "5", %s, .c_str(),)
}

TEST(parser, infinite_for_loop){
    std::vector<Token*> tokens = tokenize("for(;;){\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* forLoop = (ForToken*) parsed[0];
    EXPECT_EQ(forLoop->init, nullptr, %p)
    EXPECT_EQ(forLoop->condition, nullptr, %p)
    EXPECT_EQ(forLoop->increment, nullptr, %p)
    EXPECT_TRUE(forLoop->body->expressions.empty())

}

TEST(parser, for_loop_with_continue){
    std::vector<Token*> tokens = tokenize("for(int i = 0; i <= 5; i += 1){\n"
                                             "    continue;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);

    ASSERT_EQ((int) parsed.size(), 1, %d)
    ForToken* forLoop = (ForToken*) parsed[0];
    ASSERT_EQ(forLoop->body->expressions.size(), 1, %d)
    ASSERT_EQ(forLoop->body->expressions[0]->type, TYPE_KEYWORD, %d)
    auto* cont = forLoop->body->expressions[0];
    EXPECT_EQ(cont->val_type, CONTINUE, %d)
}

TEST(parser, for_loop_with_break){
    std::vector<Token*> tokens = tokenize("for(int i = 0; i <= 5; i += 1){\n"
                                             "    break;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);

    ASSERT_EQ((int) parsed.size(), 1, %d)
    ForToken* forLoop = (ForToken*) parsed[0];
    ASSERT_EQ(forLoop->body->expressions.size(), 1, %d)
    ASSERT_EQ(forLoop->body->expressions[0]->type, TYPE_KEYWORD, %d)
    auto* cont = forLoop->body->expressions[0];
    EXPECT_EQ(cont->val_type, BREAK, %d)
}

TEST(parser, while_loop_with_empty_body){
    std::vector<Token*> tokens = tokenize("while(1){\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* whileLoop = (WhileToken*) parsed[0];
    ASSERT_EQ(whileLoop->condition->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(whileLoop->condition->lexeme, "1", %s, .c_str(),)
    EXPECT_TRUE(whileLoop->body->expressions.empty())
}

TEST(parser, while_loop_with_continue){
    std::vector<Token*> tokens = tokenize("while(1){\n"
                                             "    continue;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);

    ASSERT_EQ((int) parsed.size(), 1, %d)
    WhileToken* whileLoop = (WhileToken*) parsed[0];
    ASSERT_EQ(whileLoop->body->expressions.size(), 1, %d)
    ASSERT_EQ(whileLoop->body->expressions[0]->type, TYPE_KEYWORD, %d)
    auto* cont = whileLoop->body->expressions[0];
    EXPECT_EQ(cont->val_type, CONTINUE, %d)
}

TEST(parser, while_loop_with_break){
    std::vector<Token*> tokens = tokenize("while(1){\n"
                                             "    break;\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);

    ASSERT_EQ((int) parsed.size(), 1, %d)
    WhileToken* whileLoop = (WhileToken*) parsed[0];
    ASSERT_EQ(whileLoop->body->expressions.size(), 1, %d)
    ASSERT_EQ(whileLoop->body->expressions[0]->type, TYPE_KEYWORD, %d)
    auto* cont = whileLoop->body->expressions[0];
    EXPECT_EQ(cont->val_type, BREAK, %d)
}

TEST(parser, while_loop_with_expr_in_header_empty_body){
    std::vector<Token*> tokens = tokenize("int x = 0; while(x < 5){\n"
                                             "}\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 2, %d)

    auto* whileLoop = (WhileToken*) parsed[1];
    ASSERT_EQ(whileLoop->condition->val_type, LT, %d)
    auto* lt = (BinaryOpToken*)whileLoop->condition;
    EXPECT_EQ(lt->left->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(lt->left->lexeme, "x", %s, .c_str(),)
    EXPECT_EQ(lt->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(lt->right->lexeme, "5", %s, .c_str(),)
    EXPECT_TRUE(whileLoop->body->expressions.empty())
}

TEST(parser, while_loop_with_body){
    std::vector<Token*> tokens = tokenize("while(1){\n"
                                             "    int x = 5;\n"
                                             "}");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* whileLoop = (WhileToken*) parsed[0];
    ASSERT_EQ(whileLoop->condition->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(whileLoop->condition->lexeme, "1", %s, .c_str(),)
    EXPECT_EQ(whileLoop->body->expressions.size(), 1, %d)
    EXPECT_EQ(whileLoop->body->expressions[0]->type, TYPE_OPERATOR, %d)
    auto* def = (DefinitionToken*)whileLoop->body->expressions[0];
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "x", %s, .c_str(),)
    EXPECT_EQ(def->value->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->value->lexeme, "5", %s, .c_str(),)
}

TEST(parser, return_outside_of_function){
    std::vector<Token*> tokens = tokenize("return 5;\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token *> parsed = parse(iterator, &scope);
        FAIL()
    } catch (std::runtime_error& e){}

}

TEST(parser, use_var_more_than_once){
    std::vector<Token*> tokens = tokenize("int x = 5; int y = x + x; int z = x;\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);

    ASSERT_EQ((int) parsed.size(), 3, %d)

    auto* def = (DefinitionToken*)parsed[0];
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "x", %s, .c_str(),)
    EXPECT_EQ(def->value->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->value->lexeme, "5", %s, .c_str(),)

    auto* def2 = (DefinitionToken*)parsed[1];
    EXPECT_EQ(def2->valueType, INT, %d)
    EXPECT_EQ(def2->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def2->name, "y", %s, .c_str(),)
    auto* add = (BinaryOpToken*)def2->value;
    ASSERT_EQ(add->val_type, ADD, %d)
    EXPECT_EQ(add->left->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(add->left->lexeme, "x", %s, .c_str(),)
    EXPECT_EQ(add->right->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(add->right->lexeme, "x", %s, .c_str(),)

    auto* def3 = (DefinitionToken*)parsed[2];
    EXPECT_EQ(def3->valueType, INT, %d)
    EXPECT_EQ(def3->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def3->name, "z", %s, .c_str(),)
    EXPECT_EQ(def3->value->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(def3->value->lexeme, "x", %s, .c_str(),)
}

TEST(parser, parse_assembly){
    std::vector<Token*> tokens = tokenize("__asm__(\n"
                                             " \"mov eax, 5\"\n"
                                             ")\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ((int) parsed.size(), 1, %d)

    auto* asmToken = (AsmToken*) parsed[0];
    EXPECT_EQ_SPECIAL(asmToken->asmCode, "mov eax, 5\n", %s, .c_str(),)
}

TEST(parser, detect_error_on_equations_with_assembly){
    std::vector<Token*> tokens = tokenize("int x = 5 + __asm__(\n"
                                             "    \"mov eax, 5\"\n"
                                             ")\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    try {
        std::vector<Token *> parsed = parse(iterator, &scope);
        FAIL()
    } catch (std::runtime_error& e){}
}

TEST(parser, define_array){
    std::vector<Token*> tokens = tokenize("int x[5];\n");
    

    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ(parsed.size(), 1, %d)

    auto* def = (DefinitionToken*)parsed[0];
    ASSERT_EQ(def->val_type, IDENTIFIER, %d)
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ(def->value, nullptr, %p)
    EXPECT_EQ_SPECIAL(def->name, "x", %s, .c_str(),)
    EXPECT_EQ(def->dimensions.size(), 1, %d)
    EXPECT_EQ(def->dimensions[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->dimensions[0]->lexeme, "5", %s, .c_str(),)
}

TEST(parser, define_array_with_assignment){
    std::vector<Token*> tokens = tokenize("int x[5] = {1, 2, 3, 4, 5};\n");
    

    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ(parsed.size(), 1, %d)

    auto* def = (DefinitionToken*)parsed[0];
    ASSERT_EQ(def->val_type, IDENTIFIER, %d)
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "x", %s, .c_str(),)
    EXPECT_EQ(def->dimensions.size(), 1, %d)
    EXPECT_EQ(def->dimensions[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->dimensions[0]->lexeme, "5", %s, .c_str(),)
    EXPECT_EQ(def->value->val_type, ARRAY, %d)
    auto* array = (ArrayInitializationToken*)def->value;
    EXPECT_EQ(array->values.size(), 5, %d)
    EXPECT_EQ(array->values[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(array->values[0]->lexeme, "1", %s, .c_str(),)
    EXPECT_EQ(array->values[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(array->values[1]->lexeme, "2", %s, .c_str(),)
    EXPECT_EQ(array->values[2]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(array->values[2]->lexeme, "3", %s, .c_str(),)
    EXPECT_EQ(array->values[3]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(array->values[3]->lexeme, "4", %s, .c_str(),)
    EXPECT_EQ(array->values[4]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(array->values[4]->lexeme, "5", %s, .c_str(),)
}

TEST(parser, initialize_2d_array){
    std::vector<Token*> tokens = tokenize("int x[2][3] = {{1, 2, 3}, {4, 5, 6}};\n");
    

    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ(parsed.size(), 1, %d)

    auto* def = (DefinitionToken*)parsed[0];
    ASSERT_EQ(def->val_type, IDENTIFIER, %d)
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "x", %s, .c_str(),)
    EXPECT_EQ(def->dimensions.size(), 2, %d)
    EXPECT_EQ(def->dimensions[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->dimensions[0]->lexeme, "2", %s, .c_str(),)
    EXPECT_EQ(def->dimensions[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->dimensions[1]->lexeme, "3", %s, .c_str(),)
    EXPECT_EQ(def->value->val_type, ARRAY, %d)
    auto* array = (ArrayInitializationToken*)def->value;
    EXPECT_EQ(array->values.size(), 2, %d)
    EXPECT_EQ(array->values[0]->val_type, ARRAY, %d)
    auto* innerArray = (ArrayInitializationToken*)array->values[0];
    EXPECT_EQ(innerArray->values.size(), 3, %d)
    EXPECT_EQ(innerArray->values[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(innerArray->values[0]->lexeme, "1", %s, .c_str(),)
    EXPECT_EQ(innerArray->values[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(innerArray->values[1]->lexeme, "2", %s, .c_str(),)
    EXPECT_EQ(innerArray->values[2]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(innerArray->values[2]->lexeme, "3", %s, .c_str(),)
    EXPECT_EQ(array->values[1]->val_type, ARRAY, %d)
    auto* innerArray2 = (ArrayInitializationToken*)array->values[1];
    EXPECT_EQ(innerArray2->values.size(), 3, %d)
    EXPECT_EQ(innerArray2->values[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(innerArray2->values[0]->lexeme, "4", %s, .c_str(),)
    EXPECT_EQ(innerArray2->values[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(innerArray2->values[1]->lexeme, "5", %s, .c_str(),)
    EXPECT_EQ(innerArray2->values[2]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(innerArray2->values[2]->lexeme, "6", %s, .c_str(),)
}

TEST(parser, array_access){
    std::vector<Token*> tokens = tokenize("int x[5];\n"
                                             "int y = x[2];\n");
    

    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);
    ASSERT_EQ(parsed.size(), 2, %d)

    auto* def = (DefinitionToken*)parsed[0];
    ASSERT_EQ(def->val_type, IDENTIFIER, %d)
    EXPECT_EQ(def->valueType, INT, %d)
    EXPECT_EQ(def->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def->name, "x", %s, .c_str(),)
    EXPECT_EQ(def->dimensions.size(), 1, %d)
    EXPECT_EQ(def->dimensions[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(def->dimensions[0]->lexeme, "5", %s, .c_str(),)

    auto* def2 = (DefinitionToken*)parsed[1];
    ASSERT_EQ(def2->val_type, IDENTIFIER, %d)
    EXPECT_EQ(def2->valueType, INT, %d)
    EXPECT_EQ(def2->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(def2->name, "y", %s, .c_str(),)
    EXPECT_EQ(def2->value->val_type, ARRAY, %d)
    auto* arrayAccess = (BinaryOpToken*)def2->value;
    EXPECT_EQ(arrayAccess->left->val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(arrayAccess->left->lexeme, "x", %s, .c_str(),)
    EXPECT_EQ(arrayAccess->right->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(arrayAccess->right->lexeme, "2", %s, .c_str(),)
}

TEST(parser, inline_function){
    std::vector<Token*> tokens = tokenize("inline int add(int x, int y){\n"
                                             "    return x + y;\n"
                                             "}\n"
                                             "int z = add(2, 3);\n");
    
    TokenIterator iterator(tokens);
    Scope scope = Scope(nullptr);
    std::vector<Token *> parsed = parse(iterator, &scope);

    ASSERT_EQ(parsed.size(), 2, %d)
    auto* expr = (DefinitionToken*)parsed[1];
    EXPECT_EQ(expr->valueType, INT, %d)
    EXPECT_EQ(expr->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(expr->name, "z", %s, .c_str(),)
    auto* call = (FunctionCallToken*)expr->value;
    ASSERT_EQ(call->type, TokenType::TYPE_OPERATOR, %d)
    ASSERT_EQ(call->val_type, TokenValue::FUNCTION, %d)
    EXPECT_EQ(call->function->val_type, TokenValue::IDENTIFIER, %d)

    EXPECT_EQ_SPECIAL(call->function->lexeme, "add", %s, .c_str(),)
    ASSERT_EQ(call->arguments.size(), 2, %d)
    EXPECT_EQ(call->arguments[0]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(call->arguments[0]->lexeme, "2", %s, .c_str(),)
    EXPECT_EQ(call->arguments[1]->val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(call->arguments[1]->lexeme, "3", %s, .c_str(),)

    auto* function = (FunctionToken*) parsed[0];
    EXPECT_EQ(function->returnType, INT, %d)
    EXPECT_EQ(function->refCount, 0, %d)
    EXPECT_TRUE(function->is_inline)
    EXPECT_EQ_SPECIAL(function->name, "add", %s, .c_str(),)
    ASSERT_EQ(function->parameters.size(), 2, %d)
    auto* x = function->parameters[0];
    EXPECT_EQ(x->valueType, INT, %d)
    EXPECT_EQ(x->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(x->name, "x", %s, .c_str(),)
    auto* y = function->parameters[1];
    EXPECT_EQ(y->valueType, INT, %d)
    EXPECT_EQ(y->refCount, 0, %d)
    EXPECT_EQ_SPECIAL(y->name, "y", %s, .c_str(),)
    EXPECT_EQ(function->body->expressions.size(), 1, %d)



}

