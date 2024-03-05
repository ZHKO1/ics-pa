#include <common.h>

void do_syscall(Context *c);
Context* schedule(Context *prev);

static Context* do_event(Event e, Context* c) {
  Context *result = c;
  switch (e.event) {
    case EVENT_YIELD:
      result = schedule(c);
      break;
    case EVENT_SYSCALL:
      do_syscall(c);
      break;
    case EVENT_IRQ_TIMER:
      Log("EVENT_IRQ_TIMER");
      result = schedule(c);
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return result;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
