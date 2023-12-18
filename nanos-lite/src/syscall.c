#include <common.h>
#include "syscall.h"
#include <fs.h>

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

  // for read and write
  int fd = (int)a[1];
  void* buf = (void*) a[2];
  size_t len = (size_t) a[3];

  switch (a[0]) {
    case SYS_exit:
      printf("=== syscall: %s --> args: %p %p %p ===  \n", syscall_name[SYS_exit], a[1], a[2], a[3]);
      c->GPRx = 0;
      halt(c->GPRx);
      break;
    case SYS_yield:
      printf("=== syscall: %s --> args: %p %p %p ===  \n", syscall_name[SYS_yield], a[1], a[2], a[3]);
      yield();
      break;
    case SYS_open:
      char* pathname = (char*)a[1];
      int flags = (int)a[2];
      int mode = (int)a[3];
      c->GPRx = fs_open(pathname, flags, mode);
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_open], a[1], a[2], a[3], c->GPRx);
      break;
    case SYS_read:
      assert(fd >= 3);
      c->GPRx = fs_read(fd, buf, len);
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_read], a[1], a[2], a[3], c->GPRx);
      break;
    case SYS_write:
      if(fd == 1 || fd == 2){
        for(int i = 0; i < len; i++) putch(((char*)buf)[i]);
        c->GPRx = len;
      }else{
        assert(fd != 0);
        c->GPRx = fs_write(fd, buf, len);
      } 
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_write], a[1], a[2], a[3], c->GPRx);
      break;
    case SYS_close:
      c->GPRx = fs_close(fd);
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_close], a[1], a[2], a[3], c->GPRx);
      break;
    case SYS_lseek:
      size_t offset = (size_t)a[2];
      int whence = (int)a[3];
      c->GPRx = fs_lseek(fd, offset, whence);
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_lseek], a[1], a[2], a[3], c->GPRx);
      break;
    case SYS_brk:
      c->GPRx = 0;
      printf("=== syscall: %s --> args: %p %p %p ret: %p ===  \n", syscall_name[SYS_brk], a[1], a[2], a[3], c->GPRx);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
