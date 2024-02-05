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

void context_uload(PCB *cur_pcb, const char *filename, char *const argv[], char *const envp[]) {
  uintptr_t entry = core_loader(filename);
  Context *context = ucontext(NULL, (Area) { cur_pcb->stack, (uint8_t *)(&cur_pcb->stack) + STACK_SIZE }, (void *)entry);

  context->GPRx = (uintptr_t)argv;
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
  // 测试hello_fun专用
  context_kload(&pcb[0], hello_fun, (void *)0x1111);

  // 尝试给PAL传递参数
  uintptr_t *area_end_ = (uintptr_t *)heap.end;
  uintptr_t *area_start_ = area_end_ - 20;
  // string area
  char *strptr = (char*)(area_start_ + 10);
  strcpy(strptr, "/bin/pal");
  strcpy(strptr + 10, "--skip");
  // argc
  *area_start_ = 2;
  // argv
  *(area_start_ + 1) = (uintptr_t)strptr;
  *(area_start_ + 2) = (uintptr_t)(strptr + 10);
  *(area_start_ + 3) = 0;
  // envp
  *(area_start_ + 4) = 0;

  context_uload(&pcb[1], "/bin/pal", (char * const*)area_start_, (char * const*)(area_start_ + 4));
  switch_boot_pcb();
  return;

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
