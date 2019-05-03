/*
 * libc.c
 */

#include <libc.h>
#include <errno.h>
#include <types.h>

int errno;
//void en C simboliza que hay 0 parametros seguro.
void perror(void){
  switch (errno) {
    case ENOSYS:
      write(1, "No existe syscall", 17);
      break;
    case EBADF:
      write(1, "Mal descriptor", 14);
      break;
    case EACCES:
      write(1, "No acceso", 9);
      break;
    case EFAULT:
      write(1, "Mal tamaño", 10);
      break;
    case EINVAL:
      write(1, "Malos parametros", strlen("Malos parametros"));
      break;  
    default:
      write(1, "No error", 8);
      break;
  }

}

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
