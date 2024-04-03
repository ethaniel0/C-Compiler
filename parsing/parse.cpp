//
// Created by Ethan Horowitz on 3/8/24.
//

#include "parse.h"
#include <map>

std::vector<Token*> toTokenRefs(std::vector<Token>& tokens){
    std::vector<Token*> tokenPtrs;
    tokenPtrs.reserve(tokens.size());
    for (Token& token : tokens) {
        tokenPtrs.push_back(&token);
    }
    return tokenPtrs;
}

TokenValue precedenceArr[9][10] = {
    {EQ, ADD_EQ, MINUS_EQ, MULT_EQ, DIV_EQ,
     MOD_EQ, BIN_AND_EQ, BIN_OR_EQ, XOR_EQ, NONE},
    {OR, NONE},
    {AND, NONE},
    {EQ_EQ, NOT_EQ, NONE},
    {LT, LTE, GT, GTE, NONE},
    {BIN_AND, BIN_OR, XOR, NONE},
    {ADD, MINUS, NONE},
    {MULT, DIV, MOD, NONE},
    {NEG, DEREF, REF, NOT, NONE},
};

int precedence(TokenValue op){
    if (op == TokenValue::NONE) return -1;
    for (int i = 0; i < 9; i++){
        for (int j = 0; j < 10; j++){
            if (precedenceArr[i][j] == NONE) break;
            if (precedenceArr[i][j] == op){
                return i + 1;
            }
        }
    }
    return 10000;
}

BinaryOpToken* makeOperator(Token* t){
    if (t->type != TokenType::TYPE_OPERATOR){
        throw std::runtime_error("Token is not an operator");
    }

    if (t->val_type == TokenValue::NEG || t->val_type == TokenValue::DEREF || t->val_type == TokenValue::REF || t->val_type == TokenValue::NOT){
        auto* tok = new BinaryOpToken(t->val_type, t->lexeme, t->line);
        tok->useLeft = false;
        tok->useRight = true;
        tok->depth = t->depth;
        return tok;
    }
    else {
        auto* tok = new BinaryOpToken(t->val_type, t->lexeme, t->line);
        tok->useLeft = true;
        tok->useRight = true;
        tok->depth = t->depth;
        return tok;
    }
}

std::vector<Token> parse(const TokenIterator& tokens, Scope* scope);

TokenIterator getCondition(TokenIterator& iter){
    std::vector<Token*> conditionTokens;
    int parenCount = 0;
    while (iter.hasNext()){
        Token* t = iter.next();
        if (t->val_type == TokenValue::LEFT_PAREN) {
            if (parenCount > 0) conditionTokens.push_back(t);
            parenCount++;
            continue;
        }
        if (t->val_type == TokenValue::RIGHT_PAREN){
            parenCount--;
            if (parenCount == 0)
                return TokenIterator(conditionTokens);
            else conditionTokens.push_back(t);
            continue;
        }

        conditionTokens.push_back(t);
    }
    throw std::runtime_error("Mismatched parentheses");
}

std::vector<Token*> parseFunctionArgs (TokenIterator& iter, Scope* scope){
    std::vector<Token*> tokens;
    if (iter.peek()->val_type != TokenValue::LEFT_PAREN) throw std::runtime_error("Expected left parenthesis after function name");
    iter.next();
    if (iter.peek() -> val_type == TokenValue::RIGHT_PAREN){
        iter.next();
        return tokens;
    }
    std::vector<Token*> expr;
    while (iter.hasNext()){
        Token* t = iter.next();
        if (t->val_type == RIGHT_PAREN){
            TokenIterator exprIter = TokenIterator(expr);
            tokens.push_back(parseExpression(exprIter, scope));
            break;
        }
        else if (t->val_type == COMMA){
            TokenIterator exprIter = TokenIterator(expr);
            tokens.push_back(parseExpression(exprIter, scope));
            expr.clear();
        }
        else expr.push_back(t);
    }
    return tokens;
}

Token* getInsideBrackets(TokenIterator& iter,Scope* scope){
    std::vector<Token*> bracketTokens;
    int bracketCount = 1;
    while (iter.hasNext()){
        Token* t = iter.next();
        if (t->val_type == TokenValue::LEFT_BRACKET) bracketCount++;
        else if (t->val_type == TokenValue::RIGHT_BRACKET){
            bracketCount--;
            if (bracketCount == 0){
                TokenIterator bracketIter = TokenIterator(bracketTokens);
                return parseExpression(bracketIter, scope);
            }
            else if (bracketCount < 0) throw std::runtime_error("Unexpected right bracket " + t->toString());
        }
        bracketTokens.push_back(t);
    }
}

