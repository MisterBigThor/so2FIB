#include <libc.h>
#include <stats.h>
char buff[24];
unsigned long stackClone[120];
int pid;

#define MODE_USER 1

void doStuff(){
	write(1, "\n", strlen("\n"));
	write(1, "PID ACTUAL: ", strlen("PID ACTUAL: "));
	itoa(getpid(), buff);
	writefast(1,buff, strlen(buff));
	for(int i = 0; i < 10; ++i)
		write(1, "doStuff", strlen("doStuff"));

	exit();
}

int __attribute__ ((__section__(".text.main")))
main(void)
{
	itoa(getpid(), buff);
	write(1, "PID ACTUAL: ", strlen("PID ACTUAL: "));
	writefast(1,buff, strlen(buff));
	int pidClone = clone(doStuff, stackClone);
	itoa(pidClone, buff);
	writefast(1,buff, strlen(buff));
	

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

