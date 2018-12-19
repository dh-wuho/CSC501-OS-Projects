/*
 * This is a test case that two processes are sharing the same backing store.
 * Here the data race and data synchronization is out of consideration in the paging implementation.
 * Synchronization is handlered by the user processes.
 * */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

#define PROC1_VADDR 0x40000000
#define PROC1_VPNO      0x40000
#define PROC2_VADDR     0x80000000
#define PROC2_VPNO      0x80000
#define TEST1_BS    1
#define TEST2_BS    2

void proc1_test1(char *msg, int lck) {
    char *addr;
    int i;

    get_bs(TEST1_BS, 200);

    if (xmmap(PROC1_VPNO, TEST1_BS, 200) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) PROC1_VADDR;
    for (i = 0; i < 200; i++) {
        *(addr + i * NBPG) = 'a' + i%26;
    }

    sleep(2);

    for (i = 0; i < 200; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    sleep(5);

    xmunmap(PROC1_VPNO);

    return;
}

void proc1_test2(char *msg, int lck) {

    char *addr;
    int i;

    get_bs(TEST2_BS, 200);

    if (xmmap(PROC2_VPNO, TEST2_BS, 200) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) PROC2_VADDR;
    for (i = 0; i < 200; i++) {
        *(addr + i * NBPG) = 'A' + i%26;
    }

    sleep(4);

    for (i = 0; i < 200; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    sleep(5);

    xmunmap(PROC2_VPNO);

    return;
}

void proc1_test3(char *msg, int lck) {
    int *x;
    int i;

    kprintf("ready to allocate heap space\n");
    for (i = 0; i < 200; i++) {
        x = vgetmem(4000);
        if (x == SYSERR) {
            kprintf("error\n");
            break;
        }
        kprintf("heap allocated at %x\n", x);
        *x = 100;
        *(x + 1) = 200;

        kprintf("heap variable: %d %d\n", *x, *(x + 1));
    }

    //vfreemem(x, 1024);
}

int main() {

    int pid1;

    kprintf("\n1: shared memory\n");
    pid1 = create(proc1_test1, 200, 20, "proc1_test1", 0, NULL);
    resume(pid1);

    sleep(1);

    int pid2;
    kprintf("\n2: shared memory\n");
    pid2 = create(proc1_test2, 200, 20, "proc1_test2", 0, NULL);
    resume(pid2);

    sleep(1);

    int pid3;
    kprintf("\n3: vheap\n");
    pid3 = vcreate(proc1_test3, 200, 200, 20, "proc1_test3", 0, NULL);
    resume(pid3);

    int pid4;
    kprintf("\n3: vheap\n");
    pid4 = vcreate(proc1_test3, 200, 200, 20, "proc1_test4", 0, NULL);
    resume(pid4);
}
