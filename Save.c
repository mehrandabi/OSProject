#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};
struct trapframe {
  // registers as pushed by pusha
  uint edi;
  uint esi;
  uint ebp;
  uint oesp;      // useless & ignored
  uint ebx;
  uint edx;
  uint ecx;
  uint eax;

  // rest of trap frame
  ushort gs;
  ushort padding1;
  ushort fs;
  ushort padding2;
  ushort es;
  ushort padding3;
  ushort ds;
  ushort padding4;
  uint trapno;

  // below here defined by x86 hardware
  uint err;
  uint eip;
  ushort cs;
  ushort padding5;
  uint eflags;

  // below here only when crossing rings, such as from user to kernel
  uint esp;
  ushort ss;
  ushort padding6;
};

int main()
{
	int i;
	struct proc *p;
	void *pg;
	
	p = malloc(sizeof(struct proc));
	
	for (i = 0; i < 5; i++)
		printf(1, "%d\n", i);

	if (save_proc(p) == 0) {
		pg = malloc(p->sz + sizeof(struct trapframe));
	
		if (save_memory(pg, p->sz) == 0) {
			printf(1, "memory saved successfuly..\n");
			int fd = open("STATE.bin", O_WRONLY | O_CREATE);
			if (write(fd, p, sizeof(struct proc)) < sizeof(struct proc))
				return -1;
			if (write(fd, pg + p->sz, sizeof(struct trapframe)) < sizeof(struct trapframe))
				return -1;
			if (write(fd, pg, p->sz) < p->sz)
				return -1;
			printf(1, "Wrote in  STATE.bin");
		
		}
	}

	for (; i < 15; i++)
		printf(1, "%d\n", i);

	free(pg);
	free(p);

	exit();
}
