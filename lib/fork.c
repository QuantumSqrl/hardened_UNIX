// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if(!((uvpt[PGNUM(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_COW))){
			panic("pgfault");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	if(sys_page_alloc(0, PFTEMP, PTE_W | PTE_P | PTE_U) < 0){
		panic("pgfault");
	}

	memcpy((void*) PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);

	if(sys_page_map(0, PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_W | PTE_P | PTE_U) < 0){
		panic("pgfault");
	}

	if(sys_page_unmap(0, PFTEMP) < 0){
		panic("pgfault");
	}
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 4: Your code here.
	if(uvpt[pn] & PTE_SHARE){
		void *page = (void*) (pn * PGSIZE);
		if(sys_page_map(0, page, envid, page, uvpt[pn] & PTE_SYSCALL) < 0){
			return -1;
		}
		return 0;
	}
	if((uvpt[pn] && PTE_W | PTE_COW) != 0)
	{
		void *page = (void*) (pn * PGSIZE);

		if(sys_page_map(0, page, envid, page, PTE_U | PTE_P | PTE_COW) < 0){
			panic("duppage");
		}

		if(sys_page_map(0, page, 0, page, PTE_U | PTE_P | PTE_COW) < 0){
			panic("duppage");
		}
	}else{
		void *page = (void*) (pn * PGSIZE);

		if(sys_page_map(0, page, envid, page, PTE_U | PTE_P) < 0){
			panic("duppage");
		}
	}
	return 0;

}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	envid_t childenv = sys_exofork();
	uintptr_t size = UXSTACKTOP - PGSIZE;

	if(childenv < 0){
		panic("fork");
	}else if(childenv == 0){
		thisenv = &envs[ENVX(sys_getenvid())];
		return childenv;
	}else{
		for(uintptr_t space = 0; space < UTOP; space += PGSIZE){
			if(!(uvpd[PDX(space)] & PTE_P)){		
				continue;
			}else if((space != size) && (uvpt[PGNUM(space)] & PTE_P)){
				duppage(childenv, PGNUM(space));
			}
		}

		sys_page_alloc(childenv, (void*)(UXSTACKTOP - PGSIZE), PTE_U | PTE_W);
		extern void _pgfault_upcall();
		sys_env_set_pgfault_upcall(childenv, _pgfault_upcall);
		sys_env_set_status(childenv, ENV_RUNNABLE);
		return childenv;
	}	

}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
