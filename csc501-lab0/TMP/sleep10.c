/* sleep10.c - sleep10 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * sleep10  --  delay the caller for a time specified in tenths of seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep10(int n)
{
	
	/* record syscall counts and start time*/
	unsigned long start = ctr1000;
	if(isTracing) {
		isCalled[currpid] = TRUE;
		proc_syscall_count[currpid][SLEEP10]++;
	}

	STATWORD ps;    
	if (n < 0  || clkruns==0) {

		/* recourd syscall end time*/
		if(isTracing) {
			proc_syscall_time[currpid][SLEEP10] = ctr1000 - start + proc_syscall_time[currpid][SLEEP10];
		}

	    return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep10(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*100);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
	
	/* recourd syscall end time*/
	if(isTracing) {
		proc_syscall_time[currpid][SLEEP10] = ctr1000 - start + proc_syscall_time[currpid][SLEEP10];
	}

	return(OK);
}
