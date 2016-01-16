#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

// Returns the address of the PTE in page table pgdir
static pte_t *pte_address(pde_t *pgdir, const void *virtualadr, int create)
{
	pde_t *pde;
	pte_t *pgtable;
	
	pde = &pgdir[PDX(virtualadr)];
	if(*pde & PTE_P){
		pgtable = (pte_t*)p2v(PTE_ADDR(*pde));
	} else {
		if(!create || (pgtable = (pte_t*)kalloc()) == 0)
			return 0;
		// Make sure all those PTE_P bits are zero.
		memset(pgtable, 0, PGSIZE);
		*pde = v2p(pgtable) | PTE_P | PTE_W | PTE_U;
	}
	return &pgtable[PTX(virtualadr)];
}

static int save_memory(void *pg, uint sz)
{
	pte_t *PTE;
	uint pgadr, k;
	for(k = 0; k < sz; k += PGSIZE){
		if((PTE = pte_address(proc->pgdir, (void *) k, 0)) == 0)
			panic("save_memory: pte does not exist");
		if(!(*PTE & PTE_P))
			panic("save_memory: page does not exist");
		pgadr = PTE_ADDR(*PTE);
		//writing page at address pg + k		
		memmove(pg + k, (char*)p2v(pgadr), PGSIZE);
	}	
	//dumping trapframe	
	*((struct trapframe *) (pg + sz)) = *(proc->tf);
	
	return 0;
}

static int save_proc(struct proc *p)
{
  	if((p->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    		kfree(p->kstack);
    		p->kstack = 0;
    		p->state = UNUSED;
    		return -1;
  	}
  	p->sz = proc->sz;
  	p->pid = proc->tf->eip;
  	safestrcpy(p->name, proc->name, sizeof(proc->name));
	return 0;
}

int sys_save_memory(void)
{
	char *p = 0;
	int sz = 0;
	if (argptr(1, &p, sz + sizeof(struct trapframe)) < 0 || argint(0, &sz) < 0)		return -1;
	return save_memory((void *) p, sz);
}
int sys_save_proc(void)
{
	char *p = 0;
	if (argptr(0, &p, sizeof(struct proc)) < 0)
		return -1;
	return save_proc((struct proc *) p);
}


