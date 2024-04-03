//
// Created by Ethan Horowitz on 3/16/24.
//

#ifndef I2C2_VARIABLETRACKER_H
#define I2C2_VARIABLETRACKER_H

#include "../parsing/tokenTypes.h"
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

struct MemLocation{
    uint32_t addr;
    bool on_stack;
    bool exists;
};

class VariableTracker {
private:
    MipsBuilder* mipsBuilder;

    std::map<std::string, uint8_t> var_to_reg;
    std::map<uint8_t, std::string> reg_to_var;

    std::map<std::string, int> track_numbers;

    std::map<std::string, uint8_t> saved_var_to_reg;

    FreqTracker regFreq;

    std::map<std::string, uint32_t> var_to_global_mem;
    std::map<uint32_t, std::string> global_mem_to_var;

    std::map<std::string, TokenValue> var_to_type;

    int mem_offset;

    int scope_level = 0;
    std::map<std::string, uint32_t> var_to_stack;
    std::map<uint32_t, std::string> stack_to_var;
    std::map<std::string, uint32_t> var_to_stack_save;

    std::vector<std::string> vars_that_must_load;

    std::vector<uint8_t> free_regs;

    int stack_offset = 0;
    int stack_save_offset = 0;
    void store_reg_in_memory(uint8_t reg, const std::string& var);

    void clear_regs();

    uint8_t getFreeReg();
    int tempVarCounter = 0;

    void add_var_that_must_load(const std::string& var);
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
    int get_mem_offset();

    // get the register of a variable, load it from memory into a register, or add if it doesn't exist
    uint8_t getReg(const std::string& var, int track_number, bool modify=true);
    int set_array(const std::string& var, int size);
    void set_var_type(const std::string& var, TokenValue type);
    TokenValue get_var_type(const std::string& var);

    bool var_exists(const std::string& var);
    void reserve_reg(uint8_t reg);
    void set_track_number(const std::string& var, int track_number);
    uint8_t add_variable(const std::string& var, int reg=-1);
    void removeVar(const std::string& var);
    void renameVar(const std::string& oldVar, const std::string& newVar);
    void store_reg_in_stack(uint8_t reg, const std::string& label);
    void store_current_regs_in_stack();
    void restore_regs_from_stack();

    void incScope();
    void decScope();

    // returns negative address for global, positive address for stack. If on positive, subtract 1
    int get_mem_addr(const std::string& var);
    void remove_from_regs(const std::string& var);
};

#endif //I2C2_VARIABLETRACKER_H
