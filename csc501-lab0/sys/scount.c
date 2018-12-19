/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <proc.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
	
	/* record syscall counts and start time*/
	unsigned long start = ctr1000;
	if(isTracing) {
		isCalled[currpid] = TRUE;
		proc_syscall_count[currpid][SCOUNT]++;
	}

	extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE){
			
		/* recourd syscall end time*/
		if(isTracing) {
			proc_syscall_time[currpid][SCOUNT] = ctr1000 - start + proc_syscall_time[currpid][SCOUNT];
		}

		return(SYSERR);
	}

	/* recourd syscall end time*/
	if(isTracing) {
		proc_syscall_time[currpid][SCOUNT] = ctr1000 - start + proc_syscall_time[currpid][SCOUNT];
	}

	return(semaph[sem].semcnt);
}
