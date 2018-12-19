#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/* requests a new mapping of npages with ID map_id */
int get_bs(bsd_t bs_id, unsigned int npages) {

  	// 256 = backstoreSize(1MB) / pageSize(4KB)
    if((npages < 0) || (npages > 256) || (bs_id < 0) || (bs_id > NBS)) {
    	return SYSERR;
    }

    // if this backstore is used by a process as private heap
    if(bsm_tab[bs_id].bs_pid > 0) {
    	return SYSERR;
    }

    if(bsm_tab[bs_id].bs_status == BSM_MAPPED) {
        bsm_tab[bs_id].bs_xmap_npages += npages;
    	return bsm_tab[bs_id].bs_npages;
    }
    else {
    	bsm_tab[bs_id].bs_status = BSM_MAPPED;
    	bsm_tab[bs_id].bs_pid = NULLPROC;
    	//bsm_tab[bs_id].bs_vpno = 0;
        bsm_tab[bs_id].bs_xmap_npages = npages;
    }

    return npages;
}


