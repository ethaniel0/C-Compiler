//
// Created by Ethan Horowitz on 3/13/24.
//

#include "tokenizerTests.h"

void run_tokenizer_tests(){
    RUN_TEST_GROUP("tokenizerTests");
}

TEST(tokenizerTests, no_tokens){
    std::string source = "";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 0, %d)
}

TEST(tokenizerTests, all_tokens){
    // left_paren
    std::string source = "()[]{},; 5 + 4 += 2- -= -1\n *x & y; *x &= f++";
    TokenValue valueOrders[] = {
            LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET, RIGHT_BRACKET, LEFT_BRACE, RIGHT_BRACE,
            COMMA, SEMICOLON, NUMBER_INT, ADD, NUMBER_INT, ADD_EQ, NUMBER_INT, MINUS, MINUS_EQ, NUMBER_INT,
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
            IDENTIFIER, BIN_AND_EQ, NUMBER_INT, BIN_AND, NUMBER_INT, AND, REF, IDENTIFIER, SEMICOLON,
            IDENTIFIER, BIN_OR_EQ, NUMBER_INT, BIN_OR, NUMBER_INT, OR, IDENTIFIER, SEMICOLON,
            IDENTIFIER, XOR_EQ, NUMBER_INT, XOR, NUMBER_INT, SEMICOLON,
            IDENTIFIER, MOD_EQ, NUMBER_INT, MOD, NUMBER_INT, SEMICOLON,
            IDENTIFIER, NOT_EQ, NOT, IDENTIFIER, SEMICOLON,
            NUMBER_INT, GTE, NUMBER_INT, GT, NUMBER_INT,
            COMMA, NUMBER_INT, LTE, NUMBER_INT, LT, NUMBER_INT, COMMA,
            IDENTIFIER, EQ, NUMBER_INT, EQ_EQ, NUMBER_INT

    };
    tokens = tokenize(source);
    EXPECT_EQ((int)tokens.size(), sizeof(valueOrders2) / sizeof(valueOrders2[0]), %d)
    for (int i = 0; i < tokens.size(); i++){
        EXPECT_EQ(tokens[i].val_type, valueOrders2[i], %d)
    }

    source = "if (x == 5) { y = 6; } else { z = 7; }";
    TokenValue valueOrders3[] = {
            IF, LEFT_PAREN, IDENTIFIER, EQ_EQ, NUMBER_INT, RIGHT_PAREN, LEFT_BRACE, IDENTIFIER, EQ, NUMBER_INT, SEMICOLON, RIGHT_BRACE, ELSE, LEFT_BRACE, IDENTIFIER, EQ, NUMBER_INT, SEMICOLON, RIGHT_BRACE
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

TEST(tokenizerTests, type_and_value_as_string){
    EXPECT_EQ_SPECIAL(tokenValueAsString(NONE), "NONE", %s, .c_str(),)
    EXPECT_EQ_SPECIAL(tokenValueAsString(LEFT_PAREN), "LEFT_PAREN", %s, .c_str(),)
    EXPECT_EQ_SPECIAL(tokenValueAsString(RIGHT_PAREN), "RIGHT_PAREN", %s, .c_str(),)
    EXPECT_EQ_SPECIAL(tokenValueAsString(LEFT_BRACKET), "LEFT_BRACKET", %s, .c_str(),)

    EXPECT_EQ_SPECIAL(tokenTypeAsString(TYPE_TYPE), "TYPE", %s, .c_str(),)
    EXPECT_EQ_SPECIAL(tokenTypeAsString(TYPE_BRACKET), "BRACKET", %s, .c_str(),)
    EXPECT_EQ_SPECIAL(tokenTypeAsString(TYPE_SEPARATOR), "SEPARATOR", %s, .c_str(),)
    EXPECT_EQ_SPECIAL(tokenTypeAsString(TYPE_OPERATOR), "OPERATOR", %s, .c_str(),)
}

TEST(tokenizerTests, multVariations){
    std::string source = "3 * 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, MULT, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 + *4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, DEREF, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)

    source = "(3 + 4) * 5";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 7, %d)
    EXPECT_EQ(tokens[0].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[1].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[2].val_type, ADD, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[4].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[5].val_type, MULT, %d)
    EXPECT_EQ(tokens[6].val_type, NUMBER_INT, %d)

    source = "3 *= 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, MULT_EQ, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)
}

TEST(tokenizerTests, divVariations){
    std::string source = "3 / 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, DIV, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 + /4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, DIV, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)

    source = "(3 + 4) / 5";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 7, %d)
    EXPECT_EQ(tokens[0].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[1].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[2].val_type, ADD, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[4].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[5].val_type, DIV, %d)
    EXPECT_EQ(tokens[6].val_type, NUMBER_INT, %d)

    source = "3 /= 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, DIV_EQ, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

}

