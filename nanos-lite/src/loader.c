#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Half Elf64_Half
# define Elf_Off Elf64_Off
# define Elf_Addr Elf64_Addr
# define Elf_PHDR_SZ Elf64_Xword
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Half Elf32_Half
# define Elf_Off Elf32_Off
# define Elf_Addr Elf32_Addr
# define Elf_PHDR_SZ Elf32_Word
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#else
# error Unsupported ISA
#endif

void *get_ramdisk_ptr_by_offset(size_t offset);
size_t ramdisk_read(void *buf, size_t offset, size_t len);

static Elf_Ehdr *elf_ehdr_ptr = NULL;
static Elf_Phdr *elf_phdrs_ptr = NULL;


static void check_elf_mag() {
  char *e_ident = (char *)elf_ehdr_ptr;
  assert(*e_ident == 0x7f);
  assert(*(e_ident + 1) == 'E');
  assert(*(e_ident + 2) == 'L');
  assert(*(e_ident + 3) == 'F');

  char el_class = *(e_ident + 4);
#ifdef __LP64__
  assert(el_class == ELFCLASS64);
#else
  assert(el_class == ELFCLASS32);
#endif
  
  Elf_Half e_type = elf_ehdr_ptr->e_type;
  assert(e_type == ET_EXEC);

  Elf_Half e_machine = elf_ehdr_ptr->e_machine;
  assert(e_machine == EXPECT_TYPE);
}

static void load_elf_program_header() {
  /*
  printf("Program header table file offset: %x\n", elf_ehdr_ptr->e_phoff);
  printf("Program header table entry size: %d\n", elf_ehdr_ptr->e_phentsize);
  printf("Program header table entry count: %d\n", elf_ehdr_ptr->e_phnum);
  printf("Program header string table index: %d\n", elf_ehdr_ptr->e_shstrndx);
  */
  size_t phdr_size = sizeof(Elf_Phdr);
  Elf_Half e_phentsize = elf_ehdr_ptr->e_phentsize;
  assert(phdr_size == e_phentsize);

  Elf_Half e_phnum = elf_ehdr_ptr->e_phnum;

  Elf_Off e_phoff = elf_ehdr_ptr->e_phoff;
  elf_phdrs_ptr = (Elf_Phdr *) get_ramdisk_ptr_by_offset(e_phoff);

  for (size_t i = 0; i < e_phnum; i++) {
    Elf_Phdr *phdr = elf_phdrs_ptr + i;
    uint32_t p_type = phdr->p_type;
    // printf("p_type = 0x%08x ", p_type);
    if (p_type == PT_LOAD) {
      Elf_Off p_offset = phdr->p_offset;
      Elf_Addr p_vaddr = phdr->p_vaddr;
      Elf_PHDR_SZ p_filesz = phdr->p_filesz;
      Elf_PHDR_SZ p_memsz = phdr->p_memsz;
      assert(p_memsz >= p_filesz);
      // printf("p_offset = 0x%08x ", p_offset);
      // printf("p_vaddr = 0x%08x ", p_vaddr);
      // printf("p_filesz = 0x%08x ", p_filesz);
      // printf("p_memsz = 0x%08x ", p_memsz);
      
      ramdisk_read((void *)p_vaddr, p_offset, p_filesz);
      if(p_filesz < p_memsz) {
        memset((void *)(p_vaddr + p_filesz), 0, p_memsz - p_filesz);
      }
    } else {
    }
    // printf("\n");
  }
}

static uintptr_t loader(PCB *pcb, const char *filename) {
  elf_ehdr_ptr = (Elf_Ehdr *)get_ramdisk_ptr_by_offset(0);
  check_elf_mag();
  load_elf_program_header();
  return elf_ehdr_ptr->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

