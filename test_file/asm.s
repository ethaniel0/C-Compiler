addi $28, $0, 0
addi $29, $29, 2047
addi $29, $0, 2047
jal main
main:
addi $4, $0, 0
addi $5, $0, 1
jal pinMode
addi $4, $0, 1
jal pinMode
addi $4, $0, 2
jal pinMode
addi $4, $0, 3
jal pinMode
addi $4, $0, 4
addi $5, $0, 0
jal pinMode
addi $4, $0, 5
jal pinMode
mainLoop:
addi $4, $0, 0
addi $5, $0, 1
jal digitalWrite
addi $4, $0, 100
jal delayMicros
addi $4, $0, 1
addi $5, $0, 0
jal digitalWrite
addi $4, $0, 100
jal delayMicros
j mainLoop
pinMode:
sw $5, 12288($4)
jr $31
digitalWrite:
sw $5, 4096($4)
jr $31
delayMicros:
addi $8, $3, 0
sub $9, $3, $8
delayMicrosLoopStart:
blt $4, $9, end
sub $9, $3, $8
j delayMicrosLoopStart
end:
jr $31