TEST(tokenizerTests, andVariations){
    std::string source = "3 & 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, BIN_AND, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 && 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, AND, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 + &4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, REF, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)

    source = "(3 + 4) & 5";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 7, %d)
    EXPECT_EQ(tokens[0].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[1].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[2].val_type, ADD, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[4].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[5].val_type, BIN_AND, %d)
    EXPECT_EQ(tokens[6].val_type, NUMBER_INT, %d)
}

TEST(tokenizerTests, orVariations){
    std::string source = "3 | 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, BIN_OR, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 || 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, OR, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 + |4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, BIN_OR, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)

    source = "(3 + 4) | 5";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 7, %d)
    EXPECT_EQ(tokens[0].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[1].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[2].val_type, ADD, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[4].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[5].val_type, BIN_OR, %d)
    EXPECT_EQ(tokens[6].val_type, NUMBER_INT, %d)
}

TEST(tokenizerTests, lt_lte_lshift){
    std::string source = "3 < 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, LT, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 <= 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, LTE, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 << 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, LSHIFT, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 < < 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, LT, %d)
    EXPECT_EQ(tokens[2].val_type, LT, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)

    source = "3 <<= 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, LSHIFT_EQ, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)
}

TEST(tokenizerTests, gt_gte_rshift){
    std::string source = "3 > 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, GT, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 >= 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, GTE, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 >> 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, RSHIFT, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 > > 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 4, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, GT, %d)
    EXPECT_EQ(tokens[2].val_type, GT, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)

    source = "3 >>= 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, RSHIFT_EQ, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)
}

TEST(tokenizerTests, get_char){
    std::string source = "char c = 'a';";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 5, %d)
    EXPECT_EQ(tokens[0].val_type, CHAR, %d)
    EXPECT_EQ(tokens[1].val_type, IDENTIFIER, %d)
    EXPECT_EQ(tokens[2].val_type, EQ, %d)
    EXPECT_EQ(tokens[3].val_type, CHARACTER, %d)
    EXPECT_EQ(tokens[4].val_type, SEMICOLON, %d)
}

TEST(tokenizerTests, numberVariations){
    std::string source = "3 + 4";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3 + 4.0";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_FLOAT, %d)

    source = "3.0 + 4";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_FLOAT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_INT, %d)

    source = "3.0 + 4.0";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_FLOAT, %d)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_FLOAT, %d)

    source = "-3 + 4.0";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(tokens[0].lexeme, "-3", %s, .c_str(),)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_FLOAT, %d)

    source = "-3.0 + 4.0";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 3, %d)
    EXPECT_EQ(tokens[0].val_type, NUMBER_FLOAT, %d)
    EXPECT_EQ_SPECIAL(tokens[0].lexeme, "-3.0", %s, .c_str(),)
    EXPECT_EQ(tokens[1].val_type, ADD, %d)
    EXPECT_EQ(tokens[2].val_type, NUMBER_FLOAT, %d)
}

TEST(tokenizerTests, neg){
    std::string source = "-x";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 2, %d)
    EXPECT_EQ(tokens[0].val_type, NEG, %d)
    EXPECT_EQ(tokens[1].val_type, IDENTIFIER, %d)
}

