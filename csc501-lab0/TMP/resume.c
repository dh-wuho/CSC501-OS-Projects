/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
SYSCALL resume(int pid)
{
	
	/* record syscall counts and start time*/
	unsigned long start = ctr1000;
	if(isTracing) {
		isCalled[currpid] = TRUE;
		proc_syscall_count[currpid][RESUME]++;
	}

	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);

		/* recourd syscall end time*/
		if(isTracing) {
			proc_syscall_time[currpid][RESUME] = ctr1000 - start + proc_syscall_time[currpid][RESUME];
		}

		return(SYSERR);
	}
	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);

	/* recourd syscall end time*/
	if(isTracing) {
		proc_syscall_time[currpid][RESUME] = ctr1000 - start + proc_syscall_time[currpid][RESUME];
	}

	return(prio);
}
