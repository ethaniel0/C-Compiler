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
jal pinMode
addi $4, $0, 5
addi $5, $0, 0
jal pinMode
mainLoop:
jal readUSSensor
bne $2, $0, dispHigh
addi $4, $0, 0
addi $5, $0, 0
jal digitalWrite
j mainLoop
dispHigh:
addi $4, $0, 0
addi $5, $0, 1
jal digitalWrite
j mainLoop
pinMode:
sw $5, 12288($4)
jr $31
digitalWrite:
sw $5, 4096($4)
jr $31
digitalRead:
lw $2, 4096($4)
jr $31
delayMicros:
addi $8, $3, 0
sub $9, $3, $8
delayMicrosLoopStart:
blt $4, $9, delayMicrosEnd
sub $9, $3, $8
j delayMicrosLoopStart
delayMicrosEnd:
jr $31
pulseIn:
addi $29, $29, -1
sw $31, 0($29)
addi $8, $4, 0
addi $9, $5, 0
addi $10, $6, 0
addi $11, $3, 0
pulseInStart:
addi $4, $8, 0
jal digitalRead
pulseWait:
bne $2, $9, pulseWaitBody
j pulseWaitEnd
pulseWaitBody:
sub $12, $3, $11
blt $10, $12, pulseInTimeout
j pulseInStart
pulseWaitEnd:
addi $11, $3, 0
pulseReadStart:
addi $4, $8, 0
jal digitalRead
bne $2, $9, pulseReadEnd
sub $12, $3, $11
blt $10, $12, pulseInTimeout
j pulseReadStart
pulseReadEnd:
lw $31, 0($29)
addi $29, $29, 1
sub $2, $3, $11
jr $31
pulseInTimeout:
lw $31, 0($29)
addi $29, $29, 1
addi $2, $0, 0
jr $31
readUSSensor:
addi $29, $29, -1
sw $31, 0($29)
addi $8, $0, 4
addi $9, $0, 5
addi $4, $8, 0
addi $5, $0, 0
jal digitalWrite
addi $29, $29, -2
sw $8, 0($29)
sw $9, 1($29)
addi $4, $0, 2
jal delayMicros
lw $8, 0($29)
lw $9, 1($29)
addi $29, $29, 2
addi $4, $8, 0
addi $5, $0, 1
jal digitalWrite
addi $29, $29, -2
sw $8, 0($29)
sw $9, 1($29)
addi $4, $0, 10
jal delayMicros
lw $8, 0($29)
lw $9, 1($29)
addi $29, $29, 2
addi $4, $8, 0
addi $5, $0, 0
jal digitalWrite
addi $4, $9, 0
addi $5, $0, 1
addi $6, $0, 23539
jal pulseIn
addi $8, $0, 1
div $2, $2, $8
lw $31, 0($29)
addi $29, $29, 1
jr $31
