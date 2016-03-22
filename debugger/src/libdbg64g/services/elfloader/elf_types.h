/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      elf-file loader class declaration.
 */

#ifndef __DEBUGGER_ELF_TYPES_H__
#define __DEBUGGER_ELF_TYPES_H__

#include <inttypes.h>

namespace debugger {

//#define EI_NIDENT 16
#define ARCH_64BITS

#ifdef ARCH_64BITS
typedef uint64_t   Elf32_Addr;
typedef uint64_t   Elf32_Off;
#else
typedef unsigned int   Elf32_Addr;
typedef unsigned int   Elf32_Off;
#endif
typedef unsigned short Elf32_Half;
typedef signed   int   Elf32_Sword;
typedef unsigned int   Elf32_Word;

static const char MAGIC_BYTES[]  = {0x7f,'E','L','F',0};

enum E_EI {
    EI_MAG0,
    EI_MAG1,
    EI_MAG2,
    EI_MAG3,
    EI_CLASS,
    EI_DATA,
    EI_VERSION,
    EI_PAD,
    EI_NIDENT=16
};
static const uint8_t ELFCLASSNONE = 0;
static const uint8_t ELFCLASS32   = 1;
static const uint8_t ELFCLASS64   = 2;

static const uint8_t ELFDATANONE  = 0;
static const uint8_t ELFDATA2LSB  = 1;
static const uint8_t ELFDATA2MSB  = 2;

static const uint8_t EV_NONE      = 0;          // Invalid version
static const uint8_t EV_CURRENT   = 1;          // Current version
//etype values:
static const Elf32_Half ET_NONE      = 0;       // no file type
static const Elf32_Half ET_REL       = 1;       // rellocatable file
static const Elf32_Half ET_EXEC      = 2;       // executable file
static const Elf32_Half ET_DYN       = 3;       // shared object file
static const Elf32_Half ET_CORE      = 4;       // core file
static const Elf32_Half ET_LOPROC    = 0xff00;  // Processor-specific
static const Elf32_Half ET_HIPROC    = 0xffff;  // Processor-specific
//emachine values:
static const Elf32_Half EM_NONE      = 0;       // No machine
static const Elf32_Half EM_M32       = 1;       // AT&T WE 32100
static const Elf32_Half EM_SPARC     = 2;       // SPARC
static const Elf32_Half EM_386       = 3;       // Intel 386
static const Elf32_Half EM_68K       = 4;       // Motorola 68000
static const Elf32_Half EM_88K       = 5;       // Motorola 88000
static const Elf32_Half EM_860       = 7;       // Intel 80860
static const Elf32_Half EM_MIPS      = 8;       // MIPS RS3000

typedef struct ElfHeaderType
{
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;      // Shared/Executable/Rellocalable etc
    Elf32_Half e_machine;   // SPARC, X86 etc
    Elf32_Word e_version;   //
    uint64_t e_entry;     // entry point
    uint64_t  e_phoff;     // Program header offset
    uint64_t  e_shoff;     // Section Header offset
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize; // size of one entry in the Program header. All entries are the same size
    Elf32_Half e_phnum;     // number of entries in a Program header
    Elf32_Half e_shentsize; // entry size in the section header table. all entries are the same size
    Elf32_Half e_shnum;     // number of section header entries
    Elf32_Half e_shstrndx;
} ElfHeaderType;
    
//sh_type:
static const Elf32_Word SHT_NULL      = 0;          // section header is inactive
static const Elf32_Word SHT_PROGBITS  = 1;          // section with CPU instructions
static const Elf32_Word SHT_SYMTAB    = 2;          // section contains symbols table (also as SHT_DYNSIM)
static const Elf32_Word SHT_STRTAB    = 3;          // section holds string table
static const Elf32_Word SHT_RELA      = 4;          // section with relocation data
static const Elf32_Word SHT_HASH      = 5;          // section with hash table. Must be for the dynamic lib
static const Elf32_Word SHT_DYNAMIC   = 6;          // section holds information for dynamic linking.
static const Elf32_Word SHT_NOTE      = 7;
static const Elf32_Word SHT_NOBITS    = 8;          // section with not initialized data
static const Elf32_Word SHT_REL       = 9;
static const Elf32_Word SHT_SHLIB     = 10;
static const Elf32_Word SHT_DYNSYM    = 11;         // section contains debug symbol table
static const Elf32_Word SHT_LOPROC    = 0x70000000;
static const Elf32_Word SHT_HIPROC    = 0x7fffffff;
static const Elf32_Word SHT_HIUSER    = 0xffffffff;
//sh_flags:
static const Elf32_Word SHF_WRITE     = 0x1;        // section contains data that should be writable during process execution.
static const Elf32_Word SHF_ALLOC     = 0x2;        // section occupies memory during process execution. 
static const Elf32_Word SHF_EXECINSTR = 0x4;        // section contains executable machine instructions.
static const Elf32_Word SHF_MASKPROC  = 0xf0000000; // processor-specific sematic

typedef struct SectionHeaderType
{
    Elf32_Word    sh_name;//Index in a header section table (gives name of section)
    Elf32_Word    sh_type;
    uint64_t        sh_flags;
    uint64_t        sh_addr;
    uint64_t        sh_offset;
    uint64_t        sh_size;
    Elf32_Word    sh_link;
    Elf32_Word    sh_info;
    uint64_t    sh_addralign;
    uint64_t    sh_entsize;
} SectionHeaderType;


#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4) + ((t)&0xf))
      
static const unsigned char STB_LOCAL   = 0;
static const unsigned char STB_GLOBAL  = 1;
static const unsigned char STB_WEAK    = 2;
static const unsigned char STB_LOPROC  = 13;
static const unsigned char STB_HIPROC  = 15;

static const unsigned char STT_NOTYPE  = 0;
static const unsigned char STT_OBJECT  = 1;
static const unsigned char STT_FUNC    = 2;
static const unsigned char STT_SECTION = 3;
static const unsigned char STT_FILE    = 4;
static const unsigned char STT_LOPROC  = 13;
static const unsigned char STT_HIPROC  = 15;

typedef struct SymbolTableType
{
    Elf32_Word    st_name;
    uint8_t st_info;
    uint8_t st_other;
    Elf32_Half    st_shndx;
    uint64_t    st_value;
    uint64_t    st_size;
} SymbolTableType;


//p_type:
static const Elf32_Word PT_NULL     = 0;
static const Elf32_Word PT_LOAD     = 1;
static const Elf32_Word PT_DYNAMIC  = 2;
static const Elf32_Word PT_INTERP   = 3;
static const Elf32_Word PT_NOTE     = 4;
static const Elf32_Word PT_SHLIB    = 5;
static const Elf32_Word PT_PHDR     = 6;
static const Elf32_Word PT_LOPROC   = 0x70000000;
static const Elf32_Word PT_HIPROC   = 0x7fffffff;

typedef struct ProgramHeaderType
{
  uint32_t    p_type;
  uint32_t    p_offset;
  uint64_t    p_vaddr;
  uint64_t    p_paddr;
  uint64_t    p_filesz;
  uint64_t    p_memsz;
  uint64_t    p_flags;
  uint64_t    p_align;
} ProgramHeaderType;

}  // namespace debugger

#endif  // __DEBUGGER_ELF_TYPES_H__
