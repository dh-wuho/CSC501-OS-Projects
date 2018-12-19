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

    addr = (char*) PROC1_VADDR1;
    //reset the accessed bit
    kprintf("backup data is %c\n", *(addr+10*NBPG));
    //this statement is expected to trigger a page fault, but no free frame is left.
    //and the frame to be evicted should be the page with virtual address (PROC1_VADDR1+11*NBPG)
    //however, this frame is reseted therefore, it would not be selected by the replacement policy. 
    kprintf("backup data is %c\n", *(addr+1*NBPG));

    /*
     * The expected output is:
     * evict frame 1031, and the dirty flag is 1.
     * evict frame 1032, and the dirty flag is 1.
     * evict frame 1033, and the dirty flag is 1.
     * evict frame 1034, and the dirty flag is 1.
     * evict frame 1035, and the dirty flag is 1.
     * evict frame 1036, and the dirty flag is 1.
     * evict frame 1037, and the dirty flag is 1.
     * evict frame 1038, and the dirty flag is 1.
     * evict frame 1039, and the dirty flag is 1.
     * evict frame 1040, and the dirty flag is 1.
     * backup data is k
     * evict frame 1042, and the dirty flag is 1. // frame 1041 is skipped since it is reseted by accessing it again.
     * backup data is b
     * */
    sleep(5);
}
