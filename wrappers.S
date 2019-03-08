#include <asm.h>




//%ebp 	<---%ebp
//@ret syscall
//%edi
//%esi
//%ebx
//%EBP
//@ret	+4
//			+8
//			+12
//			+16
ENTRY(writefast)
	push %ebp;
	movl %esp, %ebp;

	push %ebx; //salvar ebx i ecx, sysexit els utilitza
	push %esi;
	push %edi;

	movl 8(%ebp), %ebx 	//pasar a registres els parametres
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	movl $0x04, %eax; //# syscall y sysenter
	push %ecx;
	push %edx;
	pushl returnSysEnter; 	//guardem @ retorn (seguent a la sysenter)

	push %ebp;
	movl %esp, %ebp; //fake dynamic link
	sysenter;

returnSysEnter:
	pop %ebp;
	pop %edx; //@ ret
	pop %edx;
	pop %ecx;
	cmpl $0, %eax; //comprobacio resultat, per %eax
	jge fi
err:
	negl %eax;
	movl %eax, errno;
	movl -1, %eax //retornara -1
fi:
	pop %edi;
	pop %esi;
	pop %ebx;
	movl %ebp, %esp;
	pop %ebp;
	ret;



ENTRY(write)
	//enllaç dinàmic
	push %ebp;
	movl %esp, %ebp;
	//salvar registres:
 	push %ebx;
	push %ecx;
	push %edx;
	push %edi;
	push %esi;
	push %eax;
	//pasar a registres els parametres
	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	//identificador de la crida a sistema
	movl $0x04, %eax;
	int $0x80;
	//resultat per %eax
returntrap:
	cmpl $0, %eax;
	jge fitrap;
errtrap:
	negl %eax;
	movl %eax, errno;
	movl $-1, (%esp) //retornara pq es error -1
fitrap:
	pop %eax;
	pop %esi; // RESTORE_ALL
	pop %edi;
	pop %edx;
	pop %ecx;
	pop %ebx;
	pop %ebp;
	ret;
