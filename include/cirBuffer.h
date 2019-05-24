#ifndef CIRBUFFER
#define CIRBUFFER


#define CIRBUFFER_SIZE 16


struct cirBuffer{
	int writePos, readPos;
	unsigned int full;
	char buffer[CIRBUFFER_SIZE];
};


void cirBuffWrite(struct cirBuffer* buff, char add);
char cirBuffRead(struct cirBuffer* buff);
int cirBuffFull(struct cirBuffer* buff);
int cirBuffLenght(struct cirBuffer* buff);

#endif