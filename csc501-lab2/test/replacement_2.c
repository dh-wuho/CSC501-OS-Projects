/*
 * This is a test case that to test swapping a page that is dirty.
 * And swap a page that is non-dirty.
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

int main() {

    int i;
    char *addr;

    get_bs(TEST1_BS, 256);
    get_bs(TEST2_BS, 256);
    get_bs(TEST3_BS, 256);
    get_bs(TEST4_BS, 256);

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
        *(addr + i * NBPG) = 'a' + i%26;
    }

    addr = (char*) PROC1_VADDR2;
    for (i = 0; i < 256; i++) {
        *(addr + i * NBPG) = 'a' + i%26;
    }

    addr = (char*) PROC1_VADDR3;
    for (i = 0; i < 256; i++) {
        *(addr + i * NBPG) = 'a' + i%26;
    }

    addr = (char*) PROC1_VADDR4;
    for (i = 0; i < 256; i++) {
        *(addr + i * NBPG) = 'a' + i%26;
    }

    for (i = 0; i < 2; i++) {
        // this 10 pages are supposed to be replaced so now get these 
        // pages back to memory to see whether data is backup.
        addr = (char*) PROC1_VADDR1;
        for (i = 0; i < 256; i++) {
            kprintf("backup data is %c\n", *(addr+i*NBPG));
        }
        addr = (char*) PROC1_VADDR2;
        for (i = 0; i < 256; i++) {
            kprintf("backup data is %c\n", *(addr+i*NBPG));
        }
        addr = (char*) PROC1_VADDR3;
        for (i = 0; i < 256; i++) {
            kprintf("backup data is %c\n", *(addr+i*NBPG));
        }
        addr = (char*) PROC1_VADDR4;
        for (i = 0; i < 256; i++) {
            kprintf("backup data is %c\n", *(addr+i*NBPG));
        }
    }

    sleep(5);
}
