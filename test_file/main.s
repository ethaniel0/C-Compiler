addi $28, $0, 0
addi $29, $29, 2047
addi $8, $0, 1
sw $8, 12288($0)
sw $8, 12289($0)
sw $8, 12290($0)
sw $8, 12291($0)
sw $0, 4096($0)
sw $0, 4097($0)
sw $0, 4098($0)
sw $0, 4099($0)
sw $0, 0($0)
j mainloop
step:
addi $8, $0, 37740
lw $9, 0($0)
addi $10, $0, 3
and $10, $9, $10
addi $11, $0, 0
step_loop_start:
bne $11, $10, step_loop_end
sra $8, $8, 4
addi $11, $11, 1
j step_loop_start
step_loop_end:

addi $12, $0, 1
and $13, $13, $12
sw $13, 4096($0)
sra $13, $8, 1
and $13, $8, $12
sw $13, 4097($0)
sra $13, $8, 2
and $13, $8, $12
sw $13, 4098($0)
sra $13, $8, 4
and $13, $8, $12
sw $13, 4099($0)
addi $9, $9, 1
addi $13, $0, 3
and $9, $9, $13
sw $9, 0($0)
jr $31
mainloop:
jal step
j mainloop
