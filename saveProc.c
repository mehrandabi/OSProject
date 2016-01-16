#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *pte_address(pde_t *pgdir, const void *va, int alloc)
{
	pde_t *pde;
	pte_t *pgtab;
	
	pde = &pgdir[PDX(va)];
	if(*pde & PTE_P){
		pgtab = (pte_t*)p2v(PTE_ADDR(*pde));
	} else {
		if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
			return 0;
		// Make sure all those PTE_P bits are zero.
		memset(pgtab, 0, PGSIZE);
		// The permissions here are overly generous, but they can
		// be further restricted by the permissions in the page table 
		// entries, if necessary.
		*pde = v2p(pgtab) | PTE_P | PTE_W | PTE_U;
	}
	return &pgtab[PTX(va)];
}

static int save_memory(void *p, uint sz)
{
	pte_t *pte;
	uint pa, i;
	for(i = 0; i < sz; i += PGSIZE){
		if((pte = pte_address(proc->pgdir, (void *) i, 0)) == 0)
			panic("save_memory: pte does not exist");
		if(!(*pte & PTE_P))
			panic("save_memory: page does not exist");
		pa = PTE_ADDR(*pte);
		memmove(p + i, (char*)p2v(pa), PGSIZE);
		cprintf("successfully written at %p\n", p + i);
	}
	
	*((struct trapframe *) (p + sz)) = *(proc->tf);
	cprintf("trapframe dumped successfully... [%p] :)\n", proc->tf->eip);
	
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

int sys_save_proc(void)
{
	char *p = 0;
	if (argptr(0, &p, sizeof(struct proc)) < 0)
		return -1;
	return save_proc((struct proc *) p);
}

int sys_save_memory(void)
{
	char *p = 0;
	int sz = 0;
	if (argint(0, &sz) < 0)
		return -1;
	if (argptr(1, &p, sz + sizeof(struct trapframe)) < 0)
		return -1;
	return save_memory((void *) p, sz);
}
