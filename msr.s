# 0 "msr.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "msr.S"
# 1 "include/asm.h" 1
# 2 "msr.S" 2

.globl writeMSR; .type writeMSR, @function; .align 0; writeMSR:
  pushl %ebp
  movl %esp, %ebp
  movl 8(%ebp), %ecx
  movl 12(%ebp), %eax
  movl 16(%ebp), %edx
  wrmsr
  movl %ebp, %esp
  popl %ebp
