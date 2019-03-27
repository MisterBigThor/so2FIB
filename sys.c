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
	return -ENOSYS; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
	int PID=-1;
	//get free task_struct
	if (list_empty(&freequeue)) return -40;
	struct list_head *list_aux = list_first(&freequeue);
	list_del(list_aux);
	struct task_struct *new = list_head_to_task_struct(list_aux);
	//inherit system data:
	copy_data(current(), new, (int) sizeof(union task_union));
	//initialize dir_pages_baseAddr:
	if(allocate_DIR(new)<0) return -40;
	//inherit user data:
	page_table_entry * pageNew = get_PT(new);
	page_table_entry * pageParent = get_PT(current());
	//system code
	for (int i = 0; i < NUM_PAG_KERNEL; ++i)
		pageNew[i].entry = pageParent[i].entry;
	//userdata + stack
	for (int i = 0; i < NUM_PAG_CODE; ++i)
		pageNew[PAG_LOG_INIT_CODE+page].entry = pageParent[PAG_LOG_INIT_CODE+page].entry;

	return PID;
}

#define TAMWRITE 4
int sys_write(int fd, char * buffer, int size){
	if(check_fd(fd, ESCRIPTURA) != 0) return check_fd(fd, ESCRIPTURA);
	if(buffer == NULL) return -EFAULT;
	if(size < 0) return -EINVAL;
	if(size == 0) return 0;

	int ret = 0;
	char buff[TAMWRITE];

	while(size >= TAMWRITE){
		copy_from_user(buffer, buff, TAMWRITE);
		ret += sys_write_console(buff,TAMWRITE);
		buffer += TAMWRITE;
		size -= TAMWRITE;
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
