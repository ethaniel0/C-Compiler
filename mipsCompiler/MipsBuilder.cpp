//
// Created by Ethan Horowitz on 3/16/24.
//

#include "MipsBuilder.h"

void MipsBuilder::addInstruction(Instruction *instr, const std::string &label) {
    instructions.push_back(instr);
    if (!label.empty()) {
        labels[label] = instr;
        invLabels[instr] = label;
    }
}

void MipsBuilder::prependInstruction(Instruction *instr) {
    instructions.insert(instructions.begin(), instr);
}

std::string MipsBuilder::genUnnamedLabel() {
    return "\"" + std::to_string(unnamedLabelCounter++) + "\"";
}

void MipsBuilder::linkLabels() {
    for (int i = 0; i < instructions.size(); i++){
        instructions[i]->line_num = i;
    }
    for (Instruction *instr : instructions) {
        instr->link_labels(labels);
    }
}

bool isNoop(Instruction* instr){
    // add 0 0 0
    if (instr->type != InstructionType::I_ADD) return false;
    InstrAdd* add = (InstrAdd*) instr;
    return add->is_noop();
}

bool MipsBuilder::replaceLabel(const std::string &oldLabel, const std::string &newLabel) {
    bool changed = false;
    for (Instruction *instr : instructions) {
        changed |= instr->replace_target(oldLabel, newLabel);
    }
    labels.erase(oldLabel);
    return changed;
}

void MipsBuilder::filterNoops() {
    for (int i = 0; i < instructions.size(); i++) {
        Instruction* instr = instructions[i];
        if (!isNoop(instr)) continue;
            // if noop has no label, remove it
        if (invLabels.find(instr) == invLabels.end()) {
            instructions.erase(instructions.begin() + i);
            continue;
        }

        // if noop has a label

        // if noop is last command, leave it there
        if (i == instructions.size() - 1) continue;

        // if next command has no label, move the label to the next command
        Instruction *next = instructions[i + 1];
        if (invLabels.find(next) == invLabels.end()) {
            std::string label = invLabels[instr];
            labels[label] = next;
            invLabels[next] = label;
            invLabels.erase(instr);
            instructions.erase(instructions.begin() + i);
            i--;
            continue;
        }

        // if next command has a label, find all occurrences of the noop label and replace them with the next label
        std::string noop_label = invLabels[instr];
        std::string next_label = invLabels[next];
        replaceLabel(noop_label, next_label);
        invLabels.erase(instr);
        instructions.erase(instructions.begin() + i);
        i--;
    }

}

void MipsBuilder::filterDoubleJJumps(){
    for (int i = 0; i < instructions.size(); i++) {
        Instruction* instr = instructions[i];
        if (instr->type != InstructionType::I_J) continue;
        auto* j = (InstrJ*) instr;
        if (invLabels.find(j) == invLabels.end()) continue;

        // if j has a label and jumps to another label, then replace the label so you only jump once
        std::string jLabel = invLabels[j];
        std::string labelTo = j->label;
        if (labels.find(labelTo) == labels.end())
            throw std::runtime_error("Label " + labelTo + " not found");
        bool result = replaceLabel(jLabel, labelTo);
        if (result) {
            invLabels.erase(j);
            instructions.erase(instructions.begin() + i);
            i--;
        }
    }
}

void MipsBuilder::filterJToNext(){
    for (int i = 0; i < instructions.size()-1; i++) {
        Instruction* instr = instructions[i];
        if (instr->type != InstructionType::I_J) continue;

        auto* j = (InstrJ*) instr;

        Instruction* next = instructions[i + 1];
        if (invLabels.find(next) == invLabels.end()) continue;

        // if j has a label and jumps to the next instruction, remove the j
        std::string j_label_to = j->label;
        std::string next_label = invLabels[next];
        if (j_label_to != next_label) continue;

        instructions.erase(instructions.begin() + i);

        if (invLabels.find(j) != invLabels.end()){
            std::string label = invLabels[j];
            replaceLabel(label, next_label);
            invLabels.erase(j);
        }
    }
}

void MipsBuilder::filterJs(){
    filterDoubleJJumps();
    filterJToNext();
}

void MipsBuilder::simplify() {
    filterNoops();
    filterJs();
}

std::vector<Instruction *> MipsBuilder::getInstructions() {
    return instructions;
}

std::string MipsBuilder::export_str() {
    std::string result;

    std::map<Instruction*, std::string> instr_to_label;
    for (const auto& pair : labels){
        instr_to_label[pair.second] = pair.first;
    }

    for (Instruction *instr : instructions) {
        if (instr_to_label.find(instr) != instr_to_label.end()){
            result += instr_to_label[instr] + ":\n";
        }
        result += instr->export_str() + "\n";
    }
    return result;
}

std::vector<uint32_t> MipsBuilder::export_mem() {
    std::vector<uint32_t> result;
    result.reserve(instructions.size());
    for (Instruction *instr : instructions) {
        result.push_back(instr->export_mem());
    }
    return result;
}