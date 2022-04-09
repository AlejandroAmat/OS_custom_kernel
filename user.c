#include <libc.h>

char buff[24];

int pid;

void write_pid_plus_time_and_exit() {
  itoa(getpid(), buff);
  write(1, buff, strlen(buff));
  write(1, " ", 1);

  itoa(gettime(), buff);
  write(1, buff, strlen(buff));
  write(1, "\n", 1);

  exit();
}

int __attribute__ ((__section__(".text.main")))
  main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  write(1, "write test\n", 11);
  for (int i = 0; i < 4; ++i) {
    pid = fork();

    if (pid == 0)
      write_pid_plus_time_and_exit();
  }

  struct stats st;
  get_stats(1, &st);

  write_pid_plus_time_and_exit();

  while(1) {}
}
