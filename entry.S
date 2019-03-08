/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>

/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/
#define EOI \
movb $0x20, %al; \
outb %al, $0x20;

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
    	popl %ebx; \
    	popl %ecx; \
    	popl %edx; \
    	popl %esi; \
    	popl %edi; \
    	popl %ebp; \
    	popl %eax; \
    	popl %ds; \
    	popl %es; \
    	popl %fs; \
    	popl %gs;


ENTRY(keyboard_handler)
	SAVE_ALL
	call keyboard_rutine
	EOI
	RESTORE_ALL
	iret

//"v1"
ENTRY(system_call_handler)
	SAVE_ALL;
	cmpl $0, %eax;
	jl err;
	cmpl $MAX_SYSCALL, %eax;
	jg err;
	call *sys_call_table(,%eax,4);
	jmp fin;
err:
  movl $-ENOSYS, %eax;
fin:
  movl %eax, 0x18(%esp);
	RESTORE_ALL;
	iret;
//V2 sysenter
ENTRY(syscall_handler_sysenter)
	push $__USER_DS
	push %EBP
	pushfl
	push $__USER_CS
	push 4(%EBP)
	SAVE_ALL
	cmpl $0, %EAX
	jl sysenter_err
	cmpl $MAX_SYSCALL, %EAX
	jg sysenter_err
	call *sys_call_table(, %eax, 0x04)
	jmp sysenter_fin
sysenter_err:
	movl $-ENOSYS, %EAX
sysenter_fin:
	movl %EAX, 0x18(%esp)
	RESTORE_ALL
	movl (%esp), %edx
	movl 12(%esp), %ecx
	sti
	sysexit

ENTRY(writeMsr)
	push %ebp
	movl %esp, %ebp
	pushl	%ebx
	pushl   %esi
	pushl   %edi
	movl 8(%ebp), %ecx
	movl 12(%ebp), %eax
	movl $0, %edx
	wrmsr
	pop %edi
	pop %esi
	pop %ebx
	movl %ebp, %esp
	pop %ebp
	ret
