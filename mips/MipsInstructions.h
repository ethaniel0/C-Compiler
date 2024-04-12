//
// Created by Ethan Horowitz on 3/15/24.
//

#ifndef I2C2_MIPSINSTRUCTIONS_H
#define I2C2_MIPSINSTRUCTIONS_H

#include <utility>
#include <bitset>
#include <stdexcept>
#include <string>

#include "MipsRunner.h"

#define RSTATUS 30

// ALU

class InstrAdd : public Instruction{
private:
    uint8_t rd, rs, rt;
public:
    InstrAdd(uint8_t rd, uint8_t rs, uint8_t rt) : Instruction(I_ADD){
        this->rs = rs;
        this->rt = rt;
        this->rd = rd;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        int32_t a = regfile->get(rs);
        int32_t b = regfile->get(rt);
        int32_t result = a + b;
        regfile->set(rd, result);
        // check overflow
        if((a > 0 && b > 0 && result < 0) || (a < 0 && b < 0 && result > 0)) {
            regfile->set(RSTATUS, 1);
        }

    }
    bool is_noop(){
        return rd == 0 && rs == 0 && rt == 0;
    }
    std::string export_str() override{
        return "add $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", $" + std::to_string(rt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t rt = this->rt & 0b11111;
        uint8_t shamt = 0;
        uint8_t aluop = 0;
        return opcode << 27 | rd << 22 | rs << 17 | rt << 12 | shamt << 7 | aluop << 2;
    }
};

class InstrAddi : public Instruction{
private:
    uint8_t rd, rs;
    int imm;
public:
    InstrAddi(uint8_t rd, uint8_t rs, int imm): Instruction(I_ADDI){
        this->rd = rd;
        this->rs = rs;
        this->imm = imm;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        int32_t a = regfile->get(rs);
        int32_t result = a + imm;
        regfile->set(rd, result);
        // check overflow
        if((a > 0 && imm > 0 && result < 0) || (a < 0 && imm < 0 && result > 0)) {
            regfile->set(RSTATUS, 2);
        }
    }
    std::string export_str() override{
        return "addi $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", " + std::to_string(imm);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b00101;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        return opcode << 27 | rd << 22 | rs << 17 | (imm & 0b11111111111111111);
    }

};

class InstrSub : public Instruction{
private:
    uint8_t rd, rs, rt;
public:
    InstrSub(uint8_t rd, uint8_t rs, uint8_t rt): Instruction(I_SUB){
        this->rd = rd;
        this->rs = rs;
        this->rt = rt;

    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        int32_t a = regfile->get(rs);
        int32_t b = regfile->get(rt);
        int32_t result = a - b;
        regfile->set(rd, result);
        // check overflow
        if((a > 0 && b < 0 && result < 0) || (a < 0 && b > 0 && result > 0)) {
            regfile->set(RSTATUS, 3);
        }
    }
    std::string export_str() override{
        return "sub $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", $" + std::to_string(rt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t rt = this->rt & 0b11111;
        uint8_t shamt = 0;
        uint8_t aluop = 1;
        return opcode << 27 | rd << 22 | rs << 17 | rt << 12 | shamt << 7 | aluop << 2;
    }
};

class InstrAnd : public Instruction{
private:
    uint8_t rd, rs, rt;
public:
    InstrAnd(uint8_t rd, uint8_t rs, uint8_t rt): Instruction(I_AND){
        this->rs = rs;
        this->rt = rt;
        this->rd = rd;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        regfile->set(rd, regfile->get(rs) & regfile->get(rt));
    }
    std::string export_str() override{
        return "and $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", $" + std::to_string(rt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t rt = this->rt & 0b11111;
        uint8_t shamt = 0;
        uint8_t aluop = 2;
        return opcode << 27 | rd << 22 | rs << 17 | rt << 12 | shamt << 7 | aluop << 2;
    }
};

