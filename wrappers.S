#include <asm.h>



ENTRY(writesys)
	push %ebp;
	movl %esp, %ebp;
	push %ebx;
	push %edi;

	movl 8(%ebp), %ebx 	//pasar a registres els parametres
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx

	movl $0x04, %eax;
	int $0x80;

 	cmpl $0, %eax;
	jge fi
err:
	negl %eax
	movl %eax, errno;
	movl -1, %eax //retornara -1
fi:
	pop %ebx
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
	//pasar a registres els parametres
	movl 8(%ebp), %ebx
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	//identificador de la crida a sistema
	movl $0x04, %eax;
	int $0x80;
	//resultat per %eax
 	cmpl $0, %eax;
	jge fitrap;
errtrap:
	negl %eax;
	movl %eax, errno;
	movl -1, %eax //retornara pq es error -1
fitrap:
	pop %esi; // RESTORE_ALL
	pop %edi;
	pop %edx;
	pop %ecx;
	pop %ebx;
	pop %ebp;
	ret;
