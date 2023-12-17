#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit:
      printf("=== SYS_call: exit ===\n");
      a[1] = c->GPR2;
      halt(a[1]);
      break;
    case SYS_yield:
      printf("=== SYS_call: yield ===\n");
      yield();
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
