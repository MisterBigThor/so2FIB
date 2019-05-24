/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <semaphores.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));



struct list_head freequeue;
struct list_head readyqueue;
struct list_head keyboardqueue;

unsigned long getEbp();
void setEsp();
void writeMsr(int msr, int data);

struct semaphore semaphores[20];

extern struct list_head blocked;

struct task_struct *list_head_to_task_struct(struct list_head *l){
  return list_entry( l, struct task_struct, list);
}


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) {
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) {
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

extern page_table_entry dir_pages[NR_TASKS][TOTAL_PAGES];



int allocate_DIR(struct task_struct *p) 
{
	for(int i = 0; i< NR_TASKS; ++i){
		if(refs_DIR[i]==0){
			p->dir_pages_baseAddr =(page_table_entry *) &dir_pages[i];
			p->n_directorio = i;
			refs_DIR[i]++;
			return 1;
		}
	}
	return -1;
}
int get_DIR_position(struct task_struct *t){
	return t->n_directorio;
}
void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	printk("Entering cpu_idle");
	while(1)
	{
	;
	}
}

#define DEFAULT_QUANTUM 15
int qLeft;
struct task_struct * idle_task= NULL; //global variable for easy access.

//update scheduling && stats information
void update_sched_data_rr(){
	qLeft--;
}
//necesary change process
int needs_sched_rr(){

	int pWaiting = !list_empty(&readyqueue);
	if(pWaiting && qLeft ==0) return 1;
	if(!pWaiting && qLeft == 0){
		current()->estadisticas.total_trans++;
		qLeft = get_quantum(current());
	}
	return 0;
}
//update state current
void update_process_state_rr(struct task_struct*t, struct list_head *dst_queue){
	if(t->estado != ST_RUN) list_del(&t->list);
	if(dst_queue == NULL){
		t->estado =ST_RUN;
	}
	else{
		list_add_tail(&t->list, dst_queue);
		if(dst_queue == &readyqueue) t->estado = ST_READY;
		else t->estado=ST_BLOCKED;
	}
	
}
//seleciona siguiente proceso a ejecutar:
void sched_next_rr(void){
	if(!list_empty(&readyqueue)){
		struct list_head* next = list_first(&readyqueue);
		struct task_struct* nextt = list_head_to_task_struct(next);

		qLeft = get_quantum(nextt);

		struct stats *st;
		st = &nextt->estadisticas; //antiguo
		st->system_ticks += get_ticks() - st->elapsed_total_ticks;
		st->elapsed_total_ticks = get_ticks();
		st = &nextt->estadisticas; //nuevo
		st->ready_ticks += get_ticks() - st->elapsed_total_ticks;
		st->elapsed_total_ticks = get_ticks();
		st->total_trans += 1;
		update_process_state_rr(nextt, NULL);
		task_switch((union task_union *) nextt);
	}
	else task_switch((union task_union *) idle_task);
}
void schedule(){
	update_sched_data_rr();
	if(needs_sched_rr()){
		exitRunCurrent(&readyqueue);
		sched_next_rr();	
	}
}
void exitRunCurrent(struct list_head *nextListCurrent){
	struct stats *st;
	st = &current()->estadisticas;
	st->system_ticks += get_ticks() - st->elapsed_total_ticks;
	st->elapsed_total_ticks = get_ticks();
	update_process_state_rr(current(), nextListCurrent);
}
void init_idle (void)
{
	struct list_head * aux = list_first(& freequeue);
	list_del(aux);
	struct task_struct * ts = list_head_to_task_struct(aux);
	union task_union * tu = (union task_union *) ts;

	ts->PID = 0; //asign PID 0
	ts->quantum = DEFAULT_QUANTUM;
	init_stats(&ts->estadisticas);
	
	allocate_DIR(ts); //asign DIR
	refs_DIR[get_DIR_position(ts)]++;
	tu->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)&cpu_idle;
	tu->stack[KERNEL_STACK_SIZE - 2] = 0;

	tu->task.kernel_esp = (char *)& (tu->stack[KERNEL_STACK_SIZE - 2]);		
	ts->processBreak = HEAP_START;
	idle_task = ts; 
}

void init_task1(void)
{
	struct list_head * aux = list_first(& freequeue);
	list_del(aux);
	struct task_struct * ts = list_head_to_task_struct(aux);
	union task_union * tu = (union task_union *) ts;

	ts->PID = 1;
	ts->quantum = DEFAULT_QUANTUM;
	ts->estado = ST_RUN;
	ts->processBreak = HEAP_START;
	qLeft = ts->quantum;

	init_stats(&ts->estadisticas);
	allocate_DIR(ts);
	refs_DIR[get_DIR_position(ts)]++;
	set_user_pages(ts);
	
	tss.esp0=(DWord)& (tu->stack[KERNEL_STACK_SIZE]);
	writeMsr(0x175, KERNEL_ESP(tu)); 
	set_cr3(ts->dir_pages_baseAddr);


}


void inner_task_switch(union task_union*t){
	tss.esp0 = KERNEL_ESP(t);
	writeMsr(0x175, (int)KERNEL_ESP(t));
	
	set_cr3(get_DIR((struct task_struct*)t)); //??
	
	current() -> kernel_esp = (char *) getEbp();

	setEsp(t->task.kernel_esp);

	return;
}

void init_sched(){
	INIT_LIST_HEAD(& keyboardqueue);
	INIT_LIST_HEAD(& freequeue);
	for(int i = 0; i < NR_TASKS; ++i){	
		task[i].task.PID = -1;
		list_add(&(task[i].task.list), &freequeue);
	}
	INIT_LIST_HEAD(& readyqueue);
	for (int i = 0; i < NR_TASKS; ++i){
		refs_DIR[i] = 0;
	}
	for(int i = 0; i < 20; ++i){
		semaphores[i].counter = 0;
		semaphores[i].state = FREE_SEM;
	}
}


struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void statsEntrySys(){
	struct stats *aux;
	aux = & current()->estadisticas;
	aux->user_ticks += get_ticks() - aux->elapsed_total_ticks;
	aux->elapsed_total_ticks = get_ticks();	
}

void statsLeaveSys(){
	struct stats *aux;
	aux = & current()->estadisticas;
	aux->system_ticks += get_ticks() - aux->elapsed_total_ticks;
	aux->elapsed_total_ticks = get_ticks();	
}

int get_quantum(struct task_struct *t){	
	return current()->quantum;
}
void set_quantum(struct task_struct *t,int q){
	current()->quantum = q;
}
void init_stats(struct stats *st){
	st->user_ticks = 0;
	st->system_ticks = 0;
	st->blocked_ticks = 0;
	st->ready_ticks = 0;
	st->elapsed_total_ticks = 0;
	st->total_trans = 0;
	st->remaining_ticks = 0;
}