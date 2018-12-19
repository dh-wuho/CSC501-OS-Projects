/*
 * This is a test case that to replace a non-dirty page.
 * */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

#define PROC1_VADDR1 0x40000000
#define PROC1_VPNO1      0x40000
#define PROC1_VADDR2 0x50000000
#define PROC1_VPNO2      0x50000
#define PROC1_VADDR3 0x60000000
#define PROC1_VPNO3      0x60000
#define PROC1_VADDR4 0x70000000
#define PROC1_VPNO4      0x70000
#define TEST1_BS    1
#define TEST2_BS    2
#define TEST3_BS    3
#define TEST4_BS    4

void proc_write(int vpno, int bs) {

    char *addr;
    int i;

    if (xmmap(vpno, bs, 256) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) (vpno << 12);
    for (i = 0; i < 256; i++) {
        *(addr + i * NBPG) = 'A' + i % 26;
    } 

    sleep(2);

    xmunmap(vpno);

    return;
}

void proc_read(int vpno, int bs) {

    char *addr;
    int i;

    if (xmmap(vpno, bs, 256) == SYSERR) {
        kprintf("xmmap call failed\n");
        sleep(3);
        return;
    }

    addr = (char*) (vpno << 12);
    for (i = 0; i < 256; i++) {
        kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
    }

    return;
}

int main() {

    int i, pid;
    char *addr;

    get_bs(TEST1_BS, 256);
    get_bs(TEST2_BS, 256);
    get_bs(TEST3_BS, 256);
    get_bs(TEST4_BS, 256);

    pid = create(proc_write, 2000, 20, "testing1", 2, PROC1_VPNO1, TEST1_BS);
    resume(pid);
    pid = create(proc_write, 2000, 20, "testing2", 2, PROC1_VPNO2, TEST2_BS);
    resume(pid);
    pid = create(proc_write, 2000, 20, "testing3", 2, PROC1_VPNO3, TEST3_BS);
    resume(pid);
    pid = create(proc_write, 2000, 20, "testing4", 2, PROC1_VPNO4, TEST4_BS);
    resume(pid);

    sleep(6);

    kprintf("finished writing\n");

    if (xmmap(PROC1_VPNO1, TEST1_BS, 256) == SYSERR) {
        kprintf("xmmap call failed 1\n");
        sleep(3);
        return;
    }

    if (xmmap(PROC1_VPNO2, TEST2_BS, 256) == SYSERR) {
        kprintf("xmmap call failed 1\n");
        sleep(3);
        return;
    }

    if (xmmap(PROC1_VPNO3, TEST3_BS, 256) == SYSERR) {
        kprintf("xmmap call failed 1\n");
        sleep(3);
        return;
    }

    if (xmmap(PROC1_VPNO4, TEST4_BS, 256) == SYSERR) {
        kprintf("xmmap call failed 1\n");
        sleep(3);
        return;
    }

    addr = (char*) PROC1_VADDR1;
    for (i = 0; i < 256; i++) {
        //*(addr + i * NBPG) = 'a' + i%26;
        kprintf("backup data is %c\n", *(addr + i * NBPG));
    }

    addr = (char*) PROC1_VADDR2;
    for (i = 0; i < 256; i++) {
        //*(addr + i * NBPG) = 'a' + i%26;
        kprintf("backup data is %c\n", *(addr + i * NBPG));
    }

    addr = (char*) PROC1_VADDR3;
    for (i = 0; i < 256; i++) {
        //*(addr + i * NBPG) = 'a' + i%26;
        kprintf("backup data is %c\n", *(addr + i * NBPG));
    }

    addr = (char*) PROC1_VADDR4;
    for (i = 0; i < 256; i++) {
        //*(addr + i * NBPG) = 'a' + i%26;
        kprintf("backup data is %c\n", *(addr + i * NBPG));
    }

    /*
    pid = create(proc_read, 2000, 20, "testinga", 2, PROC1_VPNO1, TEST1_BS);
    resume(pid);
    pid = create(proc_read, 2000, 20, "testingb", 2, PROC1_VPNO2, TEST2_BS);
    resume(pid); 
    pid = create(proc_read, 2000, 20, "testingc", 2, PROC1_VPNO3, TEST3_BS);
    resume(pid);
    pid = create(proc_read, 2000, 20, "testingd", 2, PROC1_VPNO4, TEST4_BS);
    resume(pid);
    */

    sleep(1);
}
