#include <memory.h>
#include <proc.h>

static void *pf = NULL;
extern void map(AddrSpace *as, void *vap, void *pap, int prot);
#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08

void* new_page(size_t nr_page) {
  void* begin = pf;
  pf += nr_page * PGSIZE;
  return begin;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  size_t nr_page = n / PGSIZE;
  if(n % PGSIZE != 0 ) nr_page++;
  void* start = new_page(nr_page);
  memset((char*)start, 0, nr_page * PGSIZE);
  return start;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  if(current->max_brk >= brk ) return 0;
  uint32_t size = brk - current->max_brk;
  uint64_t va = current->max_brk;
  for(int i = 0; i < size; i += PGSIZE){
    uint64_t pa = (uint64_t)new_page(1);
    int prot = PTE_R | PTE_W | PTE_X;
    map(&current->as, (void*)va, (void*)pa, prot);
    va += PGSIZE;
  }
  current->max_brk = va;
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
