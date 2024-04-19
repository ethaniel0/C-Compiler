//
// Created by Ethan Horowitz on 3/15/24.
//

#include "MipsCompiler.h"

#ifndef SP
#define SP 29
#endif

#ifndef HP
#define HP 28
#endif

#ifndef RSTATUS
#define RSTATUS 30
#endif

void compile_array_init(ArrayInitializationToken* token, int* mem, bool on_stack, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    for (int i = 0; i < token->values.size(); i++){
        Token* t = token->values[i];
        if (t->val_type == ARRAY){
            // compile array
            compile_array_init((ArrayInitializationToken*) t, mem, on_stack, mipsBuilder, varTracker);
        }
        else {
            std::string value = compile_op("", t, mipsBuilder, varTracker);
            uint8_t reg = varTracker->getReg(value);
            if (on_stack){
                mipsBuilder->addInstruction(new InstrSw(reg, SP, *mem), "");
                *mem -= 1;
            }
            else {
                mipsBuilder->addInstruction(new InstrSw(reg, 0, *mem), "");
                *mem += 1;
            }

        }
    }
}

std::string compile_value_def(Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* def = (DefinitionToken*) token;

    if(!def->dimensions.empty()){
        // is an array
        // length is stored in $1 and the length recorded below
        int length = 1;
        for (Token* i : def->dimensions){
            if (i->type == TYPE_VALUE && i->val_type == TokenValue::NUMBER_INT){
                length *= std::stoi(i->lexeme);
            }
            else {
                throw std::runtime_error("Array length must be a number (known at compile time) at " + i->toString());
            }
        }

        // allocate memory
        int mem = varTracker->set_array(def->name, length);
        bool on_stack = mem > 0;
        if (mem <= 0) mem = -mem;
        else mem -= 1;

        if (def->value != nullptr){
            auto* init = (ArrayInitializationToken*) def->value;
            int cur_mem_addr = mem;
            compile_array_init(init, &cur_mem_addr, on_stack, mipsBuilder, varTracker);
        }

        // get register, save mem location
        uint8_t reg = varTracker->getReg(def->name);
        if (on_stack){
            mipsBuilder->addInstruction(new InstrAddi(reg, SP, mem), "");
        }
        else {
            mipsBuilder->addInstruction(new InstrAddi(reg, 0, mem), "");
        }

        varTracker->set_var_type(def->name, def->valueType);
        varTracker->set_var_type_refs(def->name, 1);

        return def->name;
    }

    std::string value = compile_op("", def->value, mipsBuilder, varTracker);


    if (def->value->type != TYPE_IDENTIFIER ){
        varTracker->renameVar(value, def->name);
    }
    else {
        uint8_t reg = varTracker->getReg(def->name);

        varTracker->set_var_type(def->name, def->valueType);
        varTracker->set_var_type_refs(def->name, def->refCount);

        std::string val = force_type(def->name, value, varTracker, mipsBuilder);
        uint8_t val_reg = varTracker->getReg(val);
        mipsBuilder->addInstruction(new InstrAdd(reg, 0, val_reg), "");
    }


    return def->name;
}

void compile_jump_condition(const std::string& break_to, Token* condition, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (condition->val_type == LT || condition->val_type == LTE ||
        condition->val_type == GT || condition->val_type == GTE ||
        condition->val_type == EQ_EQ || condition->val_type == NOT_EQ ||
        condition->val_type == NOT){
        compile_op(break_to, condition, mipsBuilder, varTracker);
    }
    else {
        std::string v = compile_op("", condition, mipsBuilder, varTracker);
        uint8_t reg = varTracker->getReg(v);
        mipsBuilder->addInstruction(new InstrBne(reg, 0, break_to), "");
    }
}

void compile_if(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* if_statement = (IfElseToken*) token;
    Token* condition = if_statement->condition;
    std::string label_true = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();
    compile_jump_condition(label_true, condition, mipsBuilder, varTracker);
    // if elses
    std::vector<std::string> elseIfLabels;
    if (!if_statement->elseIfConditions.empty()){
        for (Token* c : if_statement->elseIfConditions){
            std::string label = mipsBuilder->genUnnamedLabel();
            elseIfLabels.push_back(label);
            compile_jump_condition(label, c, mipsBuilder, varTracker);
        }
    }
    // write else, then jump
    if (if_statement->elseBody != nullptr){
        compile_instructions(breakScope, if_statement->elseBody->expressions, mipsBuilder, varTracker);
    }
    mipsBuilder->addInstruction(new InstrJ(label_end), "");

    // write if elses
    for (int i = 0; i < if_statement->elseIfBodies.size(); i++){
        // noop as start to label
        mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), elseIfLabels[i]);
        compile_instructions(breakScope, if_statement->elseIfBodies[i]->expressions, mipsBuilder, varTracker);
        mipsBuilder->addInstruction(new InstrJ(label_end), "");
    }

    // write if
    // noop as start
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_true);
    compile_instructions(breakScope, if_statement->ifBody->expressions, mipsBuilder, varTracker);

    // noop at end as end label
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end);
}

