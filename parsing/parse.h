//
// Created by Ethan Horowitz on 3/8/24.
//

#ifndef I2C_PARSE_H
#define I2C_PARSE_H

#include <map>
#include <utility>

#include "tokenize.h"
#include "tokenTypes.h"

int precedence(TokenValue op);

std::vector<Token*> toTokenRefs(std::vector<Token>& tokens);

struct TokenIterator {
private:
    std::vector<Token*> tokens;
    int index;
public:
    explicit TokenIterator(const std::vector<Token*>& tokens){
        this->tokens = tokens;
        index = 0;
    }

    Token* next(){
        if (tokens.empty() || index >= tokens.size()) return nullptr;
        Token* token = tokens[index];
        index++;
        return token;
    }

    Token* peek() {
        if (tokens.empty() || index >= tokens.size()) return nullptr;
        return tokens[index];
    }

    [[nodiscard]] bool hasNext() const{
        return !tokens.empty() && index < tokens.size();
    }
};

class Scope {
private:
    std::map<std::string, Token*> context;
    Scope* parentContext;

public:
    explicit Scope(Scope* parent){
        this->parentContext = parent;
    }
    Token* find(const std::string& name){
        if (context.find(name) != context.end()){
            return context[name];
        }
        if (parentContext == nullptr) return nullptr;
        return parentContext->find(name);
    }
    void add(const std::string& name, Token* value){
        context[name] = value;
    }
    bool isBaseScope(){
        return parentContext == nullptr;
    }

};

Token* parseExpression(TokenIterator& iter, Scope* scope);
std::vector<Token*> parse(TokenIterator& tokens, Scope* scope);

#endif //I2C_PARSE_H
