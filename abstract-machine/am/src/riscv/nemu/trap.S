#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS_LO16(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15)
#ifndef __riscv_e
#define REGS_HI16(f) \
                                    f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)
#define NR_REGS 32
#else
#define REGS_HI16(f)
#define NR_REGS 16
#endif

#define REGS(f) REGS_LO16(f) REGS_HI16(f)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((NR_REGS + 3 + 2) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  ((NR_REGS + 0) * XLEN)
#define OFFSET_STATUS ((NR_REGS + 1) * XLEN)
#define OFFSET_EPC    ((NR_REGS + 2) * XLEN)
#define OFFSET_NP     ((NR_REGS + 4) * XLEN)

#define KERNEL 0
#define USER 1

.align 3
.globl __am_asm_trap
.globl g_np
/*
之前内核态和用户态切换会出bug的原因是因为我理所当然把mscratch寄存器和ksp之间直接挂等号
实际上执行csrrw sp, mscratch, sp指令后，mscratch寄存器就不再是ksp了，所以cte.c里根据mscratch寄存器判断c->np是错误的
正确解法参考了https://htchz.cc/posts/ics-pa/fb155443/
*/
__am_asm_trap:
  csrrw sp, mscratch, sp   # (1) atomically exchange sp and mscratch
  // if (ksp != 0) { $sp = ksp; }
  bnez sp, save_np_user    # (2) take the branch if we trapped from user
  csrr sp, mscratch        # (3) if we trapped from kernel, restore the original sp

save_np_kernel:
  addi sp, sp, -2*XLEN     // push-----------------------
  STORE t0, 0(sp)          // push t0
  STORE t1, XLEN(sp)       // push t1

  lui t0, KERNEL
  la t1, g_np              // load g_np address
  STORE t0, 0(t1)          // store np to g_np

  LOAD t0, 0(sp)           // pop t0
  LOAD t1, XLEN(sp)        // pop t1
  addi sp, sp, 2*XLEN      // pop------------------------
  j save_ctx

save_np_user:
  addi sp, sp, -2*XLEN     // push-----------------------
  STORE t0, 0(sp)          // push t0
  STORE t1, XLEN(sp)       // push t1

  lui t0, USER
  la t1, g_np              // load g_ksp address
  STORE t0, 0(t1)          // store np to g_np

  LOAD t0, 0(sp)           // pop t0
  LOAD t1, XLEN(sp)        // pop t1
  addi sp, sp, 2*XLEN      // pop------------------------

save_ctx:  
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc
  csrr t3, mscratch

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
  STORE t3, OFFSET_SP(sp)     // c->sp = $sp;

  la t0, g_np              // load g_np address
  LOAD t0, 0(t0)           // g_np
  STORE t0, OFFSET_NP(sp)  // c->np = np

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  jal __am_irq_handle

  mv sp, a0

  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mstatus, t1
  csrw mepc, t2

  LOAD t1, OFFSET_NP(sp)
  beqz t1, restore_ctx // if (c->np == USER) { ksp = $sp; } 
  addi t0, sp, CONTEXT_SIZE
  csrw mscratch, t0
restore_ctx:
  MAP(REGS, POP)
  LOAD sp, OFFSET_SP(sp)
  mret
