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
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

Context *context_kload(PCB* kpcb, void (*entry)(void *), void *arg) {
  Context *cp = kcontext((Area) { kpcb->stack, kpcb + 1 }, entry, arg);
  kpcb->cp = cp;
  return cp;
}


void init_proc() {
  context_kload(&pcb[0], hello_fun, (void*)1);
  context_kload(&pcb[1], hello_fun, (void*)2);

  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  naive_uload(NULL, NULL);
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  printf("Schedule Context: 0x%016x ==> 0x%016x\n", (uint64_t)prev, (uint64_t)current->cp);
  return current->cp;
}
