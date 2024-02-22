#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Half Elf64_Half
# define Elf_Off Elf64_Off
# define Elf_Addr Elf64_Addr
# define Elf_PHDR_SZ Elf64_Xword
# define Elf_Shdr Elf64_Shdr
# define Elf_Sym Elf64_Sym
# define Elf_SHDR_SZ Elf64_Xword
#define ELF_ST_TYPE ELF64_ST_TYPE
#define ELF_ST_BIND ELF64_ST_BIND
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Half Elf32_Half
# define Elf_Off Elf32_Off
# define Elf_Addr Elf32_Addr
# define Elf_PHDR_SZ Elf32_Word
# define Elf_Shdr Elf32_Shdr
# define Elf_Sym Elf32_Sym
# define Elf_SHDR_SZ Elf32_Word
#define ELF_ST_TYPE ELF32_ST_TYPE
#define ELF_ST_BIND ELF32_ST_BIND
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#else
# error Unsupported ISA
#endif

static size_t get_argv_len(char *argv[]);
void map(AddrSpace *as, void *va, void *pa, int prot);

static void *get_data_from_ramdisk_ptr(int elf_fd, size_t offset, size_t size) {
  void *buf = malloc(size);
  fs_lseek(elf_fd, offset, SEEK_SET);
  size_t read_len = fs_read(elf_fd, buf, size);
  assert(read_len == size);
  return buf;
}

static void check_elf_mag(Elf_Ehdr *elf_ehdr_ptr) {
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

static void map_program(AddrSpace *as, void *va, void *pa, int prot, size_t nr_page) {
  for(size_t i = 0; i < nr_page; i++) {
    size_t offset = i * PGSIZE;
    map(as, (void *)va + offset, (void *)pa + offset, -1);
  }
}

static Elf_PHDR_SZ get_program_size(AddrSpace *as, Elf_Half e_phnum, Elf_Phdr *elf_phdrs_ptr) {
  Elf_Addr vaddr_max = 0;
  for (size_t i = 0; i < e_phnum; i++) {
    Elf_Phdr *phdr = elf_phdrs_ptr + i;
    uint32_t p_type = phdr->p_type;
    if (p_type == PT_LOAD) {
      Elf_Addr p_vaddr = phdr->p_vaddr;
      Elf_PHDR_SZ p_memsz = phdr->p_memsz;
      Elf_Addr p_vaddr_end = p_vaddr + p_memsz;
      if (vaddr_max < p_vaddr_end) {
        vaddr_max = p_vaddr_end;      
      }
    }
  }
  return vaddr_max - (Elf_Addr)as->area.start;
}

static void load_elf_program(AddrSpace *as, int elf_fd, Elf_Ehdr *elf_ehdr_ptr) {
  /*
  printf("Program header table file offset: %d\n", elf_ehdr_ptr->e_phoff);
  printf("Program header table entry size: %d\n", elf_ehdr_ptr->e_phentsize);
  printf("Program header table entry count: %d\n", elf_ehdr_ptr->e_phnum);
  */
  size_t phdr_size = sizeof(Elf_Phdr);
  Elf_Half e_phentsize = elf_ehdr_ptr->e_phentsize;
  assert(phdr_size == e_phentsize);

  Elf_Half e_phnum = elf_ehdr_ptr->e_phnum;

  Elf_Off e_phoff = elf_ehdr_ptr->e_phoff;
  Elf_Phdr *elf_phdrs_ptr = (Elf_Phdr *)get_data_from_ramdisk_ptr(elf_fd, e_phoff, sizeof(Elf_Phdr) * e_phnum);

  Elf_PHDR_SZ program_size = get_program_size(as, e_phnum, elf_phdrs_ptr);
  size_t nr_page = ROUNDUP(program_size, PGSIZE) / PGSIZE;
  void *p_paddr_start = new_page(nr_page);
  map_program(as, (void *)as->area.start, (void *)p_paddr_start, -1, nr_page);

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
      
      Elf_Addr p_paddr = (Elf_Addr)p_paddr_start + p_vaddr - (Elf_Addr)as->area.start;      
      fs_lseek(elf_fd, p_offset, SEEK_SET);
      fs_read(elf_fd, (void *)p_paddr, p_filesz);
      if(p_filesz < p_memsz) {
        memset((void *)(p_paddr + p_filesz), 0, p_memsz - p_filesz);
      }
    } else {
    }
    // printf("\n");
  }
  free(elf_phdrs_ptr);
}

