//
// Created by Ethan Horowitz on 3/15/24.
//

#include "mipsTests.h"

void run_mips_tests(){
    RUN_TEST_GROUP("mipsCommands");
}

TEST(mipsCommands, all_commands_once){
    /*
     0  addi $1, $1, 3     // $1 = 3
     1  addi $2, $1, 17    // $2 = 20
     2  add $3, $1, $1     // $3 = 6
     3  sub $4, $2, $1     // $4 = 17
     4  or $5, $1, $2      // $5 = 23
     5  and $6, $1, $3     // $6 = 2
     6  sll $7 $6 2        // $7 = 8
     7  sra $8 $7 2        // $8 = 2
     8  mul $9 $8 $7       // $9 = 16
     9  div $10 $9 $1      // $10 = 5
     10 sw $1 0($0)        // dmem[0] = 3
     11 lw $11 0($0)       // $11 = dmem[0] = 3
     12 j jumpTarget               // jump to 14
badBneTarget:
     13 addi $1, $1, 1     // $1 = 4 (incorrect)
jumpTarget:
     14 addi $12, $7, 24   // $12 = 32
     15 bne $1, $1, badBneTarget     // jump to 13 (incorrect)
     16 bne $1, $2, bneTarget      // jump to 17
badBltTarget:
     17 addi $1, $1, 1     // $1 += 1 (incorrect)
bneTarget:
     18 addi $13, $9 1     // $13 = 17
     19 blt $2, $1, -3     // jump to 17 (incorrect)
     20 blt $1, $2, 1      // jump to 22
badBexTarget:
     21 addi $1, $1, 1     // $1 += 1 (incorrect)
bltTarget:
     22 addi $14, $5, 3    // $14 = 26
     23 bex 21             // jump to 21 (incorrect)
     24 setx 25            // $30 = 25
     25 bex 27             // jump to 27
     26 addi $1 1          // $1 += 1 (incorrect)
bexTarget:
     27 addi $15 $1 1      // $15 = 4
     28 jal 30             // jump to 30, $31 = 29
     29 addi $1 1          // $1 += 1 (incorrect)
jalTarget:
     30 addi $16 $1 30     // $16 = 33
     31 jr $16             // jump to 33
     32 addi $1 $1 1       // $1 += 1
     33 addi $17 $1 6      // $17 = 9

     */

    std::map<std::string, Instruction*> label_map;

    int imem_size = 34;
    Instruction* imem[imem_size];
    auto instr1 = InstrAddi(1, 1, 3);
    auto instr2 = InstrAddi(2, 1, 17);
    auto instr3 = InstrAdd(3, 1, 1);
    auto instr4 = InstrSub(4, 2, 1);
    auto instr5 = InstrOr(5, 1, 2);
    auto instr6 = InstrAnd(6, 1, 3);
    auto instr7 = InstrSll(7, 6, 2);
    auto instr8 = InstrSra(8, 7, 2);
    auto instr9 = InstrMul(9, 8, 7);
    auto instr10 = InstrDiv(10, 9, 1);
    auto instr11 = InstrSw(1, 0, 0);
    auto instr12 = InstrLw(11, 0, 0);
    auto instr13 = InstrJ("jumpTarget");
    auto instr14 = InstrAddi(1, 1, 1);
    label_map["badBneTarget"] = &instr13;
    auto instr15 = InstrAddi(12, 7, 24);
    label_map["jumpTarget"] = &instr15;
    auto instr16 = InstrBne(1, 1, "badBneTarget");
    auto instr17 = InstrBne(1, 2, "bneTarget");
    auto instr18 = InstrAddi(1, 1, 1);
    label_map["badBltTarget"] = &instr18;
    auto instr19 = InstrAddi(13, 9, 1);
    label_map["bneTarget"] = &instr19;
    auto instr20 = InstrBlt(2, 1, "badBltTarget");
    auto instr21 = InstrBlt(1, 2, "bltTarget");
    auto instr22 = InstrAddi(1, 1, 1);
    label_map["badBexTarget"] = &instr22;
    auto instr23 = InstrAddi(14, 5, 3);
    label_map["bltTarget"] = &instr23;
    auto instr24 = InstrBex("badBexTarget");
    auto instr25 = InstrSetx(25);
    auto instr26 = InstrBex("bexTarget");
    auto instr27 = InstrAddi(1, 1, 1);
    auto instr28 = InstrAddi(15, 1, 1);
    label_map["bexTarget"] = &instr28;
    auto instr29 = InstrJal("jalTarget");
    auto instr30 = InstrAddi(1, 1, 1);
    auto instr31 = InstrAddi(16, 1, 30);
    label_map["jalTarget"] = &instr31;
    auto instr32 = InstrJr(16);
    auto instr33 = InstrAddi(1, 1, 1);
    auto instr34 = InstrAddi(17, 1, 6);

    imem[0] = &instr1;
    imem[1] = &instr2;
    imem[2] = &instr3;
    imem[3] = &instr4;
    imem[4] = &instr5;
    imem[5] = &instr6;
    imem[6] = &instr7;
    imem[7] = &instr8;
    imem[8] = &instr9;
    imem[9] = &instr10;
    imem[10] = &instr11;
    imem[11] = &instr12;
    imem[12] = &instr13;
    imem[13] = &instr14;
    imem[14] = &instr15;
    imem[15] = &instr16;
    imem[16] = &instr17;
    imem[17] = &instr18;
    imem[18] = &instr19;
    imem[19] = &instr20;
    imem[20] = &instr21;
    imem[21] = &instr22;
    imem[22] = &instr23;
    imem[23] = &instr24;
    imem[24] = &instr25;
    imem[25] = &instr26;
    imem[26] = &instr27;
    imem[27] = &instr28;
    imem[28] = &instr29;
    imem[29] = &instr30;
    imem[30] = &instr31;
    imem[31] = &instr32;
    imem[32] = &instr33;
    imem[33] = &instr34;

    for (int i = 0; i < imem_size; i++){
        imem[i]->line_num = i;
    }
    for (int i = 0; i < imem_size; i++){
        imem[i]->link_labels(label_map);
    }

    MipsRunner runner(100, imem, imem_size);
    runner.run(imem_size);
    EXPECT_EQ(runner.get_reg(1), 3, %d)
    EXPECT_EQ(runner.get_reg(2), 20, %d)
    EXPECT_EQ(runner.get_reg(3), 6, %d)
    EXPECT_EQ(runner.get_reg(4), 17, %d)
    EXPECT_EQ(runner.get_reg(5), 23, %d)
    EXPECT_EQ(runner.get_reg(6), 2, %d)
    EXPECT_EQ(runner.get_reg(7), 8, %d)
    EXPECT_EQ(runner.get_reg(8), 2, %d)
    EXPECT_EQ(runner.get_reg(9), 16, %d)
    EXPECT_EQ(runner.get_reg(10), 5, %d)
    EXPECT_EQ(runner.get_reg(11), 3, %d)
    EXPECT_EQ(runner.get_mem(0), 3, %d)
    EXPECT_EQ(runner.get_reg(12), 32, %d)
    EXPECT_EQ(runner.get_reg(13), 17, %d)
    EXPECT_EQ(runner.get_reg(14), 26, %d)
    EXPECT_EQ(runner.get_reg(15), 4, %d)
    EXPECT_EQ(runner.get_reg(16), 33, %d)
    EXPECT_EQ(runner.get_reg(17), 9, %d)
    EXPECT_EQ(runner.get_reg(RSTATUS), 25, %d)
    EXPECT_EQ(runner.get_reg(31), 29, %d)
}
