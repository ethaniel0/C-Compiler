//
// Created by Ethan Horowitz on 3/16/24.
//

#include "VariableTracker.h"

void FreqTracker::remove(const std::string &var) {
    if(head == nullptr){
        return;
    }
    if(head->var == var){
        head = head->next;
        if (head != nullptr && head->prev != nullptr) {
            delete head->prev;
            head->prev = nullptr;
        }
        return;
    }
    if(tail->var == var){
        tail = tail->prev;
        delete tail->next;
        tail->next = nullptr;
        return;
    }
    FreqTrackerNode* node = head;
    while(node != nullptr){
        if(node->var == var){
            node->prev->next = node->next;
            node->next->prev = node->prev;
            delete node;
            return;
        }
        node = node->next;
    }
}

void FreqTracker::use(const std::string &var) {
    if(head == nullptr){
        head = new FreqTrackerNode();
        head->var = var;
        tail = head;
        return;
    }

    // if variable exists, move it to tail
    FreqTrackerNode* ref = this->head;
    while(ref != nullptr){
        if(ref->var == var){
            if(ref == tail) return;
            if (ref->prev != nullptr) ref->prev->next = ref->next;
            ref->next->prev = ref->prev;
            tail->next = ref;
            ref->prev = tail;
            tail = ref;
            tail->next = nullptr;
            return;
        }
        ref = ref->next;
    }
    // if variable doesn't exist, make it the tail
    auto* newNode = new FreqTrackerNode();
    newNode->var = var;
    tail->next = newNode;
    newNode->prev = tail;
    newNode->next = nullptr;
    tail = newNode;
}

std::string FreqTracker::getLeastUsed() {
    if(head == nullptr) return "";
    return head->var;
}

void VariableTracker::store_reg_in_memory(uint8_t reg, const std::string &var) {
    std::string name = std::to_string(scope_level) + "-" + var;
    if(var_to_global_mem.find(name) != var_to_global_mem.end()){
        throw std::runtime_error("Variable already in memory");
    }
    regFreq.remove(name);
    var_to_reg.erase(name);
    reg_to_var.erase(reg);
    free_regs.push_back(reg);

    var_to_global_mem[name] = mem_offset;
    global_mem_to_var[mem_offset] = name;
    mipsBuilder->addInstruction(new InstrSw(reg, 0, (int16_t) mem_offset), "");
    mem_offset++;
}

uint8_t VariableTracker::getFreeReg() {
    if(free_regs.empty()){
        std::string least_used = regFreq.getLeastUsed();
        uint8_t leased_used_reg = var_to_reg[least_used];
        store_reg_in_memory(leased_used_reg, least_used);
    }
    return free_regs.back();
}

uint8_t VariableTracker::add_variable(const std::string &var) {
    std::string name = std::to_string(scope_level) + "-" + var;
    if(var_to_reg.find(name) != var_to_reg.end()){
        throw std::runtime_error("Variable already in use");
    }
    if (var_to_global_mem.find(name) != var_to_global_mem.end()){
        throw std::runtime_error("Variable already in memory");
    }
    regFreq.use(var);
    uint8_t new_reg = getFreeReg();
    free_regs.pop_back();
    var_to_reg[name] = new_reg;
    reg_to_var[new_reg] = name;
    regFreq.use(name);
    return new_reg;
}

std::string VariableTracker::add_temp_variable() {
    uint8_t new_reg = getFreeReg();
    std::string var = "<temp" + std::to_string(tempVarCounter++) + ">";
    std::string name = std::to_string(scope_level) + "-" + var;
    free_regs.pop_back();
    var_to_reg[name] = new_reg;
    reg_to_var[new_reg] = name;
    regFreq.use(name);
    return var;
}

uint8_t VariableTracker::getReg(const std::string &var) {
    int scope = scope_level;

    while (scope >= 0) {
        std::string name = std::to_string(scope) + "-" + var;

        // if variable is in a register, return that register
        if (var_to_reg.find(name) != var_to_reg.end()) {
            return var_to_reg[name];
        }

        if (var_to_stack.find(name) != var_to_stack.end()) {
            uint8_t reg = getFreeReg();
            uint32_t mem = stack_offset - var_to_stack[name];
            mipsBuilder->addInstruction(new InstrLw(reg, 29, (int16_t) mem), "");
            return reg;
        }

        // if variable is in memory, load it into a register
        if (var_to_global_mem.find(name) != var_to_global_mem.end()) {
            uint8_t reg = getFreeReg();
            uint16_t mem = var_to_global_mem[name];
            mipsBuilder->addInstruction(new InstrLw(reg, 0, (int16_t) mem), "");
            var_to_reg[name] = reg;
            reg_to_var[reg] = name;
            regFreq.use(name);
            free_regs.erase(std::remove(free_regs.begin(), free_regs.end(), reg), free_regs.end());
            return reg;
        }
        scope--;
    }
    return add_variable(var);
}

void VariableTracker::removeVar(const std::string &var) {
    std::string name = std::to_string(scope_level) + "-" + var;
    regFreq.remove(name);
    if(var_to_reg.find(name) != var_to_reg.end()){
        regFreq.remove(name);
        free_regs.push_back(var_to_reg[name]);
        uint8_t reg = var_to_reg[name];
        var_to_reg.erase(name);
        reg_to_var.erase(reg);
    }
    if(var_to_global_mem.find(name) != var_to_global_mem.end()){
        var_to_global_mem.erase(name);
        global_mem_to_var.erase(var_to_global_mem[name]);
    }
}

