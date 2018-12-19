/*
 * This is a test case that is trying to vcreate 9 processes, however, only 8 bs is available
 * therefore, a failure is expected to vcreate the last process.
 * */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

void proc1_test1(char *msg, int lck) {
    int *x;

    kprintf("ready to allocate heap space\n");
    x = vgetmem(4096*2);
    kprintf("heap allocated at %x\n", x);
    *x = 100;
    *(x + 1) = 200;

    kprintf("heap variable: %d %d\n", *x, *(x + 1));
    vfreemem(x, 1024);
    
    sleep(2);
}

int main() {
    
    int pid, i;
    
    for (i = 0; i < 9; i++) {
        kprintf("\n2: vgetmem/vfreemem\n");
        pid = vcreate(proc1_test1, 2000, 2, 20, "proc1_test3", 0, NULL);
        if (pid != SYSERR) {
            kprintf("pid %d has private heap\n", pid);
            resume(pid); 
        } else {
            break;
        }
    }
}
