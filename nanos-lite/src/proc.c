#include <proc.h>

#define MAX_NR_PROC 4

void naive_uload(PCB *pcb, const char *filename);

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void context_kload (PCB *cur_pcb, void(*fun)(void *), void *args) {
  Context *context = kcontext((Area) { cur_pcb->stack, (uint8_t *)(&cur_pcb->stack) + STACK_SIZE }, fun, args);
  cur_pcb->cp = context;
}

void context_uload (PCB *cur_pcb, const char *filename) {
  uintptr_t entry = core_loader(filename);
  Context *context = ucontext(NULL, (Area) { cur_pcb->stack, (uint8_t *)(&cur_pcb->stack) + STACK_SIZE }, (void *)entry);
  context->GPRx = (uintptr_t)heap.end;
  cur_pcb->cp = context;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    // for (int volatile i = 0; i < 1000000; i++) ;
    yield();
  }
}

void init_proc() {
  /*
  // 测试hello_fun专用
  context_kload(&pcb[0], hello_fun, (void *)0x1111);
  context_uload(&pcb[1], "/bin/pal");
  switch_boot_pcb();
  return;
  */

  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}
