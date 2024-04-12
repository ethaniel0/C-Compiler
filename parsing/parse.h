//
// Created by Ethan Horowitz on 3/8/24.
//

#ifndef I2C_PARSE_H
#define I2C_PARSE_H

#include <map>
#include <utility>
#include <vector>
#include <stdexcept>

#include "tokenize.h"
#include "tokenTypes.h"

int precedence(TokenValue op);


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
    std::map<std::string, Token*> identifiers;
    Scope* parentContext;
    int depth = 0;

public:
    explicit Scope(Scope* parent){
        this->parentContext = parent;
        if (parent != nullptr){
            depth = parent->depth + 1;
        }
        else {
            depth = 0;
        }
    }
    Token* find(const std::string& name, Token* token){
        if (context.find(name) != context.end()){
            if (identifiers.find(name) == identifiers.end()){
                identifiers[name] = token;
                token->track = 0;
            }
            else {
                identifiers[name]->track = 1;
                token->track = 0;
                identifiers[name] = token;
            }

            return context[name];
        }
        if (parentContext == nullptr) return nullptr;
        return parentContext->find(name, token);
    }
    void add(const std::string& name, Token* value){
        context[name] = value;

        if (identifiers.find(name) == identifiers.end()){
            identifiers[name] = value;
            value->track = 0;
        }
        else {
            identifiers[name]->track = 1;
            value->track = 0;
            identifiers[name] = value;
        }
    }
    bool isBaseScope(){
        return parentContext == nullptr;
    }
    int getDepth(){
        return depth;
    }

};

Token* parseExpression(TokenIterator& iter, Scope* scope);
std::vector<Token*> parse(TokenIterator& tokens, Scope* scope);

#endif //I2C_PARSE_H
