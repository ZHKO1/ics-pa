#include <am.h>
#include <common.h>
#include <fs.h>
#include "syscall.h"

#define STRACE(ID, ret) {uintptr_t result = (ret);\
  strace(SYS_##ID, #ID, c, result); \
  c->GPRx = result;}

int mm_brk(uintptr_t brk);

static inline void strace(int id, char *system_call_name, Context *c, uintptr_t result) {
#ifdef CONFIG_ETRACE
  // TODO 需要考虑64位的情况，可能要新增支持%lx这样的形式
  switch (id) {
    case SYS_open:
      Log("%s (%s, %2d, %2d) = %2d", system_call_name, (char *)(uintptr_t)c->GPR2, c->GPR3, c->GPR4, result );
      break;
    case SYS_read:
    case SYS_write:
    case SYS_lseek:
    case SYS_close:
      Log("%s (%s, %2d, %2d) = %2d", system_call_name, fs_get_name_by_fd(c->GPR2), c->GPR3, c->GPR4, result );
      break;
    case SYS_exit:
      Log("%s (%d, %2d, %2d) = ???", system_call_name, c->GPR2, c->GPR3, c->GPR4 );
      break;
    default:
      Log("%s (%2d, %2d, %2d) = %2d", system_call_name, c->GPR2, c->GPR3, c->GPR4, result );
      break;
  }
#endif
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  switch (a[0]) {
    case SYS_exit:
      int exit_status = c->GPR2;
      strace(SYS_exit, "exit", c, 0);
      halt(exit_status);
      break;
    case SYS_yield:
      yield();
      STRACE(yield, 0);
      break;
    case SYS_open:
      char *path = (char *)c->GPR2;
      int flags = c->GPR3;
      int mode = c->GPR4;
      STRACE(open, fs_open(path, flags, mode));
      break;
    case SYS_read:
      {
        int fd = c->GPR2;
        char *buf = (char *)c->GPR3;
        size_t len = c->GPR4;
        STRACE(read, fs_read(fd, buf, len));
      }
      break;
    case SYS_write:
      {
        int fd = c->GPR2;
        char *buf = (char *)c->GPR3;
        size_t len = c->GPR4;
        STRACE(write, fs_write(fd, buf, len));
      }
      break;
    case SYS_lseek:
      {
        int fd = c->GPR2;
        size_t offset = c->GPR3;
        int whence = c->GPR4;
        STRACE(lseek, fs_lseek(fd, offset, whence));
      }
      break;
    case SYS_close:
      {
        int fd = c->GPR2;
        STRACE(close, fs_close(fd));
      }
      break;
    case SYS_brk:
      uintptr_t program_break = c->GPR2;
      STRACE(brk, mm_brk(program_break));
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
