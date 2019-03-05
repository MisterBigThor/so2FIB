#include <asm.h>



ENTRY(write)
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


ENTRY(writetrap)

	push %ebp;
	movl %esp, %ebp;
	push %ebx;
	push %edi;

	movl 8(%ebp), %ebx 	//pasar a registres els parametres
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx

	movl $0x04, %eax;
	sysenter;

 	cmpl $0, %eax;
	jge fitrap
errtrap:
	negl %eax
	movl %eax, errno;
	movl -1, %eax //retornara -1
fitrap:
	pop %ebx
	pop %ebp;
	ret;

