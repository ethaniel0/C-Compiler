addi $28, $0, 1
addi $29, $29, 2047
addi $8, $0, 0
sw $8, 0($0)
jal main
add $8, $0, $2
j "53"
delayMicroseconds:
addi $8, $3, 0
sub $9, $3, $8
delayMicrosLoopStart:
blt $4, $9, "1"
sub $9, $3, $8
j delayMicrosLoopStart
"1":
jr $31
pulseIn:
addi $8, $3, 0
"7":
lw $9, 4096($4)
bne $9, $5, "8"
j "9"
"8":
addi $10, $3, 0
sub $11, $10, $8
blt $6, $11, "12"
j "7"
"12":
addi $11, $0, 0
add $2, $0, $11
j "4"
"9":
addi $12, $3, 0
add $8, $0, $12
"15":
lw $12, 4096($4)
bne $12, $5, "17"
addi $13, $3, 0
sub $14, $13, $8
blt $6, $14, "21"
j "15"
"21":
addi $14, $0, 0
add $2, $0, $14
j "4"
"17":
addi $15, $3, 0
sub $15, $15, $8
add $2, $0, $15
"4":
jr $31
addi $8, $0, 1042
add $9, $0, $4
addi $10, $0, 1
sw $10, 4096($4)
add $4, $0, $8
addi $29, $29, -4
sw $31, 0($29)
sw $8, 1($29)
sw $9, 2($29)
sw $5, 3($29)
jal delayMicroseconds
lw $31, 0($29)
lw $8, 1($29)
lw $9, 2($29)
lw $5, 3($29)
addi $29, $29, 4
addi $10, $0, 0
"28":
addi $11, $0, 8
blt $10, $11, "29"
j "25"
"29":
add $11, $0, $9
addi $12, $0, 1
and $12, $5, $12
sw $12, 4096($9)
addi $10, $10, 1
j "28"
"25":
jr $31
stepMotor:
addi $8, $0, 37740
addi $9, $0, 3
lw $10, 0($0)
and $9, $10, $9
addi $11, $0, 0
"36":
blt $11, $9, "37"
j "39"
"37":
sra $12, $8, 4
add $8, $0, $12
addi $11, $11, 1
j "36"
"39":
addi $12, $0, 0
addi $13, $0, 1
and $13, $8, $13
sw $13, 4096($12)
addi $13, $0, 1
sra $12, $8, 1
addi $14, $0, 1
and $14, $12, $14
sw $14, 4096($13)
addi $14, $0, 2
sra $13, $8, 2
addi $12, $0, 1
and $12, $13, $12
sw $12, 4096($14)
addi $12, $0, 3
sra $14, $8, 3
addi $13, $0, 1
and $13, $14, $13
sw $13, 4096($12)
addi $13, $10, 1
addi $12, $0, 3
and $12, $13, $12
add $10, $0, $12
sw $10, 0($0)
jr $31
us_sensor_dist:
addi $8, $0, 4
addi $9, $0, 5
addi $10, $0, 0
sw $10, 4096($8)
addi $10, $0, 2
add $4, $0, $10
addi $29, $29, -3
sw $31, 0($29)
sw $9, 1($29)
sw $8, 2($29)
jal delayMicroseconds
lw $31, 0($29)
lw $9, 1($29)
lw $8, 2($29)
addi $29, $29, 3
addi $10, $0, 1
sw $10, 4096($8)
addi $10, $0, 10
add $4, $0, $10
addi $29, $29, -3
sw $31, 0($29)
sw $9, 1($29)
sw $8, 2($29)
jal delayMicroseconds
lw $31, 0($29)
lw $9, 1($29)
lw $8, 2($29)
addi $29, $29, 3
addi $10, $0, 0
sw $10, 4096($8)
addi $10, $0, 1
addi $11, $0, 23529
add $4, $0, $9
add $5, $0, $10
add $6, $0, $11
addi $29, $29, -3
sw $31, 0($29)
sw $9, 1($29)
sw $8, 2($29)
jal pulseIn
lw $31, 0($29)
lw $9, 1($29)
lw $8, 2($29)
addi $29, $29, 3
add $10, $0, $2
add $2, $0, $10
jr $31
jr $31
main:
addi $8, $0, 0
addi $9, $0, 1
sw $9, 12288($8)
addi $9, $0, 1
addi $8, $0, 1
sw $8, 12288($9)
addi $8, $0, 2
addi $9, $0, 1
sw $9, 12288($8)
addi $9, $0, 3
addi $8, $0, 1
sw $8, 12288($9)
addi $8, $0, 4
addi $9, $0, 1
sw $9, 12288($8)
addi $9, $0, 5
addi $8, $0, 0
sw $8, 12288($9)
addi $8, $0, 8
addi $9, $0, 1
sw $9, 12288($8)
addi $9, $0, 9
addi $8, $0, 1
sw $8, 12288($9)
addi $8, $0, 10
addi $9, $0, 1
sw $9, 12288($8)
addi $9, $0, 11
addi $8, $0, 1
sw $8, 12288($9)
addi $8, $0, 0
addi $9, $0, 0
sw $9, 4096($8)
addi $9, $0, 1
addi $8, $0, 0
sw $8, 4096($9)
addi $8, $0, 2
addi $9, $0, 0
sw $9, 4096($8)
addi $9, $0, 3
addi $8, $0, 0
sw $8, 4096($9)
addi $8, $0, 5000
addi $9, $0, 1
sll $9, $9, 16
addi $1, $0, 17232
sll $1, $1, 1
addi $1, $1, 0
or $9, $9, $1
"70":
addi $10, $0, 1
bne $10, $0, "71"
j "54"
"71":
addi $11, $0, 0
"73":
addi $12, $0, 64
blt $11, $12, "74"
j "76"
"74":
addi $29, $29, -5
sw $31, 0($29)
sw $10, 1($29)
sw $8, 2($29)
sw $11, 3($29)
sw $9, 4($29)
jal stepMotor
lw $31, 0($29)
lw $10, 1($29)
lw $8, 2($29)
lw $11, 3($29)
lw $9, 4($29)
addi $29, $29, 5
add $4, $0, $8
addi $29, $29, -5
sw $31, 0($29)
sw $10, 1($29)
sw $8, 2($29)
sw $11, 3($29)
sw $9, 4($29)
jal delayMicroseconds
lw $31, 0($29)
lw $10, 1($29)
lw $8, 2($29)
lw $11, 3($29)
lw $9, 4($29)
addi $29, $29, 5
addi $11, $11, 1
j "73"
"76":
addi $29, $29, -5
sw $31, 0($29)
sw $10, 1($29)
sw $8, 2($29)
sw $11, 3($29)
sw $9, 4($29)
jal us_sensor_dist
lw $31, 0($29)
lw $10, 1($29)
lw $8, 2($29)
lw $11, 3($29)
lw $9, 4($29)
addi $29, $29, 5
add $12, $0, $2
addi $13, $0, 8
addi $14, $0, 0
sgt $14, $12, $14
sw $14, 4096($13)
addi $14, $0, 9
addi $13, $0, 1000
sgt $13, $12, $13
sw $13, 4096($14)
addi $13, $0, 10
addi $14, $0, 2000
sgt $14, $12, $14
sw $14, 4096($13)
addi $14, $0, 11
addi $13, $0, 3000
sgt $13, $12, $13
sw $13, 4096($14)
add $4, $0, $9
addi $29, $29, -6
sw $31, 0($29)
sw $10, 1($29)
sw $8, 2($29)
sw $12, 3($29)
sw $11, 4($29)
sw $9, 5($29)
jal delayMicroseconds
lw $31, 0($29)
lw $10, 1($29)
lw $8, 2($29)
lw $12, 3($29)
lw $11, 4($29)
lw $9, 5($29)
addi $29, $29, 6
j "70"
"54":
jr $31
"53":
add $0, $0, $0
