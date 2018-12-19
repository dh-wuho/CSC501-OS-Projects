/*
 * This is a test case:
 * One process is mapping to one backing store, write something to the va and then unmap the backing store explicitly.
 * Then the process reuse the same bs again by releasing the bs.
 * At this time, the older data is expected to be cleaned for the release.
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
    
    kprintf("the first time of mapping\n");

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

    get_bs(TEST1_BS, 100);

    kprintf("the second time of mapping\n");

    if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    for (i = 0; i < 26; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    addr = (char*) PROC1_VADDR;
    for (i = 0; i < 26; i++) {
        *(addr + i * NBPG) = 'a' + i;
    }

    sleep(1);

    for (i = 0; i < 26; i++) {
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
    sleep(10);
}
