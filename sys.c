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

#include <system.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define TAMWRITE 4

extern int zeos_ticks;
extern struct list_head freequeue, readyqueue;
extern int qLeft;
extern int refs_DIR[NR_TASKS];
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
	if (list_empty(&freequeue)) return -ENOMEM;
	struct list_head *list_aux = list_first(&freequeue);
	list_del(list_aux);

	struct task_struct *new = list_head_to_task_struct(list_aux);
	//inherit system data:
	copy_data(current(), new, (int) sizeof(union task_union));
	//initialize dir_pages_baseAddr:
	if(allocate_DIR(new)<0) return -ENOMEM;
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
			return -ENOMEM;
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
  	init_stats(&new->estadisticas);
	int i = (getEbp() - (int)(current()))/sizeof(int);

	((union task_union*) new)->stack[i] = & ret_from_fork;
	((union task_union*) new)->stack[i-1] = 0;
	new->kernel_esp = &((union task_union*)new)->stack[i-1];

	list_add_tail(list_aux, &readyqueue);

	return PID;
}


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
	--refs_DIR[get_DIR_position(current())];
	if(refs_DIR[get_DIR_position(current())] == 0){
		page_table_entry * pt = get_PT(current());	
		for(int p = 0; p < NUM_PAG_DATA; ++p){
			free_frame(get_frame(pt, PAG_LOG_INIT_DATA+p));
			del_ss_pag(pt, PAG_LOG_INIT_DATA+p);
		}
	}
	list_add_tail(&(current()->list), &freequeue);
	current()->PID = -1;
	sched_next_rr();
}




int sys_get_stats(int pid, struct stats *st){
	if(pid < 0) return -EINVAL;
	if(!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT;
	int i;
	for(i = 0;i<NR_TASKS; i++){
		if(task[i].task.PID==pid){
			copy_to_user(&(task[i].task.estadisticas),st,sizeof(struct stats));
			st->remaining_ticks=qLeft;
			return 0;
		}
	}
	return -ESRCH;
}

int sys_clone(void (*function)(void), void *stack){
	if (!access_ok(VERIFY_WRITE, stack, 4) || !access_ok(VERIFY_READ, function, 4)) return -EFAULT;
	if(list_empty(&freequeue)) return -ENOMEM;
	
	struct list_head * freePCB = list_first(&freequeue);
	list_del(freePCB);
	struct task_struct *tsThread = list_head_to_task_struct(freePCB);
	union task_union *tuThred = (union task_union*) current();

	copy_data((union task_union*)current(), tuThred,sizeof(union task_union));
	++refs_DIR[get_DIR_position(tsThread)];

	int pEBP =((unsigned long) getEbp() - (unsigned long) current())/4;
	tsThread->kernel_esp = (unsigned long) &(tuThred->stack[pEBP]);
	tuThred->stack[KERNEL_STACK_SIZE-5] = (unsigned long) function;
	tuThred->stack[KERNEL_STACK_SIZE-2] = (unsigned long) stack;

	tsThread->PID = incrementalPID++;
	tsThread->estado = ST_READY;
	list_add_tail(&(tsThread->list), &readyqueue);

	init_stats(&(tsThread->estadisticas));

	return tsThread->PID;
}

extern struct semaphore semaphores[20];

int sys_sem_init(int n_sem, unsigned int value){
	if(n_sem <0 || n_sem >= 20) return -EINVAL;
	if(semaphores[n_sem].state == USED_SEM) return EBUSY;
	semaphores[n_sem].state = FREE_SEM;
	semaphores[n_sem].counter = value;
	semaphores[n_sem].owner = current();
	INIT_LIST_HEAD(& (semaphores[n_sem].blocked_queue));
	return 0;
}
int sys_sem_destroy(int n_sem){
	if(n_sem <0 || n_sem >= 20) return -EINVAL;
	if(semaphores[n_sem].state == FREE_SEM) return EINVAL;
	if(semaphores[n_sem].owner != current()) return -EPERM;
	while(!list_empty(&(semaphores[n_sem].blocked_queue))){
		struct task_struct *tsUnblock = 
			list_head_to_task_struct(list_first(&semaphores[n_sem].blocked_queue));
		update_process_state_rr(tsUnblock, &readyqueue);
	}
	semaphores[n_sem].state = FREE_SEM;
	return 0;
}
int sys_sem_signal(int n_sem){
	if(n_sem <0 || n_sem >= 20) return -EINVAL;
	if(semaphores[n_sem].state == FREE_SEM) return EINVAL;
	return 0;
}
int sys_sem_wait(int n_sem){
	if(n_sem <0 || n_sem >= 20) return -EINVAL;
	if(semaphores[n_sem].state == FREE_SEM) return EINVAL;
	return 0;
}