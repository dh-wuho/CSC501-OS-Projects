/*
 * This test case is designed to test whether a process can map to one bs within a va space and unmap, then map to another
 * bs within the same va space. Page fault is expected at the second time that the va is referenced.
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

    get_bs(TEST1_BS, 100);

    if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) PROC1_VADDR;
    for (i = 0; i < 26; i++) {
        *(addr + i * NBPG) = 'A' + i;
    }

    sleep(1);

    for (i = 0; i < 26; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    xmunmap(PROC1_VPNO);

    release_bs(TEST1_BS);

    get_bs(TEST2_BS, 100);

    if (xmmap(PROC2_VPNO, TEST2_BS, 100) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) PROC2_VADDR;
    for (i = 0; i < 26; i++) {
        *(addr + i * NBPG) = 'A' + i;
    }

    sleep(1);

    for (i = 0; i < 26; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    xmunmap(PROC2_VPNO);

    release_bs(TEST2_BS);
    return;
}

int main() {

    int pid1;

    kprintf("\n1: shared memory\n");
    pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
    resume(pid1);
    sleep(10);
}
