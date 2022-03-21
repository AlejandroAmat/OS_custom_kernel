#include <asm.h>

ENTRY(sys_fork_asm)
  pushl %ebp
  movl %esp, %ebp
  call current
  addl $4096, %eax                # eax <- current()->stack[KERNEL_STACK_SIZE]
  subl (%ebp), %eax               # eax <- current()->stack[KERNEL_STACK_SIZE] - %ebp
  movl 8(%ebp), %ecx              # ecx <- new
  movl %ecx, %edx                 # edx <- new
  addl $4096, %ecx                # ecx <- (union task_union *) (new)->stack[KERNEL_STACK_SIZE]
  subl %eax, %ecx                 # ecx <- new->stack[KERNEL_STACK_SIZE] - (current()->stack[KERNEL_STACK_SIZE] - %ebp)
  movl $0, -4(%ecx)               # añade ebp a la pila de new
  movl $ret_from_fork, (%ecx)     # añade @ret_from_fork a la pila de new
  addl $-4, %ecx
  movl %ecx, 4(%edx)              # new->kernel_esp <- ecx
  movl %ebp, %esp
  popl %ebp
  ret
