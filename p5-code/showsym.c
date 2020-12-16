// Template for parsing an ELF file to print its symbol table
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

// The below macros help to prevent errors when doing pointer
// arithmetic. Adding an offset to a pointer is the most common
// operation here while the other macros may not be needed.

#define PTR_PLUS_BYTES(ptr,off) ((void *) (((size_t) (ptr)) + ((size_t) (off))))
// macro to add a byte offset to a pointer, arguments are a pointer
// and a # of bytes (usually size_t)

#define PTR_MINUS_BYTES(ptr,off) ((void *) (((size_t) (ptr)) - ((size_t) (off))))
// macro to subtract a byte offset from a pointer, arguments are a pointer
// and a # of bytes (usually size_t)

#define PTR_MINUS_PTR(ptr,ptq) ((long) (((size_t) (ptr)) - ((size_t) (ptq))))
// macro to subtract one pointer from another

int DEBUG = 0;                  
// Controls whether to print debug messages
// Can be used in conditionals like if(DEBUG){ ... }
// and running 'showsym -d x.o' will set DEBUG=1

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("usage: %s [-d] <file>\n",argv[0]);
    return 0;
  }

  char *objfile_name = argv[1];

  // check for debug mode
  if(argc >=3){
    if(strcmp("-d",argv[1])==0){
      DEBUG = 1;
      objfile_name = argv[2];
    }
    else{
      printf("incorrect usage\n");
      return 1;
    }
  }
                        
  // Open file descriptor and set up memory map for objfile_name
  int fd = open(objfile_name,O_RDONLY);
  struct stat stat_buf;
  fstat(fd, &stat_buf);
  int size = stat_buf.st_size;
  char *file_chars = mmap (NULL, size,PROT_READ, MAP_SHARED,fd,0);

  // CREATE A POINTER to the intial bytes of the file which are an ELF64_Ehdr struct
  Elf64_Ehdr *ehdr = (Elf64_Ehdr*) file_chars;

  // CHECK e_ident field's bytes 0 to for for the sequence {0x7f,'E','L','F'}.
  // Exit the program with code 1 if the bytes do not match
  int ident_matches = 
    ehdr->e_ident[0] == 0x7f &&
    ehdr->e_ident[1] == 'E' &&
    ehdr->e_ident[2] == 'L' &&
    ehdr->e_ident[3] == 'F';

  if(!ident_matches){
    printf("ERROR: Magic bytes wrong, this is not an ELF file");
    exit(1);
  }

  
  // PROVIDED: check for a 64-bit file
  if(ehdr->e_ident[EI_CLASS] != ELFCLASS64){
    printf("Not a 64-bit file ELF file\n");
    return 1;
  }

  // PROVIDED: check for x86-64 architecture
  if(ehdr->e_machine != EM_X86_64){
    printf("Not an x86-64 file\n");
    return 1;
  }

  // DETERMINE THE OFFSET of the Section Header Array (e_shoff), the
  // number of sections (e_shnum), and the index of the Section Header
  // String table (e_shstrndx). These fields are from the ELF File
  // Header.

  long offset = ehdr->e_shoff;//OFFSET of the Section Header Array
  int sec = ehdr->e_shnum;  //number of sections
  int idx = ehdr->e_shstrndx; //  index of the Section Header String table

  // Set up a pointer to the array of section headers. Use the section
  // header string table index to find its byte position in the file
  // and set up a pointer to it.
  Elf64_Shdr *sec_hdrs = (Elf64_Shdr *) (file_chars + offset);
  uint64_t sec_strs = (sec_hdrs[idx].sh_offset); // byte position
  char *sec_names = (char *) (file_chars + sec_strs);
  

  // Search the Section Header Array for the secion with name .symtab
  // (symbol table) and .strtab (string table).  Note their positions
  // in the file (sh_offset field).  Also note the size in bytes
  // (sh_size) and and the size of each entry (sh_entsize) for .symtab
  // so its number of entries can be computed.
  long sym_offset = 0;
  long str_offset = 0;
  long sym_size = 0;
  // long str_size = -1;
  long sym_entries = 0;
  // long str_entries = -1;

  for(int i=0; i<sec; i++){
    char* sec_pos = sec_names+(sec_hdrs[i].sh_name);
    if(strcmp(sec_pos,".symtab") ==0 ){
      sym_offset = sec_hdrs[i].sh_offset;
      sym_size = sec_hdrs[i].sh_size;
      sym_entries = sec_hdrs[i].sh_entsize;
    }else if(strcmp(sec_pos,".strtab") ==0 ){
      str_offset = sec_hdrs[i].sh_offset;
    }
  }


  if(sym_offset <= 0){
    printf("ERROR: Couldn't find symbol table\n");
    return 1;
  }

  if(str_offset <= 0){
    printf("ERROR: Couldn't find string table\n");
    return 1;
  }
  long symtab_num = sym_size / sym_entries;
  // PRINT byte information about where the symbol table was found and
  // its sizes. The number of entries in the symbol table can be
  // determined by dividing its total size in bytes by the size of
  // each entry.
  printf("Symbol Table\n");
  printf("- %ld bytes offset from start of file\n",sym_offset);
  printf("- %ld bytes total size\n",sym_size);
  printf("- %ld bytes per entry\n",sym_entries);
  printf("- %ld entries\n",symtab_num);


  // Set up pointers to the Symbol Table and associated String Table
  // using offsets found earlier.
  Elf64_Sym *sym_tab = (Elf64_Sym *)(file_chars + sym_offset);
  char *str_tab = (char *) (file_chars + str_offset);


  // Print column IDs for info on each symbol
  printf("[%3s]  %8s %4s %s\n",
         "idx","TYPE","SIZE","NAME");

  // Iterate over the symbol table entries
  for(int i=0; i<symtab_num; i++){
    // Determine size of symbol and name. Use <NONE> name has zero
    // length.
    int name_size = sym_tab[i].st_name;
    long size = sym_tab[i].st_size;
    char *name = (char *) (str_tab + sym_tab[i].st_name);

    if(name_size == 0){
       name = "<NONE>";
    }
    // Determine type of symbol. See assignment specification for
    // fields, macros, and definitions related to this.
    unsigned char typec = ELF64_ST_TYPE(sym_tab[i].st_info);
    char * type = "";
    if(typec == STT_NOTYPE){
      type = "NOTYPE";  
    }else if( typec == STT_OBJECT){
      type = "OBJECT";
    }else if(typec == STT_FUNC){
      type = "FUNC";
    }else if(typec == STT_FILE){
      type = "FILE";
    }else if(typec == STT_SECTION){
      type = "SECTION";
    }
    // Print symbol information
    printf("[%3d]: %8s %4lu %s\n",i,type,size,name);
  }


  // Unmap file from memory and close associated file descriptor 
  munmap(file_chars,size);
  close(fd);
  return 0;
}
