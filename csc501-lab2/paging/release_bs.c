#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
	STATWORD ps;
	disable(ps);

  /* release the backing store with ID bs_id */
	if(bsm_tab[bs_id].bs_npages != 0) {
		kprintf("Cann't release this BS, this BS is mapped!\n");
   		restore(ps);
   		return SYSERR;
   	}
   	else {
   		free_bsm(bs_id);
   		restore(ps);
   		return OK;
   	}
}

