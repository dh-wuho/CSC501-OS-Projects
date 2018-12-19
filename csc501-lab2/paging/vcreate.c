/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	disable(ps);

	int pid = create(procaddr, ssize, priority, name, nargs, args);
	int bs_id;
	if(get_bsm(&bs_id) == SYSERR) {
		kprintf("There is no available backstore.\n");
		restore(ps);
		return SYSERR;
	}

	proctab[pid].store = bs_id;
	proctab[pid].vhpno = VHEAP_H_VPNO;
	proctab[pid].vhpnpages = hsize;
	bsm_map(pid, VHEAP_H_VPNO, bs_id, hsize, VHEAP);
	
	proctab[pid].vmemlist = getmem(sizeof(struct mblock*));
	proctab[pid].vmemlist->mnext = (struct mblock*)roundmb(VHEAP_H_VPNO * NBPG);
    proctab[pid].vmemlist->mlen = hsize*NBPG;

    struct mblock *heap_base = BACKING_STORE_BASE + bs_id * BACKING_STORE_UNIT_SIZE;
    heap_base->mnext = 0;
    heap_base->mlen = hsize * NBPG;

	restore(ps);	
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
