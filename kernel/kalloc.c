// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct klock{
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct klock kmems[NCPU];

void
kinit()
{
  //initlock(&kmem.lock, "kmem");

  for (int i = 0; i < NCPU; i++)
  {
    initlock(&(kmems[i]).lock, "kmem");
  }

  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int cpu = cpuid();
  pop_off();

  acquire(&(kmems[cpu].lock));
  r->next = kmems[cpu].freelist;
  kmems[cpu].freelist = r;
  release(&(kmems[cpu].lock));

  /*
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock); 
  */
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  struct run *t;

  push_off();
  int cid = cpuid();
  pop_off();

  acquire(&(kmems[cid].lock));

  r = kmems[cid].freelist;
  if (r)
  {
    kmems[cid].freelist = r->next;
  }
  else
  {
    for (int i = 0; i < NCPU; i++)
    {
      if (i == cid)
        continue;
      acquire(&(kmems[i].lock));
      t = kmems[i].freelist;
      if (t)
      {
        r = t;
        
        kmems[i].freelist = t->next;
        release(&(kmems[i].lock));
        break;
      }
      release(&(kmems[i].lock));
    }
  }

  release(&(kmems[cid].lock));

  /*
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);
  */

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk

  return (void*)r;
  
}
