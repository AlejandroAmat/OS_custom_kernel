#include <asm.h>

ENTRY(write)
  pushl %ebp
  movl %esp, %ebp
  pushl %ebx
  pushl %ecx
  pushl %edx
  movl 8(%ebp), %ebx
  movl 12(%ebp), %ecx
  movl 16(%ebp), %edx
  movl $4, %eax
  pushl write_return
  pushl %ebp
  movl %esp, %ebp
  sysenter
write_return:
  movl %ebp, %esp
  popl %ebp
  cmpl $0, %eax
  jge endif
  neg %eax
  movl %eax, errno
  movl $-1, %eax
endif:
  popl %edx
  popl %ecx
  popl %ebx
  movl %ebp, %esp
  popl %ebp
  ret