void compile_break(Token* token, BreakScope* breakScope, MipsBuilder* mipsBuilder){
    if (breakScope->breakLabel.empty()) throw std::runtime_error("Break outside of loop at " + token->toString());
    mipsBuilder->addInstruction(new InstrJ(breakScope->breakLabel), "");
}
void compile_continue(Token* token, BreakScope* breakScope, MipsBuilder* mipsBuilder){
    if (breakScope->continueLabel.empty()) throw std::runtime_error("Continue outside of loop at " + token->toString());
    mipsBuilder->addInstruction(new InstrJ(breakScope->continueLabel), "");
}
void continue_return(Token* token, BreakScope* breakScope, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (breakScope->returnLabel.empty()) throw std::runtime_error("Return outside of function at " + token->toString());
    // get value
    auto* ret = (ReturnToken*) token;
    if (ret->value != nullptr){
        std::string value = compile_op("", ret->value, mipsBuilder, varTracker);
        uint8_t reg = varTracker->getReg(value);
        mipsBuilder->addInstruction(new InstrAdd(2, 0, reg), "");
    }
    mipsBuilder->addInstruction(new InstrJ(breakScope->returnLabel), "");
}

void compile_for(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* for_statement = (ForToken*) token;
    std::string label_loop_condition = mipsBuilder->genUnnamedLabel();
    std::string label_loop = mipsBuilder->genUnnamedLabel();
    std::string label_incr = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();

    /*
     Structure:
     [init]
     loopCondition:
        [condition]
        j loopEnd
     loop:
        [loop]
        [increment]
        j loopCondition
     loopEnd: noop
     */

    // init
    if (for_statement->init != nullptr){
        compile_expr(breakScope, for_statement->init, mipsBuilder, varTracker);
    }
    // loop condition
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_loop_condition);
    if (for_statement->condition != nullptr){
        compile_jump_condition(label_loop, for_statement->condition, mipsBuilder, varTracker);
        mipsBuilder->addInstruction(new InstrJ(label_end), "");
    }

    // configure break scope
    std::string prev_break = breakScope->breakLabel;
    std::string prev_continue = breakScope->continueLabel;

    breakScope->breakLabel = label_end;
    breakScope->continueLabel = label_incr;

    // loop body
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_loop);
    if (for_statement->body != nullptr){
        compile_instructions(breakScope, for_statement->body->expressions, mipsBuilder, varTracker);
    }
    // increment
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_incr);
    if (for_statement->increment != nullptr){
        compile_expr(breakScope, for_statement->increment, mipsBuilder, varTracker);
    }
    // jump back to condition and label end
    mipsBuilder->addInstruction(new InstrJ(label_loop_condition), "");
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end);

    // reset break and continue
    breakScope->breakLabel = prev_break;
    breakScope->continueLabel = prev_continue;
}

void compile_while(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    auto* while_statement = (WhileToken*) token;
    std::string label_loop_condition = mipsBuilder->genUnnamedLabel();
    std::string label_loop = mipsBuilder->genUnnamedLabel();
    std::string label_end = mipsBuilder->genUnnamedLabel();

    /*
     Structure:
     loopCondition:
        [condition]
        j loopEnd
     loop:
        [loop]
        j loopCondition
     loopEnd: noop
     */

    // loop condition
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_loop_condition);
    compile_jump_condition(label_loop, while_statement->condition, mipsBuilder, varTracker);
    mipsBuilder->addInstruction(new InstrJ(label_end), "");

    // configure break scope
    std::string prev_break = breakScope->breakLabel;
    std::string prev_continue = breakScope->continueLabel;

    breakScope->breakLabel = label_end;
    breakScope->continueLabel = label_loop_condition;

    // loop body
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_loop);
    if (while_statement->body != nullptr){
        compile_instructions(breakScope, while_statement->body->expressions, mipsBuilder, varTracker);
    }
    // jump back to condition and label end
    mipsBuilder->addInstruction(new InstrJ(label_loop_condition), "");
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), label_end);

    // reset break and continue
    breakScope->breakLabel = prev_break;
    breakScope->continueLabel = prev_continue;
}

