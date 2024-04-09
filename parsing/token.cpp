//
// Created by Ethan Horowitz on 3/8/24.
//

#include "tokenize.h"
#include <iostream>

#include <map>

std::map<TokenType, std::string> tokenTypeMap = {
        {TokenType::TYPE_BRACKET, "BRACKET"},
        {TokenType::TYPE_SEPARATOR, "SEPARATOR"},
        {TokenType::TYPE_OPERATOR, "OPERATOR"},
        {TokenType::TYPE_IDENTIFIER, "IDENTIFIER"},
        {TokenType::TYPE_VALUE, "VALUE"},
        {TokenType::TYPE_TYPE, "TYPE"},
        {TokenType::TYPE_KEYWORD, "KEYWORD"},
        {TokenType::TYPE_NONE, "NONE"}
};

std::map<TokenValue, std::string> tokenValueMap = {
        {TokenValue::NONE,          "NONE"},
        {TokenValue::LEFT_PAREN,    "LEFT_PAREN"},
        {TokenValue::RIGHT_PAREN,   "RIGHT_PAREN"},
        {TokenValue::LEFT_BRACE,    "LEFT_BRACE"},
        {TokenValue::RIGHT_BRACE,   "RIGHT_BRACE"},
        {TokenValue::LEFT_BRACKET,  "LEFT_BRACKET"},
        {TokenValue::RIGHT_BRACKET, "RIGHT_BRACKET"},

        {TokenValue::COMMA,         "COMMA"},
        {TokenValue::SEMICOLON,     "SEMICOLON"},

        {TokenValue::ADD,           "ADD"},
        {TokenValue::ADD_EQ,        "ADD_EQ"},
        {TokenValue::MINUS,         "MINUS"},
        {TokenValue::MINUS_EQ,      "MINUS_EQ"},
        {TokenValue::NEG,           "NEG"},
        {TokenValue::MULT,          "MULT"},
        {TokenValue::MULT_EQ,       "MULT_EQ"},
        {TokenValue::DEREF,         "DEREF"},
        {TokenValue::DIV,           "DIV"},
        {TokenValue::DIV_EQ,        "DIV_EQ"},
        {TokenValue::BIN_AND,       "BIN_AND"},
        {TokenValue::BIN_AND_EQ,    "BIN_AND_EQ"},
        {TokenValue::AND,           "AND"},
        {TokenValue::REF,           "REF"},
        {TokenValue::BIN_OR,        "BIN_OR"},
        {TokenValue::BIN_OR_EQ,     "BIN_OR_EQ"},
        {TokenValue::OR,            "OR"},
        {TokenValue::XOR,           "XOR"},
        {TokenValue::XOR_EQ,        "XOR_EQ"},
        {TokenValue::MOD,           "MOD"},
        {TokenValue::MOD_EQ,        "MOD_EQ"},
        {TokenValue::NOT,           "NOT"},
        {TokenValue::NOT_EQ,     "NOT_EQ"},
        {TokenValue::GT,         "GT"},
        {TokenValue::GTE,        "GTE"},
        {TokenValue::LT,         "LT"},
        {TokenValue::LTE,        "LTE"},
        {TokenValue::EQ,         "EQ"},
        {TokenValue::EQ_EQ,      "EQ_EQ"},

        {TokenValue::IDENTIFIER, "IDENTIFIER"},
        {TokenValue::STRING,     "STRING"},
        {TokenValue::NUMBER_INT, "NUMBER_INT"},
        {TokenValue::NUMBER_FLOAT, "NUMBER_FLOAT"},
        {TokenValue::CHARACTER,  "CHARACTER"},
        {TokenValue::NIL,        "NIL"},
        {TokenValue::FUNCTION,   "FUNCTION"},

        {TokenValue::INT,        "INT"},
        {TokenValue::FLOAT,      "FLOAT"},
        {TokenValue::CHAR,       "CHAR"},
        {TokenValue::DOUBLE,     "DOUBLE"},
        {TokenValue::LONG,       "LONG"},
        {TokenValue::SHORT,      "SHORT"},
        {TokenValue::VOID,          "VOID"},

        {TokenValue::IF,            "IF"},
        {TokenValue::ELSE,          "ELSE"},
        {TokenValue::FOR,           "FOR"},
        {TokenValue::WHILE,         "WHILE"},
        {TokenValue::BREAK,         "BREAK"},
        {TokenValue::CONTINUE,      "CONTINUE"},
        {TokenValue::NIL,           "NIL"},
        {TokenValue::RETURN,        "RETURN"},
};

std::string tokenTypeAsString(TokenType type) {
    return tokenTypeMap[type];
}

std::string tokenValueAsString(TokenValue value) {
    return tokenValueMap[value];
}


