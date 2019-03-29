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
#include <stats.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_ticks;
extern struct list_head freequeue, readyqueue;
extern int qLeft;

int incrementalPID = 2;

int getEbp();

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

int ret_from_fork(){return 0;}

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
		pageNew[1+i].entry = pageParent[1+i].entry;
	//userdata + stack
	for (int i = 0; i < NUM_PAG_CODE; ++i)
		pageNew[PAG_LOG_INIT_CODE+i].entry = pageParent[PAG_LOG_INIT_CODE+i].entry;

	int framesNew[NUM_PAG_DATA];
	for(int i = 0; i < NUM_PAG_DATA; ++i){
		if((framesNew[i] = alloc_frame())<0){ //alloc frame error!
			for(int aux = i-1; aux >= 0; --aux) //revert, si no queda mem
				free_frame(framesNew[aux]);
			return -40;
		}
		else {
			set_ss_pag(pageNew, PAG_LOG_INIT_DATA+i, framesNew[i]);
			set_ss_pag(pageParent, PAG_LOG_INIT_DATA+NUM_PAG_DATA+i, framesNew[i]);
			copy_data((int*)((PAG_LOG_INIT_DATA+i)<<12),(int*)((PAG_LOG_INIT_DATA+NUM_PAG_DATA+i)<<12), PAGE_SIZE);
			del_ss_pag(pageParent, PAG_LOG_INIT_DATA+NUM_PAG_DATA+i); //free
		}
	}

	set_cr3(get_DIR(current()));

	PID = incrementalPID++;
	new->PID = PID;
	new->estado = ST_READY;
	struct stats aux;
	new->estadisticas = aux;

	int i = (getEbp() - (int)(current()))/sizeof(int);

	((union task_union*) new)->stack[i] = & ret_from_fork;
	((union task_union*) new)->stack[i-1] = 0;
	new->kernel_esp = &((union task_union*)new)->stack[i-1];

	list_add_tail(list_aux, &readyqueue);

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
	struct task_struct *aux = current();

	page_table_entry * pt = get_PT(aux);
	for(int p = 0; p < NUM_PAG_DATA; ++p){
		free_frame(pt[PAG_LOG_INIT_DATA+p].bits.pbase_addr);
		del_ss_pag(pt, PAG_LOG_INIT_DATA+p);
	}

	update_process_state_rr(aux, &freequeue);
	sched_next_rr();
}
int sys_get_stats(int pid, struct stats *st){
	if(pid < 0) return -40;
	if(!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -20;
	struct task_struct *act;
	int i = 0 ;
	for(act = &(task[i].task); i<NR_TASKS; act = &(task[++i].task)){
		if(act->PID == pid){
			act->estadisticas.remaining_ticks = qLeft;
			copy_to_user(&(act->estadisticas),st,sizeof(struct stats));
			return 0;
		}
	}
	return -33;
}