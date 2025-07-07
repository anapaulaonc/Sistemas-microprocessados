.cdecls "msp430.h"
	.global main

    .data
saida: .space 4

    .text
main:
  mov #0x89AB, R12
  mov #saida, R13
  calla #w16_asc

  jmp $


w16_asc:
    mov r13, r5
    
    mov r12, r14
    swpb r14
    rra r14
    rra r14
    rra r14
    rra r14
    and #0x000f, r14
    calla #nib_asc
    mov.b r14, 0(r5)
    
    mov r12, r14
    swpb r14
    and #0x000f, r14
    calla #nib_asc
    mov.b r14, 1(r5)

    
    mov r12, r14
    rra r14
    rra r14
    rra r14
    rra r14
    and #0x000f, r14
    calla #nib_asc
    mov.b r14, 2(r5)
            
    mov r12, r14
    and #0x000f, r14
    calla #nib_asc
    mov.b r14, 3(r5)
    
    ret

nib_asc:
    cmp #0x0a, r14
    jhs is_letter
    
    add #0x30, r14
    jmp end_nib_asc
    
is_letter:
    add #0x37, r14
    ret

end_nib_asc:
    mov R14, R15
    ret