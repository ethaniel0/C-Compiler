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

void FreqTracker::clear(){
    FreqTrackerNode* node = head;
    while(node != nullptr){
        FreqTrackerNode* next = node->next;
        delete node;
        node = next;
    }
    head = nullptr;
    tail = nullptr;
}

VariableTracker::~VariableTracker() {
    for(auto& [name, loc] : var_to_location){
        delete loc;
    }
    var_to_location.clear();
    var_to_stack_save.clear();
    var_to_reg_save.clear();
    free_regs.clear();
}

int VariableTracker::get_mem_offset() {
    return mem_offset;
}

void VariableTracker::store_var_in_memory(const std::string& var) {
    if (var_to_location.find(var) == var_to_location.end()) {
        throw std::runtime_error("Variable not found");
    }
    VarLocation* loc = var_to_location[var];
    if (loc->in_global_mem) throw std::runtime_error("Variable already in memory");

    loc->global_mem = mem_offset;
    loc->in_global_mem = true;

    regFreq.remove(var);
    if (loc->in_reg){
        free_regs.push_back(loc->reg);
        mipsBuilder->addInstruction(new InstrSw(loc->reg, 0, (int16_t) mem_offset), "");
    }
    loc->in_reg = false;

    mem_offset++;
}

void VariableTracker::store_reg_in_stack(uint8_t reg, const std::string& label) {
    uint32_t mem = stack_offset;

    mipsBuilder->addInstruction((new InstrAddi(29, 29, -1)), label);
    mipsBuilder->addInstruction(new InstrSw(reg, 29, (int16_t) mem), "");
    stack_offset += 1;

    for (auto& [name, loc] : var_to_location) {
        if (loc->in_reg && loc->reg == reg) {
            loc->in_stack = true;
            loc->stack_mem = mem;
            break;
        }
    }
}
void VariableTracker::store_var_in_stack(const std::string &var, const std::string& label) {
    if (var_to_location.find(var) == var_to_location.end()) {
        throw std::runtime_error("Variable " + var + " not found");
    }

    auto* loc = var_to_location[var];

    if (!loc->in_reg) return;

    uint32_t mem = stack_offset;

    mipsBuilder->addInstruction((new InstrAddi(29, 29, -1)), label);
    mipsBuilder->addInstruction(new InstrSw(loc->reg, 29, (int16_t) mem), "");
    loc->in_stack = true;
    loc->stack_mem = mem;
    stack_offset++;
}
void VariableTracker::add_stack_offset(int offset) {
    stack_offset += offset;
    mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) -offset), "");
}
void VariableTracker::reduce_stack_offset(int offset) {
    stack_offset -= offset;
    mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) offset), "");
}

uint8_t VariableTracker::getFreeReg() {
    if(free_regs.empty()){
        std::string least_used = regFreq.getLeastUsed();
        if (var_to_location.find(least_used) == var_to_location.end()) {
            throw std::runtime_error("Compilation error) Variable " + least_used + " not found");
        }
        store_var_in_memory(least_used);
    }
    uint8_t reg = free_regs.back();
    free_regs.pop_back();
    return reg;
}

uint8_t VariableTracker::add_variable(const std::string &var, int reg) {
    std::string name = get_varname(var);
    if (var_to_location.find(name) != var_to_location.end()){
        throw std::runtime_error("Variable already in use");
    }
    regFreq.use(var);
    uint8_t new_reg;
    if (reg == -1){
        new_reg = getFreeReg();
    }
    else{
        reserve_reg(reg);
        new_reg = reg;
    }

    auto* loc = new VarLocation();
    loc->reg = new_reg;
    loc->in_reg = true;
    var_to_location[name] = loc;

    regFreq.use(name);
    return new_reg;
}

std::string VariableTracker::add_temp_variable() {
    uint8_t new_reg = getFreeReg();
    std::string var = "<temp" + std::to_string(tempVarCounter++) + ">";
    std::string name = get_varname(var);
    auto loc = new VarLocation();
    loc->reg = new_reg;
    loc->in_reg = true;
    var_to_location[name] = loc;
    regFreq.use(name);
    return var;
}

uint8_t VariableTracker::getReg(const std::string &var, bool modify) {
    int scope = scope_level;

    while (scope >= 0) {
        std::string name = get_varname(var, scope);
        if (var_to_location.find(name) == var_to_location.end()){
            scope--;
            continue;
        }
        VarLocation* loc = var_to_location[name];

        bool must_load = modify && loc->must_load;

        // if variable is in a register, return that register
        if (loc->in_reg && !must_load) {
            regFreq.use(name);
            return loc->reg;
        }

        // if variable is in memory, load it into a register
        if (loc->in_stack || loc->in_stack_save) {
            uint32_t mem;
            if (loc->in_stack) {
                mem = stack_offset - loc->stack_mem;
            }
            else {
                mem = stack_offset - loc->save_location;
            }
            uint8_t reg = getFreeReg();
//            uint32_t mem = stack_offset - loc->stack_mem;
            mipsBuilder->addInstruction(new InstrLw(reg, 29, (int16_t) mem), "");

            regFreq.use(name);
            loc->in_reg = true;
            loc->reg = reg;

            return reg;
        }

        if (loc->in_global_mem){
            uint8_t reg = getFreeReg();
            mipsBuilder->addInstruction(new InstrLw(reg, 0, (int16_t) loc->global_mem), "");

            regFreq.use(name);
            loc->in_reg = true;
            loc->reg = reg;

            return reg;
        }
        scope--;
    }
    return add_variable(var);
}

