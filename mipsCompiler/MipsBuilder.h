//
// Created by Ethan Horowitz on 3/16/24.
//

#ifndef I2C2_MIPSBUILDER_H
#define I2C2_MIPSBUILDER_H

#include <vector>
#include <stdexcept>
#include "../mips/MipsInstructions.h"

class MipsBuilder {
private:
    std::vector<Instruction*> instructions;
    std::map<std::string, Instruction*> labels;
    std::map<Instruction*, std::string> invLabels;
    int unnamedLabelCounter = 0;

    bool replaceLabel(const std::string& oldLabel, const std::string& newLabel);
    void filterNoops();
    void filterDoubleJJumps();
    void filterJToNext();
    void filterJs();
public:
    MipsBuilder() = default;
    void addInstruction(Instruction* instr, const std::string& label);
    void prependInstruction(Instruction* instr);
    std::string genUnnamedLabel();
    void linkLabels();
    void simplify();
    std::vector<Instruction*> getInstructions();
    std::string export_str();
    std::vector<uint32_t> export_mem();
};

#endif //I2C2_MIPSBUILDER_H
