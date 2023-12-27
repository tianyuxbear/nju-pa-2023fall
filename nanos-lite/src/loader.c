#include <proc.h>
#include <fs.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08

extern uint8_t ramdisk_start;
extern void* new_page(size_t nr_page);
extern void map(AddrSpace *as, void *vap, void *pap, int prot);

uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);

  Elf64_Ehdr elf_header;
  fs_read(fd, &elf_header, sizeof(elf_header));
  assert(*(uint64_t *)elf_header.e_ident == 0x00010102464c457f);

  // read program headers
  Elf64_Off e_phoff = elf_header.e_phoff;
  uint16_t e_phentsize = elf_header.e_phentsize;
  uint16_t e_phnum = elf_header.e_phnum;

  fs_lseek(fd, e_phoff, SEEK_SET);
  Elf64_Phdr elf_phdrs[e_phentsize];
  fs_read(fd, elf_phdrs, sizeof(elf_phdrs));


  for(int i = 0; i < e_phnum; i++){
    if(elf_phdrs[i].p_type == PT_LOAD){
      Elf64_Off p_offset = elf_phdrs[i].p_offset;
      Elf64_Addr p_vaddr = elf_phdrs[i].p_vaddr;
      uint64_t p_filesz = elf_phdrs[i].p_filesz;
      //uint64_t p_memsz = elf_phdrs[i].p_memsz;

      fs_lseek(fd, p_offset, SEEK_SET);
      uint64_t va = p_vaddr;
      printf("====== once load ======\n");
      for(int i = 0; i < p_filesz; i += PGSIZE){
        printf("Enter loop: 0x%x\n", va);
        uint64_t pa = (uint64_t)new_page(1);
        fs_read(fd, (void*)pa, PGSIZE);
        if(i + PGSIZE > p_filesz){
          uint64_t lastbytes = p_filesz - i;
          memset((void*)(pa + lastbytes), 0, PGSIZE - lastbytes);
        }
        int prot = PTE_R | PTE_W | PTE_X;
        map(&pcb->as, (void*)va, (void*)pa, prot);
        printf("Loader map ==> va: 0x%x   pa: 0x%x\n", va, pa);
        va += PGSIZE;
        printf("Leave loop: 0x%x\n", va);
      }
    }
  }

  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