void compile_function(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // $29 is the stack pointer
    // $31 is the return address
    auto* function = (FunctionToken*) token;

    if (function->is_inline){
        varTracker->add_inline_function(function->name, function);
        return;
    }

    if (function->body == nullptr) return;

    // skip over compile function definition so it's only run when called
    std::string after_function = mipsBuilder->genUnnamedLabel();
    std::string just_jump = mipsBuilder->genUnnamedLabel();
    mipsBuilder->addInstruction(new InstrJ(after_function), "");

    std::string name = function->name;
    /*
        Structure:
        1. save stack pointer in stack
        2. save duplicated registers that are in use
        3. run code
        4. restore duplicated registers
        5. restore stack pointer
        6. return
     */
    varTracker->incScope();
    // save ra
    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), name);

    // load first four arguments into registers
    for (int i = 0; i < function->parameters.size() && i < 4; i++){
        varTracker->add_variable(function->parameters[i]->name, 4+i);
        varTracker->set_var_type_refs(function->parameters[i]->name, function->parameters[i]->refCount);
    }

    // if more arguments, load them into stack pointer
    if (function->parameters.size() > 4){
        // load arguments into stack pointer
        for (int i = 4; i < function->parameters.size(); i++){
            uint8_t reg = varTracker->getReg(function->parameters[i]->name);
            mipsBuilder->addInstruction(new InstrLw(reg, SP, i - 4), "");
        }
    }

    // adjust breakscope
    std::string func_end = mipsBuilder->genUnnamedLabel();
    std::string prev_return = breakScope->returnLabel;
    breakScope->returnLabel = just_jump;

    // run code
    compile_instructions(breakScope, function->body->expressions, mipsBuilder, varTracker);

    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), just_jump);
    varTracker->decScope();
    // jd $ra
    mipsBuilder->addInstruction(new InstrJr(31), "");

    mipsBuilder->addInstruction(new InstrAdd(0, 0, 0), after_function);
}

std::string compile_expr(BreakScope* breakScope, Token* token, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    if (token == nullptr) return "";
    if (token->type == TokenType::TYPE_OPERATOR && token->val_type == TokenValue::IDENTIFIER){
        return compile_value_def(token, mipsBuilder, varTracker);
    }
    else if (token->type == TokenType::TYPE_OPERATOR){
        return compile_op("", token, mipsBuilder, varTracker);
    }
    else if (token->type == TokenType::TYPE_KEYWORD){
        if (token->val_type == TokenValue::IF) compile_if(breakScope, token, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::FOR) compile_for(breakScope, token, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::WHILE) compile_while(breakScope, token, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::BREAK) compile_break(token, breakScope, mipsBuilder);
        else if (token->val_type == TokenValue::CONTINUE) compile_continue(token, breakScope, mipsBuilder);
        else if (token->val_type == TokenValue::RETURN) continue_return(token, breakScope, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::FUNCTION) compile_function(breakScope, token, mipsBuilder, varTracker);
        else if (token->val_type == TokenValue::ASM) {
            auto* asmToken = (AsmToken*) token;
            assembleMips(asmToken->asmCode, mipsBuilder, varTracker);
        }
    }
    return "";
}

void compile_instructions(BreakScope* breakScope, const std::vector<Token*>& tokens, MipsBuilder* mipsBuilder, VariableTracker* varTracker){
    // tokens are the heads of parse trees - parse each one individually, then append them together
    // make main function first, so the first instruction starts there
    for (Token* token : tokens){
        compile_expr(breakScope, token, mipsBuilder, varTracker);
    }
}

bool sort_ast(std::vector<Token*>* tokens, Scope* scope){
    std::vector<Token*> functions;
    bool has_main = false;
    for (int i = 0; i < tokens->size(); i++){
        if ((*tokens)[i]->type == TokenType::TYPE_KEYWORD && (*tokens)[i]->val_type == TokenValue::FUNCTION){
            auto* f = (FunctionToken*) (*tokens)[i];

            if (f->is_inline){
                continue;
            }

            if (f->name == "main"){
                has_main = true;
            }

            functions.push_back((*tokens)[i]);
            // remove token
            tokens->erase(tokens->begin() + i);
            i--;
        }
    }

    if (has_main){
        // insert a main() call before function calls
        std::string call_main = "main();";
        std::vector<Token*> main_ptrs = tokenize(call_main);
        TokenIterator main_iter(main_ptrs);
        std::vector<Token*> main_ast = parse(main_iter, scope);
        tokens->push_back(main_ast[0]);
    }

    for (Token* function : functions) {
        tokens->push_back(function);
    }
    return has_main;
}