/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>


union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));



struct list_head freequeue;
struct list_head readyqueue;

unsigned long getEbp();
void setEsp();
void writeMsr(int msr, int data);



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

void init_stats(struct stats *s){
	s->user_ticks = 0;
	s->system_ticks = 0;
	s->blocked_ticks = 0;
	s->ready_ticks = 0;
	s->elapsed_total_ticks = get_ticks();
	s->total_trans = 0;
	s->remaining_ticks = get_ticks();
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
	printk("Entering cpu_idle");
	while(1)
	{
	;
	}
}

#define DEFAULT_QUANTUM 10
int qLeft;

int get_quantum(struct task_struct *t){
	return t->quantum;
}
void set_quantum(struct task_struct *t,int q){
	t->quantum = q;
}

struct task_struct * idle_task= NULL; //global variable for easy access.

//update scheduling && stats information
void update_sched_data_rr(){
	qLeft--;
}
//necesary change process
int needs_sched_rr(){
	return (qLeft <= 0 && !list_empty(&readyqueue));
}
//update state current
void update_process_state_rr(struct task_struct*t, struct list_head *dst_queue){
	if(t->estado != ST_RUN) list_del(&t->list);
	if(dst_queue == NULL){
		t->estado =ST_RUN;
	}
	else{
		list_add_tail(&t->list, dst_queue);
		if(dst_queue = &readyqueue) t->estado = ST_READY;
		else t->estado=ST_BLOCKED;
	}
	
}
//seleciona siguiente proceso a ejecutar:
void sched_next_rr(void){
	if(!list_empty(&readyqueue)){
		struct list_head* next = list_first(&readyqueue);
		struct task_struct* nextt = list_head_to_task_struct(next);
		qLeft = nextt->quantum;
		struct stats *st;
		st = &nextt->estadisticas;
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
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();	
	}
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
	
	tu->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)&cpu_idle;
	tu->stack[KERNEL_STACK_SIZE - 2] = 0;

	tu->task.kernel_esp = (char *)& (tu->stack[KERNEL_STACK_SIZE - 2]);

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
	qLeft = ts->quantum;

	init_stats(&ts->estadisticas);
	allocate_DIR(ts);
	set_user_pages(ts);
	
	tss.esp0=(DWord)& (tu->stack[KERNEL_STACK_SIZE]);
	writeMsr(0x175, KERNEL_ESP(tu));
	set_cr3(ts->dir_pages_baseAddr);
}


void inner_task_switch(union task_union*t){
	tss.esp0 = KERNEL_ESP(t);
	writeMsr(0x175, (int) KERNEL_ESP(t));

	set_cr3(t->task.dir_pages_baseAddr);
	current() -> kernel_esp = (char *) getEbp();

	setEsp(t->task.kernel_esp);

	return;
}

void init_sched(){
	INIT_LIST_HEAD(& freequeue);
	for(int i = 0; i < NR_TASKS; ++i){	
		task[i].task.PID = -1;
		list_add(&(task[i].task.list), &freequeue);
	}
	INIT_LIST_HEAD(& readyqueue);
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

void update_stats_sysenter(){
	struct stats *aux;
	aux = & current()->estadisticas;
	aux->user_ticks += get_ticks() - aux->elapsed_total_ticks;
	aux->elapsed_total_ticks = get_ticks();	
}
void update_stats_exitSys(){
	struct stats *aux;
	aux = & current()->estadisticas;
	aux->system_ticks += get_ticks() - aux->elapsed_total_ticks;
	aux->elapsed_total_ticks = get_ticks();	
}

void update_stats(unsigned long *v, unsigned long *elapsed){
	unsigned long current_ticks = get_ticks();
	*v += current_ticks - *elapsed;
	*elapsed = current_ticks;
}
