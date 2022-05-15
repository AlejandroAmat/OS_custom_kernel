#include <libc.h>

char buff[24];

int pid;

void screen_callback(char *screen_buffer) {
  return;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  set_screen_callback(screen_callback);
  pid = fork();

  char (*screen)[80][2] = (char (*)[80][2]) get_screen();
  screen[24][32][1] = 'c';
  write(1, &screen[24][32][1], 1);
  write(1, "\n", 1);

  remove_screen(screen);
//   screen[24][32][1] = 'c';

  itoa(pid, buff);
  write(1, buff, strlen(buff));
  write(1, "\n", 1);

  char c, d = '\0';
  char t[80];
  while(1) {
    int code = get_key(&c);
    if (code >= 0 && d != c) {
      write(1, &c, 1);
      write(1, "\n", 1);
      d = c;
    }
  }
}
