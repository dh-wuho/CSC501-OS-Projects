/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vvmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;
	disable(ps);

	struct mblock *p, *q;
	unsigned top;
	struct mblock *vmemlist = proctab[currpid].vmemlist;
	unsigned vheap_head = VHEAP_H_VPNO * NBPG;
	unsigned vheap_tail = (VHEAP_H_VPNO + proctab[currpid].vhpnpages) * NBPG;


	if (size==0 || (unsigned)block>(unsigned)vheap_tail
	    || ((unsigned)block)<((unsigned) vheap_head))
		return(SYSERR);
	size = (unsigned)roundmb(size);
	disable(ps);
	for( p=vmemlist->mnext,q= vmemlist;
	     p != (struct mblock *) NULL && p < block ;
	     q=p,p=p->mnext )
		;
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &vmemlist) ||
	    (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
		restore(ps);
		return(SYSERR);
	}
	if ( q!= &vmemlist && top == (unsigned)block )
			q->mlen += size;
	else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}

	restore(ps);
	return(OK);
}
