#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

void tester4sem_L(char msg, int sem) {
	int i, j;
	int count = 0;
	kprintf ("  %c starts.\n", msg);
	kprintf ("  %c to decrease sem.\n", msg);
	wait(sem);
	kprintf ("  %c decreased semaphore.\n", msg);
	sleep (1);
	// for hold cpu
	for (i = 0; i < 100000; i++) {
        for (j = 0; j < 10000; j++) {
        }
        count += 1;
    }
	signal(sem);
}

void tester4sem_H(char msg, int sem) {
	kprintf ("  %c starts.\n", msg);
	kprintf ("  %c to decrease sem.\n", msg);
	wait(sem);
	kprintf ("  %c decreased semaphore.\n", msg);
	signal(sem);
}

void tester4lck_L(char msg, int lck) {
	int i, j;
	int count = 0;
	kprintf ("  %c starts.\n", msg);
	kprintf ("  %c to acquire lock.\n", msg);
	lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	kprintf ("  %c acquired lock.\n", msg);
	sleep (1);
	// for hold cpu
	for (i = 0; i < 100000; i++) {
        for (j = 0; j < 10000; j++) {
        }
        count += 1;
    }
	releaseall (1, lck);
}

void tester4lck_H(char msg, int lck) {
	kprintf ("  %c starts.\n", msg);
	kprintf ("  %c to acquire lock.\n", msg);
	lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	kprintf ("  %c acquired lock.\n", msg);
	releaseall (1, lck);
}

void preemptor(char msg, int lck) {
	int i;

	kprintf ("  %c starts.\n", msg);
	kprintf ("  %c holds cpu and ouputs itself.\n", msg);
	for(i = 0; i < 50; i++) {
		kprintf ("%c", msg);
	}
	kprintf("\n");
}

void sem_test() {
	int sem0, sem1, sem2;
	int sem;

	kprintf("\nTest by using semaphores. --- BEGIN\n\n");

	sem = screate(1);
	sem0 = create(tester4sem_L, 2000, 20, "tester4sem_L", 2, 'L', sem);
	sem1 = create(preemptor,  2000, 30, "preemptor",  2, 'M', sem);
	sem2 = create(tester4sem_H, 2000, 50, "tester4sem_H", 2, 'H', sem);

	resume(sem0);
	sleep(1);
	resume(sem2);
	resume(sem1);
	sleep(5);

	sdelete(sem);
	kprintf("\nTest by using semaphores. --- END\n\n");
}

void lock_test() {
	int lck0, lck1, lck2;
	int lck;

	kprintf("\nTest by using locks. --- BEGIN\n\n");

	lck = lcreate();
	lck0 = create(tester4lck_L, 2000, 20, "tester4lck_L", 2, 'L', lck);
	lck1 = create(preemptor,  2000, 30, "preemptor",  2, 'M', lck);
	lck2 = create(tester4lck_H, 2000, 50, "tester4lck_H", 2, 'H', lck);

	resume(lck0);
	sleep(1);
	resume(lck2);
	resume(lck1);
	sleep(5);

	ldelete(lck);
	kprintf("\nTest by using locks. --- END\n\n");
}

int main() {
	
	sem_test();
	lock_test();

	shutdown();
	return 0;
}