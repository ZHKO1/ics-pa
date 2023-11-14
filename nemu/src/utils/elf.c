#include <common.h>
#include <elf.h>

char *load_elf_data(long offset, size_t sizes);
void load_elf_section_header_table();
void load_elf_symtab();

typedef MUXDEF(CONFIG_ISA64, Elf64_Ehdr, Elf32_Ehdr) Elf_Ehdr;
typedef MUXDEF(CONFIG_ISA64, Elf64_Half, Elf32_Half) Elf_Half;
typedef MUXDEF(CONFIG_ISA64, Elf64_Off,  Elf32_Off ) Elf_Off;
typedef MUXDEF(CONFIG_ISA64, Elf64_Word, Elf32_Word) Elf_Word;
typedef MUXDEF(CONFIG_ISA64, Elf64_Addr, Elf32_Addr) Elf_Addr;

typedef MUXDEF(CONFIG_ISA64, Elf64_Shdr, Elf32_Shdr) Elf_Shdr;
typedef MUXDEF(CONFIG_ISA64, Elf64_Sym,  Elf32_Sym)  Elf_Sym;
#define ELF_ST_TYPE MUXDEF(CONFIG_ISA64, ELF64_ST_TYPE,  ELF32_ST_TYPE)


FILE *elf_fp = NULL;
static Elf_Ehdr *elf_ehdr_ptr = NULL;
static Elf_Shdr *elf_shdrs_ptr = NULL;
static char *shstrtab_ptr = NULL;
static char *strtab_ptr = NULL;
static Elf_Sym *symtab_ptr = NULL;
static Elf_Word symtab_size = 0;

char *load_elf_data(long offset, size_t sizes) {
  char *ptr = malloc(sizes);
  fseek(elf_fp, offset, SEEK_SET);
  int ret = fread(ptr, sizes, 1, elf_fp);
  assert(ret == 1);
  return ptr;
}

void load_elf(char *elf_file) {
  elf_fp = stdout;
  if (elf_file != NULL) {
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp = fp;
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    Log("The Elf is %s, size = %ld", elf_file, size);

    long ehdr_size = sizeof (Elf_Ehdr);
    elf_ehdr_ptr = (Elf_Ehdr *) load_elf_data(0, ehdr_size);

    load_elf_section_header_table();
    // load_elf_symtab();
    
    fclose(fp);
  } else {
    Log("No Elf is given.");
  }
}

void load_elf_section_header_table() {
  // Log("Section header table file offset: "FMT_PADDR"", elf_ehdr_ptr->e_shoff);
  // Log("Section header table entry size: %d", elf_ehdr_ptr->e_shentsize);
  // Log("Section header table entry count: %d", elf_ehdr_ptr->e_shnum);
  // Log("Section header string table index: %d", elf_ehdr_ptr->e_shstrndx);

  long shdr_size = sizeof (Elf_Shdr);
  assert(shdr_size == (elf_ehdr_ptr->e_shentsize));
  long shdrs_size = elf_ehdr_ptr->e_shentsize * elf_ehdr_ptr->e_shnum;  

  assert(elf_ehdr_ptr->e_shoff);
  elf_shdrs_ptr = (Elf_Shdr *) load_elf_data(elf_ehdr_ptr->e_shoff, shdrs_size);

  assert(elf_ehdr_ptr->e_shstrndx != SHN_UNDEF);  
  Elf_Shdr *shdr_shstrtab = elf_shdrs_ptr + elf_ehdr_ptr->e_shstrndx;
  assert(shdr_shstrtab->sh_type == SHT_STRTAB);
  shstrtab_ptr = load_elf_data(shdr_shstrtab->sh_offset, shdr_shstrtab->sh_size);
  
  for(size_t i = 0; i < elf_ehdr_ptr->e_shnum; i++){
    Elf_Shdr *shdr = elf_shdrs_ptr + i;
    char *shdr_name = shstrtab_ptr + shdr->sh_name;
    Elf_Off sh_offset = shdr->sh_offset;
    Elf_Word sh_size = shdr->sh_size;
    // Log("Section header name=[%s] offset=["FMT_PADDR"] size=[%d]", shdr_name, sh_offset, sh_size);
    if(strcmp(shdr_name, ".strtab") == 0){
      strtab_ptr = load_elf_data(sh_offset, sh_size);
    } else if(strcmp(shdr_name, ".symtab") == 0) {
      symtab_ptr = (Elf_Sym *) load_elf_data(sh_offset, sh_size);
      symtab_size = sh_size;
    } else {
    }
  }
}

void load_elf_symtab() {
  Elf_Word sym_size = sizeof (Elf_Sym);
  size_t len = symtab_size / sym_size;
  for(size_t i = 0; i < len; i++){
    Elf_Sym *sym = symtab_ptr + i;
    char *sym_name = strtab_ptr + sym->st_name;
    unsigned char st_info = sym->st_info;
    Elf_Addr st_value = sym->st_value;
    Elf_Word st_size = sym->st_size;
    Log("sym_name=[%s] value=["FMT_PADDR"] type=[%d] size=[%d]", sym_name, st_value, ELF_ST_TYPE(st_info), st_size);
  }
}

char *load_func_from_elf(paddr_t address, bool is_exact){
  if( elf_fp == NULL || elf_shdrs_ptr == NULL || shstrtab_ptr == NULL || strtab_ptr == NULL || symtab_ptr == NULL ) {
    return NULL;
  }
  Elf_Word sym_size = sizeof (Elf_Sym);
  size_t len = symtab_size / sym_size;
  for(size_t i = 0; i < len; i++){
    Elf_Sym *sym = symtab_ptr + i;
    unsigned char st_info = sym->st_info;
    if (ELF_ST_TYPE(st_info) == STT_FUNC) {
      char *sym_name = strtab_ptr + sym->st_name;
      Elf_Addr st_value = sym->st_value;    
      Elf_Word st_size = sym->st_size;
      if (is_exact) {
        if (st_value == address) {
          return sym_name;
        }
      } else {
        if ((address >= st_value) && (address < st_value + st_size)) {
          return sym_name;
        }
      }
    }
  }
  return NULL;
}
