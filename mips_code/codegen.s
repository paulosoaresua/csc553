# --------------------------
# FUNCTION println
# --------------------------
.align 2
.data
nl: .asciiz "\n"
.align 2
.text
_println:
  li $v0, 1
  lw $a0, 0($sp)
  syscall
  li $v0, 4
  la $a0, nl
  syscall
  jr $ra

# --------------------------
# FUNCTION main
# --------------------------
main: j _main

# --------------------------
# FUNCTION main
# --------------------------
_main:
  la $sp, -8($sp)
  sw $fp, 4($sp)
  sw $ra, 0($sp)
  la $fp, 0($sp)
  la $sp, -8($sp)

  # OP_Assign_Int
  li $t0, 123
  sw $t0, -8($fp)

  # OP_Assign
  lw $t0, -8($fp)
  sw $t0, -4($fp)

  # OP_Param
  lw $t0, -4($fp)
  la $sp, -4($sp)
  sw $t0, 0($sp)

  # OP_Call
  jal _println
  la $sp, 4($sp)

  # OP_Return
  la $sp, 0($fp)
  lw $ra, 0($sp)
  lw $fp, 4($sp)
  la $sp, 8($sp)
  jr $ra