std::vector<Token> tokenize(std::string source) {
    std::vector<Token> tokens;
    int start;
    int current = 0;
    int line = 1;

    std::map<std::string, std::vector<Token>> defines;

    Token none_token = Token(TokenType::TYPE_NONE, TokenValue::NONE, "", line);

    bool is_neg = false;

    while (current < source.length()) {
        start = current;
        char c = source.at(current);
        Token* prev_token;
        if (!tokens.empty()) prev_token = &tokens[tokens.size() - 1];
        else prev_token = &none_token;

        switch (c) {
            case '\n':
                line++;
                break;
            case '\r':
                break;
            case ' ':
            case '\t':
                break;
            // BRACKET
            case '(':
                tokens.emplace_back(TokenType::TYPE_BRACKET, TokenValue::LEFT_PAREN, "(", line);
                break;
            case ')':
                tokens.emplace_back(TokenType::TYPE_BRACKET, TokenValue::RIGHT_PAREN, ")", line);
                break;
            case '{':
                tokens.emplace_back(TokenType::TYPE_BRACKET, TokenValue::LEFT_BRACE, "{", line);
                break;
            case '}':
                tokens.emplace_back(TokenType::TYPE_BRACKET, TokenValue::RIGHT_BRACE, "}", line);
                break;
            case '[':
                tokens.emplace_back(TokenType::TYPE_BRACKET, TokenValue::LEFT_BRACKET, "[", line);
                break;
            case ']':
                tokens.emplace_back(TokenType::TYPE_BRACKET, TokenValue::RIGHT_BRACKET, "]", line);
                break;

            // SEPARATOR
            case ',':
                tokens.emplace_back(TokenType::TYPE_SEPARATOR, TokenValue::COMMA, ",", line);
                break;
            case ';':
                tokens.emplace_back(TokenType::TYPE_SEPARATOR, TokenValue::SEMICOLON, ";", line);
                break;

            // OPERATOR
            case '+':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::ADD_EQ, "+=", line);
                    current++;
                } else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::ADD, "+", line);
                }
                break;
            case '-':
                // if negative number skip to negative
                if (current < source.size() - 1 && isdigit(source.at(current+1)) && !(
                        prev_token->val_type == IDENTIFIER ||
                        prev_token->val_type == NUMBER_INT ||
                        prev_token->val_type == NUMBER_FLOAT ||
                        prev_token->val_type == RIGHT_PAREN
                        )) {
                    is_neg = true;
                    break;
                }
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::MINUS_EQ, "-=", line);
                    current++;
                } else if (prev_token->type != TokenType::TYPE_IDENTIFIER &&
                           prev_token->type != TokenType::TYPE_VALUE &&
                           prev_token->type != TokenType::TYPE_BRACKET) {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::NEG, "-", line);
                } else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::MINUS, "-", line);
                }
                break;

            case '*':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::MULT_EQ, "*=", line);
                    current++;
                }
                else if (prev_token->type != TokenType::TYPE_IDENTIFIER && prev_token->type != TokenType::TYPE_VALUE && prev_token->val_type != TokenValue::RIGHT_PAREN){
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::DEREF, "*", line);
                }
                else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::MULT, "*", line);
                }
                break;
            case '/':
                if (current < source.size() - 1 && source.at(current + 1) == '/') {
                    while (current < source.size() && source.at(current) != '\n') {
                        current++;
                    }
                    if (source.at(current) == '\n') line++;
                    break;
                }
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::DIV_EQ, "/=", line);
                    current++;
                } else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::DIV, "/", line);
                }
                break;
            case '&':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::BIN_AND_EQ, "&=", line);
                    current++;
                }
                else if (current < source.size() - 1 && source.at(current + 1) == '&') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::AND, "&&", line);
                    current++;
                }
                else if (prev_token->type != TokenType::TYPE_IDENTIFIER && prev_token->type != TokenType::TYPE_VALUE && prev_token->val_type != TokenValue::RIGHT_PAREN){
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::REF, "&", line);
                }
                else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::BIN_AND, "&", line);
                }
                break;
            case '|':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::BIN_OR_EQ, "|=", line);
                    current++;
                }
                else if (source.at(current + 1) == '|') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::OR, "||", line);
                    current++;
                }
                else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::BIN_OR, "|", line);
                }
                break;
            case '^':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::XOR_EQ, "^=", line);
                    current++;
                } else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::XOR, "^", line);
                }
                break;
            case '%':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::MOD_EQ, "%=", line);
                    current++;
                } else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::MOD, "%", line);
                }
                break;
            case '!':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::NOT_EQ, "!=", line);
                    current++;
                } else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::NOT, "!", line);
                }
                break;
            case '>':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::GTE, ">=", line);
                    current++;
                }
                else if (current < source.size() - 1 && source.at(current + 1) == '>') {
                    if (current < source.size() - 2 && source.at(current + 2) == '=') {
                        tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::RSHIFT_EQ, ">>=", line);
                        current++;
                    } else {
                        tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::RSHIFT, ">>", line);
                    }
                    current++;
                }
                else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::GT, ">", line);
                }
                break;
            case '<':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::LTE, "<=", line);
                    current++;
                }
                else if (current < source.size() - 1 && source.at(current + 1) == '<') {
                    if (current < source.size() - 2 && source.at(current + 2) == '=') {
                        tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::LSHIFT_EQ, "<<=", line);
                        current++;
                    } else {
                        tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::LSHIFT, "<<", line);
                    }
                    current++;
                }
                else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::LT, "<", line);
                }
                break;
            case '=':
                if (current < source.size() - 1 && source.at(current + 1) == '=') {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::EQ_EQ, "==", line);
                    current++;
                } else {
                    tokens.emplace_back(TokenType::TYPE_OPERATOR, TokenValue::EQ, "=", line);
                }
                break;
            default:
                if (c == '"') {
                    current++;
                    while (
                            source.at(current) != c ||
                            (source.at(current) == c && source.at(current - 1) == '\\')
                          ) {
                        current++;
                    }
                    tokens.emplace_back(TokenType::TYPE_VALUE, TokenValue::STRING, source.substr(start + 1, current - start - 1), line);
                }
                else if (c == '\'') {
                    current++;
                    tokens.emplace_back(TokenType::TYPE_VALUE, TokenValue::CHARACTER, source.substr(current, 1), line);
                    current++;
                }
                else if (isdigit(c)) {
                    while (current < source.size() && isdigit(source.at(current))) {
                        current++;
                    }
                    if (current < source.size() && source.at(current) == '.' && isdigit(source.at(current + 1))) {
                        current++;
                        while (current < source.size() && isdigit(source.at(current))) {
                            current++;
                        }
                    }
                    std::string lexeme = "";
                    if (is_neg) {
                        lexeme += "-";
                        is_neg = false;
                    }
                    lexeme += source.substr(start, current - start);
                    TokenValue val_type = lexeme.find('.') != std::string::npos ? TokenValue::NUMBER_FLOAT : TokenValue::NUMBER_INT;
                    tokens.emplace_back(TokenType::TYPE_VALUE, val_type, lexeme, line);
                    current--;
                }
                else if (isalpha(c) || c == '_' || c == '#') {
                    current++;
                    while (current < source.size() && (isalnum(source.at(current)) || source.at(current) == '_')) {
                        current++;
                    }
                    std::string word = source.substr(start, current - start);
                    current--;

                    // data types
                    if (word == "int") tokens.emplace_back(TokenType::TYPE_TYPE, TokenValue::INT, word, line);
                    else if (word == "float") tokens.emplace_back(TokenType::TYPE_TYPE, TokenValue::FLOAT, word, line);
                    else if (word == "char") tokens.emplace_back(TokenType::TYPE_TYPE, TokenValue::CHAR, word, line);
                    else if (word == "double") tokens.emplace_back(TokenType::TYPE_TYPE, TokenValue::DOUBLE, word, line);
                    else if (word == "long") tokens.emplace_back(TokenType::TYPE_TYPE, TokenValue::LONG, word, line);
                    else if (word == "short") tokens.emplace_back(TokenType::TYPE_TYPE, TokenValue::SHORT, word, line);
                    else if (word == "void") tokens.emplace_back(TokenType::TYPE_TYPE, TokenValue::VOID, word, line);

                    // keywords
                    else if (word == "if") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::IF, word, line);
                    else if (word == "else") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::ELSE, word, line);
                    else if (word == "for") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::FOR, word, line);
                    else if (word == "while") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::WHILE, word, line);
                    else if (word == "break") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::BREAK, word, line);
                    else if (word == "continue") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::CONTINUE, word, line);
                    else if (word == "NULL") tokens.emplace_back(TokenType::TYPE_VALUE, TokenValue::NIL, word, line);
                    else if (word == "return") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::RETURN, word, line);
                    else if (word == "inline") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::INLINE, word, line);

                    else if (word == "__asm__") tokens.emplace_back(TokenType::TYPE_KEYWORD, TokenValue::ASM, word, line);

                    else if (word == "#define"){
                        std::string define_key;
                        std::string define;
                        current++;
                        while (current < source.size() && source.at(current) == ' '){
                            current++;
                        }
                        while (current < source.size() && source.at(current) != ' '){
                            define_key += source.at(current);
                            current++;
                        }
                        while (current < source.size() && source.at(current) == ' '){
                            current++;
                        }
                        while (current < source.size() && source.at(current) != '\n'){
                            define += source.at(current);
                            current++;
                        }
                        if (source.at(current) == '\n') line++;

                        defines[define_key] = tokenize(define);
                    }
                    else if (defines.find(word) != defines.end()){
                        tokens.insert(tokens.end(), defines[word].begin(), defines[word].end());
                    }
                    else {
                        tokens.emplace_back(TokenType::TYPE_IDENTIFIER, TokenValue::IDENTIFIER, word, line);
                    }
                }
                else {
                    throw std::runtime_error("Unexpected character: " + std::string(1, c) + " at line " + std::to_string(line));
                }
                break;
        }
        current++;
    }
    return tokens;
}

