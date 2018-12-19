/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint() {
	STATWORD ps;
	disable(ps);

	virt_addr_t vaddr;
	int bs_id, pageth;
	int pf_addr_vpno;	// record vpno of page fault address
	int frmno;
	unsigned long cr2;
	pd_t *pde;
	pt_t *pte;


	/* 1, get the virtual address that lead to page fault 	*/
	cr2 = read_cr2();

    /* 2, get its vpno 										*/
    pf_addr_vpno = cr2/NBPG;
	
	/* 4, check if this virtual is valid(is it mapped?) 	*
	 * if not, kill current process and return error    	*/
    if(bsm_lookup(currpid, pf_addr_vpno, &bs_id, &pageth) == SYSERR) {
    	kprintf("Virtual address %x is not a legal address!(Page Fault)\n", cr2);
    	kprintf("pid: %d, vpno: %d, bs_id: %d, pageth: %d\n",
            currpid, pf_addr_vpno, bs_id, pageth);
        kill(currpid);
    	restore(ps);
    	return SYSERR;
	}

	/* 3, get the current pdbr, which is the first pde's 	*
	 * physical address										*/
	pde = proctab[currpid].pdbr;

	/* 5,6, get pd, pt offset 								*/
	vaddr.pg_offset = cr2&0x00000FFF;
    vaddr.pt_offset = (cr2&0x003FF000)>>12;
    vaddr.pd_offset = cr2>>22;

    /* 7, if this pt is present, corresponding fr_refcnt++. *
     * It means this frame is referenced by multiple page 	*
     * tables.												*/
    if((pde + vaddr.pd_offset)->pd_pres == 1) {
    	frmno = (pde + vaddr.pd_offset)->pd_base - FRAME0;
    	frm_tab[frmno].fr_refcnt++;
    	frm_tab[frmno].fr_vpno[currpid] = pf_addr_vpno;
    }
    // if not existed, get a frame and map it.
    else {
    	create_pt(currpid, &frmno);

    	// change the pt to present
    	(pde + vaddr.pd_offset)->pd_pres = 1;		/* page table present?		*/
    	(pde + vaddr.pd_offset)->pd_write = 1;
		(pde + vaddr.pd_offset)->pd_user = 0;		/* is use level protection?	*/
		(pde + vaddr.pd_offset)->pd_pwt = 0;		/* write through cachine for pt?*/
		(pde + vaddr.pd_offset)->pd_pcd = 0;		/* cache disable for this pt?	*/
		(pde + vaddr.pd_offset)->pd_acc = 0;		/* page table was accessed?	*/
		(pde + vaddr.pd_offset)->pd_mbz	= 0;		/* must be zero			*/
		(pde + vaddr.pd_offset)->pd_fmb	= 0;		/* four MB pages?		*/
		(pde + vaddr.pd_offset)->pd_global = 0;		/* global (ignored)		*/
		(pde + vaddr.pd_offset)->pd_avail = 0;		/* for programmer's use		*/
		(pde + vaddr.pd_offset)->pd_base = FRAME0 + frmno;
    }

    /* 8, swap and change pages								*
     * get the physical address of the first pte.			*/
    pte = (FRAME0 + frmno) * NBPG;
    lookup_frm(bs_id, pageth, &frmno);
    if(frmno == -1) {
    	get_frm(&frmno);
    	map_frm(currpid, FR_PAGE, frmno, pf_addr_vpno, bs_id, pageth);
    	read_bs((frmno+FRAME0) * NBPG, bs_id, pageth);
    }
    else {
    	frm_tab[frmno].fr_refcnt++;
    	frm_tab[frmno].fr_vpno[currpid] = pf_addr_vpno;
    }  
    (pte + vaddr.pt_offset)->pt_pres = 1;		/* page is present?		*/
	(pte + vaddr.pt_offset)->pt_write = 1;		/* page is writable?		*/
	(pte + vaddr.pt_offset)->pt_user = 0;		/* is use level protection?	*/
	(pte + vaddr.pt_offset)->pt_pwt = 0;		/* write through for this page? */
	(pte + vaddr.pt_offset)->pt_pcd = 0;		/* cache disable for this page? */
	(pte + vaddr.pt_offset)->pt_acc = 0;		/* page was accessed?		*/
	(pte + vaddr.pt_offset)->pt_dirty = 0;		/* page was written?		*/
	(pte + vaddr.pt_offset)->pt_mbz = 0;		/* must be zero			*/
	(pte + vaddr.pt_offset)->pt_global = 0;		/* should be zero in 586	*/
	(pte + vaddr.pt_offset)->pt_avail = 0;		/* for programmer's use		*/
	(pte + vaddr.pt_offset)->pt_base = frmno + FRAME0;		/* location of page?*/	 	

    // flush tlb
    write_cr3(pde);
    restore(ps);
	return OK;
}