void VariableTracker::renameVar(const std::string& oldVar, const std::string& newVar){
    std::string name_old = std::to_string(scope_level) + "-" + oldVar;
    std::string name_new = std::to_string(scope_level) + "-" + newVar;
    if(var_to_reg.find(name_old) != var_to_reg.end()){
        uint8_t reg = var_to_reg[name_old];
        var_to_reg.erase(name_old);
        var_to_reg[name_new] = reg;
        reg_to_var[reg] = name_new;
    }
    if(var_to_global_mem.find(name_old) != var_to_global_mem.end()){
        uint32_t mem = var_to_global_mem[name_old];
        var_to_global_mem.erase(name_old);
        var_to_global_mem[name_new] = mem;
        global_mem_to_var[mem] = name_new;
    }
}

void VariableTracker::clear_regs() {
    for(auto& [name, reg] : var_to_reg){
        free_regs.push_back(reg);
    }
    var_to_reg.clear();
    reg_to_var.clear();
    free_regs.clear();
    for(uint8_t i = 27; i >= 8; i--){
        free_regs.push_back(i);
    }
}

void VariableTracker::store_reg_in_stack(uint8_t reg, std::string label) {
    uint32_t mem = stack_offset;

    if (reg_to_var.find(reg) != reg_to_var.end()) {
        std::string name = reg_to_var[reg];
        var_to_stack[name] = mem;
        stack_to_var[mem] = name;
        return;
    }

    mipsBuilder->addInstruction((new InstrAddi(29, 29, -1)), label);
    mipsBuilder->addInstruction(new InstrSw(reg, 29, (int16_t) mem), "");
    stack_offset += 1;
}

void VariableTracker::store_current_regs_in_stack() {

//    std::vector<std::string> names;
//
//    for (auto &[name, reg]: var_to_reg) {
//        // if name contains "<temp", remove
//        if (name[2] == '<' && name.substr(3, 4) == "temp") {
//            names.push_back(name);
//        }
//    }
//
//    for (auto &name : names) {
//        uint8_t reg = var_to_reg[name];
//        var_to_reg.erase(name);
//        reg_to_var.erase(reg);
//    }

    int size = (int) var_to_reg.size();

    if (scope_level > 0) {
        // store reg31 in stack
        size += 1;
        mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) -size), "");
        mipsBuilder->addInstruction(new InstrSw(31, 29, (int16_t) 0), "");
        int16_t i = 1;
        stack_offset += size;
        for (auto &[name, reg]: var_to_reg) {
            mipsBuilder->addInstruction(new InstrSw(reg, 29, i), "");
            var_to_stack[name] = i;
            stack_to_var[i] = name;
            i++;
        }

    }
    else {
        // store in mem
        for (auto &[name, reg]: var_to_reg) {
            mipsBuilder->addInstruction(new InstrSw(reg, 0, (int16_t) mem_offset), "");
            var_to_global_mem[name] = mem_offset;
            global_mem_to_var[mem_offset] = name;
            mem_offset++;
        }
    }
    clear_regs();
}

void VariableTracker::restore_regs_from_stack() {
    if (scope_level == 0) return;

    // restore reg31
    mipsBuilder->addInstruction(new InstrLw(31, 29, 0), "");

    for (auto &[name, mem]: var_to_stack) {
        uint8_t reg = getFreeReg();
        mipsBuilder->addInstruction(new InstrLw(reg, 29, (int16_t) mem), "");
        var_to_reg[name] = reg;
        reg_to_var[reg] = name;
        free_regs.erase(std::remove(free_regs.begin(), free_regs.end(), reg), free_regs.end());
        regFreq.use(name);
    }

    // restore stack pointer
    mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) stack_offset), "");
    stack_offset = 0;
    var_to_stack.clear();
    stack_to_var.clear();

}

void VariableTracker::incScope() {
    scope_level++;

    for (auto &[name, mem]: var_to_stack) {
        regFreq.remove(name);
        regFreq.use(name);
    }

    saved_var_to_reg.clear();

    for (auto &[name, reg]: var_to_reg) {
        regFreq.remove(name);
        regFreq.use(name);
        saved_var_to_reg[name] = reg;
    }

    clear_regs();
}

void VariableTracker::decScope() {
    scope_level--;
    stack_to_var.clear();
//    mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) stack_offset), "");
//    stack_offset = 0;

    for(auto& [name, mem] : var_to_stack){
        regFreq.remove(name);
        regFreq.use(name);
    }

    for(auto& [name, reg] : var_to_reg){
        // on scope level 0, store word back into appropriate memory location
        if (name[0] == '0'){
            uint32_t mem = var_to_global_mem[name];
            mipsBuilder->addInstruction(new InstrSw(reg, 0, (int16_t) mem), "");
        }
        regFreq.remove(name);
    }

    clear_regs();

    var_to_stack.clear();
    stack_to_var.clear();

    // restore saved variables
    for(auto& [name, reg] : saved_var_to_reg){
        var_to_reg[name] = reg;
        reg_to_var[reg] = name;
        regFreq.use(name);
    }
}

int VariableTracker::get_stack_offset() {
    return stack_offset;
}

