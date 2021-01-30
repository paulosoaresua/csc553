main: j _main
# func println
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
# const int
_main:
	# enter main
	la $sp, -8($sp) # allocate space for old $fp and $ra
    sw $fp, 4($sp)  # save old $fp
    sw $ra, 0($sp)  # save return address
    la $fp, 0($sp)  # set up frame pointer
    la $sp, 0($sp) # allocate stack frame: n = space for locals/temps, in bytes

	li $3, 45
	la $sp -4($sp)
	sw $3 0($sp)
	jal _println  # Jump to function
	la $sp 4($sp) # Pop the actual from the stack

	# leave main
	la $sp, 0($fp) # deallocate locals
    lw $ra, 0($sp) # restore return address
    lw $fp, 4($sp) # restore frame pointer
    la $sp, 8($sp) # restore stack pointer
    jr $ra