class InstrOr : public Instruction{
private:
    uint8_t rd, rs, rt;
public:
    InstrOr(uint8_t rd, uint8_t rs, uint8_t rt): Instruction(I_OR){
        this->rs = rs;
        this->rt = rt;
        this->rd = rd;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        regfile->set(rd, regfile->get(rs) | regfile->get(rt));
    }
    std::string export_str() override{
        return "or $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", $" + std::to_string(rt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t rt = this->rt & 0b11111;
        uint8_t shamt = 0;
        uint8_t aluop = 3;
        return opcode << 27 | rd << 22 | rs << 17 | rt << 12 | shamt << 7 | aluop << 2;
    }
};

class InstrSll : public Instruction{
private:
    uint8_t rd, rs;
    uint8_t shamt;
public:
    InstrSll(uint8_t rd, uint8_t rs, uint8_t shamt): Instruction(I_SLL){
        this->rs = rs;
        this->rd = rd;
        this->shamt = shamt;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        regfile->set(rd, regfile->get(rs) << shamt);
    }
    std::string export_str() override{
        return "sll $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", " + std::to_string(shamt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t aluop = 0b00100;
        return opcode << 27 | rd << 22 | rs << 17 | 0 << 12 | (shamt & 0b11111) << 7 | aluop << 2;
    }
};

class InstrSra : public Instruction{
private:
    uint8_t rd, rs;
    uint8_t shamt;
public:
    InstrSra(uint8_t rd, uint8_t rs, uint8_t shamt): Instruction(I_SRA){
        this->rs = rs;
        this->rd = rd;
        this->shamt = shamt;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        regfile->set(rd, (int32_t)regfile->get(rs) >> shamt);
    }
    std::string export_str() override{
        return "sra $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", " + std::to_string(shamt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t aluop = 0b00101;
        return opcode << 27 | rd << 22 | rs << 17 | 0 << 12 | (shamt & 0b11111) << 7 | aluop << 2;
    }
};

class InstrMul : public Instruction{
private:
    uint8_t rd, rs, rt;
public:
    InstrMul(uint8_t rd, uint8_t rs, uint8_t rt): Instruction(I_MUL){
        this->rs = rs;
        this->rt = rt;
        this->rd = rd;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        int32_t a = regfile->get(rs);
        int32_t b = regfile->get(rt);
        int32_t result = a * b;
        regfile->set(rd, result);
        // check overflow
        if((a > 0 && b > 0 && result < 0) || (a < 0 && b < 0 && result < 0) || (a > 0 && b < 0 && result > 0) || (a < 0 && b > 0 && result > 0)) {
            regfile->set(RSTATUS, 4);
        }
    }
    std::string export_str() override{
        return "mul $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", $" + std::to_string(rt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t rt = this->rt & 0b11111;
        uint8_t aluop = 0b00110;
        return opcode << 27 | rd << 22 | rs << 17 | rt << 12 | 0 << 7 | aluop << 2;
    }

};

class InstrDiv : public Instruction{
private:
    uint8_t rd, rs, rt;
public:
    InstrDiv(uint8_t rd, uint8_t rs, uint8_t rt): Instruction(I_DIV){
        this->rs = rs;
        this->rt = rt;
        this->rd = rd;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        int32_t a = regfile->get(rs);
        int32_t b = regfile->get(rt);
        int32_t result = a / b;
        regfile->set(rd, result);
        // check overflow
        if(b == 0) {
            regfile->set(RSTATUS, 5);
        }
    }
    std::string export_str() override{
        return "div $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", $" + std::to_string(rt);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        uint8_t rt = this->rt & 0b11111;
        uint8_t aluop = 0b00111;
        return opcode << 27 | rd << 22 | rs << 17 | rt << 12 | 0 << 7 | aluop << 2;
    }

};

// MEMORY