Token* parseExpression(TokenIterator& iter, Scope* scope){
    BinaryOpToken topNode(TokenValue::NONE, "", 0);
    topNode.useRight = true;
    topNode.depth = -1;

    Token* head = &topNode;
    Token* working = head;

    int parenCount = 0;

    std::map<std::string, Token*> vars;

    while (iter.hasNext()){
        Token* tok = iter.next();
        tok->depth = parenCount;
        if (tok->type == TokenType::TYPE_OPERATOR){
            BinaryOpToken* op;
            op = makeOperator(tok);

            if (op->useLeft && working == nullptr){
                if (op->left == nullptr)
                    throw std::runtime_error("Operator " + tok->toString() + " expected value on left");
                // if expression starts with parentheses, working will be null when operator is full
                working = op;
                continue;
            }

            if (op->useLeft){
                // go left of any non-operators -> can assume working is of type operator
                while (working != nullptr &&
                       precedence(op->val_type) <= precedence(working->val_type) &&
                       op->depth <= working->depth
                       ){
                    if (working->type == TYPE_OPERATOR){
                        auto* working_op = (BinaryOpToken*) working;
                        if (working_op->useRight && working_op->right == nullptr) throw std::runtime_error("Operator " + working_op->toString() + " expected value on right");
                    }
                    working = working->parseParent;
                }

                auto* working_op = (BinaryOpToken*) working;
                if (!working_op->useRight) throw std::runtime_error("Invalid Operator Combination at " + op->toString());
                if (working_op->right == nullptr) throw std::runtime_error("Operator " + working_op->toString() + " expected value on right");
                op->left = working_op->right;
                working_op->right = op;
                op->parseParent = working_op;
                op->left->parseParent = op;

                working = op;
            }
            else if (op->useRight){
                if (working == nullptr) throw std::runtime_error("Operator " + op->toString() + " expected value on left");
                auto* working_op = (BinaryOpToken*) working;
                if (!working_op->useRight) throw std::runtime_error("Invalid Operator Combination at " + op->toString());
                if (working_op->right != nullptr) throw std::runtime_error("Operator " + working_op->toString() + " already has right value");
                working_op->right = op;
                op->parseParent = working_op;
                working = op;
            }
        }
        else if (tok->val_type == TokenValue::SEMICOLON){
            if (parenCount != 0) throw std::runtime_error("Mismatched parentheses");
            break;
        }
        else if (tok->val_type == TokenValue::LEFT_PAREN){
            parenCount++;
        }
        else if (tok->val_type == TokenValue::RIGHT_PAREN){
            parenCount--;
            if (parenCount < 0) throw std::runtime_error("Mismatched parentheses");
            if (working->parseParent == nullptr) throw std::runtime_error("Unexpected right parenthesis");
            working = working->parseParent;
        }
        else {
            Token* next = iter.peek();
            if (tok->val_type == TokenValue::IDENTIFIER && scope->find(tok->lexeme) == nullptr) throw std::runtime_error(tok->lexeme + " not defined");
            if (tok->val_type == TokenValue::IDENTIFIER && next != nullptr && next->val_type == TokenValue::LEFT_PAREN){
                Token* f = scope->find(tok->lexeme);
                if (f->type != TokenType::TYPE_KEYWORD || f->val_type != FUNCTION) throw std::runtime_error(tok->lexeme + " is not a function");
                TokenValue returnType = ((FunctionToken*) f)->returnType;
                std::vector<Token*> args = parseFunctionArgs(iter, scope);
                auto* func = new FunctionCallToken(tok, args, tok->lexeme, tok->line);
                func->returnType = returnType;
                tok = func;
            }
            else if (tok->val_type == TokenValue::IDENTIFIER && next != nullptr && next->val_type == TokenValue::LEFT_BRACKET){
                // find right bracket
                std::vector<Token*> bracketTokens;
                iter.next();
                Token* inside = getInsideBrackets(iter, scope);
                BinaryOpToken* arr = new BinaryOpToken(TokenValue::ARRAY, "[]", tok->line);
                arr->right = inside;
                arr->left = tok;
                tok = arr;
            }
            if (working->type != TokenType::TYPE_OPERATOR) throw std::runtime_error("Expected operator before " + tok->toString());
            auto* op = (BinaryOpToken*) working;
            if (!op->useRight) throw std::runtime_error("Invalid Operator Combination at " + tok->toString());
            if (op->right != nullptr) throw std::runtime_error("Operator " + op->toString() + " already has right value");
            if ((op->val_type == TokenValue::REF || op->val_type == TokenValue::DEREF) && tok->val_type != TokenValue::IDENTIFIER) throw std::runtime_error("Must use variable after ref or deref on line " + std::to_string(tok->line));
            op->right = tok;
            tok->parseParent = op;
        }
    }

    if (working->type == TokenType::TYPE_OPERATOR){
        auto* working_op = (BinaryOpToken*) working;
        if (working_op->useRight && working_op->right == nullptr) throw std::runtime_error("Operator " + working_op->toString() + " expected value on right");
    }

    return topNode.right;
}

