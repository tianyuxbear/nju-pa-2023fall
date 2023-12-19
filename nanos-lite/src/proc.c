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

void context_kload(PCB* kpcb, void (*entry)(void *), void *arg) {
  Area stack;
  stack.start = kpcb->stack;
  stack.end = kpcb->stack + STACK_SIZE;
  kpcb->cp = kcontext(stack, entry, arg);
  printf("kpcb: 0x%016x     kpcb->cp: 0x%016x\n", kpcb, kpcb->cp);
}


void init_proc() {
  context_kload(&pcb[0], hello_fun, (void*)1);
  context_kload(&pcb[1], hello_fun, (void*)2);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  //naive_uload(NULL, NULL);
}

Context* schedule(Context *prev) {
  current->cp = prev;
  printf("current: 0x%016x    current->cp: 0x%016x\n", current, current->cp);
  printf("&pcb[0]: 0x%016x    pcb[0].cp: 0x%016x\n", &pcb[0], pcb[0].cp);
  printf("&pcb[1]: 0x%016x    pcb[1].cp: 0x%016x\n", &pcb[1], pcb[1].cp);
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  printf("current: 0x%016x     current->cp: 0x%016x\n", current, current->cp);
  printf("Schedule Context: 0x%016x ==> 0x%016x\n", (uint64_t)prev, (uint64_t)current->cp);
  return current->cp;
}
