addi $28, $0, 2058
addi $29, $29, 2047
addi $8, $0, 0
addi $9, $0, 513
addi $10, $0, 1026
addi $11, $0, 1539
addi $12, $0, 0
addi $13, $0, 0
sw $12, 2052($0)
sw $13, 2053($0)
sw $8, 2054($0)
sw $10, 2055($0)
sw $9, 2056($0)
sw $11, 2057($0)
jal main
add $8, $0, $2
j "82"
abs:
addi $8, $0, 0
blt $4, $8, "3"
j "4"
"3":
addi $8, $0, -1
mul $8, $4, $8
add $2, $0, $8
j "1"
"4":
add $2, $0, $4
"1":
jr $31
log_2:
addi $8, $0, 0
"8":
sra $4, $4, 1
bne $4, $0, "9"
j "10"
"9":
addi $8, $8, 1
j "8"
"10":
add $2, $0, $8
jr $31
initBoids:
addi $8, $0, 0
"14":
addi $9, $0, 512
blt $8, $9, "15"
j "12"
"15":
addi $10, $0, 2048
sll $10, $10, 16
addi $10, $10, 0
add $9, $8, $10
lw $10, 2054($0)
add $8, $8, $10
sw $9, 0($8)
addi $9, $0, 1
addi $12, $0, 2048
sll $12, $12, 16
addi $12, $12, 0
add $11, $9, $12
lw $9, 2056($0)
add $8, $8, $9
sw $11, 0($8)
addi $11, $0, 320
sll $11, $11, 16
addi $11, $11, 0
lw $12, 2055($0)
add $8, $8, $12
sw $11, 0($8)
addi $11, $0, 320
sll $11, $11, 16
addi $11, $11, 0
lw $13, 2057($0)
add $8, $8, $13
sw $11, 0($8)
addi $8, $8, 1
j "14"
"12":
sw $10, 2054($0)
sw $12, 2055($0)
sw $9, 2056($0)
sw $13, 2057($0)
jr $31
keepWithinBounds:
lw $8, 2054($0)
add $4, $4, $8
lw $9, 0($4)
addi $10, $0, 2048
sll $10, $10, 16
addi $10, $10, 0
blt $9, $10, "21"
add $4, $4, $8
lw $10, 0($4)
addi $9, $0, 14336
sll $9, $9, 16
addi $9, $9, 0
sgt $9, $10, $9
bne $9, $0, "23"
j "22"
"23":
addi $9, $0, 32
sll $9, $9, 16
addi $9, $9, 0
lw $10, 2052($0)
sub $10, $10, $9
j "22"
"21":
addi $9, $0, 32
sll $9, $9, 16
addi $9, $9, 0
add $10, $10, $9
"22":
lw $11, 2056($0)
add $4, $4, $11
lw $12, 0($4)
addi $13, $0, 2048
sll $13, $13, 16
addi $13, $13, 0
blt $12, $13, "24"
add $4, $4, $11
lw $13, 0($4)
addi $12, $0, 14336
sll $12, $12, 16
addi $12, $12, 0
sgt $12, $13, $12
bne $12, $0, "26"
j "19"
"26":
addi $12, $0, 32
sll $12, $12, 16
addi $12, $12, 0
lw $13, 2053($0)
sub $13, $13, $12
j "19"
"24":
addi $12, $0, 32
sll $12, $12, 16
addi $12, $12, 0
add $13, $13, $12
"19":
sw $10, 2052($0)
sw $13, 2053($0)
sw $8, 2054($0)
sw $11, 2056($0)
jr $31
distance:
lw $8, 2054($0)
add $4, $4, $8
lw $9, 0($4)
add $5, $5, $8
lw $10, 0($5)
sub $10, $9, $10
addi $9, $4, 0
add $4, $0, $10
addi $29, $29, -4
sw $31, 0($29)
sw $8, 1($29)
sw $9, 2($29)
sw $5, 3($29)
jal abs
lw $31, 0($29)
lw $8, 1($29)
lw $9, 2($29)
lw $5, 3($29)
addi $29, $29, 4
add $10, $0, $2
lw $11, 2056($0)
add $9, $9, $11
lw $12, 0($9)
add $5, $5, $11
lw $13, 0($5)
sub $13, $12, $13
add $4, $0, $13
addi $29, $29, -6
sw $31, 0($29)
sw $8, 1($29)
sw $11, 2($29)
sw $10, 3($29)
sw $9, 4($29)
sw $5, 5($29)
jal abs
lw $31, 0($29)
lw $8, 1($29)
lw $11, 2($29)
lw $10, 3($29)
lw $9, 4($29)
lw $5, 5($29)
addi $29, $29, 6
add $12, $0, $2
add $12, $10, $12
add $2, $0, $12
sw $8, 2054($0)
sw $11, 2056($0)
jr $31
fly_towards_center:
addi $8, $0, 0
addi $9, $0, 0
addi $10, $0, 0
"33":
addi $11, $0, 4
blt $10, $11, "34"
j "36"
"34":
add $10, $10, $5
lw $11, 0($10)
lw $12, 2054($0)
add $11, $11, $12
lw $13, 0($11)
sra $14, $13, 2
add $8, $8, $14
lw $14, 2056($0)
add $11, $11, $14
lw $15, 0($11)
sra $16, $15, 2
add $9, $9, $16
addi $10, $10, 1
j "33"
"36":
add $4, $4, $12
lw $16, 0($4)
sub $16, $8, $16
sra $17, $16, 6
lw $18, 2052($0)
add $18, $18, $17
add $4, $4, $14
lw $17, 0($4)
sub $17, $9, $17
sra $19, $17, 6
lw $20, 2053($0)
add $20, $20, $19
sw $18, 2052($0)
sw $20, 2053($0)
sw $12, 2054($0)
sw $14, 2056($0)
jr $31
avoid_others:
addi $8, $0, 0
addi $9, $0, 0
addi $10, $0, 0
"40":
addi $11, $0, 4
blt $10, $11, "41"
j "43"
"41":
add $10, $10, $5
lw $11, 0($10)
addi $12, $4, 0
add $4, $0, $12
addi $13, $5, 0
add $5, $0, $11
addi $29, $29, -7
sw $31, 0($29)
sw $12, 1($29)
sw $10, 2($29)
sw $11, 3($29)
sw $13, 4($29)
sw $8, 5($29)
sw $9, 6($29)
jal distance
lw $31, 0($29)
lw $12, 2($29)
lw $14, 3($29)
addi $8, $12, 0
lw $12, 1($29)
lw $10, 2($29)
lw $11, 3($29)
lw $13, 4($29)
addi $9, $8, 0
lw $8, 5($29)
addi $15, $9, 0
lw $9, 6($29)
addi $29, $29, 7
add $16, $0, $2
addi $17, $0, 320
sll $17, $17, 16
addi $17, $17, 0
blt $16, $17, "44"
j "42"
"44":
add $12, $12, $15
lw $17, 0($12)
add $11, $11, $15
lw $16, 0($11)
sub $16, $17, $16
add $8, $8, $16
add $12, $12, $14
lw $16, 0($12)
add $11, $11, $14
lw $17, 0($11)
sub $17, $16, $17
add $9, $9, $17
"42":
addi $10, $10, 1
j "40"
"43":
sra $17, $8, 4
lw $16, 2052($0)
add $16, $16, $17
sra $17, $9, 4
lw $18, 2053($0)
add $18, $18, $17
sw $16, 2052($0)
sw $18, 2053($0)
sw $15, 2054($0)
sw $14, 2056($0)
jr $31
match_velocity:
addi $8, $0, 0
addi $9, $0, 0
addi $10, $0, 0
"49":
addi $11, $0, 4
blt $10, $11, "50"
j "52"
"50":
add $10, $10, $5
lw $11, 0($10)
lw $12, 2055($0)
add $11, $11, $12
lw $13, 0($11)
sra $14, $13, 2
add $8, $8, $14
lw $14, 2057($0)
add $11, $11, $14
lw $15, 0($11)
sra $16, $15, 2
add $9, $9, $16
addi $10, $10, 1
j "49"
"52":
lw $16, 2052($0)
add $16, $16, $8
sra $17, $16, 2
lw $18, 2053($0)
add $18, $18, $9
sra $19, $18, 2
sw $16, 2052($0)
sw $18, 2053($0)
sw $12, 2055($0)
sw $14, 2057($0)
jr $31
limit_speed:
lw $8, 2052($0)
add $4, $0, $8
addi $29, $29, -2
sw $31, 0($29)
sw $8, 1($29)
jal abs
lw $31, 0($29)
lw $8, 1($29)
lw $15, -3($29)
lw $14, -2($29)
addi $29, $29, 2
add $9, $0, $2
lw $10, 2053($0)
add $4, $0, $10
addi $29, $29, -6
sw $31, 0($29)
sw $8, 1($29)
sw $10, 2($29)
sw $15, 3($29)
sw $14, 4($29)
sw $9, 5($29)
jal abs
lw $31, 0($29)
lw $8, 1($29)
lw $10, 2($29)
lw $15, 3($29)
lw $14, 4($29)
lw $9, 5($29)
addi $29, $29, 6
add $11, $0, $2
add $11, $9, $11
add $4, $0, $11
addi $29, $29, -6
sw $31, 0($29)
sw $8, 1($29)
sw $10, 2($29)
sw $15, 3($29)
sw $14, 4($29)
sw $11, 5($29)
jal log_2
lw $31, 0($29)
lw $8, 1($29)
lw $10, 2($29)
lw $15, 3($29)
lw $14, 4($29)
lw $11, 5($29)
addi $29, $29, 6
add $9, $0, $2
addi $12, $0, 23
sub $12, $9, $12
addi $13, $0, 0
blt $12, $13, "56"
j "57"
"56":
addi $13, $0, 0
add $2, $0, $13
j "54"
"57":
addi $16, $0, 0
"58":
blt $16, $12, "59"
j "54"
"59":
sra $8, $8, 1
sra $10, $10, 1
addi $16, $16, 1
j "58"
"54":
sw $8, 2052($0)
sw $10, 2053($0)
sw $15, 2054($0)
sw $14, 2056($0)
jr $31
updateBoids:
addi $8, $0, 0
"65":
addi $9, $0, 512
blt $8, $9, "66"
j "63"
"66":
addi $29, $29, -4
addi $9, $0, -1
sw $9, 0($29)
addi $10, $29, 0
addi $11, $0, 0
"69":
addi $12, $0, 512
blt $11, $12, "70"
j "72"
"70":
add $12, $0, $11
addi $13, $0, 0
"73":
addi $14, $0, 4
blt $13, $14, "74"
j "71"
"74":
add $13, $13, $10
lw $14, 0($13)
addi $15, $0, -1
bne $14, $15, "78"
add $13, $13, $10
sw $12, 0($13)
j "71"
"78":
add $4, $0, $8
add $5, $0, $12
addi $29, $29, -8
sw $31, 0($29)
sw $9, 1($29)
sw $15, 2($29)
sw $8, 3($29)
sw $11, 4($29)
sw $13, 5($29)
sw $10, 6($29)
sw $12, 7($29)
jal distance
lw $31, 0($29)
lw $8, 7($29)
lw $10, 8($29)
lw $15, 9($29)
lw $14, 10($29)
lw $9, 1($29)
addi $11, $15, 0
lw $15, 2($29)
addi $12, $8, 0
lw $8, 3($29)
addi $13, $11, 0
lw $11, 4($29)
addi $16, $13, 0
lw $13, 5($29)
addi $17, $10, 0
lw $10, 6($29)
addi $18, $12, 0
lw $12, 7($29)
addi $29, $29, 8
add $19, $0, $2
add $13, $13, $10
lw $20, 0($13)
add $4, $0, $8
add $5, $0, $20
addi $29, $29, -13
sw $31, 0($29)
sw $18, 1($29)
sw $17, 2($29)
sw $16, 3($29)
sw $14, 4($29)
sw $9, 5($29)
sw $15, 6($29)
sw $19, 7($29)
sw $8, 8($29)
sw $11, 9($29)
sw $13, 10($29)
sw $10, 11($29)
sw $12, 12($29)
jal distance
lw $31, 0($29)
lw $18, 1($29)
lw $17, 2($29)
lw $16, 3($29)
lw $14, 4($29)
lw $9, 5($29)
lw $15, 6($29)
lw $19, 7($29)
lw $8, 8($29)
lw $11, 9($29)
lw $13, 10($29)
lw $10, 11($29)
lw $12, 12($29)
addi $29, $29, 13
add $20, $0, $2
blt $19, $20, "80"
j "75"
"80":
add $13, $13, $10
lw $20, 0($13)
add $13, $13, $10
sw $12, 0($13)
j "71"
add $12, $0, $20
"75":
addi $13, $13, 1
j "73"
"71":
addi $11, $11, 1
j "69"
"72":
lw $19, 2055($0)
add $8, $8, $19
lw $21, 0($8)
add $18, $0, $21
lw $21, 2057($0)
add $8, $8, $21
lw $22, 0($8)
add $17, $0, $22
add $4, $0, $8
add $5, $0, $10
addi $29, $29, -15
sw $31, 0($29)
sw $18, 1($29)
sw $17, 2($29)
sw $16, 3($29)
sw $19, 4($29)
sw $14, 5($29)
sw $21, 6($29)
sw $9, 7($29)
sw $15, 8($29)
sw $8, 9($29)
sw $11, 10($29)
sw $13, 11($29)
sw $10, 12($29)
sw $12, 13($29)
sw $20, 14($29)
jal fly_towards_center
lw $31, 0($29)
lw $18, 1($29)
lw $17, 2($29)
lw $16, 3($29)
lw $19, 4($29)
lw $14, 5($29)
lw $21, 6($29)
lw $9, 7($29)
lw $15, 8($29)
lw $8, 9($29)
lw $11, 10($29)
lw $13, 11($29)
lw $10, 12($29)
lw $12, 13($29)
lw $20, 14($29)
addi $29, $29, 15
add $4, $0, $8
add $5, $0, $10
addi $29, $29, -15
sw $31, 0($29)
sw $18, 1($29)
sw $17, 2($29)
sw $16, 3($29)
sw $19, 4($29)
sw $14, 5($29)
sw $21, 6($29)
sw $9, 7($29)
sw $15, 8($29)
sw $8, 9($29)
sw $11, 10($29)
sw $13, 11($29)
sw $10, 12($29)
sw $12, 13($29)
sw $20, 14($29)
jal avoid_others
lw $31, 0($29)
lw $18, 1($29)
lw $17, 2($29)
lw $16, 3($29)
lw $19, 4($29)
lw $14, 5($29)
lw $21, 6($29)
lw $9, 7($29)
lw $15, 8($29)
lw $8, 9($29)
lw $11, 10($29)
lw $13, 11($29)
lw $10, 12($29)
lw $12, 13($29)
lw $20, 14($29)
addi $29, $29, 15
add $4, $0, $8
add $5, $0, $10
addi $29, $29, -15
sw $31, 0($29)
sw $18, 1($29)
sw $17, 2($29)
sw $16, 3($29)
sw $19, 4($29)
sw $14, 5($29)
sw $21, 6($29)
sw $9, 7($29)
sw $15, 8($29)
sw $8, 9($29)
sw $11, 10($29)
sw $13, 11($29)
sw $10, 12($29)
sw $12, 13($29)
sw $20, 14($29)
jal match_velocity
lw $31, 0($29)
lw $18, 1($29)
lw $17, 2($29)
lw $16, 3($29)
lw $19, 4($29)
lw $14, 5($29)
lw $21, 6($29)
lw $9, 7($29)
lw $15, 8($29)
lw $8, 9($29)
lw $11, 10($29)
lw $13, 11($29)
lw $10, 12($29)
lw $12, 13($29)
lw $20, 14($29)
addi $29, $29, 15
addi $29, $29, -15
sw $31, 0($29)
sw $18, 1($29)
sw $17, 2($29)
sw $16, 3($29)
sw $19, 4($29)
sw $14, 5($29)
sw $21, 6($29)
sw $9, 7($29)
sw $15, 8($29)
sw $8, 9($29)
sw $11, 10($29)
sw $13, 11($29)
sw $10, 12($29)
sw $12, 13($29)
sw $20, 14($29)
jal limit_speed
lw $31, 0($29)
lw $18, 1($29)
lw $17, 2($29)
lw $16, 3($29)
lw $19, 4($29)
lw $14, 5($29)
lw $21, 6($29)
lw $9, 7($29)
lw $15, 8($29)
lw $8, 9($29)
lw $11, 10($29)
lw $13, 11($29)
lw $10, 12($29)
lw $12, 13($29)
lw $20, 14($29)
addi $29, $29, 15
add $4, $0, $8
addi $29, $29, -15
sw $31, 0($29)
sw $18, 1($29)
sw $17, 2($29)
sw $16, 3($29)
sw $19, 4($29)
sw $14, 5($29)
sw $21, 6($29)
sw $9, 7($29)
sw $15, 8($29)
sw $8, 9($29)
sw $11, 10($29)
sw $13, 11($29)
sw $10, 12($29)
sw $12, 13($29)
sw $20, 14($29)
jal keepWithinBounds
lw $31, 0($29)
lw $18, 1($29)
lw $17, 2($29)
lw $16, 3($29)
lw $19, 4($29)
lw $14, 5($29)
lw $21, 6($29)
lw $9, 7($29)
lw $15, 8($29)
lw $8, 9($29)
lw $11, 10($29)
lw $13, 11($29)
lw $10, 12($29)
lw $12, 13($29)
lw $20, 14($29)
addi $29, $29, 15
add $8, $8, $19
sw $18, 0($8)
add $8, $8, $21
sw $17, 0($8)
add $8, $8, $16
lw $22, 0($8)
add $8, $8, $19
lw $23, 0($8)
add $22, $22, $23
add $8, $8, $14
lw $23, 0($8)
add $8, $8, $21
lw $22, 0($8)
add $23, $23, $22
addi $8, $8, 1
j "65"
"63":
sw $18, 2052($0)
sw $17, 2053($0)
sw $16, 2054($0)
sw $19, 2055($0)
sw $14, 2056($0)
sw $21, 2057($0)
addi $29, $29, 4
jr $31
main:
addi $29, $29, -1
sw $31, 0($29)
jal initBoids
lw $31, 0($29)
lw $18, -17($29)
lw $17, -16($29)
lw $16, -15($29)
lw $19, -14($29)
lw $14, -13($29)
lw $21, -12($29)
addi $29, $29, 1
addi $8, $0, 1
"85":
bne $8, $0, "86"
j "87"
"86":
addi $29, $29, -8
sw $31, 0($29)
sw $18, 1($29)
sw $17, 2($29)
sw $16, 3($29)
sw $19, 4($29)
sw $14, 5($29)
sw $21, 6($29)
sw $8, 7($29)
jal updateBoids
lw $31, 0($29)
lw $18, 1($29)
lw $17, 2($29)
lw $16, 3($29)
lw $19, 4($29)
lw $14, 5($29)
lw $21, 6($29)
lw $8, 7($29)
addi $29, $29, 8
j "85"
"87":
addi $9, $0, 0
add $2, $0, $9
sw $18, 2052($0)
sw $17, 2053($0)
sw $16, 2054($0)
sw $19, 2055($0)
sw $14, 2056($0)
sw $21, 2057($0)
jr $31
"82":
add $0, $0, $0