int VariableTracker::set_array(const std::string &var, int size) {
    std::string name = get_varname(var);
    if (var_to_location.find(name) != var_to_location.end()) {
        throw std::runtime_error("Variable " + var + " already in use");
    }
    if (scope_level == 0){
        // store in global memory
        uint32_t mem = mem_offset;
        mem_offset += size;
        return -mem;
    }
    else {
        // store in stack
        uint32_t mem = stack_offset;
        stack_offset += size;
        mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) -size), "");
        return mem + 1;
    }
}

bool VariableTracker::var_exists(const std::string &var) {
    int scope = scope_level;
    while (scope >= 0) {
        std::string name = get_varname(var, scope);
        if (var_to_location.find(name) != var_to_location.end()) return true;
        scope--;
    }
    return false;
}

void VariableTracker::reserve_reg(uint8_t reg) {
    for (auto& [name, loc] : var_to_location) {
        if (loc->in_reg && loc->reg == reg) {
            uint8_t new_reg = getFreeReg();
            loc->reg = new_reg;
            if (reg >= 8 && reg <= 27) free_regs.push_back(reg);
            mipsBuilder->addInstruction(new InstrAddi(new_reg, reg, 0), "");
            break;
        }
    }
}

void VariableTracker::removeVar(const std::string &var) {
    std::string name = get_varname(var);
    regFreq.remove(name);
    if (var_to_location.find(name) == var_to_location.end()) {
        return;
    }
    VarLocation* loc = var_to_location[name];
    if (loc->in_reg){
        free_regs.push_back(loc->reg);
    }
    var_to_location.erase(name);
    regFreq.remove(name);
    delete loc;

    remove_alias(var);

    if (var_to_stack_save.find(name) != var_to_stack_save.end()) {
        var_to_stack_save.erase(name);
    }
}

void VariableTracker::removeIfTemp(const std::string &var) {
    if (var.find("<temp") != std::string::npos) {
        removeVar(var);
    }
}

void VariableTracker::renameVar(const std::string& oldVar, const std::string& newVar){
    std::string name_old = get_varname(oldVar);
    std::string name_new = get_varname(newVar);

    if (var_to_location.find(name_old) == var_to_location.end()){
        throw std::runtime_error("Variable " + oldVar + " not found");
    }

    VarLocation* loc = var_to_location[name_old];
    var_to_location.erase(name_old);
    var_to_location[name_new] = loc;
}

void VariableTracker::clear_regs() {
    for (auto& [name, loc] : var_to_location) {
        if (loc->in_reg) {
            free_regs.push_back(loc->reg);
            loc->in_reg = false;
        }
    }
    free_regs.clear();
    for(uint8_t i = 27; i >= 8; i--){
        free_regs.push_back(i);
    }
}

void VariableTracker::store_current_regs_in_stack() {

    std::vector<VarLocation*> to_store;
    std::vector<std::string> to_store_names;

    for (auto &[name, loc]: var_to_location) {
        if (loc->in_reg) {
            to_store.push_back(loc);
            to_store_names.push_back(name);
        }
    }

    if (scope_level > 0) {
        // store reg31 in stack
        int size = to_store.size() + 1;
        mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) -size), "");
        mipsBuilder->addInstruction(new InstrSw(31, 29, (int16_t) 0), "");
        stack_offset += size;
        stack_save_offset += size;
        for (int i = 1; i < size; i++){
            auto loc = to_store[i-1];
            mipsBuilder->addInstruction(new InstrSw(loc->reg, 29, i), "");
            loc->save_location = stack_offset - i;
            loc->in_stack_save = true;
            var_to_stack_save[to_store_names[i-1]] = loc;
        }
    }
    else {
        // store in mem
        for (auto loc : to_store) {
            loc->in_global_mem = true;
            loc->global_mem = mem_offset;
            mipsBuilder->addInstruction(new InstrSw(loc->reg, 0, (int16_t) mem_offset), "");
            mem_offset++;
        }
    }
    clear_regs();
}

void VariableTracker::restore_regs_from_stack() {
    if (scope_level == 0) return;

    // restore reg31
    mipsBuilder->addInstruction(new InstrLw(31, 29, 0), "");

    for (auto &[name, loc]: var_to_stack_save) {
        reserve_reg(loc->reg);
        loc->in_reg = true;
        loc->in_stack_save = false;
        free_regs.erase(std::remove(free_regs.begin(), free_regs.end(), loc->reg), free_regs.end());
        uint32_t mem = stack_offset - loc->save_location;
        mipsBuilder->addInstruction(new InstrLw(loc->reg, 29, (int16_t) mem), "");
        regFreq.use(name);
    }

    // restore stack pointer
    mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) stack_save_offset), "");
    stack_offset -= stack_save_offset;
    stack_save_offset = 0;
}

