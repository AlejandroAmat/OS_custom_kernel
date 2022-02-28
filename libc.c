/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror(void)
{
  switch (errno) {
    case EACCES:
      write(1, "Permission denied\n", 18);
      break;
    case EBADF:
      write(1, "Bad file descriptor\n", 20);
      break;
    case EFAULT:
      write(1, "Bad address\n", 12);
      break;
    case ENOSYS:
      write(1, "System call not yet implemented\n", 32);
  }
}

