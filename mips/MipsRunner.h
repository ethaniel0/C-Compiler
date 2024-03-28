//
// Created by Ethan Horowitz on 3/15/24.
//

#ifndef I2C2_MIPSRUNNER_H
#define I2C2_MIPSRUNNER_H

#include <cstdlib>
#include <cstdint>
#include <string>
#include <map>

class RegisterFile{
private:
    int32_t reg[32]{};
public:
    RegisterFile(){
        for(int32_t & i : reg){
            i = 0;
        }
    }
    int32_t get(uint8_t regNum){
        if (regNum == 0) return 0;
        return reg[regNum];
    }
    void set(uint8_t regNum, int32_t value){
        if (regNum == 0) return;
        reg[regNum] = value;
    }
};

class TestLog {
private:
    std::vector<int> logs;
public:
    void add(int val) {
        logs.push_back(val);
    }

    std::vector<int> getLogs() {
        return logs;
    }
};

enum InstructionType{
    I_ADD, I_ADDI, I_SUB, I_AND, I_OR, I_SLL, I_SRA, I_MUL, I_DIV,
    I_SW, I_LW,
    I_J, I_BNE, I_JR, I_JAL, I_BLT,
    I_BEX, I_SETX,
    I_TEST_LOG
};

class Instruction{
public:
    int line_num;
    InstructionType type;
    Instruction(InstructionType type){
        this->type = type;
        this->line_num = -1;
    }
    virtual void link_labels(std::map<std::string, Instruction*> label_map){}
    virtual bool replace_target(std::string old_target, std::string new_target){return false;}
    virtual void execute(int32_t *dmem, RegisterFile* regfile, uint32_t* pc) = 0;
    virtual std::string export_str() = 0;
};


class MipsRunner {
private:
    int32_t* dmem;
    uint32_t dmem_size;
    Instruction** imem;
    uint32_t imem_size;
    RegisterFile regfile;
    uint32_t pc;
public:
    MipsRunner(uint32_t dmem_size, Instruction** imem, uint32_t imem_size){
        this->dmem_size = dmem_size;
        this->dmem = new int32_t[dmem_size];
        this->imem = imem;
        this->imem_size = imem_size;
        this->pc = 0;
    }
    void load_imem(Instruction** new_imem, uint32_t new_imem_size){
        this->imem = new_imem;
        this->imem_size = new_imem_size;
    }
    int32_t get_reg(uint8_t regNum){
        return regfile.get(regNum);
    }
    int32_t get_mem(uint32_t addr){
        return dmem[addr];
    }
    int run(int maxIter);
};

#endif //I2C2_MIPSRUNNER_H
