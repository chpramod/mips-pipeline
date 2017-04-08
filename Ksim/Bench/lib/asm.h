#ifndef _ASM_H
#define _ASM_H

#define zero    $0      /* wired zero */
#define AT      $at     /* assembler temp */
#define v0      $2      /* return value, also syscall number */
#define v1      $3
#define a0      $4      /* argument registers */
#define a1      $5
#define a2      $6
#define a3      $7
#define t0      $8      /* caller saved */
#define t1      $9
#define t2      $10
#define t3      $11
#define t4      $12     /* caller saved - 32 bit env arg reg 64 bit */
#define ta0     $12     /* caller saved in 32 bit - arg regs in 64 bit */
#define t5      $13
#define ta1     $13
#define t6      $14
#define ta2     $14
#define t7      $15
#define ta3     $15
#define s0      $16     /* callee saved */
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23
#define t8      $24     /* code generator */
#define t9      $25
#define jp      $25     /* PIC jump register */
#define k0      $26     /* kernel temporary */
#define k1      $27
#define gp      $28     /* global pointer */
#define sp      $29     /* stack pointer */
#define fp      $30     /* frame pointer */
#define s8      $30     /* calle saved */
#define ra      $31     /* return address */

/* General Purpose Registers */
#define REG_ZERO   0
#define REG_AT     1
#define REG_V0     2
#define REG_V1     3
#define REG_A0     4
#define REG_A1     5
#define REG_A2     6
#define REG_A3     7
#define REG_T0     8
#define REG_T1     9
#define REG_T2     10
#define REG_T3     11
#define REG_T4     12
#define REG_T5     13
#define REG_T6     14
#define REG_T7     15
#define REG_S0     16
#define REG_S1     17
#define REG_S2     18
#define REG_S3     19
#define REG_S4     20
#define REG_S5     21
#define REG_S6     22
#define REG_S7     23
#define REG_T8     24
#define REG_T9     25
#define REG_K0     26
#define REG_K1     27
#define REG_GP     28
#define REG_SP     29
#define REG_FP     30
#define REG_S8     30
#define REG_RA     31
#define REG_PC     32
#define REG_NPC    33
#define REG_HI     34
#define REG_LO     35

#define REG_NONE   48

#define REG_TA0    REG_T4
#define REG_TA1    REG_T5
#define REG_TA2    REG_T6
#define REG_TA3    REG_T7

/* A regdef.h style register naming convention */
/* Zero Register */
#define G0 0

/* Assembler Register */
#define ATR 1

/* used for interger return and static link */
#define V0 2
#define V1 3

/* Argument Registers */
#define A0 4
#define A1 5
#define A2 6
#define A3 7

/* Temporary Registers */
#define T0 8
#define T1 9
#define T2 10
#define T3 11
#define T4 12
#define T5 13
#define T6 14
#define T7 15

/* Saved Registers */
#define S0 16
#define S1 17
#define S2 18
#define S3 19
#define S4 20
#define S5 21
#define S6 22
#define S7 23

#define T8 24
#define T9 25

#define K0 26
#define K1 27
#define GP 28

#define SP 29

#define S8 30
#define FP 30

#define RA 31


/* Cop 0 SR */
#define assC0_SR 12

/* Floating Point Coprocessor (1) registers */
/* Each FPR is the concatenation of two FGR's */

#define REG_FPR_0   BASE_FPR_OFFSET+0
#define REG_FPR_2   BASE_FPR_OFFSET+8
#define REG_FPR_4   BASE_FPR_OFFSET+16
#define REG_FPR_6   BASE_FPR_OFFSET+24
#define REG_FPR_8   BASE_FPR_OFFSET+32
#define REG_FPR_10  BASE_FPR_OFFSET+40
#define REG_FPR_12  BASE_FPR_OFFSET+48
#define REG_FPR_14  BASE_FPR_OFFSET+56
#define REG_FPR_16  BASE_FPR_OFFSET+64
#define REG_FPR_18  BASE_FPR_OFFSET+72
#define REG_FPR_20  BASE_FPR_OFFSET+80
#define REG_FPR_22  BASE_FPR_OFFSET+88
#define REG_FPR_24  BASE_FPR_OFFSET+96
#define REG_FPR_26  BASE_FPR_OFFSET+104
#define REG_FPR_28  BASE_FPR_OFFSET+112
#define REG_FPR_30  BASE_FPR_OFFSET+120

/*
 * asm.h -- cpp definitions for assembler files
 */

/*
 * Notes on putting entry pt and frame info into symbol table for debuggers
 *
 *      .ent    name,lex-level  # name is entry pt, lex-level is 0 for c
 * name:                        # actual entry point
 *      .frame  fp,framesize,saved_pc_reg
 *                              # fp -- register which is pointer to base
 *                              #       of previous frame, debuggers are special
 *                              #       cased if "sp" to add "framesize"
 *                              #       (sp is usually used)
 *                              # framesize -- size of frame
 *                              #       the expression:
 *                              #               new_sp + framesize == old_sp
 *                              #       should be true
 *                              # saved_pc_reg -- either a register which
 *                              #       contains callers pc or $0, if $0
 *                              #       saved pc is assumed to be in
 *                              #       (fp)+framesize-4
 *
 * Notes regarding multiple entry points:
 * LEAF is used when including the profiling header is appropriate
 * XLEAF is used when the profiling header is in appropriate (e.g.
 * when a entry point is known by multiple names, the profiling call
 * should appear only once.)  The correct ordering of ENTRY/XENTRY in this
 * case is:
 * LEAF(copyseg)                # declare globl and emit profiling code
 * XLEAF(copypage)              # declare globl and alternate entry
 */
/*
 * LEAF -- declare leaf routine
 */
#define LEAF(x)                                         \
	.globl  x;					\
	.ent    x,0;                                    \
x:;                                                     \
	.frame  sp,0,ra

/*
 * XLEAF -- declare alternate entry to leaf routine
 */
#define XLEAF(x)                                        \
        .globl  x;                                      \
        .aent   x,0;                                    \
x:

/*
 * VECTOR -- declare exception routine entry
pp */
#define VECTOR(x, regmask)                              \
        .globl  x;                                      \
        .ent    x,0;                                    \
x:;                                                     \
        .frame  sp,EF_SIZE,$0;                          \
        .mask   +(regmask)|M_EXCFRM,-(EF_SIZE-(EF_RA+4))

/*
 * NESTED -- declare nested routine entry point
 */
#define NESTED(x, fsize, rpc)                           \
        .globl  x;                                      \
        .ent    x,0;                                    \
x:;                                                     \
        .frame  sp,fsize, rpc

/*
 * XNESTED -- declare alternate entry point to nested routine
 */
#define XNESTED(x)                                      \
        .globl  x;                                      \
        .aent   x,0;                                    \
x:

/*
 * END -- mark end of procedure
 */
#define END(proc)                                       \
        .end    proc

/*
 * IMPORT -- import external symbol
 */
#define IMPORT(sym, size)                               \
        .extern sym,size

/*
 * ABS -- declare absolute symbol
 */
#define ABS(x, y)                                       \
        .globl  x;                                      \
x       =       y

/*
 * EXPORT -- export definition of symbol
 */
#define EXPORT(x)                                       \
        .globl  x;                                      \
x:

/*
 * BSS -- allocate space in bss
 */
#define BSS(x,y)                \
        .comm   x,y

/*
 * LBSS -- allocate static space in bss
 */
#define LBSS(x,y)               \
        .lcomm  x,y

#endif
