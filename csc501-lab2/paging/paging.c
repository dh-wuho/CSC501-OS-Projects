/* paging.c - some functions for paging */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int GPT_frmno[4];
/* initialize 4 global page tables, map them to the first *
 * 16M virtual memory(Virtual:Physical = 1:1)             */
SYSCALL init_gpt() {
	STATWORD ps;
	disable(ps);

	int frmno;
	int i, j;
	for(i = 0; i < 4; i++) {
		get_frm(&frmno);
		map_frm(NULLPROC, FR_TBL, frmno, frmno + FRAME0);

		// get the vpno of the page table
		GPT_frmno[i] = FRAME0 + frmno;
		pt_t *pte = (FRAME0 + frmno) * NBPG;

		// 1024 = pageSize(4KB) / addressSize(4 Bytes for 32 bits)
		for(j = 0; j < 1024; j++) {
			(pte+j)->pt_pres = 1;		/* page is present?		*/
			(pte+j)->pt_write = 1;		/* page is writable?		*/
			(pte+j)->pt_user = 0;		/* is use level protection?	*/
			(pte+j)->pt_pwt = 0;		/* write through for this page? */
			(pte+j)->pt_pcd = 0;		/* cache disable for this page? */
			(pte+j)->pt_acc = 0;		/* page was accessed?		*/
			(pte+j)->pt_dirty = 0;		/* page was written?		*/
			(pte+j)->pt_mbz = 0;		/* must be zero			*/
			(pte+j)->pt_global = 1;		/* should be zero in 586	*/
			(pte+j)->pt_avail = 0;		/* for programmer's use		*/
			(pte+j)->pt_base = i * 1024 + j;		/* location of page?		*/
		}
	}

	restore(ps);
	return OK;
}

// create pd and initialize it
SYSCALL create_pd(int pid) {
	STATWORD ps;
	disable(ps);

	int frmno;
	get_frm(&frmno);
	map_frm(pid, FR_DIR, frmno, frmno + FRAME0, -1, -1);
	proctab[pid].pdbr = (frmno + FRAME0) * NBPG;

	// map 4 global pts for every pd
	int i;
	pd_t *pde = proctab[pid].pdbr;

	// initialize this pd
	for(i = 0; i < 1024; i++) {
		if(i < 4) {
			(pde+i)->pd_pres = 1;
			(pde+i)->pd_global = 1;		/* global (ignored)		*/
			(pde+i)->pd_base = GPT_frmno[i];		/* location of page table?	*/
		} 
		else {
			(pde+i)->pd_pres = 0;
			(pde+i)->pd_global = 0;		/* global (ignored)		*/
			(pde+i)->pd_base = 0;		/* location of page table?	*/
		}
		(pde+i)->pd_write = 1;
		(pde+i)->pd_user = 0;		/* is use level protection?	*/
		(pde+i)->pd_pwt = 0;		/* write through cachine for pt?*/
		(pde+i)->pd_pcd = 0;		/* cache disable for this pt?	*/
		(pde+i)->pd_acc = 0;		/* page table was accessed?	*/
		(pde+i)->pd_mbz	= 0;		/* must be zero			*/
		(pde+i)->pd_fmb	= 0;		/* four MB pages?		*/
		(pde+i)->pd_avail = 0;		/* for programmer's use		*/
	}
	

	restore(ps);
	return OK;
}

// create pt and initialize it
SYSCALL create_pt(int pid, int *frmno) { 
	STATWORD ps;
	disable(ps);

	int i;
	get_frm(frmno);
	map_frm(pid, FR_TBL, *frmno, *frmno + FRAME0, -1, -1);
	pt_t *pte = (FRAME0 + *frmno) * NBPG;

	for(i = 0; i < 1024; i++) {
		(pte+i)->pt_pres = 0;		/* page is present?		*/
		(pte+i)->pt_write = 1;		/* page is writable?		*/
		(pte+i)->pt_user = 0;		/* is use level protection?	*/
		(pte+i)->pt_pwt = 0;		/* write through for this page? */
		(pte+i)->pt_pcd = 0;		/* cache disable for this page? */
		(pte+i)->pt_acc = 0;		/* page was accessed?		*/
		(pte+i)->pt_dirty = 0;		/* page was written?		*/
		(pte+i)->pt_mbz = 0;		/* must be zero			*/
		(pte+i)->pt_global = 0;		/* should be zero in 586	*/
		(pte+i)->pt_avail = 0;		/* for programmer's use		*/
		(pte+i)->pt_base = 0;		/* location of page?		*/
	}
	restore(ps);
	return OK;
}