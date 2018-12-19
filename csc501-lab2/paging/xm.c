/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage_no, bsd_t bs_id, int npages) {
	STATWORD ps;
	disable(ps);

	if((bs_id < 0) || (bs_id > NBS)|| (virtpage_no < 4096) 
		|| (npages < 0) || (npages > 256)) {
		restore(ps);
		return SYSERR;
	}

	// size of file that is mapped must less than or equal to file
	if(bsm_tab[bs_id].bs_xmap_npages < npages) {
		kprintf("The size that will be mapped is less than the size from get_bs.\n");
		restore(ps);
		return SYSERR;	
	}

	// back store must be mapped
	if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED) {
		restore(ps);
		return SYSERR;
	}

	// back store must be public
	if(bsm_tab[bs_id].bs_pid > 0) {
		restore(ps);
		return SYSERR;
	}
	
	// map
	if(bsm_map(currpid, virtpage_no, bs_id, npages, XMMAP) == SYSERR) {
      	restore(ps);
      	return SYSERR;
	}

	restore(ps);
 	return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage_no) {
	STATWORD ps;
	disable(ps);

	if(virtpage_no < 4096) {
		restore(ps);
		return SYSERR;
	}
	
	bsm_unmap(currpid, virtpage_no, XMMAP);

	restore(ps);
 	return OK;
}
