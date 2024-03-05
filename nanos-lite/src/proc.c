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
  static int pcb_count = 0;
  #define PCB_COUNT_MAX 200
  current->cp = prev;
  if (pcb_count < PCB_COUNT_MAX) {
    pcb_index = 1;
    pcb_count++;
  } else if(pcb_count == PCB_COUNT_MAX){
    pcb_index = 0;
    pcb_count = 0;
  }
  current = &pcb[pcb_index];
  return current->cp;
}