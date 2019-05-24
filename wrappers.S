#include <asm.h>
/*
Para hacer una llamada a sistema hace falta:
	pasar el numero de llamada por 
	%eax, salvar %edx y %ecx (sysexit lo utiliza)
	push @retorno
	fake dinamic link
	trap
	pops(4)
	resultado sysenter y return
*/
#define SYSCALL(syscalln) \
	leal syscall_ret##syscalln, %eax; \
	push %eax; \
	movl $syscalln, %eax; \
	push %ebp; \
	movl %esp, %ebp; \
	sysenter; \
	syscall_ret##syscalln: pop %ebp; \
	addl $4, %esp

#define CHECK_ERROR(end) \
	cmpl $0, %eax; \
	jge  end; \
	negl %eax; \
	leal errno, %ebx; \
	movl %eax, (%ebx); \
	movl $-1, %eax

ENTRY(gettime)
	push %ebp; 
	movl %esp, %ebp;
	SYSCALL(10)
	pop %ebp;
	ret;

ENTRY(writefast)
	push %ebp
	movl %esp, %ebp
	push %ebx
	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	SYSCALL(4);
	CHECK_ERROR(write.no_error);
write.no_error:
	pop %ebx;
	pop %ebp;
	ret;

ENTRY(read)
	push %ebp;
	movl %esp, %ebp;
	push %ebx;
	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	SYSCALL(30);
	CHECK_ERROR(read.noError);
read.noError:
	pop %ebx;
	pop %ebp;
	ret;

ENTRY(sbrk)
	push %ebp;
	movl %esp, %ebp;
	push %ebx;
	movl 8(%ebp), %ebx
	SYSCALL(31)
	CHECK_ERROR(sbrk.noError);
sbrk.noError:
	pop %ebx;
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
	SYSCALL(20);
	pop %ebp;
	ret;

ENTRY(fork)
	push %ebp
	movl %esp, %ebp
	push %ebx
	SYSCALL(2)
	CHECK_ERROR(fork.no_error)
	fork.no_error:
	pop %ebx;
	pop %ebp;
	ret;

ENTRY(exit)
	push %ebp;
	movl %esp, %ebp;
	SYSCALL(1);	
	pop %ebp;
	ret;

ENTRY(get_stats)
	push %ebp
	movl %esp, %ebp
	push %ebx
	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	SYSCALL(35)
	CHECK_ERROR(get_stats.no_error)
	get_stats.no_error:
	pop %ebx
	pop %ebp
	ret

ENTRY(clone)
	push %ebp;
	movl %esp, %ebp;
	push %ebx;
	movl 8(%ebp), %ebx;
	movl 12(%ebp), %ecx;
	SYSCALL(19)
	CHECK_ERROR(clone.noError)
	clone.noError:
	pop %ebx;
	pop %ebp;
	ret;

ENTRY(sem_init)
	push %ebp
	movl %esp, %ebp;
	push %ebx;
	movl 8(%ebp),%ebx;
	movl 12(%ebp),%ecx;
	SYSCALL(21);
	CHECK_ERROR(semInit.noErr)
	semInit.noErr:
	popl %ebx;
	popl %ebp;
	ret;

ENTRY(sem_wait)
	push %ebp
	movl %esp, %ebp;
	push %ebx;
	movl 8(%ebp), %ebx;
	SYSCALL(22);
	CHECK_ERROR(semWait.noErr)
	semWait.noErr:
	popl %ebx;
	popl %ebp;
	ret;

ENTRY(sem_signal)
	push %ebp
	movl %esp, %ebp;
	push %ebx;
	movl 8(%ebp), %ebx;
	SYSCALL(23);
	CHECK_ERROR(semSignal.noErr)
	semSignal.noErr:
	popl %ebx;
	popl %ebp;
	ret;	

ENTRY(sem_destroy)
	push %ebp
	movl %esp, %ebp;
	push %ebx;
	movl 8(%ebp), %ebx;
	SYSCALL(24);
	CHECK_ERROR(semDestroy.noErr)
	semDestroy.noErr:
	popl %ebx;
	popl %ebp;
	ret;	
