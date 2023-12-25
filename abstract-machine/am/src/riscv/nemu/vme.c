#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

// translation use
#define OFFSET(addr) (addr & 0x0fff)
#define VA_VPN0(va) ((va >> 12) & 0x01ff)
#define VA_VPN1(va) ((va >> 21) & 0x01ff)
#define VA_VPN2(va) ((va >> 30) & 0x01ff) 
#define PA2PTE(addr) ((addr >> 12) << 10)
#define PTE2PA(pte) ((pte >> 10) << 12)


static inline void set_satp(void *pdir) {
  printf("enter set_satp\n");
  uint64_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uint64_t)pdir >> 12)));
  printf("leave set_satp\n");
}

static inline uint64_t get_satp() {
  uint64_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }
  set_satp(kas.ptr);
  vme_enable = 1;
  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *vap, void *pap, int prot) {
  uint64_t* root = (uint64_t*)as->ptr;
  uint64_t va = (uint64_t)vap;
  uint64_t pa = (uint64_t)pap;

  assert(OFFSET(va) == 0);
  assert(OFFSET(pa) == 0);

  uint32_t va_vpn2 = VA_VPN2(va);
  uint32_t va_vpn1 = VA_VPN1(va);
  uint32_t va_vpn0 = VA_VPN0(va);
  uint64_t* pte2 = root + va_vpn2;
  if(*pte2 == 0){
    uint64_t addr = (uint64_t)pgalloc_usr(PGSIZE);
    int flag = PTE_V;
    *pte2 = PA2PTE(addr) | flag; 
  }
  uint64_t* pte1 = (uint64_t*)(PTE2PA(*pte2)) + va_vpn1;
  if(*pte1 == 0){
    uint64_t addr = (uint64_t)pgalloc_usr(PGSIZE);
    int flag = PTE_V;
    *pte1 = PA2PTE(addr) | flag; 
  } 
  uint64_t* pte0 = (uint64_t*)(PTE2PA(*pte1)) + va_vpn0;
  int flag = PTE_V | prot;
  *pte0 = PA2PTE(pa) | flag;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context* cp = (Context*)kstack.end - 1;
  cp->mepc = (uint64_t)entry;
  return cp;
}
