/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>
#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1


int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -EACCES; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process

  return PID;
}
#define SECCION 64
int sys_write(int fd, char * buffer, int size){
	if(check_fd(fd, ESCRIPTURA) != 0) return check_fd(fd, ESCRIPTURA);
	if(buffer == NULL) return -EFAULT;
	if(size < 0) return -EINVAL;
  char aux[SECCION];
  int i = 0;
  int escrito = 0;
  while(i<(size-SECCION)){
    copy_from_user(buffer+i, aux, SECCION);
    escrito += sys_write_console(aux, SECCION);
    i += SECCION;
  }
  int restant = size - i;
  copy_from_user(buffer+i, aux, restant);
  escrito += sys_write_console(aux, restant);
  return escrito;
}

void sys_exit()
{
}
