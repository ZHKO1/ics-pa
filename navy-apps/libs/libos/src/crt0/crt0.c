#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = (int)*args;
  char **argv = (char **)(args + 1);
  char **envp = argv;
  while(*envp != NULL){
    envp = envp + 1;
  }
  envp = envp + 1;
  environ = envp;
  __libc_init_array();
  exit(main(argc, argv, envp));
  assert(0);
}