GroupToken* parseGroup(TokenIterator& iter, Scope* scope){
    std::vector<Token*> tokens;

    Token* start = iter.peek();
    if (start->val_type != TokenValue::LEFT_BRACE) throw std::runtime_error("Expected left brace");
    iter.next();

    auto* innerScope = new Scope(scope);

    int bracketCount = 1;
    while (iter.hasNext()){
        Token* t = iter.next();
        if (t->val_type == TokenValue::LEFT_BRACE) bracketCount++;
        else if (t->val_type == TokenValue::RIGHT_BRACE){
            bracketCount--;
            if (bracketCount == 0){
                auto* group = new GroupToken(TokenType::TYPE_GROUP, "{}", t->line);
                TokenIterator innerIter = TokenIterator(tokens);
                group->expressions = parse(innerIter, innerScope);
                return group;
            }
            else if (bracketCount < 0) throw std::runtime_error("Unexpected right bracket " + t->toString());
        }
        tokens.push_back(t);
    }
    Token* last = iter.next();
    if (last->val_type == TokenValue::RIGHT_BRACE && bracketCount == 0){
        auto* group = new GroupToken(TokenType::TYPE_GROUP, "{}", last->line);
        TokenIterator innerIter = TokenIterator(tokens);
        group->expressions = parse(innerIter, innerScope);
        return group;
    }

    throw std::runtime_error("Left bracket " + start->toString() + " has no matching right bracket");
}

IfElseToken* parseIf(TokenIterator& iter, Scope* scope){
    Token* condition = nullptr;
    std::vector<GroupToken*> elseIfBodies;
    std::vector<Token*> elseIfConditions;
    GroupToken* elseBody = nullptr;
    GroupToken* ifBody = nullptr;

    while (iter.hasNext()){
        Token* t = iter.peek();
        if (!(
            (ifBody == nullptr && t->val_type == TokenValue::IF) ||
            (ifBody != nullptr && t->val_type == TokenValue::ELSE)
            )) break;
        iter.next();
        // first if statement
        if (ifBody == nullptr){
            if (t->val_type != TokenValue::IF){
                throw std::runtime_error("Expected if statement");
            }
            Token* lParen = iter.peek();
            if (lParen->val_type != TokenValue::LEFT_PAREN){
                throw std::runtime_error("Expected left parentece after if statement on line " + std::to_string(t->line));
            }
            TokenIterator conditionIter = getCondition(iter);
            condition = parseExpression(conditionIter, scope);

            if (iter.peek()->val_type != TokenValue::LEFT_BRACE){
                throw std::runtime_error("Expected left brace after if statement on line " + std::to_string(t->line));
            }
            ifBody = parseGroup(iter, scope);
            continue;
        }
            // after
        else if (t->val_type == TokenValue::ELSE){
            if (iter.peek()->val_type == TokenValue::IF){
                iter.next();
                Token* lParen = iter.peek();
                if (lParen->val_type != TokenValue::LEFT_PAREN){
                    throw std::runtime_error("Expected left parentece after if statement on line " + std::to_string(t->line));
                }
                TokenIterator conditionIter = getCondition(iter);
                elseIfConditions.push_back(parseExpression(conditionIter, scope));

                GroupToken* group = parseGroup(iter, scope);
                elseIfBodies.push_back(group);
            }
            else {
                if (iter.peek()->val_type != TokenValue::LEFT_BRACE){
                    throw std::runtime_error("Expected left brace after else statement on line " + std::to_string(t->line));
                }
                elseBody = parseGroup(iter, scope);
            }
            continue;
        }
        break;
    }

    return new IfElseToken{condition, ifBody, elseIfConditions, elseIfBodies, elseBody, 0};
}

