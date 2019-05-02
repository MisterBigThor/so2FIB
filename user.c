#include <libc.h>
#include <stats.h>
char buff[24];

int pid;
#define MODE_USER 0
int __attribute__ ((__section__(".text.main")))
main(void)
{

	if(MODE_USER){
		write(1, "Escrito con write \n", strlen("Escrito con write \n"));
		writefast(1,"Escrito con write fast \n",strlen("Escrito con write fast \n"));	
		itoa(getpid(), buff);
		write(1, "PID ACTUAL: ", strlen("PID ACTUAL: "));
		writefast(1, buff, strlen(buff));
		write(1, "\n", strlen("\n"));
				
		struct stats aux;
		int i = fork();
		if(i == 0) {
			writefast(1, "hijo",strlen("hijo\n"));
			get_stats(getpid(), & aux);
			writefast(1, "saliendo hijo\n", strlen("saliendo hijo\n"));
			exit();
		}

		else {
			writefast(1,"padre",strlen("padre\n"));
			itoa(i, buff);
			write(1, "PID hijo: ", strlen("PID HIJO: "));
			write(1, buff, strlen(buff));
			get_stats(getpid(), & aux);
			write(1, "\n", strlen("\n"));
		}
	}

	else{
		runjp();
		//runjp_rank(0,0);
	}

	while(1) { }
}
