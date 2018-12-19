#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>


#define PROC1_VADDR	0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS	8

void proc1_test1_A(char *msg, int lck) {
	char *addr;
	int i, j;

	for(i = 0; i < 8; i++) {
		get_bs(i, 1);

		if (xmmap(PROC1_VPNO + i * 0x1000, i, 1) == SYSERR) {
			kprintf("xmmap call failed\n");
			sleep(3);
			return;
		}
	}

	for(i = 0; i < 8; i++) {
		addr = (char*) PROC1_VADDR + i * 0x1000 * NBPG;
		for (j = 0; j < 1; j++) {
			*(addr + j * NBPG) = 'A' + j % 26;
		}		
	}

	kprintf("Process A --- Before sleep\n");
	for(i = 0; i < 8; i++) {
		addr = (char*) PROC1_VADDR + i * 0x1000 * NBPG;
		for (j = 0; j < 1; j++) {
			kprintf("0x%08x: %c\n", addr + j * NBPG, *(addr + j * NBPG));
		}		
	}

	sleep(50);
	
	kprintf("Process A --- After sleep\n");
	for(i = 0; i < 8; i++) {
		addr = (char*) PROC1_VADDR + i * 0x1000 * NBPG;
		for (j = 0; j < 1; j++) {
			kprintf("0x%08x: %c\n", addr + j * NBPG, *(addr + j * NBPG));
		}		
	}

	for(i = 0; i < 8; i++) {
		xmunmap(PROC1_VPNO + i * 0x1000);
	}

	return;
}

void proc1_test1_B(char * msg, int lck) {
	char *addr;
	int i, j;

	for(i = 0; i < 8; i++) {
		get_bs(i, 1);

		if (xmmap(PROC1_VPNO + i * 0x1000, i, 1) == SYSERR) {
			kprintf("xmmap call failed\n");
			sleep(3);
			return;
		}
	}

	sleep(50);

	kprintf("Process B --- After sleep\n");
	for(i = 0; i < 8; i++) {
		addr = (char*) PROC1_VADDR + i * 0x1000 * NBPG;
		for (j = 0; j < 1; j++) {
			kprintf("0x%08x: %c\n", addr + j * NBPG, *(addr + j * NBPG));
		}		
	}

	for(i = 0; i < 8; i++) {
		xmunmap(PROC1_VPNO + i * 0x1000);
	}

	return;	
}

void proc1_test1_C(char * msg, int lck) {
	
	char *addr;
	int i, j;

	for(i = 0; i < 8; i++) {
		get_bs(i, 1);

		if (xmmap(PROC1_VPNO + i * 0x1000, i, 1) == SYSERR) {
			kprintf("xmmap call failed\n");
			sleep(3);
			return;
		}
	}

	for(i = 0; i < 8; i++) {
		addr = (char*) PROC1_VADDR + i * 0x1000 * NBPG;
		for (j = 0; j < 1; j++) {
			*(addr + j * NBPG) = '0' + j % 10;
		}		
	}

	kprintf("Process C --- After changing\n");
	for(i = 0; i < 8; i++) {
		addr = (char*) PROC1_VADDR + i * 0x1000 * NBPG;
		for (j = 0; j < 1; j++) {
			kprintf("0x%08x: %c\n", addr + j * NBPG, *(addr + j * NBPG));
		}		
	}

	for(i = 0; i < 8; i++) {
		xmunmap(PROC1_VPNO + i * 0x1000);
	}

	return;	
}

void proc1_test2(char *msg, int lck) {
	int *x;

	kprintf("ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;

	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x, 1024);
}

void proc1_test3(char *msg, int lck) {

	char *addr;
	int i;

	addr = (char*) 0x0;

	for (i = 0; i < 1024; i++) {
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 1024; i++) {
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}

int main() {	
	int pid1;
	int pid2;
	int pid3;

	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1_A, 200, 20, "proc1_test1_A", 0, NULL);
	resume(pid1);
	sleep(3);
	kprintf("next create b\n");
	pid2 = create(proc1_test1_B, 200, 20, "proc1_test1_B", 0, NULL);
	resume(pid2);
	sleep(3);
	kprintf("next create c\n");
	pid3 = create(proc1_test1_C, 200, 20, "proc1_test1_C", 0, NULL);
	resume(pid3);
	sleep(3);
	resume(pid1);
	sleep(3);
	resume(pid2);
	sleep(3);

	/*kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);

	kprintf("\n3: Frame test\n");
	pid1 = create(proc1_test3, 2000, 20, "proc1_test3", 0, NULL);
	resume(pid1);
	sleep(3);*/

}
