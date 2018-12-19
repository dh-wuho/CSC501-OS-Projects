/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
bs_map_t bsm_tab[NBS];	/* define back stores map tables */

SYSCALL init_bsm() {
	STATWORD ps;
	disable(ps);
	
	int i;
	for(i = 0; i < NBS; i++) {
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = -1;
		bsm_tab[i].bs_vpno = -1;
		bsm_tab[i].bs_npages = 0;
		bsm_tab[i].bs_xmap_npages = 0;
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail) {
	STATWORD ps;
	disable(ps);

	int i;
	for(i = 0; i < NBS; i++) {
		if(bsm_tab[i].bs_status == BSM_UNMAPPED) {
			*avail = i;
			restore(ps);
			return OK;
		}
	}

	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i) {
	STATWORD ps;
	disable(ps);

	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_pid = -1;
	bsm_tab[i].bs_vpno = -1;
	bsm_tab[i].bs_npages =	0;
	
	proctab[currpid].mapbs[i].bs_status = BSM_UNMAPPED;
	proctab[currpid].mapbs[i].bs_pid = -1;
	proctab[currpid].mapbs[i].bs_vpno = -1;
	proctab[currpid].mapbs[i].bs_npages =	-1;

	proctab[currpid].vmemlist = NULL;

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, int vpno, int* bs_id, int* pageth) {
	STATWORD ps;
	disable(ps);

	int i;
	for(i = 0; i < NBS; i++) {
		bs_map_t *pproc_mapbs = &proctab[pid].mapbs[i];
		// if this store is private or unmapped, lookup next
		if(bsm_tab[i].bs_status == BSM_UNMAPPED) {
			continue;
		}
		// if vpno is in the address range of this map
		if((vpno >= pproc_mapbs->bs_vpno) && 
			(vpno < pproc_mapbs->bs_vpno + pproc_mapbs->bs_npages)) {
			*bs_id = i;
			*pageth = vpno - pproc_mapbs->bs_vpno; 
			restore(ps);
			return OK;
		}
	}

	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int bs_id, int npages, int map_type) {
	STATWORD ps;
	disable(ps);
	
	// if the back store is a private store or this store is full.
	if ((bsm_tab[bs_id].bs_pid > 0) || 
		(bsm_tab[bs_id].bs_npages + npages > 256)) {
		restore(ps);
		return SYSERR;
	}

	bsm_tab[bs_id].bs_status = BSM_MAPPED;
	bsm_tab[bs_id].bs_pid = NULLPROC;	// when bs_pid = NULLPROC, it means this store is a public store.
	if(map_type == VHEAP) {
		bsm_tab[bs_id].bs_npages += npages;
	}
	proctab[pid].mapbs[bs_id].bs_status = BSM_MAPPED;
	proctab[pid].mapbs[bs_id].bs_pid = pid;
	proctab[pid].mapbs[bs_id].bs_vpno = vpno;
	proctab[pid].mapbs[bs_id].bs_npages = npages;
	
	restore(ps);
	return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int map_type) {
	STATWORD ps;
	disable(ps);

	int bs_id, pageth;
	int frmno, npages;
	int i;
	// get the bs_id that the vpno in.
	if(bsm_lookup(pid, vpno, &bs_id, &pageth) == SYSERR) {
		restore(ps);
		return SYSERR;
	}

	npages = proctab[pid].mapbs[bs_id].bs_npages - pageth;	 

	for(i = 0; i < npages; i++) {
		lookup_frm(bs_id, pageth+i, &frmno);
		if(frmno != -1) {
			free_frm(pid, frmno);
		}
	}

	proctab[pid].mapbs[bs_id].bs_status = BSM_UNMAPPED;
	proctab[pid].mapbs[bs_id].bs_pid = -1;
	proctab[pid].mapbs[bs_id].bs_vpno = -1;
	proctab[pid].mapbs[bs_id].bs_npages = 0;

	if(map_type == XMMAP) {
		bsm_tab[bs_id].bs_xmap_npages -= npages;
	}
	else{
		bsm_tab[bs_id].bs_npages -= npages;
	}

	if((bsm_tab[bs_id].bs_npages == 0)&&(bsm_tab[bs_id].bs_xmap_npages == 0)) {
		bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
		bsm_tab[bs_id].bs_pid = -1;
	}

	restore(ps);
	return OK;
}