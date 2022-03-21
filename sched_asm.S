#include <asm.h>

ENTRY(task_switch)
  pushl %ebp
  movl %esp, %ebp
  pushl %esi
  pushl %edi
  pushl %ebx
  pushl 8(%ebp)
  call inner_task_switch
  popl %ebx
  popl %edi
  popl %esi
  movl %ebp, %esp
  popl %ebp
  ret

ENTRY(inner_task_switch_asm)
  pushl %ebp
  movl %esp, %ebp
  call current
  movl (%ebp), %ecx   # ecx <- %ebp inner_task_switch
  movl %ecx, 4(%eax)  # current()->kernel_esp <- ecx
  movl 8(%ebp), %eax  # eax <- @new
  movl 4(%eax), %esp  # esp <- new.kernel_esp
  popl %ebp
  ret
