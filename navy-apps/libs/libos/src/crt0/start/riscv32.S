.globl  _start
_start:
  move s0, zero
  ; move a1, a0
  ; move a0, sp
  ; move a0, a1
  move sp, a0
  move a0, a1
  ; move a0, sp
  jal call_main
