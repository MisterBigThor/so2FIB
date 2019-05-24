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

#include <semaphores.h>

#include <cirBuffer.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define TAMWRITE 4

extern int zeos_ticks;
extern struct list_head freequeue, readyqueue;
extern int qLeft;
extern int refs_DIR[NR_TASKS];
extern int qLeft;
extern struct semaphore semaphores[20];

int incrementalPID = 2;

unsigned long getEbp();

int check_fd(int fd, int permissions)
{
	if (fd!=1 && fd != 0) return -EBADF; /*EBADF*/
	if (permissions!=ESCRIPTURA && fd == 1) return -EACCES; /*EACCES*/
	if (permissions!=LECTURA && fd == 0) return -EACCES; /*EACCES*/
	return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS; /*ENOSYS*/
}

int sys_getpid(){
	return current()->PID;
}

int ret_from_fork(){return 0;}

int sys_fork(){
	
	if (list_empty(&freequeue)) return -ENOMEM;
	struct list_head *list_aux = list_first(&freequeue);
	list_del(list_aux);

	struct task_struct *new = list_head_to_task_struct(list_aux);
	
	copy_data(current(), new, (int) sizeof(union task_union));
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

	
	new->PID = incrementalPID++;
	new->estado = ST_READY;
  	init_stats(&new->estadisticas);
	int i = (getEbp() - (int)(current()))/sizeof(int);

	((union task_union*) new)->stack[i] = & ret_from_fork;
	((union task_union*) new)->stack[i-1] = 0;
	new->kernel_esp = &((union task_union*)new)->stack[i-1];

	list_add_tail(list_aux, &readyqueue);

	return new->PID;
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
void sys_exit(){	
	--refs_DIR[get_DIR_position(current())];
	if(refs_DIR[get_DIR_position(current())] == 0){
		page_table_entry * pt = get_PT(current());	
		for(int p = 0; p < NUM_PAG_DATA; ++p){
			free_frame(get_frame(pt, PAG_LOG_INIT_DATA+p));
			del_ss_pag(pt, PAG_LOG_INIT_DATA+p);
		}
	}
	list_add_tail(&(current()->list), &freequeue);
	for(int i = 0; i < 20; ++i)
		if(semaphores[i].owner == current()->PID) sys_sem_destroy(semaphores[i].n_sem);
	current()->PID = -1;
	update_process_state_rr(current(), &freequeue);
	sched_next_rr();
}

int sys_read(int fd, char *buf, int count){
	if(buf == NULL) return -EFAULT;
	if(count < 0) return -EINVAL;
	if(check_fd(fd, LECTURA) < 0) return -EBADF;
	if(!access_ok(VERIFY_WRITE, buf, count)) return -EFAULT;
	return sys_read_keyboard(buf, count);
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
	union task_union *tuThred = (union task_union*) tsThread;

	copy_data((union task_union*)current(), tsThread,sizeof(union task_union));
	
	++refs_DIR[current()->n_directorio];

	int pEBP =(getEbp() - (int) current())/sizeof(int);
	tsThread->kernel_esp = &tuThred->stack[pEBP];
	tuThred->stack[KERNEL_STACK_SIZE-2] = (int) stack;
	tuThred->stack[KERNEL_STACK_SIZE-5] = (int) function;
	

	tsThread->PID = incrementalPID++;
	tsThread->estado = ST_READY;
	list_add_tail(&(tsThread->list), &readyqueue);

	init_stats(&(tsThread->estadisticas));

	return tsThread->PID;
}

int cerca_sem_lliure(){
	struct semaphore s;	
	for(int i = 0; i<20; ++i){
		s = semaphores[i];
		if(s.state == FREE_SEM) return i;
	}
	return -1;
}
int cerca_sem(int n_sem){
	struct semaphore s;
	for(int i = 0; i < 20; ++i){
		s = semaphores[i];
		if(s.n_sem == n_sem && s.state == USED_SEM) return i;
	}
	return -1;
}

int sys_sem_init(int n_sem, unsigned int value){
	if(cerca_sem(n_sem) != -1) return -EBUSY; //ya existe sem
	int i = cerca_sem_lliure();
	struct semaphore *s = &semaphores[i];
	s->n_sem = n_sem;
	s->state = USED_SEM;
	s->counter = value;
	s->owner = current()->PID;
	INIT_LIST_HEAD(&(s->blockedQueue));
	return 0;
}
int sys_sem_destroy(int n_sem){
	int i = cerca_sem(n_sem);
	if(i == -1) return -EINVAL;
	struct semaphore *s = &semaphores[i];
	if(s->state == FREE_SEM) return -EINVAL;
	if(s->owner != current()->PID) return -EPERM;
	while(!list_empty(&(s->blockedQueue))){
		struct task_struct *tsUnblock = 
			list_head_to_task_struct(list_first(&s->blockedQueue));
		tsUnblock->sem_ret = -1;			
		update_process_state_rr(tsUnblock, &readyqueue);
	}
	s->state = FREE_SEM;
	s->n_sem = -1;
	s->owner = 0;
	return 0;
}
int sys_sem_signal(int n_sem){
	int i = cerca_sem(n_sem);
	if(i == -1) return -EINVAL;
	struct semaphore *s = &semaphores[i];
	if(s->state == FREE_SEM) return -EINVAL;
	if(list_empty(&s->blockedQueue)) s->counter++;
	else update_process_state_rr(list_head_to_task_struct(list_first(&s->blockedQueue)), &readyqueue);
	return 0;
}
int sys_sem_wait(int n_sem){
	int i = cerca_sem(n_sem);
	if(i == -1) return -EINVAL;
	struct semaphore *s = &semaphores[i];
	if(s->state == FREE_SEM) return -EINVAL;
	current()->sem_ret = 0;
	if(s->counter <= 0){
		exitRunCurrent(&s->blockedQueue);
		sched_next_rr();
		return current()->sem_ret;
	}
	else s->counter--;
	return 0;
}

void *sys_sbrk(int incr){
	void *previousProgramBreak = (void *) current()->processBreak;
	page_table_entry *TP = get_PT(current());
	if(current()->processBreak + incr < HEAP_START)
		incr = HEAP_START - current()->processBreak;
	int iPage = current()->processBreak >> 12;
	int fPage = (current()->processBreak + incr) >> 12;
	if(fPage > TOTAL_PAGES) return (void *) -ENOMEM;

	int aux;
	if((current()->processBreak + incr)%PAGE_SIZE == 0) aux = 0; 
	else aux = 1; //pagina extra

	if(incr > 0){
		for(int i = iPage; i < (fPage + aux) ; i++){
			if(TP[i].bits.present){
				int pagF;
				if((pagF = alloc_frame())>= 0) set_ss_pag(TP, i, pagF);
				else {
					for(int j = i; j > (iPage+aux); j--){
						free_frame(get_frame(TP, j));
						del_ss_pag(TP, j);
					}
					return (void *) -ENOMEM;
				}
			}
		}
	}
	else if(incr < 0){
		for(int i = iPage; i>fPage+aux; i--){
			if(i<PAG_LOG_INIT_HEAP){
				current()->processBreak = HEAP_START;
				set_cr3(get_DIR(current()));
				return (void *) -EFAULT;
			}
			free_frame(get_frame(TP, i));
			del_ss_pag(TP, i);
		}
		set_cr3(get_DIR(current()));
	}

	current()->processBreak += incr;
	return previousProgramBreak;
}