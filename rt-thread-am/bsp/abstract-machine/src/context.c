#include <am.h>
#include <klib.h>
#include <rtthread.h>

static rt_ubase_t global_from = 0;
static rt_ubase_t global_to = 0;

static void rt_hw_wrapper();

static Context* ev_handler(Event e, Context *c) {
  Context *result = c;
  switch (e.event) {
    case EVENT_YIELD:
      if (global_from) {
        *((Context **)(uintptr_t)global_from) = c;
        global_from = 0;
      }
      if (global_to) {
        result = *((Context **)(uintptr_t)global_to);
        global_to = 0;
      }
      break;
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return result;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  global_to = to;
  yield();
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  global_from = from;
  global_to = to;
  yield();
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  uintptr_t *new_stack_addr = (uintptr_t *)stack_addr;
  new_stack_addr = (uintptr_t *)((uintptr_t)new_stack_addr  & ~(sizeof(uintptr_t) - 1));
  
  new_stack_addr = new_stack_addr - 4;
  *(uintptr_t *)(new_stack_addr) = (uintptr_t)tentry;
  *(uintptr_t *)(new_stack_addr + 1) = (uintptr_t)parameter;
  *(uintptr_t *)(new_stack_addr + 2) = (uintptr_t)texit;

  size_t context_size = sizeof(Context);
  Context *context = kcontext((Area) { (void *)new_stack_addr - context_size, (void *)new_stack_addr }, rt_hw_wrapper, NULL);

  return (rt_uint8_t *)context;
}

static void rt_hw_wrapper() {
#if defined(__ISA_RISCV32__)
  asm volatile("lw	t1,0(sp);lw	a0,4(sp);jalr	t1;lw	t3,8(sp);jalr	t3;");
#else
// TODO 这里暂且不清楚native的AM在这情况下该怎么实现，需要结合abstract-machine/am/src/native/下的代码分析
# error unsupported ISA __ISA__
#endif
}