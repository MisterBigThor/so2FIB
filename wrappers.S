#include <asm.h>


//pila general antes de una syscall:
//STACK ABANS SYSCALL:
//%ebp 		<---%esp, %ebp
//@ret-syscall
//%edi
//%esi
//%ebx
//%ebp
//@ret		+4
//p1		+8
//p2		+12
//p3		+16

ENTRY(gettime)
	push %ebp; //e. dinàmic
	movl %esp, %ebp;

	push %ebx; //salvar ebx i ecx, sysexit els utilitza
	push %esi;
	push %edi;

	movl $10,%eax; 	
	push %ecx;
	push %edx;
	push $returntime
	push %ebp;
	movl %esp, %ebp; //fake dynamic link
	sysenter;
returntime:
	pop %edi;
	pop	%esi;	
	pop %ebx;
	pop %ebp;
	ret

ENTRY(writefast)
	push %ebp;
	movl %esp, %ebp;

	push %ebx; //salvar ebx i ecx, sysexit els utilitza
	push %esi;
	push %edi;

	movl 8(%ebp), %ebx 	//pasar a registres els parametres
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	movl $0x04, %eax; //num syscall y sysenter
	push %ecx;
	push %edx;
	pushl $returnSysEnter; 	//guardem @ retorn (seguent a la sysenter)

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
	movl $-1, %eax //retornara pq es error -1
fitrap:
	pop %ebx;
	pop %ebp;
	ret;