std::vector<DefinitionToken*> parseFunctionParams(TokenIterator& iter){
    std::vector<DefinitionToken*> tokens;
    if (iter.peek()->val_type != TokenValue::LEFT_PAREN) throw std::runtime_error("Expected left parenthesis after function name");

    iter.next();

    if (iter.peek()->val_type == TokenValue::RIGHT_PAREN){
        iter.next();
        return tokens;
    }

    while (iter.hasNext()){
        // expect type
        Token* type = iter.next();
        int refCount = 0;
        while (iter.peek()->val_type == TokenValue::DEREF){
            refCount++;
            iter.next();
        }

        // expect name
        Token* name = iter.next();
        if (name->val_type != TokenValue::IDENTIFIER) throw std::runtime_error("Expected identifier in function argument " + name->toString());
        auto* def = new DefinitionToken(type->val_type, name->lexeme, name->line);
        def->refCount = refCount;
        tokens.push_back(def);

        if (iter.peek()->val_type == TokenValue::COMMA){
            iter.next();
        }
        else break;
    }
    if (iter.peek()->val_type != TokenValue::RIGHT_PAREN) throw std::runtime_error("Expected right parenthesis after function arguments");
    iter.next();
    return tokens;
}

ArrayInitializationToken* parseArrayInit(TokenIterator& iter, Scope* scope){
    std::vector<Token*> tokens;
    std::vector<Token*> expr;
    while (iter.hasNext()){
        Token* t = iter.next();

        if (t->val_type == TokenValue::LEFT_BRACE){
            expr.push_back(parseArrayInit(iter, scope));
        }
        else if (t->val_type == TokenValue::RIGHT_BRACE){
            if (!expr.empty()){
                TokenIterator exprIter = TokenIterator(expr);
                tokens.push_back(parseExpression(exprIter, scope));
                expr.clear();
            }
            return new ArrayInitializationToken(tokens, TokenValue::ARRAY, t->line);
        }
        else if (t->val_type == COMMA){
            TokenIterator exprIter = TokenIterator(expr);
            tokens.push_back(parseExpression(exprIter, scope));
            expr.clear();
        }
        else expr.push_back(t);
    }
    throw std::runtime_error("Expected right bracket");
}

Token* parseDefine(TokenIterator& iter, Scope* scope){
    Token* type = iter.next();
    if (type->type != TokenType::TYPE_TYPE) throw std::runtime_error("Expected type when defining value");

    int refCount = 0;
    while (iter.peek()->val_type == TokenValue::DEREF){
        refCount++;
        iter.next();
    }

    Token* name = iter.next();
    if (name->type != TokenType::TYPE_IDENTIFIER) throw std::runtime_error("Expected identifier when defining value");

    Token* existing = scope->find(name->lexeme);

    bool is_function = false;
    if (existing != nullptr && existing->depth == scope->getDepth()){
        if (existing->val_type == TokenValue::FUNCTION){
            FunctionToken* func = (FunctionToken*) existing;
            if (func->body != nullptr) throw std::runtime_error("\'" + name->lexeme + "\" already defined on line " + std::to_string(name->line));
            is_function = true;
        }
        else throw std::runtime_error("\'" + name->lexeme + "\" already defined on line " + std::to_string(name->line));
    }


    Token* next = iter.peek();
    // variable definition
    if (next->val_type == TokenValue::EQ){
        if (is_function) throw std::runtime_error("Function " + name->lexeme + " already defined");
        iter.next();
        auto* t = new DefinitionToken(type->val_type, name->lexeme, name->line);
        t->refCount = refCount;
        Token* expr = parseExpression(iter, scope);
        scope->add(name->lexeme, t);
        t->value = expr;

        return t;
    }
    // define an array
    else if (next->val_type == TokenValue::LEFT_BRACKET){
        std::vector<Token*> bracketTokens;
        // defining an array
        while (next->val_type == TokenValue::LEFT_BRACKET) {
            iter.next();
            Token* inside = getInsideBrackets(iter, scope);
            bracketTokens.push_back(inside);
            next = iter.peek();
        }

        ArrayInitializationToken* value = nullptr;
        if (next->val_type == TokenValue::EQ){
            // parse array initialization
            iter.next();
            iter.next();
            value = parseArrayInit(iter, scope);
            value->valueType = type->val_type;
        }

        auto* t = new DefinitionToken(type->val_type, name->lexeme, name->line);
        t->value = value;
        t->refCount = refCount;
        t->dimensions = bracketTokens;
        scope->add(name->lexeme, t);
        return t;
    }

    if (!scope->isBaseScope()) throw std::runtime_error("Cannot define function within a function");

    // else it's a function definition
    if (next->val_type != TokenValue::LEFT_PAREN) throw std::runtime_error("Expected left parenthesis after function name");
    std::vector<DefinitionToken*> params = parseFunctionParams(iter);

    if (iter.peek()->val_type == TokenValue::SEMICOLON){ // function header
        if (is_function) throw std::runtime_error("Function " + name->lexeme + " already defined");
        auto* t = new FunctionToken(type->val_type, name->lexeme, params, name->line);
        t->refCount = refCount;
        scope->add(name->lexeme, t);
        iter.next();
        return t;
    }

    if (is_function){
        // check if same function header
        auto* func = (FunctionToken*) existing;
        if (func->parameters.size() != params.size()) throw std::runtime_error("Function " + name->lexeme + " already defined with different parameters");
        Scope funcScope(scope);
        for (int i = 0; i < params.size(); i++){
            if (params[i]->valueType != func->parameters[i]->valueType) throw std::runtime_error("Function " + name->lexeme + " already defined with different parameters");
            funcScope.add(params[i]->name, params[i]);
        }
        GroupToken* body = parseGroup(iter, &funcScope);
        func->body = body;
        return func;
    }

    // expect {
    if (iter.peek()->val_type != TokenValue::LEFT_BRACE) throw std::runtime_error("Expected left brace after function definition");
    auto* func = new FunctionToken(type->val_type, name->lexeme, params, name->line);
    func->refCount = refCount;
    scope->add(name->lexeme, func);

    // create new scope
    Scope funcScope(scope);
    // add parameters to scope
    for (DefinitionToken* param : params){
        funcScope.add(param->name, param);
    }

    GroupToken* body = parseGroup(iter, &funcScope);
    func->body = body;

    return func;
}

