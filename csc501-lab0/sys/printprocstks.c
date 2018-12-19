#include <stdio.h>
#include <kernel.h>
#include <proc.h>

unsigned long	*esp;

void printprocstks(int priority) {
	int i;
	struct pentry *proc;

	kprintf("\nvoid printprocstks(int priority)\n");

	for(i=0; i < NPROC; i++) {

		proc = &proctab[i];

		if(proc->pprio > priority && proc->pstate != PRFREE) {
			
			if(proc->pstate == PRCURR) {
				asm("movl %esp, esp");	
			} else {
				esp = proc->pesp;
			}

			kprintf("Process [%s]\n", proc->pname);
			kprintf("\tpid: %d\n", i);
			kprintf("\tpriority: %d\n", proc->pprio);
			kprintf("\tbase: 0x%08x\n", proc->pbase);
			kprintf("\tlimit: 0x%08x\n", proc->plimit);
			kprintf("\tlen: %d\n", proc->pstklen);
			kprintf("\tpesp: 0x %08x\n", esp);
		}
	}
}