class InstrSw : public Instruction{
private:
    uint8_t rd, rs;
    int16_t imm;
public:
    InstrSw(uint8_t rd, uint8_t rs, int16_t imm): Instruction(I_SW){
        this->rd = rd;
        this->rs = rs;
        this->imm = imm;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        uint32_t addr = regfile->get(rs) + imm;
        if (addr < 4096) {
            dmem[addr] = regfile->get(rd);
        }
        else {
            bool writing_mode = addr >= 12288;
            int pin = (addr & 0b11111);
            uint32_t mode = regfile->get(rd);
            std::string mode_str = writing_mode ? mode ? "OUTPUT" : "INPUT" : mode ? "HIGH" : "LOW";
            printf("Writing pin %d %s to %s\n", pin, writing_mode ? "mode" : "value", mode_str.c_str());
        }
    }
    std::string export_str() override{
        return "sw $" + std::to_string(rd) + ", " + std::to_string(imm) + "($" + std::to_string(rs) + ")";
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b00111;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        return opcode << 27 | rd << 22 | rs << 17 | (imm & 0b11111111111111111);
    }
};

class InstrLw : public Instruction{
private:
    uint8_t rd, rs;
    int16_t imm;
public:
    InstrLw(uint8_t rd, uint8_t rs, int16_t imm): Instruction(I_LW){
        this->rd = rd;
        this->rs = rs;
        this->imm = imm;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        uint32_t addr = regfile->get(rs) + imm;
        if (addr < 4096) {
            regfile->set(rd, dmem[addr]);
        }
        else {
            int pin = (addr & 0b111111);
            printf("Reading pin %d\n", pin);
        }
    }
    std::string export_str() override{
        return "lw $" + std::to_string(rd) + ", " + std::to_string(imm) + "($" + std::to_string(rs) + ")";
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b01000;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        return opcode << 27 | rd << 22 | rs << 17 | (imm & 0b11111111111111111);
    }
};

// JUMPS

class InstrJ : public Instruction{
private:
    uint32_t target;
public:
    std::string label;
    explicit InstrJ(std::string label): Instruction(I_J){
        this->label = std::move(label);
        this->target = 0;
    }
    void link_labels(std::map<std::string, Instruction*> label_map) override{
        Instruction* i = label_map[label];
        if (i == nullptr) throw std::runtime_error("Label " + label + " not found");
        target = i->line_num;
    }
    bool replace_target(std::string old_target, std::string new_target) override{
        if (label == old_target){
            label = new_target;
            return true;
        }
        return false;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        *pc = target;
    }
    std::string export_str() override{
        return "j " + label;
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b00001;
        return opcode << 27 | (target & 0x7FFFFFF);
    }
};

class InstrBne : public Instruction{
private:
    uint8_t rd, rs;
    int16_t imm;
    std::string label;
public:

    InstrBne(uint8_t rd, uint8_t rs, std::string label): Instruction(I_BNE){
        this->rd = rd;
        this->rs = rs;
        this->label = std::move(label);
        this->imm = 0;
    }
    void link_labels(std::map<std::string, Instruction*> label_map) override{
        Instruction* i = label_map[label];
        if (i == nullptr) throw std::runtime_error("Label " + label + " not found");
        imm = i->line_num - line_num - 1;
    }
    bool replace_target(std::string old_target, std::string new_target) override{
        if (label == old_target){
            label = new_target;
            return true;
        }
        return false;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        int32_t a = regfile->get(rd);
        int32_t b = regfile->get(rs);
        if(a != b){
            *pc += imm;
        }
    }
    std::string export_str() override{
        return "bne $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", " + label;
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b00010;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        return opcode << 27 | rd << 22 | rs << 17 | (imm & 0b11111111111111111);
    }
};

class InstrJr : public Instruction{
private:
    uint8_t rd;
public:
    explicit InstrJr(uint8_t rd): Instruction(I_JR){
        this->rd = rd;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        *pc = (uint32_t)regfile->get(rd);
    }
    std::string export_str() override{
        return "jr $" + std::to_string(rd);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b00100;
        uint8_t rd = this->rd & 0b11111;
        return opcode << 27 | rd << 22;
    }
};

