/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>
#include <math.h>
#include <stdio.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
// record the quantity of processes whose priorities are the same.
int rr_count = 1;	

/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	
	int sched_class = getschedclass();
	if(sched_class == 1) {
		return expdistsched();
	}
	if(sched_class == 2) {
		return linuxsched();
	}
	if(sched_class == 3) {
		return defaultsched();
	}
	return SYSERR;
}

int defaultsched() {

	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
	
	#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
	#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}

int expdistsched() {

	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	double rvalue;

	/* round-robin is implemented when the function resched 		*
	 * manipulates readylist. 										*/

	optr = &proctab[currpid];
	if(optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid, rdyhead, optr->pprio);
	}

	if((rr_count != 0)&&(preempt == 0)) {
		nptr = &proctab[currpid = getitembykey(rdytail, optr->pprio)];
	}
	else {
		rr_count = 1;
		rvalue = expdev(0.1);
		nptr = &proctab[currpid = getitembyrvalue(rdytail, rvalue)];
	}
	
	nptr->pstate = PRCURR;
	
	#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
	#endif

	rr_count--;
 
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	return OK;
}

int linuxsched() {
	
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	int next;

	optr = &proctab[currpid];
	optr->counter = preempt;
	if(optr->counter == 0) {
		optr->goodness = 0;
	}	
	
	if(isnewepoch()) {
		if((optr->pstate == PRCURR) && (optr->counter > 0)) {
			#ifdef RTCLOCK
				preempt = optr->counter;
			#endif
			return OK;
		}
		newepoch();
	}
	
	next = getnext();
	if(optr->pstate == PRCURR) {
		if(next == currpid) {
			#ifdef RTCLOCK
				preempt = optr->counter;
			#endif
			return OK;
		}
		optr->pstate = PRREADY;
		insert(currpid, rdyhead, optr->pprio);
	}
	
	nptr = &proctab[(currpid = dequeue(next))];
	nptr->pstate = PRCURR;

	if(currpid == 0) {
		#ifdef RTCLOCK
			preempt = 1;
		#endif
	} 
	else {
		#ifdef RTCLOCK
			preempt = nptr->counter;
		#endif
	}

	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	return OK;
}

int isnewepoch() {
	int i;
	for(i = 0; i < NPROC; i++) {
		if((proctab[i].pstate == PRREADY)&&(proctab[i].counter > 0)) {
			return FALSE;
		}
	}
	return TRUE;
}

int newepoch() {
	int i;
	for(i = 0; i < NPROC; i++) {
		if(proctab[i].pstate != PRFREE) {
			proctab[i].goodness = proctab[i].pprio + proctab[i].counter;
			proctab[i].counter = proctab[i].pprio + proctab[i].counter/2;		
		}
	}
	return TRUE;
}

int getnext() {
	int i;
	int next = 0;
	int maxgoodness = 0;
	for(i = 0; i < NPROC; i++) {
		if((proctab[i].pstate == PRREADY)&&(proctab[i].goodness > maxgoodness)) {
			next = i;
			maxgoodness = proctab[i].goodness;
		}
	}
	return next;
}