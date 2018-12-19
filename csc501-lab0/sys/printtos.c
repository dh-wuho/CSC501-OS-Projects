#include <stdio.h>

unsigned long	*ebp;
unsigned long	*esp;

void printtos() {
	int i;

	asm("movl %ebp, ebp");
	asm("movl %esp, esp");

	kprintf("\nvoid printtos()\n");
	kprintf("Before[0x%08x]: 0x%08x\n", ebp+2, *(ebp+2));
	kprintf("After [0x%08x]: 0x%08x\n", ebp, *(ebp));
	for(i=0; i<4; i++) {
		kprintf("\telement[0x%08x]: 0x%08x\n", esp+i, *(esp+i));
	}
}