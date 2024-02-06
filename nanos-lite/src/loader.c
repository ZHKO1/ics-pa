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

static size_t get_argv_len(char *argv[]);

static int elf_fd = 0;
static Elf_Ehdr *elf_ehdr_ptr = NULL;
static Elf_Phdr *elf_phdrs_ptr = NULL;

static void *get_data_from_ramdisk_ptr(size_t offset, size_t size) {
  void *buf = malloc(size);
  fs_lseek(elf_fd, offset, SEEK_SET);
  size_t read_len = fs_read(elf_fd, buf, size);
  assert(read_len == size);
  return buf;
}

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
  printf("Program header table file offset: %d\n", elf_ehdr_ptr->e_phoff);
  printf("Program header table entry size: %d\n", elf_ehdr_ptr->e_phentsize);
  printf("Program header table entry count: %d\n", elf_ehdr_ptr->e_phnum);
  */
  size_t phdr_size = sizeof(Elf_Phdr);
  Elf_Half e_phentsize = elf_ehdr_ptr->e_phentsize;
  assert(phdr_size == e_phentsize);

  Elf_Half e_phnum = elf_ehdr_ptr->e_phnum;

  Elf_Off e_phoff = elf_ehdr_ptr->e_phoff;
  elf_phdrs_ptr = (Elf_Phdr *)get_data_from_ramdisk_ptr(e_phoff, sizeof(Elf_Phdr) * e_phnum);

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
      
      fs_lseek(elf_fd, p_offset, SEEK_SET);
      fs_read(elf_fd, (void *)p_vaddr, p_filesz);
      if(p_filesz < p_memsz) {
        memset((void *)(p_vaddr + p_filesz), 0, p_memsz - p_filesz);
      }
    } else {
    }
    // printf("\n");
  }
}

static uintptr_t core_loader (const char *filename) {
  elf_fd = fs_open(filename, 0, 0);
  elf_ehdr_ptr = (Elf_Ehdr *)get_data_from_ramdisk_ptr(0, sizeof(Elf_Ehdr));
  check_elf_mag();
  load_elf_program_header();
  return elf_ehdr_ptr->e_entry;
}

static uintptr_t loader(PCB *pcb, const char *filename) {
  return core_loader(filename);
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
  uintptr_t entry = core_loader(filename);
  Context *context = ucontext(NULL, (Area) { cur_pcb->stack, (uint8_t *)(&cur_pcb->stack) + STACK_SIZE }, (void *)entry);

  size_t nr_page = 8;
  uintptr_t *stack_start = (uintptr_t *)new_page(nr_page);
  assert(stack_start);
  uintptr_t *stack_end = (uintptr_t *)((uintptr_t)stack_start + (nr_page << 12));
  
  // 用户栈 给string_area分配1024字节的容量
  char *stack_string_area = (char *)stack_end - (1 << 10);
  char *stack_string_area_curptr = stack_string_area;
  // 用户栈 分配envp，并且更新string_area
  size_t envp_size = get_argv_len((char **)envp);
  char **stack_envp = (char **)stack_string_area - (envp_size + 1);
  char **stack_cur_envp = stack_envp;
  for (size_t i = 0; i < envp_size; i++) {
    strcpy(stack_string_area_curptr, envp[i]);
    *stack_cur_envp = stack_string_area_curptr;
    stack_string_area_curptr = stack_string_area_curptr + strlen(envp[i]) + 1;
    assert(stack_string_area_curptr < (char *)stack_end);
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
    assert(stack_string_area_curptr < (char *)stack_end);
    stack_cur_argv = stack_cur_argv + 1;
  }
  *stack_cur_argv = NULL;
  
  uintptr_t *stack_argc = (uintptr_t *)stack_argv - 1;
  *stack_argc = argv_size;

  context->GPRx = (uintptr_t)stack_argc;
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