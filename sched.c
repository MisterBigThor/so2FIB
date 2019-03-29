/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>


#define QUANTUMINICIAL 15


union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct * idle_task; //global variable for easy access.

struct list_head freequeue;
struct list_head readyqueue;

int getEbp();
void setEsp();
#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	printk("Entering cpu idle mode...\n");
	while(1)
	{
	;
	}
}
void enqueue_current(struct list_head *next_queue){
	struct stats *st;
	st = &current()->estadisticas;
	st->system_ticks += get_ticks() - st->elapsed_total_ticks;
	st->elapsed_total_ticks = get_ticks();
	update_process_state_rr(current(), next_queue);
}
void init_idle (void)
{
	struct list_head * aux = list_first(& freequeue);
	list_del(aux);
	struct task_struct * ts = list_head_to_task_struct(aux);
	ts->PID = 0; //asign PID 0
	allocate_DIR(ts); //asign DIR
	union task_union * tu = (union task_union *) ts;
	tu->task.kernel_esp = (char *)& (tu->stack[KERNEL_STACK_SIZE - 2]);
	tu->stack[KERNEL_STACK_SIZE - 2] = 0;
	tu->stack[KERNEL_STACK_SIZE - 1] = cpu_idle;

	ts->estado = ST_READY;


	ts->estadisticas.system_ticks = 0;
	ts->estadisticas.blocked_ticks = 0;
	ts->estadisticas.ready_ticks = 0;
	ts->estadisticas.elapsed_total_ticks = 0;
	ts->estadisticas.total_trans = 0;
	ts->estadisticas.remaining_ticks = QUANTUMINICIAL;

	idle_task = ts; 	
}

void init_task1(void)
{
	struct list_head * aux = list_first(& freequeue);
	list_del(aux);
	struct task_struct * ts = list_head_to_task_struct(aux);
	union task_union * tu = (union task_union *) ts;
	ts->PID = 1;
	allocate_DIR(ts);
	set_user_pages(ts);
	
	ts -> kernel_esp =  (unsigned long *) KERNEL_ESP(tu);

	tss.esp0 = (unsigned long) KERNEL_ESP(tu); //kernel stack
	ts->kernel_esp = KERNEL_ESP(tu);
	writeMsr(0x175, KERNEL_ESP(tu));
<<<<<<< HEAD
	ts->quantum = QUANTUMINICIAL;
	ts->estado = ST_RUN;
	
	ts->estadisticas.user_ticks = 0;
	ts->estadisticas.system_ticks = 0;
	ts->estadisticas.blocked_ticks = 0;
	ts->estadisticas.ready_ticks = 0;
	ts->estadisticas.elapsed_total_ticks = 0;
	ts->estadisticas.total_trans = 0;
	ts->estadisticas.remaining_ticks = QUANTUMINICIAL;

=======
	ts->quantum = 15;
	ts->estado = ST_RUN;
>>>>>>> 49d6338838638c0f9949fb6e8cec4518179a05c7
	set_cr3(ts->dir_pages_baseAddr);
}


void inner_task_switch(union task_union*t){
	tss.esp0 = KERNEL_ESP(t);
	writeMsr(0x175, (int) KERNEL_ESP(t));

	set_cr3(t->task.dir_pages_baseAddr);
	current() -> kernel_esp = (char *) getEbp();

	setEsp(t->task.kernel_esp);

	t->task.estadisticas.total_trans++;

	return;
}

void init_sched(){
	INIT_LIST_HEAD(& freequeue);
	for(int i = 0; i < NR_TASKS; ++i){	

		list_add(&(task[i].task.list), &freequeue);
	}
	INIT_LIST_HEAD(& readyqueue);
}


//update scheduling && stats information
void update_sched_data_rr(){
	current()->quantum--;
	current()->estadisticas.remaining_ticks--;
	current()->estadisticas.user_ticks += get_ticks() - current()->estadisticas.elapsed_total_ticks;
	current()->estadisticas.elapsed_total_ticks = get_ticks();

//necesary change process
int needs_sched_rr(){
	return 	(current()->quantum <= 0) &&
			! list_empty(&readyqueue) &&
			(current()->PID != 0);
}
//update state current
void update_process_state_rr(struct task_struct*t, struct list_head *dst_queue){
	if(t->estado != ST_RUN) list_del(&t->list);
	if(dst_queue == NULL)t->estado = ST_RUN;

	else{
		list_add_tail(&t->list, dst_queue);
		if(dst_queue == &readyqueue) t->estado = ST_READY;
	}
}
//seleciona siguiente proceso a ejecutar:
void sched_next_rr(void){
	if(!list_empty(&readyqueue)){
		struct list_head *aux = list_first(&readyqueue);
		struct task_struct *next = list_head_to_task_struct(aux);
		update_process_state_rr(next, NULL);
		task_switch((union task_union*) next);
	}

	else task_switch((union task_union*) idle_task);
}
int get_quantum(struct task_struct *t){
	return t->quantum;
}
void set_quantum(struct task_struct *t,int q){
	t->quantum = q;
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

