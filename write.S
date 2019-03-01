#include <asm.h>


// int write (int fd, char* buffer, int size);
//los parametros se pasan de derecha(primero) a izquierda(ultimo)

ENTRY(write)
	push %ebp;
	movl %esp, %ebp; //ebp <= esp
	movl 8(%ebp), %ebx 	//pasar a registres els parametres
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	movl $4, %eax //identificador de llamada
	int $0x80;
 	cmpl $0, %EAX
	jl err;
err:	movl %eax, errno;
fi:
	pop %ebp;	
	ret;
