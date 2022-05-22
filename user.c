#include <libc.h>

char buff[24];

int pid;

char a = '0';
char b = '0';
char c = '0';

void screen_callback(char *screen_buffer) {
  char (*screen)[80][2] = (char (*)[80][2]) screen_buffer;
  screen[0][2][0] = a++;
  screen[0][2][1] = 4;

  if (a > '9') {
    a = '0';
    b++;
  }

  screen[0][1][0] = b;
  screen[0][1][1] = 4;

  if (b > '9') {
    b = '0';
    c++;
  }

  screen[0][0][0] = c;
  screen[0][0][1] = 4;
}

void screen_callback_wrapper(void (*callback_function)(char *), char *screen_buffer) {
  callback_function(screen_buffer);
  return_from_callback();
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  system_info_screen_callback_wrapper(screen_callback_wrapper);
  set_screen_callback(screen_callback);
//   pid = fork();

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
