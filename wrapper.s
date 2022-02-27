# 0 "wrapper.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrapper.S"
# 1 "include/asm.h" 1
# 2 "wrapper.S" 2

.globl write; .type write, @function; .align 0; write:
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
