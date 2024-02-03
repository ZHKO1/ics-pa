#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case MCAUSE_ENVIRONMENTCALL:
        switch (c->GPR1) {
          case -1:
            ev.event = EVENT_YIELD;
            break;
          default:
            ev.event = EVENT_SYSCALL;
            break;
        }
        c->mepc = c->mepc + 4;
        break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  size_t context_size = sizeof(Context);
  void *kstack_start = kstack.start;
  void *kstack_end = kstack.end;
  memset(kstack_start, 0, (uintptr_t)kstack_end - (uintptr_t)kstack_start);
  Context *context = (Context *)((uintptr_t)kstack_end - context_size);
  // 设置上下文的sp寄存器
  // TODO 这里sp寄存器应该要考虑换成抽象层写法
  context->gpr[2] = (uintptr_t)context;
  // 上下文的a1寄存器保存参数arg
  context->gpr[10] = (uintptr_t)arg;
  context->mepc = (uintptr_t)entry;
#ifdef CONFIG_DIFFTEST
  // TODO 经测试，会报mcause寄存器错误，预期值是0x8，实际值是0xb
  // 查询了下0x8，对应 Environment call from U-mode
  // 猜测Spike那边因为什么条件触发判断为用户态，但条件不明
  // 看了下4.4的“用户态和栈指针”，猜测大概是根据栈指针判断
  context->mstatus = 0x1800;
#endif
  return context;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
