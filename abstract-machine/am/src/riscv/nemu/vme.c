#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  /**
   * 按照我对4.3[支持虚存管理的多道程序]这一段的理解，内核线程上下文的pdir应该一直都是NULL不变
   * 但是在__am_irq_handle函数里对__am_get_cur_as的调用，会将当前页表地址保存到pdir字段，导致内核线程的pdir变化
   * 所以__am_get_cur_as内需要加上判断用户线程还是内核线程的逻辑，这里根据kas.ptr来判断
  */
  void *pdir = (void *)get_satp();
  c->pdir = (vme_enable && (pdir != kas.ptr)? (void *)pdir: NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

// 寻找一级页表里的表项位置，检查是否为空，如果为空就设置表项
static PTE *init_pte_root(PTE *dir_root, uint32_t VPN1) {
  PTE *pte_root = dir_root + VPN1;
  PTE *dir_next = NULL;
  if (pte_root->bitfield_detail.V == 0) {
    dir_next = (PTE*)pgalloc_usr(PGSIZE);
    pte_root->bitfield.PPN = (uintptr_t)dir_next >> 12;
    pte_root->bitfield_detail.V = 1;
    pte_root->bitfield_detail.R = 0;
    pte_root->bitfield_detail.W = 0;
    pte_root->bitfield_detail.X = 0;
  } else {
    dir_next = (PTE *)(pte_root->bitfield.PPN << 12);
  }
  return dir_next;
}

// 寻找二级页表里的表项位置，设置表项
// 如果is_soft是false，根据pa参数设置表项
// 如果is_soft是true，自动申请空间设置表项，除非表项已不为空
static void init_pte_next(PTE *dir_next, uint32_t VPN0, bool is_soft, void *pa) {
  PTE *pte_next = dir_next + VPN0;
  if (is_soft && pte_next->bitfield_detail.V) {
    return;
  }
  if (is_soft) {
    pa = pgalloc_usr(PGSIZE);
  }
  pte_next->bitfield.PPN = (uintptr_t)pa >> 12;  
  pte_next->bitfield_detail.V = 1;
  pte_next->bitfield_detail.R = 1;
  pte_next->bitfield_detail.W = 1;
  pte_next->bitfield_detail.X = 1;

}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  PTE *dir_root = (PTE *)as->ptr;
  
  VA virtual_address = (VA)(uint32_t)va;
  uint32_t VPN1 = virtual_address.bitfield.VPN1;
  uint32_t VPN0 = virtual_address.bitfield.VPN0;

  PTE *dir_next = init_pte_root(dir_root, VPN1);
  init_pte_next(dir_next, VPN0, false, pa);
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  assert(as);

  size_t context_size = sizeof(Context);
  void *kstack_start = kstack.start;
  void *kstack_end = kstack.end;
  memset(kstack_start, 0, (uintptr_t)kstack_end - (uintptr_t)kstack_start);
  Context *context = (Context *)((uintptr_t)kstack_end - context_size);
  context->mepc = (uintptr_t)entry;
  context->pdir = as->ptr;
#ifdef CONFIG_DIFFTEST
  context->mstatus = 0x1800;
#endif
  context->mstatus = context->mstatus | 0x80; 
  return context;
}
