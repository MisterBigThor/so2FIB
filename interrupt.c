/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <system.h>
#include <sched.h>
#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;
extern int zeos_ticks;
extern struct task_struct * idle_task;
extern struct  list_head readyqueue;
char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};
//===========RUTINAS DE SERVICIO===========
//service rutine keyboard
void keyboard_rutine(){
	unsigned char entrada = inb(0x60);
	unsigned char MakeOrBreak = entrada >> 7;
 	unsigned char ScanCode = entrada & 0x7f ;
	if(!MakeOrBreak){ //make 0, break 1
		char c = char_map[ScanCode];
		if(c == '\0') printc_xy(0x00, 0x00, 'C');
		else printc_xy(0x00, 0x00, c);
    cirBuffWrite(&keyboardBuff, c);
    if(!list_empty(& keyboardqueue)){
      struct list_head * l = list_first(&keyboardqueue);
      struct task_struct *ts = list_head_to_task_struct(l);
      if(cirBuffFull(&keyboardBuff) || 
        cirBuffLenght(&keyboardBuff)>= ts->requiredKeys - ts->readKeys)
        update_process_state_rr(ts, &readyqueue);
    }
	}
	return;

}
//Service rutine clock
void clock_rutine(){
  zeos_show_clock();
  zeos_ticks += 1;
  schedule();
  return;  
}



void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void setIdt()
{
	/* Program interrups/exception service routines */
	idtR.base  = (DWord)idt;
	idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

	set_handlers();
	/* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
	writeMsr(0x174,__KERNEL_CS);
	writeMsr(0x175,INITIAL_ESP);
	writeMsr(0x176, syscall_handler_sysenter);
	/*INICIALIZAR TALBA IDT*/
	setInterruptHandler(33, keyboard_handler, 0);
	setInterruptHandler(0x80, system_call_handler, 3);
  setInterruptHandler(32, clock_handler ,3);
	set_idt_reg(&idtR);
}

