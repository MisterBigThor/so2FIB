#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
	char * buffer = "aaaabbbbc \n";
	int size = 10;
	int n = write(1, buffer, size);
	if(n != 4)	perror();
	//perror();
	runjp();
	//runjp_rank(1,1);
	while(1) { }
}