class InstrJal : public Instruction{
private:
    uint32_t target;
    std::string label;
public:
    explicit InstrJal(std::string label): Instruction(I_JAL){
        this->label = std::move(label);
        this->target = 0;
    }
    void link_labels(std::map<std::string, Instruction*> label_map) override{
        Instruction* i = label_map[label];
        if (i == nullptr) throw std::runtime_error("Label " + label + " not found");
        target = i->line_num;
    }
    bool replace_target(std::string old_target, std::string new_target) override{
        if (label == old_target){
            label = new_target;
            return true;
        }
        return false;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        regfile->set(31, (int32_t)*pc);
        *pc = target;
    }
    std::string export_str() override{
        return "jal " + label;
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b00011;
        return opcode << 27 | (target & 0x7FFFFFF);
    }
};

class InstrBlt : public Instruction{
private:
    uint8_t rd, rs;
    int16_t imm;
    std::string label;
public:
    InstrBlt(uint8_t rd, uint8_t rs, std::string label): Instruction(I_BLT){
        this->rd = rd;
        this->rs = rs;
        this->label = std::move(label);
        this->imm = 0;
    }
    void link_labels(std::map<std::string, Instruction*> label_map) override{
        Instruction* i = label_map[label];
        if (i == nullptr) throw std::runtime_error("Label " + label + " not found");
        imm = i->line_num - line_num - 1;
    }
    bool replace_target(std::string old_target, std::string new_target) override{
        if (label == old_target){
            label = new_target;
            return true;
        }
        return false;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        int32_t a = regfile->get(rd);
        int32_t b = regfile->get(rs);
        if(a < b){
            *pc += imm;
        }
    }
    std::string export_str() override{
        return "blt $" + std::to_string(rd) + ", $" + std::to_string(rs) + ", " + label;
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b00110;
        uint8_t rd = this->rd & 0b11111;
        uint8_t rs = this->rs & 0b11111;
        return opcode << 27 | rd << 22 | rs << 17 | (imm & 0b11111111111111111);
    }
};

// SPECIAL
class InstrBex : public Instruction{
private:
    uint32_t target;
    std::string label;
public:
    explicit InstrBex(std::string label): Instruction(I_BEX){
        this->label = std::move(label);
        this->target = 0;
    }
    void link_labels(std::map<std::string, Instruction*> label_map) override{
        Instruction* i = label_map[label];
        if (i == nullptr) throw std::runtime_error("Label " + label + " not found");
        target = i->line_num;
    }
    bool replace_target(std::string old_target, std::string new_target) override{
        if (label == old_target){
            label = new_target;
            return true;
        }
        return false;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        if (regfile->get(RSTATUS) != 0)
            *pc = target;
    }
    std::string export_str() override{
        return "bex " + label;
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b10110;
        return opcode << 27 | (target & 0x7FFFFFF);
    }
};

class InstrSetx : public Instruction{
private:
    uint32_t target;
public:
    explicit InstrSetx(uint32_t target): Instruction(I_SETX){
        this->target = target;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        regfile->set(RSTATUS, (int32_t)target);
    }
    std::string export_str() override{
        return "setx " + std::to_string(target);
    }
    uint32_t export_mem() override{
        uint8_t opcode = 0b10101;
        return opcode << 27 | (target & 0x7FFFFFF);
    }
};

class InstrTestLog : public Instruction{
private:
    uint8_t rd;
public:
    explicit InstrTestLog(uint8_t rd): Instruction(I_TEST_LOG){
        this->rd = rd;
    }
    void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) override{
        printf("Register %d = %d\n", rd, regfile->get(rd));
    }
    std::string export_str() override{
        return "";
    }
    uint32_t export_mem() override{
        return 0;
    }
};

#endif //I2C2_MIPSINSTRUCTIONS_H
