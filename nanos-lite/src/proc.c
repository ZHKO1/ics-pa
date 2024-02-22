#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
size_t pcb_index = 0;

void switch_boot_pcb() {
  current = &pcb_boot;
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
  context_kload(&pcb[0], hello_fun, (void *)0x1111);

  /*
  char *agv[] = {
    "/bin/pal",
    "--skip",
    NULL
  };
  context_uload(&pcb[1], "/bin/pal", agv, NULL);
  */

  char user_program_path[50] = "/bin/pal";
  char *argv[] = {
    user_program_path,
    NULL
  };
  context_uload(&pcb[1], user_program_path, argv, NULL);
  switch_boot_pcb();
  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  pcb_index = (current == &pcb[0] ? 1 : 0);
  current = &pcb[pcb_index];
  /**
   * 按照我对4.3[支持虚存管理的多道程序]这一段的理解，内核线程上下文的pdir应该一直都是NULL不变
   * 但是在__am_irq_handle函数里对__am_irq_handle的调用，会将当前页表地址保存到pdir字段，导致内核线程的pdir变化
   * 但是AM这一层按理说是无法判断用户线程还是内核线程，所以目前只能在nanos-lite这一层的schedule函数里做出丑陋的重新初始化pdir
  */
  if(pcb_index == 0){
    current->cp->pdir=NULL;
  }
  return current->cp;
}

