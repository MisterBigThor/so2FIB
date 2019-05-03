#ifndef _SEMA
#define _SEMA
#include <list.h>

enum state_sem {FREE_SEM,USED_SEM};

struct semaphore{
	enum state_sem state;
	struct task_struct* owner;
	int counter;
	struct list_head blockedQueue;
};


#endif