static Elf_Addr get_elf_maxbrk(int elf_fd, Elf_Ehdr *elf_ehdr_ptr) {
  size_t shdr_size = sizeof (Elf_Shdr);
  assert(shdr_size == (elf_ehdr_ptr->e_shentsize));
  size_t shdrs_size = elf_ehdr_ptr->e_shentsize * elf_ehdr_ptr->e_shnum;  

  assert(elf_ehdr_ptr->e_shoff);
  Elf_Shdr *elf_shdrs_ptr = (Elf_Shdr *)get_data_from_ramdisk_ptr(elf_fd, elf_ehdr_ptr->e_shoff, shdrs_size);

  assert(elf_ehdr_ptr->e_shstrndx != SHN_UNDEF);  
  Elf_Shdr *shdr_shstrtab = elf_shdrs_ptr + elf_ehdr_ptr->e_shstrndx;
  assert(shdr_shstrtab->sh_type == SHT_STRTAB);

  char *shstrtab_ptr = (char *)get_data_from_ramdisk_ptr(elf_fd, shdr_shstrtab->sh_offset, shdr_shstrtab->sh_size);

  char *strtab_ptr = NULL;
  Elf_Sym *symtab_ptr = NULL;
  Elf_SHDR_SZ symtab_size = 0;

  for(size_t i = 0; i < elf_ehdr_ptr->e_shnum; i++){
    Elf_Shdr *shdr = elf_shdrs_ptr + i;
    char *shdr_name = shstrtab_ptr + shdr->sh_name;
    Elf_Off sh_offset = shdr->sh_offset;
    Elf_SHDR_SZ sh_size = shdr->sh_size;
    if(strcmp(shdr_name, ".strtab") == 0){
      strtab_ptr = (char *)get_data_from_ramdisk_ptr(elf_fd, sh_offset, sh_size);
    } else if(strcmp(shdr_name, ".symtab") == 0) {
      symtab_ptr = (Elf_Sym *)get_data_from_ramdisk_ptr(elf_fd, sh_offset, sh_size);
      symtab_size = sh_size;
    } else {
    }
  }

  Elf_Addr result = 0;

  size_t sym_size = sizeof (Elf_Sym);
  size_t len = symtab_size / sym_size;
  for(size_t i = 0; i < len; i++){
    Elf_Sym *sym = symtab_ptr + i;
    unsigned char st_info = sym->st_info;
    char *sym_name = strtab_ptr + sym->st_name;
    if ((strcmp(sym_name, "end") == 0) && (ELF_ST_TYPE(st_info) == STT_NOTYPE) && (ELF_ST_BIND(st_info) == STB_GLOBAL)) {
      result = sym->st_value;
    }
  }

  free(elf_shdrs_ptr);
  free(shstrtab_ptr);
  free(strtab_ptr);
  free(symtab_ptr);
  return result;
}

// 加载文件到虚拟内存里，同时更新pcb->max_brk为end符号的地址
static uintptr_t loader(PCB *pcb, const char *filename) {
  int elf_fd = fs_open(filename, 0, 0);
  Elf_Ehdr *elf_ehdr_ptr = (Elf_Ehdr *)get_data_from_ramdisk_ptr(elf_fd, 0, sizeof(Elf_Ehdr));
  check_elf_mag(elf_ehdr_ptr);
  load_elf_program(&pcb->as, elf_fd, elf_ehdr_ptr);
  Elf_Addr e_entry = elf_ehdr_ptr->e_entry;
  Elf_Addr max_brk = get_elf_maxbrk(elf_fd, elf_ehdr_ptr);
  pcb->max_brk = max_brk;
  free(elf_ehdr_ptr);
  return e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload (PCB *cur_pcb, void(*fun)(void *), void *args) {
  Context *context = kcontext((Area) { cur_pcb->stack, (uint8_t *)(&cur_pcb->stack) + STACK_SIZE }, fun, args);
  cur_pcb->cp = context;
}

void context_uload(PCB *cur_pcb, const char *filename, char *const argv[], char *const envp[]) {
  protect(&cur_pcb->as);
  uintptr_t entry = loader(cur_pcb, filename);

  size_t nr_page = 8;
  uintptr_t *stack_pa_start = (uintptr_t *)new_page(nr_page);
  assert(stack_pa_start);
  uintptr_t *stack_pa_end = (uintptr_t *)((uintptr_t)stack_pa_start + (nr_page << 12));
  
  // 用户栈 给string_area分配1024字节的容量
  char *stack_string_area = (char *)stack_pa_end - (1 << 10);
  char *stack_string_area_curptr = stack_string_area;
  // 用户栈 分配envp，并且更新string_area
  size_t envp_size = get_argv_len((char **)envp);
  char **stack_envp = (char **)stack_string_area - (envp_size + 1);
  char **stack_cur_envp = stack_envp;
  for (size_t i = 0; i < envp_size; i++) {
    strcpy(stack_string_area_curptr, envp[i]);
    *stack_cur_envp = stack_string_area_curptr;
    stack_string_area_curptr = stack_string_area_curptr + strlen(envp[i]) + 1;
    assert(stack_string_area_curptr < (char *)stack_pa_end);
    stack_cur_envp = stack_cur_envp + 1;
  }
  *stack_cur_envp = NULL;
  // 用户栈 分配argv，并且更新string_area
  size_t argv_size = get_argv_len((char **)argv);
  char **stack_argv = (char **)stack_envp - (argv_size + 1);
  char **stack_cur_argv = stack_argv;
  for (size_t i = 0; i < argv_size; i++) {
    strcpy(stack_string_area_curptr, argv[i]);
    *stack_cur_argv = stack_string_area_curptr;
    stack_string_area_curptr = stack_string_area_curptr + strlen(argv[i]) + 1;
    assert(stack_string_area_curptr < (char *)stack_pa_end);
    stack_cur_argv = stack_cur_argv + 1;
  }
  *stack_cur_argv = NULL;
  
  uintptr_t *stack_argc = (uintptr_t *)stack_argv - 1;
  *stack_argc = argv_size;

  void* stack_va_start = cur_pcb->as.area.end - nr_page * PGSIZE;
  map_program(&cur_pcb->as, stack_va_start, (void *)stack_pa_start, -1, nr_page);

  Context *context = ucontext(&cur_pcb->as, (Area) { &cur_pcb->max_brk + 1, (uint8_t *)(&cur_pcb->stack) + STACK_SIZE }, (void *)entry);
  context->GPRx = (uintptr_t)stack_argc - (uintptr_t)stack_pa_start + (uintptr_t)stack_va_start;
  cur_pcb->cp = context;
}

static size_t get_argv_len(char *argv[]) {
  size_t argc = 0;
  if (argv) {
    char **cur_argv = argv;
    while (*cur_argv != NULL) {
      cur_argv = cur_argv + 1;
      argc++;
    }
  }
  return argc;
}