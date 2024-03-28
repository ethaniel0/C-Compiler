//
// Created by Ethan Horowitz on 3/15/24.
//

#include "MipsRunner.h"

int MipsRunner::run(int maxIter) {
    int count = 0;
    while(pc < imem_size && count < maxIter){
        uint32_t next_pc = pc + 1;
        imem[pc]->execute(dmem, &regfile, &next_pc);
        pc = next_pc;
        count++;
    }
    return count;
}