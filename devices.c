#include <io.h>
#include <utils.h>
#include <list.h>

#include <cirBuffer.h>
#include <sched.h>
#include <system.h>
// Queue for blocked processes in I/O 
struct list_head blocked;

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}

int sys_read_keyboard(char *buff, int size){
	current()->readKeys = 0;
	current()->requiredKeys = size;
	if(!list_empty(&keyboardqueue)){
		update_process_state_rr(current(), &keyboardqueue);
		sched_next_rr();
	}
	while(1){
		int requiredKeys = current()->requiredKeys - current()->readKeys;
		if(cirBuffLenght(& keyboardBuff) >= requiredKeys){
			for(int i = current()->readKeys; i < current()->readKeys + current()->requiredKeys; i++){
				char key = cirBuffRead(& keyboardBuff);
				copy_to_user(key, buff++, 1);
			}
			current()->readKeys += requiredKeys;
			return current()->readKeys;
		}
		else if(cirBuffFull(& keyboardBuff)){
			for(int i = current()->readKeys; i < current()->readKeys + CIRBUFFER_SIZE; i++){
				char key = cirBuffRead(& keyboardBuff);
				copy_to_user(key, buff++, 1);
			}
			current()->readKeys+= CIRBUFFER_SIZE;
		}
		list_add(& (current()->list), &keyboardqueue);
		current()->estado = ST_BLOCKED;
		sched_next_rr();
	}
}