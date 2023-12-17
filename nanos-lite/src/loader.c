#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf64_Ehdr elf_header;
  ramdisk_read(&elf_header, 0, sizeof(elf_header));

  // read program headers
  Elf64_Off e_phoff = elf_header.e_phoff;
  uint16_t e_phentsize = elf_header.e_phentsize;
  uint16_t e_phnum = elf_header.e_phnum;

  Elf64_Phdr elf_phdrs[e_phentsize];
  ramdisk_read(elf_phdrs, e_phoff, sizeof(elf_phdrs));
  for(int i = 0; i < e_phnum; i++){
    if(elf_phdrs[i].p_type == PT_LOAD){
      Elf64_Off p_offset = elf_phdrs[i].p_offset;
      Elf64_Addr p_vaddr = elf_phdrs[i].p_vaddr;
      uint64_t p_filesz = elf_phdrs[i].p_filesz;
      uint64_t p_memsz = elf_phdrs[i].p_memsz;
      ramdisk_read((void*)p_vaddr, p_offset, p_filesz);
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

