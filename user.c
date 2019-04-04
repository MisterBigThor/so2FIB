#include <libc.h>
#include <stats.h>
char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	

	runjp();

	//runjp_rank(0,0);
	//writefast(1, "abcd", 4);
	//int i = getpid();
	//char a[4];
	//itoa(i, a);
	//writefast(1,a,strlen(a));	
	//runjp_rank(2,32);

	
	struct stats aux;
	int i = fork();
	if(i == 0) {
		writefast(1, "hijo",strlen("hijo\n"));
		get_stats(getpid(), & aux);
		exit();
	}

	else {
		writefast(1,"padre",strlen("padre\n"));
		get_stats(getpid(), & aux);
	}
	


//	runjp();
	//runjp_rank(6,12);
	//runjp_rank(14,32);
	while(1) { }
}
