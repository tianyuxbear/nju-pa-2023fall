#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

extern uintptr_t loader(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char*)arg, j);
    j++;
    for(int volatile i = 0; i < 100000; i++) ;
    yield();
  }
}

void context_kload(PCB* kpcb, void (*entry)(void *), void *arg) {
  Area stack;
  stack.start = kpcb->stack;
  stack.end = kpcb->stack + STACK_SIZE;
  kpcb->cp = kcontext(stack, entry, arg);
  printf("kpcb: 0x%016x    kpcb->cp: 0x%016x\n", kpcb, kpcb->cp);
}

static int get_strnum(char* const strv[]){
  int i = 0;
  while(strv[i] != NULL) i++;
  return i;
}

void context_uload(PCB* upcb, const char* filename, char* const argv[], char* const envp[]){
  Area stack;
  stack.start = upcb->stack;
  stack.end = upcb->stack + STACK_SIZE;

  uintptr_t entry = loader(upcb, filename);
  upcb->cp = ucontext(&upcb->as, stack, (void*)entry);

  uint64_t down = (uint64_t)new_page(8);
  uint64_t top = down + 8 * PGSIZE;

  int argc = get_strnum(argv);
  int envc = get_strnum(envp);
  uint32_t offset = (1 + argc + 1 + envc + 1) * 8 + (argc + envc) * 32;
  uint64_t addr = top - offset;
  uint64_t str_area = addr + (1 + argc + 1 + envc + 1) * 8;

  *((int*)addr) = argc;
  addr += 8;

  for(int i = 0; i < argc; i++){
    memcpy((void*)(str_area + i * 32), (void*)argv[i], 32);
    *((uint64_t*)(addr + i * 8)) = str_area + i * 32;
  }
  addr += argc * 8;
  str_area += argc * 32;
  *((uint64_t*)addr) = 0;
  addr += 8;

  for(int i = 0; i < envc; i++){
    memcpy((void*)(str_area + i * 32), (void*)envp[i], 32);
    *((uint64_t*)(addr + i * 8)) = str_area + i * 32;
  }
  addr += envc * 8;
  str_area += envc * 32;
  *((uint64_t*)addr) = 0;
  addr += 8;

  upcb->cp->GPRx = top - offset;


  printf("=== heap.start: 0x%016x    heap.end: 0x%016x    ===\n", (uint64_t)heap.start, (uint64_t)heap.end);
  printf("upcb: 0x%016x    upcb->cp: 0x%016x\n", upcb, upcb->cp);
}


void init_proc() {
  context_kload(&pcb[0], hello_fun, (void*)"A");
  //context_kload(&pcb[1], hello_fun, (void*)"B");
  //context_uload(&pcb[0], "/bin/hello");
  //char* argv[] = {"arg1", "arg2", "arg3", NULL};
  //char* envp[] = {NULL};
  context_uload(&pcb[1], "/bin/exec-test", NULL, NULL);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  //naive_uload(NULL, NULL);
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  printf("Schedule: 0x%016x ===> 0x%016x\n", (uint64_t)prev, (uint64_t)current->cp);
  return current->cp;
}
