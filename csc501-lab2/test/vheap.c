/*
 * Test cases for vheap: normal case, heap overflow, use up the vheap.
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
    int *x;

    kprintf("ready to allocate heap space\n");
    x = vgetmem(4096*2);
    kprintf("heap allocated at %x\n", x);
    *x = 100;
    *(x + 1) = 200;

    kprintf("heap variable: %d %d\n", *x, *(x + 1));
    vfreemem(x, 1024);
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
    int *x;
    int i;

    kprintf("ready to allocate heap space\n");
    for (i = 0; i < 10; i++) {
        x = vgetmem(1024);
        kprintf("heap allocated at %x\n", x);
        *x = 100;
        *(x + 1) = 200;

        kprintf("heap variable: %d %d\n", *x, *(x + 1));
    }

    //vfreemem(x, 1024);
}

int main() {
    int pid1, pid2, pid3;

    kprintf("\n1: vgetmem/vfreemem, requests memory space larger than vheap.\n");
    pid1 = vcreate(proc1_test1, 2000, 1, 20, "proc1_test2", 0, NULL);
    kprintf("pid %d has private heap\n", pid1);
    resume(pid1);
    sleep(3);

    kprintf("\n2: vgetmem/vfreemem\n");
    pid2 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
    kprintf("pid %d has private heap\n", pid2);
    resume(pid2);
    sleep(3);

    kprintf("\n2: vgetmem/vfreemem\n");
    pid3 = vcreate(proc1_test3, 2000, 2, 20, "proc1_test3", 0, NULL);
    kprintf("pid %d has private heap\n", pid3);
    resume(pid3);
    sleep(3);
}