Token* parseFor(TokenIterator& iter, Scope* scope){
    // eat for token
    if (iter.peek()->val_type != TokenValue::FOR) throw std::runtime_error("Expected for statement");
    iter.next();
    // check for left parenthesis
    if (iter.peek()->val_type != TokenValue::LEFT_PAREN) throw std::runtime_error("Expected left parenthesis after for statement");
    TokenIterator conditionIter = getCondition(iter);
    // parse first expression, parseExpression eats semicolon already
    Token* init;
    if (conditionIter.peek()->type == TokenType::TYPE_TYPE) init = parseDefine(conditionIter, scope);
    else if (conditionIter.peek()->val_type == TokenValue::SEMICOLON) {
        init = nullptr;
        conditionIter.next();
    }
    else init = parseExpression(conditionIter, scope);

    Token* condition;
    if (conditionIter.peek()->val_type == TokenValue::SEMICOLON){
        condition = nullptr;
        conditionIter.next();
    }
    else condition = parseExpression(conditionIter, scope);

    Token* incr;
    if (!conditionIter.hasNext() || conditionIter.peek()->val_type == TokenValue::RIGHT_PAREN) incr = nullptr;
    else incr = parseExpression(conditionIter, scope);

    if (conditionIter.hasNext()) throw std::runtime_error("Too many expressions in for statement");
    // check for right parenthesis
    GroupToken* body = parseGroup(iter, scope);
    return new ForToken(init, condition, incr, body, 0);
}

Token* parseWhile(TokenIterator& iter, Scope* scope){
    // eat while token
    if (iter.peek()->val_type != TokenValue::WHILE) throw std::runtime_error("Expected while statement");
    iter.next();
    // check for left parenthesis
    if (iter.peek()->val_type != TokenValue::LEFT_PAREN) throw std::runtime_error("Expected left parenthesis after while statement");
    TokenIterator conditionIter = getCondition(iter);
    // parse first expression, parseExpression eats semicolon already
    Token* condition = parseExpression(conditionIter, scope);
    if (conditionIter.hasNext()) throw std::runtime_error("Too many expressions in while statement");
    // check for right parenthesis
    GroupToken* body = parseGroup(iter, scope);
    return new WhileToken(condition, body, 0);
}

