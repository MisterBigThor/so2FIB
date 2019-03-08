#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
	//runjp();
	//writefast(1, "abcd", 4);
	int t = gettime();
	char e[4];
	itoa(t, e);
	writefast(1, e, 4);
	int s = 1;
	for(int i = 0; i < 10000000; ++i){
		s += 2;
	}

	t = gettime();
	itoa(t, e);
	writefast(1, e, 4);
	while(1) { }
}
