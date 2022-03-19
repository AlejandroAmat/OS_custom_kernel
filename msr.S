#include <asm.h>

ENTRY(writeMSR)
  pushl %ebp
  movl %esp, %ebp
  movl 8(%ebp), %ecx
  movl 12(%ebp), %eax
  movl $0, %edx
  wrmsr
  movl %ebp, %esp
  popl %ebp
  ret
