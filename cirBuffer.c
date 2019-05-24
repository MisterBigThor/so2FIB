#include <cirBuffer.h>


void cirBuffWrite(struct cirBuffer* buff, char add){
	if(buff->full) return;
	buff->buffer[buff->writePos] = add;
	buff->writePos += 1;
	if(buff->writePos >= CIRBUFFER_SIZE)
		buff->writePos = 0;
	if(buff->writePos == buff->readPos)
		buff->full = 1;
	return;
}
char cirBuffRead(struct cirBuffer* buff){
	char ret = buff->buffer[buff->readPos++];
	if(buff->readPos == buff->writePos) buff->full = 0;
	return ret;
}
int cirBuffFull(struct cirBuffer* buff){
	return buff->full;
}
int cirBuffLenght(struct cirBuffer* buff){
	if(buff->writePos > buff->readPos)
		return buff->writePos - buff->readPos;
	else if(buff->writePos == buff->readPos){
		if(buff->full) return CIRBUFFER_SIZE;
		else return 0;
	}
	return (CIRBUFFER_SIZE - buff->readPos + buff->writePos);
}