    .cdecls "msp430.h"
    .global main

    .text

main:

    mov #4, R12
    mov #2, R13
    call #mult8
    jmp $

mult8:
    push R4
    clr R4

mult8_loop:

    add R12, R4
    dec R13
    jnz mult8_loop

mult8_end:

    mov R4, R12
    pop R4
    ret