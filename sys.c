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

extern int zeos_ticks;

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

#define tamLectura 4
int sys_write(int fd, char * buffer, int size){
  if(check_fd(fd, ESCRIPTURA) != 0) return check_fd(fd, ESCRIPTURA);
  if(buffer == NULL) return -EFAULT;
  if(size < 0) return -EINVAL;
  if(size == 0) return 0;

  int ret = 0;
  char buff[4];

  while(size >= 4){
    copy_from_user(buffer, buff, 4);
    ret += sys_write_console(buff,4);
		buffer += 4;
		size -= 4;
  }
  copy_from_user(buffer, buff, size);
	ret += sys_write_console(buff,size);
  return ret;
}
int sys_gettime(){

  return zeos_ticks;

}
void sys_exit()
{
}
