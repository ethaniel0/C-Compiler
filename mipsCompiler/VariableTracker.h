//
// Created by Ethan Horowitz on 3/16/24.
//

#ifndef I2C2_VARIABLETRACKER_H
#define I2C2_VARIABLETRACKER_H

#include "../parsing/tokenTypes.h"
#include "MipsBuilder.h"
#include <vector>
#include <stdexcept>


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
    FreqTrackerNode* next = nullptr;
    FreqTrackerNode* prev = nullptr;

    explicit FreqTrackerNode(const std::string& var){
        this->var = var;
    }
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
    void clear();
};

struct VarLocation{
    uint8_t reg;
    bool in_reg;

    uint32_t global_mem;
    bool in_global_mem;

    uint32_t stack_mem;
    bool in_stack;

    uint32_t stack_save;
    bool in_stack_save;

    uint8_t reg_save;

    uint32_t save_location;
    bool must_load;

    TokenValue type;
    int typeRefs;

    int tag;

    VarLocation(){
        reg = 0;
        in_reg = false;
        global_mem = 0;
        in_global_mem = false;
        stack_mem = 0;
        in_stack = false;
        stack_save = 0;
        in_stack_save = false;
        must_load = false;
        type = TokenValue::NONE;
        typeRefs = 0;
        tag = 0;
    }
};

class VariableTracker {
private:
    MipsBuilder* mipsBuilder;
    FreqTracker regFreq;

    std::map<std::string, VarLocation*> var_to_location;
    std::map<std::string, VarLocation*> var_to_stack_save;
    std::map<std::string, VarLocation*> var_to_reg_save;
    std::map<std::string, std::string> aliases;

    std::map<std::string, FunctionToken*> inline_functions;

    int mem_offset = 0;
    int stack_offset = 0;
    int stack_save_offset = 0;

    int scope_level = 0;

    int tag_level = 0;

    std::vector<uint8_t> free_regs;

    void clear_regs();

    uint8_t getFreeReg();
    int tempVarCounter = 0;

    void store_reg_in_stack(uint8_t reg, const std::string& label);
    void store_var_in_stack(const std::string& var, const std::string& label);
    void store_var_in_memory(const std::string& var);
    std::string get_varname(const std::string& var, int scope=-1);

public:
    explicit VariableTracker(MipsBuilder* builder){
        mem_offset = 0;
        mipsBuilder = builder;
        for(uint8_t i = 27; i >= 8; i--){
            free_regs.push_back(i);
        }
    }
    ~VariableTracker();
    
    std::string add_temp_variable();
    int get_mem_offset();

    /// Gets the register of a variable, loads it from memory into a register, or adds if it doesn't exist
    uint8_t getReg(const std::string &var, bool modify = true);

    /// Allocates a constant amount amount of memory in the stack or heap for an array at compile time
    int set_array(const std::string& var, int size);

    /// Sets the type of a variable
    void set_var_type(const std::string& var, TokenValue type);
    void set_var_type_refs(const std::string& var, int refs);
    /// Gets the type of a variable
    TokenValue get_var_type(const std::string& var);
    int get_var_type_refs(const std::string& var);

    /// Returns if a variable exists
    bool var_exists(const std::string& var);

    /// Makes sure that a register is free. If not already free,
    /// the variable already existing in that register will be moved.
    void reserve_reg(uint8_t reg);

    /// Adds a variable to a register. If not specified, the register will be the first one available.
    uint8_t add_variable(const std::string& var, int reg=-1);

    /// Removes a variable from the tracker
    void removeVar(const std::string& var);
    /// Removes a temporary variable from the tracker (will not remove a non-temporary value)
    void removeIfTemp(const std::string& var);

    /// Renames a variable
    void renameVar(const std::string& oldVar, const std::string& newVar);

    /// Stores all current working variables in the stack (to use before a function call)
    void store_current_regs_in_stack();

    /// Restores all working variables from the stack (to use after a function call)
    void restore_regs_from_stack();

    void add_stack_offset(int offset);
    void reduce_stack_offset(int offset);

    /// Increases the scope. Clears register frequency tracking.
    void incScope(bool is_inline = false);

    /// Decreases the scope. Clears register frequency tracking. Loads any changed global variable values back into memory.
    void decScope(bool is_inline = false);

    /// Gets a variable's address in memory.
    /// Returns a negative or zero address for global variables, positive address for stack.
    /// If positive (on stack), subtract 1
    int get_mem_addr(const std::string& var);

    void set_alias(const std::string& alias, const std::string& var);
    void remove_alias(const std::string& alias);

    void add_inline_function(const std::string& name, FunctionToken* function);
    FunctionToken* get_inline_function(const std::string& name);

    void inc_tag_level();
    void dec_tag_level();
};

#endif //I2C2_VARIABLETRACKER_H
