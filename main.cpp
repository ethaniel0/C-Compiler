#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include "parsing/tokenize.h"
#include "parsing/parse.h"
#include "mipsCompiler/MipsCompiler.h"

int main(int argc, char** argv) {
    /*
     -o = output file
     -type = type of output (s, mem)
     -r a b ... = run, print variables a, b, ... at end
     */

    std::string output_file;
    std::vector<std::string> files;
    std::vector<std::string> runVars;
    std::string output_type = "s";
    bool run = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-o") {
            output_file = argv[++i];
        } else if (std::string(argv[i]) == "-r") {
            run = true;
            i++;
            for (; i < argc; i++) {
                runVars.emplace_back(argv[i]);
            }
        }
        else if (std::string(argv[i]) == "-type"){
            output_type = argv[++i];
        }
        else {
            files.emplace_back(argv[i]);
        }
    }
    if (files.empty()) {
        std::cerr << "No input files" << std::endl;
        return 1;
    }
    if (output_file.empty()) {
        std::cerr << "No output file" << std::endl;
        return 1;
    }
    std::vector<Token*> tokens;
    for (const std::string& file : files) {
        std::ifstream in(file);
        if (!in.is_open()) {
            std::cerr << "Could not open file " << file << std::endl;
            return 1;
        }
        std::string line;
        std::string file_text = "";
        while (std::getline(in, line)) {
            file_text += line + "\n";
        }
        std::vector<Token*> newTokens = tokenize(file_text);
        tokens.insert(tokens.end(), newTokens.begin(), newTokens.end());
    }

    TokenIterator tokens_iter(tokens);
    Scope scope(nullptr);
    std::vector<Token*> ast = parse(tokens_iter, &scope);

    sort_ast(&ast, &scope);

    MipsBuilder builder;
    builder.addInstruction(new InstrAddi(29, 29, 2047), "");
    VariableTracker tracker(&builder);
    BreakScope breakScope;
    compile_instructions(&breakScope, ast, &builder, &tracker);

    for (Token* token : ast) {
        delete token;
    }

    int mem_loc = tracker.get_mem_offset();
    builder.prependInstruction(new InstrAddi(28, 0, mem_loc));

    builder.simplify();
    builder.linkLabels();

    // save to file
    if (output_type == "s") {
        std::ofstream out(output_file);
        if (!out.is_open()) {
            std::cerr << "Could not open output file " << output_file << std::endl;
            return 1;
        }
        std::string assembly_code = builder.export_str();
        out << assembly_code;
    }
    else if (output_type == "mem") {
        std::ofstream out(output_file, std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "Could not open output file " << output_file << std::endl;
            return 1;
        }
        std::vector<uint32_t> mem = builder.export_mem();
        for (uint32_t word : mem) {
            std::bitset<32> bits(word);
            out.write(bits.to_string().c_str(), 32);
            out << std::endl;
        }
        if (mem.size() < 4096){
            for (int i = 0; i < 4095 - mem.size(); i++) {
                out << "00000000000000000000000000000000" << std::endl;
            }
        }
    }
    else if (output_type == "numbers"){
        std::vector<uint32_t> mem = builder.export_mem();
        std::ofstream out(output_file);
        if (!out.is_open()) {
            std::cerr << "Could not open output file " << output_file << std::endl;
            return 1;
        }
        out << "{";
        for (int i = 0; i < mem.size(); i++) {
            out << mem[i];
            if (i < mem.size() - 1) out << ", ";
        }
        out << "}";
    }
    else {
        std::cerr << "Invalid output type " << output_type << std::endl;
        return 1;
    }

    if (run){
        std::vector<Instruction*> instructions = builder.getInstructions();
        MipsRunner runner(2048, instructions.data(), instructions.size());
        int num_cycles = runner.run(2000);
        printf("Ran for %d cycles\n", num_cycles);
        for (const std::string& var : runVars){
            uint8_t reg = tracker.getReg(var, false);
            int mem = tracker.get_mem_addr(var);
            if (mem <= 0) mem = -mem;
            else mem -= 1;

            printf("%s = %d (reg %d) or %d (mem %d)\n",
                   var.c_str(),
                   runner.get_reg(reg),
                   reg,
                   runner.get_mem(mem),
                   mem);
        }
    }

}
