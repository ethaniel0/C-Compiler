//
// Created by Ethan Horowitz on 3/16/24.
//

#ifndef I2C2_VARIABLETRACKER_H
#define I2C2_VARIABLETRACKER_H

#include "MipsBuilder.h"

/*
my register conventions:
 $0: 0
 $1: for pseudo-instructions (not, li, etc)
 $2 - $3: return values
 $4 - $7: arguments
 $8 - $27: temporary variables
 $28: heap pointer
 $29: stack pointer
 $30: rstatus
 $31: return address
*/

struct FreqTrackerNode{
    std::string var;
    FreqTrackerNode* next{};
    FreqTrackerNode* prev{};
};

class FreqTracker{
private:
    FreqTrackerNode* head;
    FreqTrackerNode* tail;
public:
    FreqTracker() {
        head = nullptr;
        tail = nullptr;
    }
    void remove(const std::string& var);
    void use(const std::string& var);
    std::string getLeastUsed();
};

class VariableTracker {
private:
    MipsBuilder* mipsBuilder;

    std::map<std::string, uint8_t> var_to_reg;
    std::map<uint8_t, std::string> reg_to_var;

    std::map<std::string, uint8_t> saved_var_to_reg;

    FreqTracker regFreq;

    std::map<std::string, uint32_t> var_to_global_mem;
    std::map<uint32_t, std::string> global_mem_to_var;

    int mem_offset;

    int scope_level = 0;
    std::map<std::string, uint32_t> var_to_stack;
    std::map<uint32_t, std::string> stack_to_var;

    std::vector<uint8_t> free_regs;

    int stack_offset = 0;
    void store_reg_in_memory(uint8_t reg, const std::string& var);

    void clear_regs();

    uint8_t getFreeReg();
    int tempVarCounter = 0;
public:
    explicit VariableTracker(MipsBuilder* builder){
        mem_offset = 0;
        mipsBuilder = builder;
        for(uint8_t i = 27; i >= 8; i--){
            free_regs.push_back(i);
        }
    }
    
    std::string add_temp_variable();
    int get_stack_offset();

    // get the register of a variable, load it from memory into a register, or add if it doesn't exist
    uint8_t getReg(const std::string& var);
    uint8_t add_variable(const std::string& var);
    void removeVar(const std::string& var);
    void renameVar(const std::string& oldVar, const std::string& newVar);
    void store_reg_in_stack(uint8_t reg, std::string label);
    void store_current_regs_in_stack();
    void restore_regs_from_stack();

    void incScope();
    void decScope();
};

#endif //I2C2_VARIABLETRACKER_H
