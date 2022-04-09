#include <asm.h>

ENTRY(exit)
  pushl %ebp
  movl %esp, %ebp
  pushl %ecx
  pushl %edx
  movl $1, %eax
  pushl $exit_return
  pushl %ebp
  movl %esp, %ebp
  sysenter
exit_return:
  movl %ebp, %esp
  popl %ebp
  cmpl $0, %eax
  jge getpid_endif
  neg %eax
  movl %eax, errno
  movl $-1, %eax
exit_endif:
  popl %edx
  popl %ecx
  movl %ebp, %esp
  popl %ebp
  ret

ENTRY(fork)
  pushl %ebp
  movl %esp, %ebp
  pushl %ecx
  pushl %edx
  movl $2, %eax
  pushl $fork_return
  pushl %ebp
  movl %esp, %ebp
  sysenter
fork_return:
  movl %ebp, %esp
  popl %ebp
  cmpl $0, %eax
  jge fork_endif
  neg %eax
  movl %eax, errno
  movl $-1, %eax
fork_endif:
  popl %edx
  popl %ecx
  movl %ebp, %esp
  popl %ebp
  ret

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
  pushl $write_return
  pushl %ebp
  movl %esp, %ebp
  sysenter
write_return:
  movl %ebp, %esp
  popl %ebp
  addl $4, %esp
  cmpl $0, %eax
  jge write_endif
  neg %eax
  movl %eax, errno
  movl $-1, %eax
write_endif:
  popl %edx
  popl %ecx
  popl %ebx
  movl %ebp, %esp
  popl %ebp
  ret

ENTRY(gettime)
  pushl %ebp
  movl %esp, %ebp
  pushl %ecx
  pushl %edx
  movl $10, %eax
  pushl $gettime_return
  pushl %ebp
  movl %esp, %ebp
  sysenter
gettime_return:
  movl %ebp, %esp
  popl %ebp
  cmpl $0, %eax
  jge gettime_endif
  neg %eax
  movl %eax, errno
  movl $-1, %eax
gettime_endif:
  popl %edx
  popl %ecx
  movl %ebp, %esp
  popl %ebp
  ret

ENTRY(getpid)
  pushl %ebp
  movl %esp, %ebp
  pushl %ecx
  pushl %edx
  movl $20, %eax
  pushl $getpid_return
  pushl %ebp
  movl %esp, %ebp
  sysenter
getpid_return:
  movl %ebp, %esp
  popl %ebp
  cmpl $0, %eax
  jge getpid_endif
  neg %eax
  movl %eax, errno
  movl $-1, %eax
getpid_endif:
  popl %edx
  popl %ecx
  movl %ebp, %esp
  popl %ebp
  ret

ENTRY(get_stats)
  pushl %ebp
  movl %esp, %ebp
  pushl %ebx
  pushl %ecx
  pushl %edx
  movl 8(%ebp), %ebx
  movl 12(%ebp), %ecx
  movl $35, %eax
  pushl $get_stats_return
  pushl %ebp
  movl %esp, %ebp
  sysenter
get_stats_return:
  movl %ebp, %esp
  popl %ebp
  addl $4, %esp
  cmpl $0, %eax
  jge get_stats_endif
  neg %eax
  movl %eax, errno
  movl $-1, %eax
get_stats_endif:
  popl %edx
  popl %ecx
  popl %ebx
  movl %ebp, %esp
  popl %ebp
  ret
