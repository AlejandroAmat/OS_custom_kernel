#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  char c;
  int error = get_key(&c);
  pid = fork();
  char *screen = get_screen();
  screen[24] = 'c';
  remove_screen(screen);
  itoa(pid, buff);
  write(1, buff, strlen(buff));
  write(1, "\n", 1);
  while(1) { }
}
