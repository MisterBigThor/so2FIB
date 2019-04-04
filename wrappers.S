#include <asm.h>



//pila antes de una syscall gettime:

//%ebp		<-- %ebp
//@retSys
//%edx
//%ecx
//%ebx
//%ebp
//@retwrapper
ENTRY(gettime)
	push %ebp; //e. dinàmic
	movl %esp, %ebp;

	push %ebx;

	movl $10,%eax; 	
	push %ecx;
	push %edx;
	push $returntime
	push %ebp;
	movl %esp, %ebp; //fake dynamic link
	sysenter;
	returntime:
	pop %ebp;
	pop %edx; //returntime
	pop %edx;
	pop %ecx;
	pop %ebx;
	pop %ebp;
	ret

//pila antes de una syscall write:
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
ENTRY(writefast)
	push %ebp;
	movl %esp, %ebp;

	push %ebx; //salvar ebx i ecx, sysexit els utilitza


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
	movl $-1, %eax //retornara -1
fi:
	pop %edi;
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

ENTRY(getpid)
	push %ebp;
	movl %esp, %ebp;

	push %ebx; //salvar ebx i ecx, sysexit els utilitza
	push %esi;
	push %edi;

	movl $20, %eax; //num syscall y sysenter
	push %ecx;
	push %edx;
	pushl $returnGetPid; 	//guardem @ retorn (seguent a la sysenter)

	push %ebp;
	movl %esp, %ebp; //fake dynamic link
	sysenter;

returnGetPid:
	pop %ebp;
	pop %edx; //pop @ ret
	pop %edx;
	pop %ecx;
	cmpl $0, %eax; //comprobacio resultat, per %eax
	jge fiPid
errPid:
	negl %eax;
	movl %eax, errno;
	movl $-1, %eax //retornara -1
fiPid:
	pop %edi;
	pop %esi;
	pop %ebx;
	movl %ebp, %esp;
	pop %ebp;
ret;

ENTRY(fork)
	push %ebp;
	movl %esp, %ebp;

	push %ebx;
	push %ecx;
	push %edx;
	push $returnfork;
	movl $2, %eax

	push %ebp;
	movl %esp, %ebp; //fake dynamic link
	sysenter;

returnfork:
	pop %ebp;
	pop %edx;	//@ret
	pop %edx;	
	pop %ecx;
	pop %ebx;	
	cmpl $0, %eax; //comprobacio resultat, per %eax
	jge fifork;
errfork:	
	negl %eax;
	movl %eax, errno;
	movl $-1, %eax //retornara -1
fifork:	
	movl %ebp, %esp;
	pop %ebp;
	ret;

ENTRY(exit)
	push %ebp;
	movl %esp, %ebp;

	push %ebx; 
	push %esi;
	push %edi;

	movl $1, %eax; //num syscall y sysenter
	push %ecx;	//salvar ebx i ecx, sysexit els utilitza
	push %edx;

	pushl $returnExit; 	//guardem @ retorn (seguent a la sysenter)

	push %ebp;
	movl %esp, %ebp; //fake dynamic link
	sysenter;

returnExit:
	pop %ebp;
	pop %edx; //pop @ ret
	pop %edx;
	pop %ecx;
fiExit:
	pop %edi;
	pop %esi;
	pop %ebx;
	movl %ebp, %esp;
	pop %ebp;
ret;

ENTRY(get_stats)
	push %ebp;
	movl %esp, %ebp;
	push %ebx; 
	
	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx

	movl $35, %eax; //num syscall y sysenter
	push %ecx;		//salvar ebx i ecx, sysexit els utilitza
	push %edx;
	pushl $returnstats; 	//guardem @ retorn (seguent a la sysenter)

	push %ebp;
	movl %esp, %ebp; //fake dynamic link
	sysenter;

returnstats:
	cmpl $0, %eax; //comprobacio resultat, per %eax
	jge fiExit;

errstats:
	negl %eax;
	movl %eax, errno;
	movl -1, %eax //retornara -1

	pop %ebp;
	pop %edx; //pop @ ret
	pop %edx;
	pop %ecx;
	pop %ebx;
fistats:
	movl %ebp, %esp;
	pop %ebp;

	ret;
