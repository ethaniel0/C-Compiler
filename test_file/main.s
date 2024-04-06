addi $28, $0, 9
addi $29, $29, 2047
addi $8, $0, 0
sw $8, 0($0)
addi $9, $0, 1
sw $9, 1($0)
addi $10, $0, 2
sw $10, 2($0)
addi $11, $0, 3
sw $11, 3($0)
addi $12, $0, 0
sw $8, 4($0)
sw $9, 5($0)
sw $10, 6($0)
sw $11, 7($0)
sw $12, 8($0)
jal main
add $8, $0, $2
j "16"
pinMode:
addi $8, $0, 12288
add $8, $8, $4
sw $5, 0($8)
"1":
jr $31
digitalWrite:
addi $8, $0, 4096
add $8, $8, $4
sw $5, 0($8)
"4":
jr $31
digitalRead:
addi $8, $0, 4096
add $8, $8, $4
lw $2, 0($8)
"7":
jr $31
stepMotor:
addi $8, $0, 38250
addi $9, $0, 3
and $9, $4, $9
addi $10, $0, 0
"12":
blt $10, $9, "13"
j "15"
"13":
sra $11, $8, 4
add $8, $0, $11
"14":
addi $10, $10, 1
j "12"
"15":
addi $11, $0, 1
and $11, $8, $11
sra $12, $8, 1
addi $13, $0, 1
and $13, $12, $13
sra $12, $8, 2
addi $14, $0, 1
and $14, $12, $14
sra $12, $8, 3
addi $15, $0, 1
and $15, $12, $15
sw $11, 4096($0)
sw $13, 4097($0)
sw $14, 4098($0)
sw $15, 4099($0)
"10":
jr $31
main:
addi $8, $0, 0
addi $9, $0, 1
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal pinMode
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 1
addi $9, $0, 1
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal pinMode
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 2
addi $9, $0, 1
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal pinMode
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 3
addi $9, $0, 1
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal pinMode
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 0
addi $9, $0, 0
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal digitalWrite
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 1
addi $9, $0, 0
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal digitalWrite
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 2
addi $9, $0, 0
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal digitalWrite
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 3
addi $9, $0, 0
add $4, $0, $8
add $5, $0, $9
addi $29, $29, -1
sw $31, 0($29)
jal digitalWrite
lw $31, 0($29)
addi $29, $29, 1
addi $8, $0, 18
"19":
addi $9, $0, 1
bne $9, $0, "20"
j "17"
"20":
add $4, $0, $8
addi $29, $29, -3
sw $31, 0($29)
sw $9, 1($29)
sw $8, 2($29)
jal stepMotor
lw $31, 0($29)
lw $9, 1($29)
lw $8, 2($29)
addi $29, $29, 3
addi $8, $8, 1

j "19"
"17":
jr $31
"16":
add $0, $0, $0
