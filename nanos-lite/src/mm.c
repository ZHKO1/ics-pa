#include <memory.h>
#include <proc.h>

void map(AddrSpace *as, void *va, void *pa, int prot);

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *pre_pf = pf;
  size_t size = nr_page << 12;
  void *new_pf = (void *)((uintptr_t)pre_pf + size); // nr_page * 4KB的连续内存区域
  if (new_pf < heap.end) {
    pf = new_pf;
    memset(pre_pf, 0, size);
    return pre_pf;
  } else {
    panic("there is not enough space to allocate pages");
    return NULL;
  }
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  return new_page(ROUNDUP(n, PGSIZE) / PGSIZE);
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  if ( (void *)brk > current->as.area.end ){
    return -1;
  }
  if (current->max_brk < brk) {
    uintptr_t va = ROUNDUP(current->max_brk, PGSIZE);
    for (; va <= ROUNDDOWN(brk, PGSIZE); va += PGSIZE) {
      void *pa = pg_alloc(PGSIZE);
      map(&current->as, (void *)va, (void *)pa, -1);
    }
    current->max_brk = brk;
  }
  return 0;
}

void init_mm() {
  // pf主要是用于new_page函数，为了避免跟klib的malloc（从heap.start开始分配）冲突，因此这里给klib的malloc 0x2000000的空间
  pf = (void *)ROUNDUP((uintptr_t)heap.start + 0x2000000, PGSIZE);
  // TODO 当klib的malloc分配过多内存，依然有可能跟pf冲突，所以可能需要加个报错逻辑

  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
