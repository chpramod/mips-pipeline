; 
# Fake boot code
#
# $Id: boot.s,v 1.1.1.1 2006/05/23 13:53:49 mainakc Exp $
# 
.set noreorder
.set noat
.globl __boot_code
__boot_code:
#
# Starting address is 0x0
#
#	clear all registers
#
	#li $0,0
	#li $1,0
	#li $2,0
	#li $3,0
	#li $4,0
	#li $5,0
	#li $6,0
	#li $7,0
	#li $8,0
	#li $9,0
	#li $10,0
	#li $11,0
	#li $12,0
	#li $13,0
	#li $14,0
	#li $15,0
	#li $16,0
	#li $17,0
	#li $18,0
	#li $19,0
	#li $20,0
	#li $21,0
	#li $22,0
	#li $23,0
	#li $24,0
	#li $25,0
	#li $26,0
	#li $27,0
	#li $28,0
	#li $29,0
	#li $30,0
	#li $31,0
	li $sp,0x7fffae50	# initial stack pointer
	## backdoor call
	li $2,1181
	li $4,0
	syscall
	li $4,0
	li $2,0
	li $1,ENTRY_POINT
	jalr $1			# jump
	nop
        li $2,1001		# syscall
   	li $4,256
	syscall			# exit (256)
	nop
	nop
.set reorder
.set at