AsmToken* parseAsm(TokenIterator& iter){
    if (iter.peek()->val_type != TokenValue::ASM) throw std::runtime_error("Expected asm statement");
    int line = iter.peek()->line;
    iter.next();
    if (iter.peek()->val_type != TokenValue::LEFT_PAREN) throw std::runtime_error("Expected left parenthesis after asm statement");
    TokenIterator asmIter = getCondition(iter);
    std::string asmStr;
    while (asmIter.hasNext()){
        Token* t = asmIter.next();
        if (t->val_type == TokenValue::RIGHT_PAREN) break;
        else if (t->val_type == TokenValue::STRING) asmStr += t->lexeme + "\n";
        else throw std::runtime_error("Unexpected token " + t->toString());
    }
    return new AsmToken(asmStr, line);
}

Token* parseInlineFunction(TokenIterator& iter, Scope* scope){
    Token* inline_keyword = iter.next();
    if (inline_keyword->val_type != TokenValue::INLINE) throw std::runtime_error("Expected inline keyword");
    Token* type = iter.peek();
    if (type->type != TokenType::TYPE_TYPE) throw std::runtime_error("Expected type when defining value");

    Token* t = parseDefine(iter, scope);

    if (t->val_type != TokenValue::FUNCTION) throw std::runtime_error("inline expects a function");
    FunctionToken* func = (FunctionToken*) t;
    func->is_inline = true;
    return func;
}

std::vector<Token*> parse(TokenIterator& tokens, Scope* scope){
    std::vector<Token*> output;
    std::map<std::string, Token*> identifiers;

    while (tokens.hasNext()){
        Token* t = tokens.peek();
        t->track = 2;
        // if, else, for, while -> parse special
        // identifier, data type, literal -> parse expression
        // left brace -> parse group
        // other -> throw error
        if (t->type == TokenType::TYPE_KEYWORD){
            if (t->val_type == TokenValue::IF){
                output.push_back(parseIf(tokens, scope));
            }
            else if (t->val_type == TokenValue::FOR){
                output.push_back(parseFor(tokens, scope));
            }
            else if (t->val_type == TokenValue::WHILE){
                output.push_back(parseWhile(tokens, scope));
            }
            else if (t->val_type == TokenValue::RETURN){
                if (scope->isBaseScope()) throw std::runtime_error("Return statement outside of function");
                tokens.next();
                Token* value = parseExpression(tokens, scope);
                auto* ret = new ReturnToken(value, t->line);
                output.push_back(ret);
            }
            else if (t->val_type == TokenValue::BREAK){
                output.push_back(tokens.next());
                if (tokens.peek()->val_type != TokenValue::SEMICOLON) throw std::runtime_error("Expected semicolon after break statement");
                tokens.next();
            }
            else if (t->val_type == TokenValue::CONTINUE){
                output.push_back(tokens.next());
                if (tokens.peek()->val_type != TokenValue::SEMICOLON) throw std::runtime_error("Expected semicolon after continue statement");
                tokens.next();
            }
            else if (t->val_type == TokenValue::ASM){
                output.push_back(parseAsm(tokens));
                tokens.next();
            }
            else if (t->val_type == TokenValue::INLINE){
                output.push_back(parseInlineFunction(tokens, scope));
            }
            else {
                // continue or return -> error
                throw std::runtime_error("(parse keyword) Unexpected token " + t->toString());
            }
        }
        else if (t->type == TokenType::TYPE_IDENTIFIER || t->type == TokenType::TYPE_VALUE || t->val_type == TokenValue::LEFT_PAREN ||
                (t->type == TokenType::TYPE_OPERATOR &&
                    (t->val_type == TokenValue::NEG || t->val_type == TokenValue::DEREF || t->val_type == TokenValue::REF || t->val_type == TokenValue::NOT)
                )){
            if (t->type == TYPE_IDENTIFIER){
                // store in identifiers

                if (identifiers.find(t->lexeme) != identifiers.end()){
                    identifiers[t->lexeme]->track = 1;
                    identifiers[t->lexeme] = t;
                }
                else {
                    identifiers[t->lexeme] = t;
                }
            }
            // parse expression
            output.push_back(parseExpression(tokens, scope));
        }
        else if (t->type == TokenType::TYPE_TYPE){
            // parse definition
            output.push_back(parseDefine(tokens, scope));
        }
        else if (t->val_type == TokenValue::LEFT_BRACE){
            // parse group
            output.push_back(parseGroup(tokens, scope));
        }
        else if (t->val_type == TokenValue::SEMICOLON){
            tokens.next();
            continue;
        }
        else {
            throw std::runtime_error("Unexpected token " + t->toString());
        }

    }
    return output;
}


