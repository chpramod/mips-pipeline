       .section .text
       .globl main
       .ent main
main:
#
# print something
#
        li $4, 1      # parameter passed to printf in $a0
        la $5, lab    # load the address of the string to be printed in $a1
        li $6, 9      # length of the string in $a2
        li $2, 1004   # load the system call number in $v0
        syscall
        nop

#
# exit
#
        li $4,10      # pass exit parameter in $a0
        li $2,1001    # select an exist sycall
        syscall
        nop

lab:    .ascii "hi there\n"
        .end main
