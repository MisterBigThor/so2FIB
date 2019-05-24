/*
 * system.h - Capçalera del mòdul principal del sistema operatiu
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <types.h>

#include <semaphores.h>

extern struct semaphore semaphores[20];

extern struct cirBuffer keyboardBuff;

extern TSS         tss;
extern Descriptor* gdt;

#endif  /* __SYSTEM_H__ */
