#include <memory.h>

static void *pf = NULL;

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
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
