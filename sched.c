/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct * idle_task; //global variable for easy access.

struct list_head freequeue;
struct list_head readyqueue;


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

	while(1)
	{
	;
	}
}
/*
struct TASK_STRUCT: 	int PID;		
  				page_table_entry * dir_pages_baseAddr;
  				char * kernel_esp;
  				struct list_head list;
union TASK_UNION:
  				struct task_struct task;
  				unsigned long stack[1023]; 
*/
void init_idle (void)
{
	struct list_head * aux = list_first(& freequeue);
	list_del(aux);
	struct task_struct * ts = list_head_to_task_struct(aux);
	ts->PID = 0; //asign PID 0
	allocate_DIR(ts); //asign DIR
	union task_union * tu = (union task_union *) ts;
	tu->task.kernel_esp = & (tu->stack[KERNEL_STACK_SIZE - 2]);
	tu->stack[KERNEL_STACK_SIZE - 2] = 0;
	tu->stack[KERNEL_STACK_SIZE - 1] = & cpu_idle;

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
	writeMsr(0x175, KERNEL_ESP(tu));

	set_cr3(ts->dir_pages_baseAddr);
}


void inner_task_switch(union task_union*t){
	tss.esp0 = KERNEL_ESP(t);
	writeMsr(0x175, (int) KERNEL_ESP(t));

	if(current() -> dir_pages_baseAddr == t->task.dir_pages_baseAddr)
		set_cr3(t->task.dir_pages_baseAddr);
	current() -> kernel_esp = (char *) getEbp();

	setEsp(t->task.kernel_esp);
	return;
}

void init_sched(){
	INIT_LIST_HEAD(& freequeue);
	for(int i = 0; i < NR_TASKS; ++i){	
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

