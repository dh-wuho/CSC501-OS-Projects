/* getitem.c - getfirst, getlast */

#include <conf.h>
#include <kernel.h>
#include <q.h>

/*------------------------------------------------------------------------
 * getfirst  --	 remove and return the first process on a list
 *------------------------------------------------------------------------
 */
int getfirst(int head)
{
	int	proc;			/* first process on the list	*/

	if ((proc=q[head].qnext) < NPROC)
		return( dequeue(proc) );
	else
		return(EMPTY);
}



/*------------------------------------------------------------------------
 * getlast  --  remove and return the last process from a list
 *------------------------------------------------------------------------
 */
int getlast(int tail)
{
	int	proc;			/* last process on the list	*/

	if ((proc=q[tail].qprev) < NPROC)
		return( dequeue(proc) );
	else
		return(EMPTY);
}

/*------------------------------------------------------------------------
 * getitembykey  --  get item by key from queue tail. Get the first item
 * if there are several item whose keys are equal.
 *------------------------------------------------------------------------
 */
int getitembykey(int tail, int key) {
	
	int prev;

	prev = q[tail].qprev;
	while(q[prev].qkey > key) {
		prev = q[prev].qprev;
	}

	if(q[prev].qkey == key) {
		return dequeue(prev);
	} 
	else {
		return EMPTY;
	}
}

/*------------------------------------------------------------------------
 * getitembyrvalue  --  get item by key from queue tail. Get the first item
 * if there are several item whose keys are equal.
 *------------------------------------------------------------------------
 */
int getitembyrvalue(int tail, double rvalue) {

	int prev;
	int next;

	prev = q[tail].qprev;
	while(rvalue < q[prev].qkey) {
		prev = q[prev].qprev;
	}
	//rr_count++;
	if(prev == q[tail].qprev) {
		return dequeue(prev);
	}
	next = q[prev].qnext;
	while(q[next].qkey == q[q[next].qnext].qkey) {
		next = q[next].qnext;
		rr_count++;
	}
	return dequeue(next);		
}