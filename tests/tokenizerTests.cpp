//
// Created by Ethan Horowitz on 3/13/24.
//

#include "tokenizerTests.h"

void run_tokenizer_tests(){
    RUN_TEST_GROUP("tokenizerTests");
}

TEST(tokenizerTests, all_tokens){
    // left_paren
    std::string source = "()[]{},; 5 + 4 += 2- -= -1\n *x & y; *x &= f++";
    TokenValue valueOrders[] = {
            LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET, RIGHT_BRACKET, LEFT_BRACE, RIGHT_BRACE,
            COMMA, SEMICOLON, NUMBER, ADD, NUMBER, ADD_EQ, NUMBER, MINUS, MINUS_EQ, NUMBER,
            MULT, IDENTIFIER, BIN_AND, IDENTIFIER, SEMICOLON, DEREF, IDENTIFIER, BIN_AND_EQ, IDENTIFIER,
            ADD, ADD
    };
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ((int)tokens.size(), sizeof(valueOrders) / sizeof(valueOrders[0]), %d)
    for (int i = 0; i < tokens.size(); i++){
        EXPECT_EQ(tokens[i].val_type, valueOrders[i], %d)
    }

    source = "y &= 5 & 6 && &h; z |= 1 | 2 || h; a ^= 3 ^ 8; b %= 7 % 8; c != !d; 3 >= 5 > 8, 4 <= 7 < 9, k = 6 == 8";
    TokenValue valueOrders2[] = {
            IDENTIFIER, BIN_AND_EQ, NUMBER, BIN_AND, NUMBER, AND, REF, IDENTIFIER, SEMICOLON,
            IDENTIFIER, BIN_OR_EQ, NUMBER, BIN_OR, NUMBER, OR, IDENTIFIER, SEMICOLON,
            IDENTIFIER, XOR_EQ, NUMBER, XOR, NUMBER, SEMICOLON,
            IDENTIFIER, MOD_EQ, NUMBER, MOD, NUMBER, SEMICOLON,
            IDENTIFIER, NOT_EQ, NOT, IDENTIFIER, SEMICOLON,
            NUMBER, GTE, NUMBER, GT, NUMBER,
            COMMA, NUMBER, LTE, NUMBER, LT, NUMBER, COMMA,
            IDENTIFIER, EQ, NUMBER, EQ_EQ, NUMBER

    };
    tokens = tokenize(source);
    EXPECT_EQ((int)tokens.size(), sizeof(valueOrders2) / sizeof(valueOrders2[0]), %d)
    for (int i = 0; i < tokens.size(); i++){
        EXPECT_EQ(tokens[i].val_type, valueOrders2[i], %d)
    }

    source = "if (x == 5) { y = 6; } else { z = 7; }";
    TokenValue valueOrders3[] = {
            IF, LEFT_PAREN, IDENTIFIER, EQ_EQ, NUMBER, RIGHT_PAREN, LEFT_BRACE, IDENTIFIER, EQ, NUMBER, SEMICOLON, RIGHT_BRACE, ELSE, LEFT_BRACE, IDENTIFIER, EQ, NUMBER, SEMICOLON, RIGHT_BRACE
    };
    tokens = tokenize(source);
    EXPECT_EQ((int)tokens.size(), sizeof(valueOrders3) / sizeof(valueOrders3[0]), %d)
    for (int i = 0; i < tokens.size(); i++){
        EXPECT_EQ(tokens[i].val_type, valueOrders3[i], %d)
    }

    source = "int float char double long short void NULL";
    TokenValue valueOrders4[] = {
            INT, FLOAT, CHAR, DOUBLE, LONG, SHORT, VOID, NIL
    };
    tokens = tokenize(source);
    EXPECT_EQ((int)tokens.size(), sizeof(valueOrders4) / sizeof(valueOrders4[0]), %d)
    for (int i = 0; i < tokens.size(); i++){
        EXPECT_EQ(tokens[i].val_type, valueOrders4[i], %d)
    }
}

TEST(tokenizerTests, multVariations){
    std::string source = "3 * 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, MULT, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER, %d)

    source = "3 + *4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, DEREF, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER, %d)

    source = "(3 + 4) * 5";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 7, %d)
    EXPECT_EQ(tokens[0].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[1].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[2].val_type, ADD, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[4].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[5].val_type, MULT, %d)
    EXPECT_EQ(tokens[6].val_type, NUMBER, %d)
}

TEST(tokenizerTests, andVariations){
    std::string source = "3 & 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, BIN_AND, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER, %d)

    source = "3 && 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, AND, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER, %d)

    source = "3 + &4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, REF, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER, %d)

    source = "(3 + 4) & 5";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 7, %d)
    EXPECT_EQ(tokens[0].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[1].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[2].val_type, ADD, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[4].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[5].val_type, BIN_AND, %d)
    EXPECT_EQ(tokens[6].val_type, NUMBER, %d)
}

TEST(tokenizerTests, orVariations){
    std::string source = "3 | 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, BIN_OR, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER, %d)

    source = "3 || 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, OR, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER, %d)

    source = "3 + |4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, BIN_OR, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER, %d)

    source = "(3 + 4) | 5";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 7, %d)
    EXPECT_EQ(tokens[0].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[1].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[2].val_type, ADD, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER, %d)
    EXPECT_EQ(tokens[4].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[5].val_type, BIN_OR, %d)
    EXPECT_EQ(tokens[6].val_type, NUMBER, %d)
}

TEST(tokenizerTests, assembly){
    std::string source = "__asm__(\"li $v0, 4\")";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, ASM, %d)
    EXPECT_EQ(tokens[1].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[2].val_type, STRING, %d)
    EXPECT_EQ(tokens[3].val_type, RIGHT_PAREN, %d)
}