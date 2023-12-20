#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char*)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* kpcb, void (*entry)(void *), void *arg) {
  Area stack;
  stack.start = kpcb->stack;
  stack.end = kpcb->stack + STACK_SIZE;
  kpcb->cp = kcontext(stack, entry, arg);
}


void init_proc() {
  context_kload(&pcb[0], hello_fun, (void*)"A");
  context_kload(&pcb[1], hello_fun, (void*)"B");
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  naive_uload(NULL, "/bin/dummy");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}
