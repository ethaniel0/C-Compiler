//
// Created by Ethan Horowitz on 3/31/24.
//

#include "MipsAssembler.h"

int getInput(const std::string& inp, VariableTracker* tracker){
    if (inp.empty()) throw std::runtime_error("empty input");
    if (inp.at(0) == '$'){
        std::string sub = inp.substr(1);
        if (sub == "zero") return 0;
        if (sub == "at") return 1;
        if (sub == "v0") return 2;
        if (sub == "v1") return 3;
        if (sub == "a0") return 4;
        if (sub == "a1") return 5;
        if (sub == "a2") return 6;
        if (sub == "a3") return 7;
        if (sub == "sp") return 29;
        if (sub == "return"){
            if (!tracker->in_inline()){
                return 2;
            }
            return tracker->getReg("return");
        }
        int reg = std::stoi(sub);
        return reg;
    }
    if (inp.at(0) == '('){
        std::string sub = inp.substr(1, inp.size() - 2);
        if (!tracker->var_exists(sub)) throw std::runtime_error("variable " + sub + " does not exist");
        return tracker->getReg(sub, false);
    }
}

struct MipsTokenizer{
    std::string code;
    int index;
};

std::string next_token(MipsTokenizer* tokenizer){
    if (tokenizer->index >= tokenizer->code.size()) return "";
    std::string token = "";
    for(; tokenizer->index < tokenizer->code.size(); tokenizer->index++){
        char c = tokenizer->code.at(tokenizer->index);
        if (c == ' ' || c == '\n' || c == '\t' || c == ','){
            if (!token.empty()) return token;
        } else {
            token += c;
        }
    }
    return token;
}

struct RInfo{
    int rs;
    int rt;
    int rd;
};

RInfo getRInfo(MipsTokenizer* tokenizer, VariableTracker* tracker){
    RInfo info{};
    std::string rs = next_token(tokenizer);
    std::string rt = next_token(tokenizer);
    std::string rd = next_token(tokenizer);
    info.rs = getInput(rs, tracker);
    info.rt = getInput(rt, tracker);
    info.rd = getInput(rd, tracker);
    return info;
}

struct MemInfo{
    int rd;
    int rs;
    int offset;
};

MemInfo getMemInfo(MipsTokenizer* tokenizer, VariableTracker* tracker){
    MemInfo info{};
    std::string rd = next_token(tokenizer);
    std::string offset_and_reg = next_token(tokenizer);

    int i = 0;
    while (i < offset_and_reg.length() && offset_and_reg[i] != '(') i++;
    if (i == offset_and_reg.length()) throw std::runtime_error("invalid memory access " + offset_and_reg);
    std::string offset = offset_and_reg.substr(0, i);
    std::string rs = offset_and_reg.substr(i + 1, offset_and_reg.length() - i - 2);
    info.rd = getInput(rd, tracker);
    info.rs = getInput(rs, tracker);
    info.offset = std::stoi(offset);
    return info;
}

void assembleMips(const std::string& mips, MipsBuilder* builder, VariableTracker* tracker){
    MipsTokenizer tokenizer;
    tokenizer.code = mips;
    tokenizer.index = 0;

    std::string token = next_token(&tokenizer);
    while (!token.empty()){
        std::string label = "";
        if (token[token.length() - 1] == ':'){
            label = token.substr(0, token.length() - 1);
            token = next_token(&tokenizer);
        }

        if (token == "add"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrAdd(info.rs, info.rt, info.rd), label);
        }
        else if (token == "addi"){
            std::string rt = next_token(&tokenizer);
            std::string rs = next_token(&tokenizer);
            std::string imm = next_token(&tokenizer);
            builder->addInstruction(new InstrAddi(getInput(rt, tracker), getInput(rs, tracker), std::stoi(imm)), label);
        }
        else if (token == "sub"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrSub(info.rs, info.rt, info.rd), label);
        }
        else if (token == "mul"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrMul(info.rs, info.rt, info.rd), label);
        }
        else if (token == "hmul"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrHMul(info.rs, info.rt, info.rd), label);
        }
        else if (token == "div"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrDiv(info.rs, info.rt, info.rd), label);
        }
        else if (token == "and"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrAnd(info.rs, info.rt, info.rd), label);
        }
        else if (token == "or"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrOr(info.rs, info.rt, info.rd), label);
        }
        else if (token == "sll"){
            RInfo info = getRInfo(&tokenizer, tracker);
            std::string shamt = next_token(&tokenizer);
            builder->addInstruction(new InstrSll(info.rs, info.rt, std::stoi(shamt)), label);
        }
        else if (token == "sra"){
            std::string rs = next_token(&tokenizer);
            std::string rt = next_token(&tokenizer);
            std::string num = next_token(&tokenizer);
            int shamt = std::stoi(num);
            builder->addInstruction(new InstrSra(getInput(rs, tracker), getInput(rt, tracker), shamt), label);
        }
        else if (token == "slt"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrSlt(info.rd, info.rs, info.rt), label);
        }
        else if (token == "sgt"){
            RInfo info = getRInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrSgt(info.rd, info.rs, info.rt), label);
        }

        else if (token == "lw"){
            MemInfo info = getMemInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrLw(info.rd, info.rs, info.offset), label);
        }
        else if (token == "sw"){
            MemInfo info = getMemInfo(&tokenizer, tracker);
            builder->addInstruction(new InstrSw(info.rd, info.rs, info.offset), label);
        }

        else if (token == "j"){
            std::string to = next_token(&tokenizer);
            builder->addInstruction(new InstrJ(to), label);
        }
        else if (token == "bne"){
            std::string rs = next_token(&tokenizer);
            std::string rt = next_token(&tokenizer);
            std::string to = next_token(&tokenizer);
            builder->addInstruction(new InstrBne(getInput(rs, tracker), getInput(rt, tracker), to), label);
        }
        else if (token == "jal"){
            std::string to = next_token(&tokenizer);
            builder->addInstruction(new InstrJal(to), label);
        }
        else if (token == "jr"){
            std::string rs = next_token(&tokenizer);
            builder->addInstruction(new InstrJr(getInput(rs, tracker)), label);
        }
        else if (token == "blt"){
            std::string rs = next_token(&tokenizer);
            std::string rt = next_token(&tokenizer);
            std::string to = next_token(&tokenizer);
            builder->addInstruction(new InstrBlt(getInput(rs, tracker), getInput(rt, tracker), to), label);
        }

        else if (token == "bex"){
            std::string to = next_token(&tokenizer);
            builder->addInstruction(new InstrBex(to), label);
        }
        else if (token == "setx"){
            std::string imm = next_token(&tokenizer);
            builder->addInstruction(new InstrSetx(std::stoi(imm)), label);
        }

        else if (token == "print"){
            std::string rs = next_token(&tokenizer);
            builder->addInstruction(new InstrTestLog(getInput(rs, tracker)), label);
        }

        else{
            throw std::runtime_error("unknown instruction " + token);
        }

        token = next_token(&tokenizer);
    }
}