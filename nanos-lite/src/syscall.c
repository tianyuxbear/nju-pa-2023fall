#include <common.h>
#include "syscall.h"

static char* syscall_name[] = {
  "SYS_exit",
  "SYS_yield",
  "SYS_open",
  "SYS_read",
  "SYS_write",
  "SYS_kill",
  "SYS_getpid",
  "SYS_close",
  "SYS_lseek",
  "SYS_brk",
  "SYS_fstat",
  "SYS_time",
  "SYS_signal",
  "SYS_execve",
  "SYS_fork",
  "SYS_link",
  "SYS_unlink",
  "SYS_wait",
  "SYS_times",
  "SYS_gettimeofday"
};


void do_syscall(Context *c) {
  uint64_t a[4];
  a[0] = c->GPR1;  //a7
  a[1] = c->GPR2;  //a0
  a[2] = c->GPR2;  //a1
  a[3] = c->GPR2;  //a2


  switch (a[0]) {
    case SYS_exit:
      printf("=== syscall: %s ===\n", syscall_name[SYS_exit]);
      halt(a[1]);
      break;
    case SYS_yield:
      printf("=== syscall: %s ===\n", syscall_name[SYS_yield]);
      yield();
      break;
    case SYS_open:
      printf("=== syscall: %s ===\n", syscall_name[SYS_open]);
      break;
    case SYS_read:
      printf("=== syscall: %s ===\n", syscall_name[SYS_read]);
      break;
    case SYS_write:
      printf("=== syscall: %s ===\n", syscall_name[SYS_write]);
      printf("fd: 0x%016x\n buf: 0x%016x\n count: 0x%016x\n", a[1], a[2], a[3]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