TEST(tokenizerTests, loops){
    std::string source = "for (int i = 0; i < 5; i += 1) { x += 1; }";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 21, %d)
    EXPECT_EQ(tokens[0].val_type, FOR, %d)
    EXPECT_EQ(tokens[1].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[2].type, TYPE_TYPE, %d)
    EXPECT_EQ(tokens[3].val_type, IDENTIFIER, %d)
    EXPECT_EQ(tokens[4].val_type, EQ, %d)
    EXPECT_EQ(tokens[5].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[6].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[7].val_type, IDENTIFIER, %d)
    EXPECT_EQ(tokens[8].val_type, LT, %d)
    EXPECT_EQ(tokens[9].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[10].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[11].val_type, IDENTIFIER, %d)
    EXPECT_EQ(tokens[12].val_type, ADD_EQ, %d)
    EXPECT_EQ(tokens[13].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[14].val_type, RIGHT_PAREN, %d)

    source = "while (x < 5) { x += 1; }";
    tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 12, %d)
    EXPECT_EQ(tokens[0].val_type, WHILE, %d)
    EXPECT_EQ(tokens[1].val_type, LEFT_PAREN, %d)
    EXPECT_EQ(tokens[2].val_type, IDENTIFIER, %d)
    EXPECT_EQ(tokens[3].val_type, LT, %d)
    EXPECT_EQ(tokens[4].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[5].val_type, RIGHT_PAREN, %d)
    EXPECT_EQ(tokens[6].val_type, LEFT_BRACE, %d)
    EXPECT_EQ(tokens[7].val_type, IDENTIFIER, %d)
    EXPECT_EQ(tokens[8].val_type, ADD_EQ, %d)
    EXPECT_EQ(tokens[9].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[10].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[11].val_type, RIGHT_BRACE, %d)

}

TEST(tokenizerTests, break_continue_null_return_inline_keywords){
    std::string source = "break; continue; NULL; return 5; inline int x = 5;";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 15, %d)
    EXPECT_EQ(tokens[0].val_type, BREAK, %d)
    EXPECT_EQ(tokens[1].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[2].val_type, CONTINUE, %d)
    EXPECT_EQ(tokens[3].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[4].val_type, NIL, %d)
    EXPECT_EQ(tokens[5].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[6].val_type, RETURN, %d)
    EXPECT_EQ(tokens[7].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[8].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[9].val_type, INLINE, %d)
    EXPECT_EQ(tokens[10].type, TYPE_TYPE, %d)
    EXPECT_EQ(tokens[11].val_type, IDENTIFIER, %d)
    EXPECT_EQ(tokens[12].val_type, EQ, %d)
    EXPECT_EQ(tokens[13].val_type, NUMBER_INT, %d)
    EXPECT_EQ(tokens[14].val_type, SEMICOLON, %d)
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

TEST(tokenizerTests, define_keyword){
    std::string source = "#define x 5\n"
                         "int y = x;";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 5, %d)
    EXPECT_EQ(tokens[0].type, TYPE_TYPE, %d)
    EXPECT_EQ(tokens[1].val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(tokens[1].lexeme, "y", %s, .c_str(),)
    EXPECT_EQ(tokens[2].val_type, EQ, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(tokens[3].lexeme, "5", %s, .c_str(),)
    EXPECT_EQ(tokens[4].val_type, SEMICOLON, %d)

}

TEST(tokenizerTests, comments){
    std::string source = "int x = 5; // this is a comment\n"
                         "int y = 6;";
    std::vector<Token> tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 10, %d)
    EXPECT_EQ(tokens[0].type, TYPE_TYPE, %d)
    EXPECT_EQ(tokens[1].val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(tokens[1].lexeme, "x", %s, .c_str(),)
    EXPECT_EQ(tokens[2].val_type, EQ, %d)
    EXPECT_EQ(tokens[3].val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(tokens[3].lexeme, "5", %s, .c_str(),)
    EXPECT_EQ(tokens[4].val_type, SEMICOLON, %d)
    EXPECT_EQ(tokens[5].type, TYPE_TYPE, %d)
    EXPECT_EQ(tokens[6].val_type, IDENTIFIER, %d)
    EXPECT_EQ_SPECIAL(tokens[6].lexeme, "y", %s, .c_str(),)
    EXPECT_EQ(tokens[7].val_type, EQ, %d)
    EXPECT_EQ(tokens[8].val_type, NUMBER_INT, %d)
    EXPECT_EQ_SPECIAL(tokens[8].lexeme, "6", %s, .c_str(),)
    EXPECT_EQ(tokens[9].val_type, SEMICOLON, %d)
}

TEST(tokenizerTests, unexpected_character){
    std::string source = "int x = 5; $ int y = 6;";
    try{
        std::vector<Token> tokens = tokenize(source);
    } catch (std::runtime_error& e){
        EXPECT_EQ_SPECIAL(std::string(e.what()), "Unexpected character: $ at line 1", %s, .c_str(),)
    }
}