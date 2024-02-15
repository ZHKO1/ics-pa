#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

struct Context {
  // TODO: fix the order of these members to match trap.S
  uintptr_t gpr[NR_REGS], mcause, mstatus, mepc;
  void *pdir;
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

typedef union {
  struct {
    uint32_t page_offset : 12;
    uint32_t VPN0 : 10;
    uint32_t VPN1 : 10;
  } bitfield;
  uint32_t value;
} RISCV32_VA;

typedef union {
  struct {
    uint32_t V : 1;
    uint32_t R : 1;
    uint32_t W : 1;
    uint32_t X : 1;
    uint32_t U : 1;
    uint32_t G : 1;
    uint32_t A : 1;
    uint32_t D : 1;
    uint32_t RSW : 1;
    uint32_t PPN0 : 10;
    uint32_t PPN1 : 12;
  } bitfield_detail;
  struct {
    uint32_t flag : 10;
    uint32_t PPN : 22;
  } bitfield;
  uint32_t value;
} RISCV32_PTE;

typedef RISCV32_VA VA;
typedef RISCV32_PTE PTE;
// TODO 这一部分代码是nemu和AM头文件的重复部分，暂且没想好该如何解决

#endif
