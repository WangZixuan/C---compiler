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

main:
  move $fp, $sp
  li $t0, 0
  sw $t0, -0($fp)
  addi $sp, $sp, -4
  li $t0, 1
  sw $t0, -4($fp)
  addi $sp, $sp, -4
  li $t0, 0
  sw $t0, -8($fp)
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal read
  lw $ra, 0($sp)
  sw $t0, -12($fp)
  addi $sp, $sp, -4
  lw $t0, -12($fp)
  sw $t0, -16($fp)
  addi $sp, $sp, -4
label1:
  lw $t0, -8($fp)
  sw $t0, -20($fp)
  addi $sp, $sp, -4
  lw $t0, -16($fp)
  sw $t0, -24($fp)
  addi $sp, $sp, -4
  lw $t0, -20($fp)
  lw $t1, -24($fp)
  blt $t0, $t1, label2
  j label3
label2:
  lw $t0, -0($fp)
  sw $t0, -28($fp)
  addi $sp, $sp, -4
  lw $t0, -4($fp)
  sw $t0, -32($fp)
  addi $sp, $sp, -4
  lw $t0, -28($fp)
  lw $t1, -32($fp)
  add $t1, $t0, $t1
  sw $t1, -36($fp)
  addi $sp, $sp, -4
  lw $t0, -36($fp)
  sw $t0, -40($fp)
  addi $sp, $sp, -4
  lw $t0, -4($fp)
  sw $t0, -44($fp)
  addi $sp, $sp, -4
  lw $t0, -44($fp)
  move $a0, $t0
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  lw $t0, -4($fp)
  sw $t0, -48($fp)
  addi $sp, $sp, -4
  lw $t0, -48($fp)
  sw $t0, -0($fp)
  lw $t0, -40($fp)
  sw $t0, -52($fp)
  addi $sp, $sp, -4
  lw $t0, -52($fp)
  sw $t0, -4($fp)
  lw $t0, -8($fp)
  sw $t0, -56($fp)
  addi $sp, $sp, -4
  lw $t0, -56($fp)
  li $t1, 1
  add $t1, $t0, $t1
  sw $t1, -60($fp)
  addi $sp, $sp, -4
  lw $t0, -60($fp)
  sw $t0, -8($fp)
  j label1
label3:
  li $t0, 0
  jr $ra
