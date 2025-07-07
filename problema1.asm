.cdecls "msp430.h"
  .global main

  .text

main:
  mov.w #(WDTPW|WDTHOLD), &WDTCTL
  
  mov.w #vector, R12
  mov.w #5, R13
  calla #ORDENA
  jmp $

ORDENA:
  push R4
  mov.w R13, R14
  dec.w R14
  
outer_loop:
  tst.w R14
  jz sort_done
  
  mov.w R12, R15
  mov.w R14, R11
  
inner_loop:
  tst.w R11
  jz next_outer
  
  mov.b @R15, R10
  mov.b 1(R15), R9
  
  cmp.b R9, R10
  jl no_swap
  
  mov.b R9, 0(R15)
  mov.b R10, 1(R15)
  
no_swap:
  inc.w R15
  dec.w R11
  jmp inner_loop
  
next_outer:
  dec.w R14
  jmp outer_loop
  
sort_done:
  pop R4
  ret

  .data
vector:
  .byte 4, 7, 3, 5, 1
  
  .end