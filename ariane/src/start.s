.macro CLEAR_GPR_REG_ITER
    mov r\@, #0
.endm

.section .text.start
.arm
.align 5
.global _start
_start:
    /* Insert NOPs for convenience (i.e. to use Nintendo's BCTs, for example) */
    .rept 16
    nop
    .endr
    /* Switch to supervisor mode, mask all interrupts, clear all flags */
    msr cpsr_cxsf, #0xDF

    /* Relocate ourselves if necessary */
    ldr r0, =__start__
    adr r1, _start
    cmp r0, r1
    beq _after_relocation

    adr r2, _relocator
    ldr r3, =__stack
    adr r4, _relocator_end
    mov r12, r3
    _copy_relocator_loop:
        ldr r5, [r2], #4
        str r5, [r3], #4
        cmp  r2, r4
        bne _copy_relocator_loop

    ldr r2, =__bss_start__
    sub r2, r2, r0
    bx  r12

    _after_relocation:
    /* Set the stack pointer */
    ldr sp, =__stack
    mov fp, #0

    /* Clear .bss */
    ldr r0, =__bss_start__
    mov r1, #0
    ldr r2, =__bss_end__
    sub r2, r2, r0
    bl  memset

    /* Call global constructors */
    bl  __libc_init_array

    /* Set r0 to r12 to 0 (because why not?) & call main */
    .rept 13
    CLEAR_GPR_REG_ITER
    .endr
    bl  main
    b   .
_relocator:
    mov r12, r0
    _relocation_loop:
        ldmia r1!, {r3-r10}
        stmia r0!, {r3-r10}
        subs  r2, #0x20
        bne _relocation_loop
    bx  r12
_relocator_end:
    b   .
.globl pivot_stack
.type pivot_stack, %function
pivot_stack:
	MOV SP, R0
	BX LR    