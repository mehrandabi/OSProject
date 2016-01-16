#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

static int load_proc(char *path, struct proc *p)
{
	int i;
	struct proc np;
	//an inode describes a single unnamed file
	struct inode *ip;
	begin_op();
	if ((ip = namei(path)) == 0) {
		end_op();
		return -1;
	}
	ilock(ip);
	if((np.pgdir = setupkvm()) == 0)
		return -1;
	if((np.sz = allocuvm(np.pgdir, 0, p->sz)) == 0)
		return -1;
	for(i = 0; i < p->sz; i+=PGSIZE) {
		if(loaduvm(np.pgdir, (void *)i, ip, sizeof(struct proc) + i,PGSIZE) < 0)	
	return -1;
	}
	iunlockput(ip);
	end_op();
	ip = 0;
	
	np.tf->eax = proc->pid;
	np.tf->eip = p->tf->eip;
	np.tf->esp = p->tf->esp;
	np.tf->ebp = p->tf->ebp;
	
	proc->pgdir = np.pgdir;
	proc->sz = PGROUNDUP(np.sz);
	*proc->tf = *np.tf;
	
	switchuvm(proc);
	
	return 0;
}
int sys_load_proc(void)
{
	char *name = 0;
	char *p = 0;
	if (argstr(0, &name) < 0)
		return -1;
	if (argptr(1, &p, sizeof(struct proc)) < 0)
		return -1;
	return load_proc(name, (struct proc *)p);
}


