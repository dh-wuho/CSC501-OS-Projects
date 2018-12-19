/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
fr_map_t frm_tab[NFRAMES]; /* define frame map tables */
frm_node *frm_qhead;

SYSCALL init_frm()
{
	STATWORD ps;
	disable(ps);
	int i, j;

	for(i = 0; i < NFRAMES; i++) {
		frm_tab[i].fr_status = FRM_UNMAPPED;
		for(j = 0; j < NPROC; j++) {
			frm_tab[i].fr_vpno[j] = -1;
		}
		frm_tab[i].fr_bsid = -1;
		frm_tab[i].fr_bspageth = -1;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_dirty = 0;
	}

	init_frmlist();


	restore(ps);
  	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail) {
	STATWORD ps;
	disable(ps);
	int i;

	for(i = 0; i < NFRAMES; i++) {
		if(frm_tab[i].fr_status == FRM_UNMAPPED) {
			*avail = i;
			restore(ps);
			return OK;
		}
	}

	// if all frames are used, replacement
	if(evict_frm(avail) == SYSERR) {
		kprintf("get_frm error, replacement failed.\n");
		restore(ps);
		return SYSERR;
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int pid, int frmno) {
	STATWORD ps;
	disable(ps);


	int vpno = frm_tab[frmno].fr_vpno[pid];
	unsigned long long_vaddr;
	pd_t *pde;
	pt_t *pte;
	virt_addr_t vaddr;
	int bs_id, pageth;
	if(debug) {
		kprintf("free_frm -- frm_tab[%d].fr_vpno[%d]: %d\n", frmno, pid, vpno);
		switch(frm_tab[frmno].fr_type) {
			case FR_PAGE: {
				kprintf("free_frm -- frm_tab[%d].fr_type: PAGE\n");
				break;
			}
			case FR_TBL: {
				kprintf("free_frm -- frm_tab[%d].fr_type: TBL\n");
				break;
			}
			case FR_DIR: {
				kprintf("free_frm -- frm_tab[%d].fr_type: DIR\n");
				break;
			}
		}		
	}

	long_vaddr = vpno * NBPG;
	vaddr.pg_offset = long_vaddr&0x00000FFF;
    vaddr.pt_offset = (long_vaddr&0x003FF000)>>12;
    vaddr.pd_offset = long_vaddr>>22;

    pde = proctab[pid].pdbr + sizeof(pd_t) * vaddr.pd_offset;
    pte = pde->pd_base * NBPG + sizeof(pt_t) * vaddr.pt_offset;

    /*if(pde->pd_pres != 1) {
    	kprintf("free_frm error -- PDE is not present.\n");
    	kprintf("pid: %d, vpno: %d, frmno: %d\n", pid, vpno, frmno);
    }

    if(pte->pt_pres != 1) {
    	kprintf("free_frm error -- PDE is not present..\n");
    	kprintf("pid: %d, vpno: %d, frmno: %d\n", pid, vpno, frmno);
    }*/

    if(debug) {
    	kprintf("vaddr: %x\n", long_vaddr);
    	kprintf("pd_offset: %d\n", vaddr.pd_offset);
    	kprintf("pt_offset: %d\n", vaddr.pt_offset);
    	kprintf("pg_offset: %d\n", vaddr.pg_offset);
    	kprintf("pdbr: %08x\n", proctab[pid].pdbr);
    	kprintf("pde: 0x%8x\n", pde);
    	kprintf("pd_base: %d\n", pde->pd_base);
    }

    switch(frm_tab[frmno].fr_type) {
    
    	case FR_PAGE: {
    		if(bsm_lookup(pid, vpno, &bs_id, &pageth) == SYSERR) {
    			kprintf("proctab[%d].mapbs[%d].vpno: %d\n", pid, bs_id, proctab[pid].mapbs[bs_id].bs_vpno);
    			kprintf("Virtual address %x is not a legal address!(Free FRAME)\n", long_vaddr);
    			kill(currpid);
    			restore(ps);
    			return SYSERR;
    		}

    		// unmap a page from this frame
    		frm_tab[frmno].fr_refcnt -= 1;
    		
    		// if this frame has 0 mapped page,
    		// write back and set pte unpresent,
    		// refcnt of the frame that pt is in minus one
    		if(frm_tab[frmno].fr_refcnt == 0) {
    			if(pte->pt_dirty == 1) {
    				write_bs(pte->pt_base * NBPG, bs_id, pageth);	
    			}
    			pte->pt_pres = 0;	
    			reset_frm(frmno);
    			delete_frm_queue(frmno);
    			frm_tab[pde->pd_base - FRAME0].fr_refcnt -= 1;
    			if(frm_tab[pde->pd_base].fr_refcnt == 0) {
    				free_frm(pid, pde->pd_base - FRAME0);
    			}
    		}
    		else {
    			frm_tab[frmno].fr_vpno[pid] = -1;
    		}
    		break;
    	}
    
    	case FR_TBL: {
    		//frm_tab[frmno].fr_refcnt -= 1;
    		if(frm_tab[frmno].fr_refcnt == 0) {
    			pde->pd_pres = 0;	
   				reset_frm(frmno);
    		}
    		break;
    	}

    	case FR_DIR: {
    		reset_frm(frmno);
    		break;
    	}
    }

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * map_frm - map a frame
 *-------------------------------------------------------------------------
 */
SYSCALL map_frm(int pid, int type, int frmno, int vpno, bsd_t bs_id, int bspageth) {
	STATWORD ps;
	disable(ps);

	frm_tab[frmno].fr_status = FRM_MAPPED;
	frm_tab[frmno].fr_vpno[pid] = vpno;
	frm_tab[frmno].fr_refcnt += 1;
	frm_tab[frmno].fr_type = type;
	frm_tab[frmno].fr_bsid = bs_id;
	frm_tab[frmno].fr_bspageth = bspageth;
	if(type == FR_PAGE) {
		insert_frm_queue(frmno);
	}

	if(debug) {
		kprintf("map_frm -- frm_tab[%d].fr_vpno[%d]: %d\n", frmno, pid, vpno);
		switch(frm_tab[frmno].fr_type) {
			case FR_PAGE: {
				kprintf("map_frm -- frm_tab[%d].fr_type: PAGE\n");
				break;
			}
			case FR_TBL: {
				kprintf("map_frm -- frm_tab[%d].fr_type: TBL\n");
				break;
			}
			case FR_DIR: {
				kprintf("map_frm -- frm_tab[%d].fr_type: DIR\n");
				break;
			}
		}		
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * lookup_frm - look up a frame which contains the same content as the page
 * map in now. For xmmap.
 *-------------------------------------------------------------------------
 */
SYSCALL lookup_frm(bsd_t bs_id, int bspageth, int* frmno) {
	STATWORD ps;
	disable(ps);

	int i;
	for(i = 0; i < NFRAMES; i++) {
		if(frm_tab[i].fr_status == FRM_UNMAPPED) {
			continue;
		} 
		if((frm_tab[i].fr_bsid == bs_id)&&(frm_tab[i].fr_bspageth == bspageth)) {
			*frmno = i;
			restore(ps);
			return OK;
		}
	}

	*frmno = -1;
	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * reset_frm - free a frame 
 *-------------------------------------------------------------------------
 */
void reset_frm(int frmno) {
	int i;
	frm_tab[frmno].fr_status = FRM_UNMAPPED;
	for(i = 0; i < NPROC; i++) {
		frm_tab[frmno].fr_vpno[i] = -1;
	}
	frm_tab[frmno].fr_refcnt = 0;
	frm_tab[frmno].fr_type = -1;
	frm_tab[frmno].fr_bsid = -1;
	frm_tab[frmno].fr_bspageth = -1;
	frm_tab[frmno].fr_dirty = 0;
}

/*-------------------------------------------------------------------------
 * evict_frm - evict a frame to implement relacement
 *-------------------------------------------------------------------------
 */
SYSCALL evict_frm(int *target_frmno) {
	STATWORD ps;
	disable(ps);

	pd_t *pde;
	pt_t *pte;
	int curr_frmno;
	int curr_frm_pid;
	int curr_frm_vpno;
	int min_age = 256;
	int i;
	*target_frmno = -1;
	frm_node *next = frm_qhead->next_node;
	
	if(grpolicy() == SC) {
		while(1) {
			curr_frmno = next->node_frmno;

			for(i = 0; i < NPROC; i++) {
				if(frm_tab[curr_frmno].fr_vpno[i] != -1) {
					curr_frm_pid = i;
					curr_frm_vpno = frm_tab[curr_frmno].fr_vpno[i];
					break;
				}
			}

			pde = proctab[curr_frm_pid].pdbr + curr_frm_vpno / 1024 * sizeof(pd_t);
			if(pde->pd_pres != 1) {
				kprintf("curr_frm_pid: %d, curr_frm_vpno: %d, curr_frmno: %d\n", 
					curr_frm_pid, curr_frm_vpno, curr_frmno);
				kprintf("evict_frm error! PDE is not present!\n");
				restore(ps);
				return SYSERR;
			}
			pte = pde->pd_base * NBPG + curr_frm_vpno % 1024 * sizeof(pt_t);
			if(pte->pt_pres != 1) {
				kprintf("curr_frm_pid: %d, curr_frm_vpno: %d, curr_frmno: %d\n", 
					curr_frm_pid, curr_frm_vpno, curr_frmno);
				kprintf("evict_frm error! PTE is not present!\n");
				restore(ps);
				return SYSERR;
			}

			if(pte->pt_acc == 1) {
				pte->pt_acc = 0;
			}
			else {
				*target_frmno = curr_frmno;
				break;
			}		

			next = next->next_node;
			if((next == NULL)&&(*target_frmno == -1)) {
				next = frm_qhead->next_node;
			}
		}
	}

	if(grpolicy() == AGING) {
		while(next != NULL) {
			next->age = next->age>>1;
			curr_frmno = next->node_frmno;
			for(i = 0; i < NPROC; i++) {
				if(frm_tab[curr_frmno].fr_vpno[i] != -1) {
					curr_frm_pid = i;
					curr_frm_vpno = frm_tab[curr_frmno].fr_vpno[i];
					break;
				}
			}

			pde = proctab[curr_frm_pid].pdbr + curr_frm_vpno / 1024 * sizeof(pd_t);
			if(pde->pd_pres != 1) {
				kprintf("curr_frm_pid: %d, curr_frm_vpno: %d, curr_frmno: %d\n", 
					curr_frm_pid, curr_frm_vpno, curr_frmno);
				kprintf("evict_frm error! PDE is not present!\n");
				restore(ps);
				return SYSERR;
			}
			pte = pde->pd_base * NBPG + curr_frm_vpno % 1024 * sizeof(pt_t);
			if(pte->pt_pres != 1) {
				kprintf("curr_frm_pid: %d, curr_frm_vpno: %d, curr_frmno: %d\n",
					curr_frm_pid, curr_frm_vpno, curr_frmno);
				kprintf("evict_frm error! PTE is not present!\n");
				restore(ps);
				return SYSERR;
			}
			
			if(pte->pt_acc == 1) {
				next->age += 128;
				next->age %= 256;
			}
		}

		next = frm_qhead;
		while(next != NULL) {
			if(next->age < min_age) {
				min_age = next->age;
				*target_frmno = next->node_frmno;
			}
			next = next->next_node;
		}
	}

	for(i = 0; i < NPROC; i++) {
		if(frm_tab[*target_frmno].fr_vpno[i] != -1) {
			curr_frm_pid = i;
			free_frm(curr_frm_pid, *target_frmno);
		}
	}
	

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * some functions for sc queue
 *-------------------------------------------------------------------------
 */
void init_frmlist() {
	frm_qhead = getmem(sizeof(frm_node));
	frm_qhead->node_frmno = -1;
	frm_qhead->age = -1;
	frm_qhead->next_node = NULL;
}

void insert_frm_queue(int frmno) {
	frm_node *next = frm_qhead;
	frm_node *newnode = getmem(sizeof(frm_node));;
	
	newnode->node_frmno = frmno;
	newnode->age = 0;
	newnode->next_node = NULL;

	while(next->next_node != NULL) {
		next = next->next_node;
	}
	
	next->next_node = newnode;
}

void delete_frm_queue(int frmno) {
	frm_node *next = frm_qhead;
	frm_node *prev;

	while(next->next_node != NULL) {
		if(next->node_frmno == frmno) {
			prev->next_node = next->next_node;
			freemem(next, sizeof(frm_node));
			break;
		}
		prev = next;
		next = next->next_node;
	}
}

void printFrmList() {
	frm_node *next = frm_qhead;
	int i = 0;
	kprintf("index: %d, frmno: %d, age: %d, next: %x\n", i, next->node_frmno, next->age, next->next_node);
	while(next->next_node != NULL) {
		i++;
		next = next->next_node;
		kprintf("index: %d, frmno: %d, age: %d, next: %x\n", i, next->node_frmno, next->age, next->next_node);
	}
}

void printFrmTab() {
	int i;
	for(i = 0; i < NFRAMES; i++) {
		kprintf("frmno: %d, status: %d, type: %d, refcnt: %d\n",
				i, frm_tab[i].fr_status, frm_tab[i].fr_type, frm_tab[i].fr_refcnt);
	}
}