void VariableTracker::incScope(){
    scope_level++;
    regFreq.clear();

    for (auto& [name, loc] : var_to_location) {
        if (loc->in_reg) {
            var_to_reg_save[name] = loc;
            loc->reg_save = loc->reg;
        }
    }

    clear_regs();
}

void VariableTracker::decScope() {
    scope_level--;
    regFreq.clear();

    std::vector<std::string> names_to_delete;

    for (auto& [name, loc] : var_to_location) {
        if (loc->in_reg && name[0] == '0') {
            mipsBuilder->addInstruction(new InstrSw(loc->reg, 0, (int16_t) loc->global_mem), "");
        }
        else if (name[0] == '1'){
            names_to_delete.push_back(name);
        }
    }
    for (auto& name : names_to_delete){
        auto* loc = var_to_location[name];
        var_to_location.erase(name);
        delete loc;
    }

    clear_regs();

    if (stack_offset > 0) {
        mipsBuilder->addInstruction(new InstrAddi(29, 29, (int16_t) stack_offset), "");
        stack_offset = 0;
    }

    // restore reg spots
    for (auto& [name, loc] : var_to_reg_save) {
        loc->in_reg = true;
        loc->reg = loc->reg_save;
        free_regs.erase(std::remove(free_regs.begin(), free_regs.end(), loc->reg), free_regs.end());
        regFreq.use(name);
    }
}

int VariableTracker::get_mem_addr(const std::string &var) {
    std::string name = get_varname(var);
    std::string name_global = get_varname(var, 0);

    if (var_to_location.find(name) == var_to_location.end() &&
        var_to_location.find(name_global) == var_to_location.end()) {
        throw std::runtime_error("Variable " + var + " not found");
    }

    bool loc_name = var_to_location.find(name) != var_to_location.end();

    auto* loc = loc_name ? var_to_location[name] : var_to_location[name_global];
    loc->must_load = true;

    if (loc->in_stack) return stack_offset - loc->stack_mem - 1;
    if (scope_level > 0 && loc->in_reg) {
        store_reg_in_stack(loc->reg, "");
        return stack_offset - loc->stack_mem;
    }
    if (loc->in_global_mem) return -loc->global_mem;

    // save in mem or stack
    if (scope_level == 0) {
        store_var_in_memory(name_global);
        return -loc->global_mem;
    }
    else {
        store_var_in_stack(name, "");
        return stack_offset - loc->stack_mem;
    }
}

void VariableTracker::set_var_type(const std::string &var, TokenValue type) {
    // find var first
    int scope = scope_level;

    while (scope >= 0) {
        std::string name = get_varname(var, scope);
        if (var_to_location.find(name) == var_to_location.end()) {
            scope--;
            continue;
        }

        auto* loc = var_to_location[name];
        loc->type = type;

        scope--;
    }
}
void VariableTracker::set_var_type_refs(const std::string &var, int refs) {
    // find var first
    int scope = scope_level;

    while (scope >= 0) {
        std::string name = get_varname(var, scope);
        if (var_to_location.find(name) == var_to_location.end()) {
            scope--;
            continue;
        }

        auto* loc = var_to_location[name];
        loc->typeRefs = refs;

        scope--;
    }
}

TokenValue VariableTracker::get_var_type(const std::string &var) {
    // find var first
    int scope = scope_level;

    while (scope >= 0) {
        std::string name = get_varname(var, scope);
        if (var_to_location.find(name) == var_to_location.end()) {
            scope--;
            continue;
        }
        auto* loc = var_to_location[name];
        return loc->type;
    }
    return TokenValue::VOID;
}
int VariableTracker::get_var_type_refs(const std::string &var) {
    // find var first
    int scope = scope_level;

    while (scope >= 0) {
        std::string name = get_varname(var, scope);
        if (var_to_location.find(name) == var_to_location.end()) {
            scope--;
            continue;
        }
        auto* loc = var_to_location[name];
        return loc->typeRefs;
    }
    return 0;

}

void VariableTracker::set_alias(const std::string& alias, const std::string& var) {
    std::string alias_name = std::to_string(scope_level) + "-" + alias;
    std::string var_name = std::to_string(scope_level) + "-" + var;
    aliases[alias_name] = var_name;
}
void VariableTracker::remove_alias(const std::string &alias) {
    std::string alias_name = std::to_string(scope_level) + "-" + alias;
    aliases.erase(alias_name);
}

std::string VariableTracker::get_varname(const std::string &var, int scope) {
    if (scope == -1) scope = scope_level;
    std::string var_name = std::to_string(scope) + "-" + var;
    if (aliases.find(var_name) != aliases.end()) {
        return aliases[var_name];
    }
    return var_name;
}

void VariableTracker::add_inline_function(const std::string &name, FunctionToken *function) {
    inline_functions[name] = function;
}

FunctionToken *VariableTracker::get_inline_function(const std::string &name) {
    if (inline_functions.find(name) == inline_functions.end()) {
        throw std::runtime_error("Function " + name + " not found");
    }
    return inline_functions[name];
}