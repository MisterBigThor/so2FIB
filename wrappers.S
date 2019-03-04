#include <asm.h>


// int write (int fd, char* buffer, int size);
//los parametros se pasan de derecha(primero) a izquierda(ultimo)
#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; 
     

#define RESTORE_ALL \
	popl %ebx; \
	popl %esi; \
	popl %edi; \
	popl %ebp; \
	popl %ds; \
	popl %es; \
	popl %fs; \
	popl %gs; 

ENTRY(write)
	push %ebp;
	movl %esp, %ebp; //ebp <= esp
	SAVE_ALL
	movl 8(%ebp), %ebx 	//pasar a registres els parametres
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
	movl $4, %eax 	//identificador de llamada
	int $0x80;
 	cmpl $0, %EAX
	jl err;
err:	
	movl %eax, errno;
fi:
	RESTORE_ALL
	pop %ebp;	
	ret;




