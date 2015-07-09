.data
_prompt: .asciiz "Enter a integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
  li $v0, 4
  la $a0, _prompt
  syscall
  li $v0, 5
  syscall
  move $t0, $v0
  jr $ra

write:
  li $v0, 1
  syscall
  li $v0, 4
  la $a0, _ret
  syscall
  move $v0, $0
  jr $ra

fact:
  move $fp, $sp
  lw $t0, 16($fp)
  sw $t0, -0($fp)
  addi $sp, $sp, -4
  lw $t0, -0($fp)
  sw $t0, -4($fp)
  addi $sp, $sp, -4
  lw $t0, -4($fp)
  li $t1, 1
  beq $t0, $t1, label1
  j label2
label1:
  lw $t0, -0($fp)
  sw $t0, -8($fp)
  addi $sp, $sp, -4
  lw $t0, -8($fp)
  lw $sp, 4($fp)
  lw $fp, 8($fp)
  jr $ra
  j label3
label2:
  lw $t0, -0($fp)
  sw $t0, -12($fp)
  addi $sp, $sp, -4
  lw $t0, -0($fp)
  sw $t0, -16($fp)
  addi $sp, $sp, -4
  lw $t0, -16($fp)
  li $t1, 1
  sub $t1, $t0, $t1
  sw $t1, -20($fp)
  addi $sp, $sp, -4
  lw $t0, -20($fp)
  sw $t0, -0($sp)
  sw $ra, -4($sp)
  sw $fp, -8($sp)
  sw $sp, -12($sp)
  addi $sp, $sp, -16
  jal fact
  lw $ra, -4($sp)
  sw $t0, -24($fp)
  addi $sp, $sp, -4
  lw $t0, -12($fp)
  lw $t1, -24($fp)
  mul $t1, $t0, $t1
  sw $t1, -28($fp)
  addi $sp, $sp, -4
  lw $t0, -28($fp)
  lw $sp, 4($fp)
  lw $fp, 8($fp)
  jr $ra
label3:
main:
  move $fp, $sp
  sw $ra, 0($sp)
  jal read
  lw $ra, 0($sp)
  sw $t0, -0($fp)
  addi $sp, $sp, -4
  lw $t0, -0($fp)
  sw $t0, -4($fp)
  addi $sp, $sp, -4
  lw $t0, -4($fp)
  sw $t0, -8($fp)
  addi $sp, $sp, -4
  lw $t0, -8($fp)
  li $t1, 1
  bgt $t0, $t1, label4
  j label5
label4:
  lw $t0, -4($fp)
  sw $t0, -12($fp)
  addi $sp, $sp, -4
  lw $t0, -12($fp)
  sw $t0, -0($sp)
  sw $ra, -4($sp)
  sw $fp, -8($sp)
  sw $sp, -12($sp)
  addi $sp, $sp, -16
  jal fact
  lw $ra, -4($sp)
  sw $t0, -16($fp)
  addi $sp, $sp, -4
  lw $t0, -16($fp)
  sw $t0, -20($fp)
  addi $sp, $sp, -4
  j label6
label5:
  li $t0, 1
  sw $t0, -20($fp)
label6:
  lw $t0, -20($fp)
  sw $t0, -24($fp)
  addi $sp, $sp, -4
  lw $t0, -24($fp)
  move $a0, $t0
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  li $t0, 0
  jr $ra
