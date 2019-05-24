#include <libc.h>
#include <stats.h>
char buff[24];
unsigned long stackClone[120];
int pid;

#define MODE_USER 455

void doStuff(){
	char buffS[12];
	write(1, "\n", strlen("\n"));
	write(1, "PID ACTUAL: ", strlen("PID ACTUAL: "));
	itoa(getpid(), buffS);//buffer buff hay data race!?
	writefast(1,buffS, strlen(buffS));
	for(int i = 0; i < 10; ++i)
		write(1, "-doStuff-", strlen("-doStuff-"));

	exit();
}

int __attribute__ ((__section__(".text.main")))
main(void)
{
	if(MODE_USER == 1){
		int pidClone = clone(doStuff, stackClone);
		itoa(pidClone, buff);
		writefast(1,buff, strlen(buff));
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
	else if(MODE_USER == 2){
		sem_init(-1, 2);
		perror();
		sem_init(2, 0);
		sem_init(2, 0);
	}
	else if(MODE_USER == 4){
		char readBuff[40];
		int r = read(0,readBuff, 1);
		itoa(r, buff);
		writefast(1,buff, strlen(buff));
		writefast(1,readBuff, strlen(readBuff));
	}
	else{
		runjp();
		//runjp_rank(0,0);
	}

	while(1) { }
}

