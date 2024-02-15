#include <memory.h>

#define USER_PROGRAM_ADDRESS 0x83000000

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *pre_pf = pf;
  size_t size = nr_page << 12;
  void *new_pf = (void *)((uintptr_t)pre_pf + size); // nr_page * 4KB的连续内存区域
  if (new_pf < (void *)USER_PROGRAM_ADDRESS) {
    pf = new_pf;
    memset(pre_pf, 0, size);
    return pre_pf;
  } else {
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
  if ( (void *)brk > heap.end ){
    return -1;
  }
  return 0;
}

void init_mm() {
  uintptr_t mm_start = (uintptr_t)heap.start;
  uintptr_t mm_end = (uintptr_t)(USER_PROGRAM_ADDRESS);
  pf = (void *)ROUNDUP((mm_start + (mm_end - mm_start) / 2), PGSIZE);
  assert(pf >= (void *)mm_start);
  assert(pf <= (void *)mm_end);

  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
