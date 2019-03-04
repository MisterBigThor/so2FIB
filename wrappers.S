#include <asm.h>


// int write (int fd, char* buffer, int size);
//los parametros se pasan de derecha(primero) a izquierda(ultimo)
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
      popl %ebx;\
      popl %ecx;\
      popl %edx;\
      popl %esi;\
      popl %edi;\
      popl %ebp;\
      popl %eax;\
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs


ENTRY(write)
	push %ebp;
	movl %esp, %ebp; //ebp <= esp
  push %ebx;
	movl 8(%ebp), %ebx 	//pasar a registres els parametres
	movl 12(%ebp), %ecx
	movl 16(%ebp), %edx
  movl $4, %eax;
	SYSCALL
 	cmpl $0, %eax;
  jge fi
err:
  negl %eax
  movl %eax, errno;
  leal errno, %ebx
  movl %eax, (%ebx) //actualizar errno
  movl -1, %eax //retornara -1
fi:
	pop %ebx
	pop %ebp;
	ret;
