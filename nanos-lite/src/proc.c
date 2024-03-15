#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
PCB *fg_pcb = NULL;
PCB *bg_pcb = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void switch_fg_pcb(int i) {
  fg_pcb = &pcb[i];
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
  // context_kload(&pcb[0], hello_fun, (void *)0x1111);

  char user_program_path_0[50] = "/bin/hello";
  char *argv_0[] = {
    user_program_path_0,
    NULL
  };
  context_uload(&pcb[0], user_program_path_0, argv_0, NULL);

  char user_program_path_1[50] = "/bin/nterm";
  char *argv_1[] = {
    user_program_path_1,
    NULL
  };
  context_uload(&pcb[1], user_program_path_1, argv_1, NULL);

  char user_program_path_2[50] = "/bin/nterm";
  char *argv_2[] = {
    user_program_path_2,
    NULL
  };
  context_uload(&pcb[2], user_program_path_2, argv_2, NULL);

  char user_program_path_3[50] = "/bin/nterm";
  char *argv_3[] = {
    user_program_path_3,
    NULL
  };
  context_uload(&pcb[3], user_program_path_3, argv_3, NULL);

  bg_pcb = &pcb[0];
  fg_pcb = &pcb[1];

  switch_boot_pcb();
  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
  static int pcb_count = 0;
  #define PCB_COUNT_MAX 300
  current->cp = prev;
  if (pcb_count < PCB_COUNT_MAX) {
    current = fg_pcb;
    pcb_count++;
  } else if(pcb_count == PCB_COUNT_MAX){
    current = bg_pcb;
    pcb_count = 0;
  }
  switch_addrspace(current->as.ptr);
  return current->cp;
}