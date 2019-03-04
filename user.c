#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
	char * buffer = "aaa";
	int size = 4;
	write(1222, buffer, size);
	//runjp();
	//runjp_rank(3,10);
	while(1) { }
}
