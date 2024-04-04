addi $28, $0, 0
addi $29, $29, 2047
jal main
add $8, $0, $2
j "23"
malloc:
add $2, $28, $4
add $28, $28, $4
"1":
jr $31
pinMode:
addi $10, $0, 12288
add $10, $10, $4
sw $5, 0($10)
"4":
jr $31
digitalWrite:
addi $10, $0, 4096
add $10, $10, $4
sw $5, 0($10)
"7":
jr $31
digitalRead:
addi $9, $0, 4096
add $9, $9, $4
lw $2, 0($9)
"10":
jr $31
wait:
addi $8, $0, 0
"15":
addi $9, $0, 1000
blt $8, $9, "16"
j "13"
"16":
addi $9, $0, 0
"19":
addi $10, $0, 1000
blt $9, $10, "21"
j "17"
"21":
addi $9, $9, 1
j "19"
"17":
addi $8, $8, 1
j "15"
"13":
jr $31
main:
addi $8, $0, 1
addi $9, $0, 12288
add $9, $9, $8
sw $8, 0($9)
addi $10, $0, 1
addi $11, $0, 4096
addi $12, $11, 1
add $9, $0, $12
"27":
sw $10, 0($9)
sw $0, 0($9)
addi $12, $0, 1
addi $11, $0, 1
add $4, $0, $12
add $5, $0, $11
addi $29, $29, -3
sw $31, 0($29)
sw $10, 1($29)
sw $8, 2($29)
jal digitalWrite
lw $31, 0($29)
lw $8, 1($29)
lw $9, 2($29)
addi $29, $29, 3
addi $10, $0, 1
addi $11, $0, 0
add $4, $0, $10
add $5, $0, $11
addi $29, $29, -3
sw $31, 0($29)
sw $8, 1($29)
sw $9, 2($29)
jal digitalWrite
lw $31, 0($29)
lw $8, 1($29)
lw $9, 2($29)
addi $29, $29, 3
j "27"
"24":
jr $31
"23":
add $0, $0, $0
