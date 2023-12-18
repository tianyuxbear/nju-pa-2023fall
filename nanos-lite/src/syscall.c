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
  a[2] = c->GPR3;  //a1
  a[3] = c->GPR4;  //a2


  switch (a[0]) {
    case SYS_exit:
      printf("=== syscall: %s --> args: %p %p %p ===  \n", syscall_name[SYS_exit], a[1], a[2], a[3]);
      c->GPR2 = 0;
      halt(c->GPR2);
      break;
    case SYS_yield:
      printf("=== syscall: %s --> args: %p %p %p ===  \n", syscall_name[SYS_yield], a[1], a[2], a[3]);
      yield();
      break;
    case SYS_open:
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_open], a[1], a[2], a[3], a[1]);
      break;
    case SYS_read:
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_read], a[1], a[2], a[3], a[1]);
      break;
    case SYS_write:
      int fd = (int)a[1];
      char* buf = (char*) a[2];
      size_t count = (uint32_t) a[3];
      if(fd == 1 || fd == 2){
        for(int i = 0; i < count; i++) putch(buf[i]);
        c->GPR2 = count;
      } 
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_write], a[1], a[2], a[3], c->GPR2);
      break;
    case SYS_brk:
      c->GPR2 = 0;
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_brk], a[1], a[2], a[3], c->GPR2);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
