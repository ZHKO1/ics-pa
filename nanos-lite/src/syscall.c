#include <am.h>
#include <common.h>
#include "syscall.h"

int mm_brk(uintptr_t brk);

static inline void strace(char *system_call_name, Context *c, bool ingnore_ret) {
#ifdef CONFIG_ETRACE
  if (ingnore_ret) {
    Log("%s (%2d, %2d, %2d, %2d) = ???", system_call_name, c->GPR1, c->GPR2, c->GPR3, c->GPR4 );    
  } else {
    Log("%s (%2d, %2d, %2d, %2d) = %2d", system_call_name, c->GPR1, c->GPR2, c->GPR3, c->GPR4, c->GPRx );
  }
#endif
}

static inline void strace_ret(char *system_call_name, Context *c) {
      strace(system_call_name, c, false);
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  switch (a[0]) {
    case SYS_exit:
      int exit_status = c->GPR2;
      strace("exit", c, true);
      halt(exit_status);
      break;
    case SYS_yield:
      yield();
      c->GPRx = 0;
      strace_ret("yield", c);
      break;
    case SYS_write:
      int fd = c->GPR2;
      char *buf = (char *)c->GPR3;
      size_t len = c->GPR4;
      if ((fd == 1) || (fd == 2)) {
        for (size_t i = 0; i < len; i++) putch(*(buf + i));
        c->GPRx = len;
      } else {
        panic("Not support fd(%d) in SYS_write", fd);
      }
      strace_ret("write", c);
      break;
    case SYS_brk:
      uintptr_t program_break = c->GPR2;
      c->GPRx = mm_brk(program_break);
      strace_ret("brk", c);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
