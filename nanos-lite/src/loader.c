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

extern uint8_t ramdisk_start;

static uintptr_t loader(PCB *pcb, const char *filename) {
  if(filename == NULL) return (uintptr_t)&ramdisk_start;

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
      uint64_t p_memsz = elf_phdrs[i].p_memsz;
      fs_lseek(fd, p_offset, SEEK_SET);
      fs_read(fd, (void*)p_vaddr, p_filesz);
      memset((void*)(p_vaddr + p_filesz), 0, p_memsz - p_filesz);
    }
  }

  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

