#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  argint(n, &fd);
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// copied mappages but cooler (no valid yo)
int fill_pgtbl_lazy(pagetable_t pgtbl, uint64 va, int size)
{
  uint64 a, last;
  pte_t *pte;

  if(size == 0)
    return 0;
  if (size < 0)
    return -1;
  
  // copied from mappages but does not mark valid 
  // will be replaced by mapping to physical page
  a = PGROUNDDOWN(va);
  last = PGROUNDDOWN(va + size - 1);
  for(;;){
    if((pte = walk(pgtbl, a, 1)) == 0)
      return -1;
    if(*pte & PTE_V)
      panic("cyber: remap");
    *pte = 0;
    if(a == last)
      break;
    a += PGSIZE;
  }
  return 0;
}


uint64
sys_mmap(void)
{
  struct proc* p = myproc();
  pagetable_t pgtbl = p->pagetable;
  struct file* f = 0;

  int len, prot, flags, fd;
  argint(1, &len);
  argint(2, &prot);
  argint(3, &flags);
  argint(4, &fd);
  argfd(4, 0, &f);
  
  fill_pgtbl_lazy(pgtbl, p->sz, len);
  p->sz += len;

  filedup(f);
  // don't allow writing to shared read-only file
  if ((get_prot(f) & prot) == 0 && (prot & 0x2) && (flags & 0x01))
    return -1;

  set_vma(&p->vmas[p->nvmas], fd, f, len, PGROUNDDOWN(p->sz), prot, flags);
  p->nvmas++;

  return p->sz;
}

uint64
sys_munmap(void)
{
  uint64 addr;
  int len;
  struct proc* p = myproc();

  argaddr(0, &addr);
  argint(1, &len);

  return unmap(p, p->pagetable, addr, len);
}
