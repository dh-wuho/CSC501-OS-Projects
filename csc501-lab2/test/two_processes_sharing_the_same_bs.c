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

void proc1_test1(char *msg, int lck) {
    char *addr;
    int i;

    get_bs(TEST1_BS, 100);

    if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) PROC1_VADDR;
    for (i = 0; i < 26; i++) {
        *(addr + i * NBPG) = 'a' + i;
    }

    sleep(2);

    for (i = 0; i < 26; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    xmunmap(PROC1_VPNO);

    return;
}

void proc1_test2(char *msg, int lck) {

    char *addr;
    int i;

    if (xmmap(PROC2_VPNO, TEST1_BS, 100) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) PROC2_VADDR;
    for (i = 0; i < 26; i++) {
        *(addr + i * NBPG + 1) = 'A' + i;
    }

    sleep(4);

    for (i = 0; i < 26; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG + 1, *(addr + i * NBPG + 1));
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    xmunmap(PROC1_VPNO);
    return;
}


int main() {

    int pid1;

    kprintf("\n1: shared memory\n");
    pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
    resume(pid1);

    sleep(1);

    int pid2;
    kprintf("\n2: shared memory\n");
    pid2 = create(proc1_test2, 2000, 20, "proc1_test2", 0, NULL);
    resume(pid2);

    sleep(1);
}
