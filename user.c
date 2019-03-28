#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
	runjp();
	//writefast(1, "abcd", 4);
	//int i = getpid();
	//char a[4];
	//itoa(i, a);
	//writefast(1,a,strlen(a));
	while(1) { }
}
