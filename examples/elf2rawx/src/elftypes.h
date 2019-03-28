/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <inttypes.h>

typedef uint64_t   ElfAddr64;
typedef uint64_t   ElfOff64;
typedef unsigned short ElfHalf;
typedef signed   int   ElfSword;
typedef unsigned int   ElfWord;
typedef uint64_t       ElfDWord;

typedef unsigned int   ElfAddr32;
typedef unsigned int   ElfOff32;

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
static const ElfHalf ET_NONE      = 0;       // no file type
static const ElfHalf ET_REL       = 1;       // rellocatable file
static const ElfHalf ET_EXEC      = 2;       // executable file
static const ElfHalf ET_DYN       = 3;       // shared object file
static const ElfHalf ET_CORE      = 4;       // core file
static const ElfHalf ET_LOPROC    = 0xff00;  // Processor-specific
static const ElfHalf ET_HIPROC    = 0xffff;  // Processor-specific
//emachine values:
static const ElfHalf EM_NONE      = 0;       // No machine
static const ElfHalf EM_M32       = 1;       // AT&T WE 32100
static const ElfHalf EM_SPARC     = 2;       // SPARC
static const ElfHalf EM_386       = 3;       // Intel 386
static const ElfHalf EM_68K       = 4;       // Motorola 68000
static const ElfHalf EM_88K       = 5;       // Motorola 88000
static const ElfHalf EM_860       = 7;       // Intel 80860
static const ElfHalf EM_MIPS      = 8;       // MIPS RS3000

struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    ElfHalf e_type;      // Shared/Executable/Rellocalable etc
    ElfHalf e_machine;   // SPARC, X86 etc
    ElfWord e_version;   //
    ElfAddr32 e_entry;     // entry point
    ElfOff32  e_phoff;     // Program header offset
    ElfOff32  e_shoff;     // Section Header offset
    ElfWord e_flags;
    ElfHalf e_ehsize;
    ElfHalf e_phentsize; // size of one entry in the Program header. All entries are the same size
    ElfHalf e_phnum;     // number of entries in a Program header
    ElfHalf e_shentsize; // entry size in the section header table. all entries are the same size
    ElfHalf e_shnum;     // number of section header entries
    ElfHalf e_shstrndx;
};

struct Elf64_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    ElfHalf e_type;      // Shared/Executable/Rellocalable etc
    ElfHalf e_machine;   // SPARC, X86 etc
    ElfWord e_version;   //
    ElfAddr64 e_entry;     // entry point
    ElfOff64  e_phoff;     // Program header offset
    ElfOff64  e_shoff;     // Section Header offset
    ElfWord e_flags;
    ElfHalf e_ehsize;
    ElfHalf e_phentsize; // size of one entry in the Program header. All entries are the same size
    ElfHalf e_phnum;     // number of entries in a Program header
    ElfHalf e_shentsize; // entry size in the section header table. all entries are the same size
    ElfHalf e_shnum;     // number of section header entries
    ElfHalf e_shstrndx;
};


static ElfHalf SwapBytes(ElfHalf v) {
     v = ((v>>8)&0xff) | ((v&0xFF)<<8);
     return v;
}

static ElfWord SwapBytes(ElfWord v) {
    v = ((v >> 24) & 0xff) | ((v >> 8) & 0xff00) 
      | ((v << 8) & 0xff0000) | ((v & 0xFF) << 24);
    return v;
}

static ElfDWord SwapBytes(ElfDWord v) {
    v = ((v >> 56) & 0xffull) | ((v >> 48) & 0xff00ull) 
      | ((v >> 40) & 0xff0000ull) | ((v >> 32) & 0xff000000ull)
      | ((v << 8) & 0xff000000ull) | ((v << 16) & 0xff0000000000ull)
      | ((v << 24) & 0xff0000000000ull) | ((v << 32) & 0xff00000000000000ull);
    return v;
}

class ElfHeaderType {
 public:
    ElfHeaderType(uint8_t *img) {
        pimg_ = img;
        isElf_ = true;
        for (int i = 0; i < 4; i++) {
            if (pimg_[i] != MAGIC_BYTES[i]) {
                isElf_ = false;
            }
        }
        is32b_ = pimg_[EI_CLASS] == ELFCLASS32;
        isMsb_ = pimg_[EI_DATA] == ELFDATA2MSB;
        if (is32b_) {
            Elf32_Ehdr *h = reinterpret_cast<Elf32_Ehdr *>(pimg_);
            if (isMsb_) {
                e_shoff_ = SwapBytes(h->e_shoff);
                e_shnum_ = SwapBytes(h->e_shnum);
                e_phoff_ = SwapBytes(h->e_phoff);
                e_entry_ = SwapBytes(h->e_entry);
            } else {
                e_shoff_ = h->e_shoff;
                e_shnum_ = h->e_shnum;
                e_phoff_ = h->e_phoff;
                e_entry_ = h->e_entry;
            }
        } else {
            Elf64_Ehdr *h = reinterpret_cast<Elf64_Ehdr *>(pimg_);
            if (isMsb_) {
                e_shoff_ = SwapBytes(h->e_shoff);
                e_shnum_ = SwapBytes(h->e_shnum);
                e_phoff_ = SwapBytes(h->e_phoff);
                e_entry_ = SwapBytes(h->e_entry);
            } else {
                e_shoff_ = h->e_shoff;
                e_shnum_ = h->e_shnum;
                e_phoff_ = h->e_phoff;
                e_entry_ = h->e_entry;
            }
        }
    }

    virtual bool isElf() { return isElf_; }
    virtual bool isElf32() { return is32b_; }
    virtual bool isElfMsb() { return isMsb_; }
    virtual uint64_t get_shoff() { return e_shoff_; }
    virtual ElfHalf get_shnum() { return e_shnum_; }
    virtual uint64_t get_phoff() { return e_phoff_; }
    virtual uint64_t get_entry() { return e_entry_; }
 protected:
    uint8_t *pimg_;
    bool isElf_;
    bool is32b_;
    bool isMsb_;
    uint64_t e_shoff_;
    ElfHalf e_shnum_;
    uint64_t e_phoff_;
    uint64_t e_entry_;
};

   
//sh_type:
static const ElfWord SHT_NULL      = 0;          // section header is inactive
static const ElfWord SHT_PROGBITS  = 1;          // section with CPU instructions
static const ElfWord SHT_SYMTAB    = 2;          // section contains symbols table (also as SHT_DYNSIM)
static const ElfWord SHT_STRTAB    = 3;          // section holds string table
static const ElfWord SHT_RELA      = 4;          // section with relocation data
static const ElfWord SHT_HASH      = 5;          // section with hash table. Must be for the dynamic lib
static const ElfWord SHT_DYNAMIC   = 6;          // section holds information for dynamic linking.
static const ElfWord SHT_NOTE      = 7;
static const ElfWord SHT_NOBITS    = 8;          // section with not initialized data
static const ElfWord SHT_REL       = 9;
static const ElfWord SHT_SHLIB     = 10;
static const ElfWord SHT_DYNSYM    = 11;         // section contains debug symbol table
static const ElfWord SHT_INIT_ARRAY    = 14;
static const ElfWord SHT_FINI_ARRAY    = 15;
static const ElfWord SHT_PREINIT_ARRAY = 16;
static const ElfWord SHT_LOPROC    = 0x70000000;
static const ElfWord SHT_HIPROC    = 0x7fffffff;
static const ElfWord SHT_HIUSER    = 0xffffffff;
//sh_flags:
static const ElfWord SHF_WRITE     = 0x1;        // section contains data that should be writable during process execution.
static const ElfWord SHF_ALLOC     = 0x2;        // section occupies memory during process execution. 
static const ElfWord SHF_EXECINSTR = 0x4;        // section contains executable machine instructions.
static const ElfWord SHF_MASKPROC  = 0xf0000000; // processor-specific sematic

struct Elf32_Shdr {
    ElfWord    sh_name;//Index in a header section table (gives name of section)
    ElfWord    sh_type;
 	ElfWord	sh_flags;	/* SHF_... */
 	ElfAddr32	sh_addr;
 	ElfOff32	sh_offset;
 	ElfWord	sh_size;
    ElfWord    sh_link;
    ElfWord    sh_info;
    ElfWord   sh_addralign;
    ElfWord   sh_entsize;
};

struct Elf64_Shdr {
    ElfWord    sh_name;//Index in a header section table (gives name of section)
    ElfWord    sh_type;
    ElfDWord   sh_flags;
    ElfAddr64  sh_addr;
    ElfOff64   sh_offset;
    ElfDWord   sh_size;
    ElfWord    sh_link;
    ElfWord    sh_info;
    ElfDWord   sh_addralign;
    ElfDWord   sh_entsize;
};

class SectionHeaderType {
 public:
    SectionHeaderType(uint8_t *img, ElfHeaderType *h) {
        if (h->isElf32()) {
            Elf32_Shdr *sh = reinterpret_cast<Elf32_Shdr *>(img);
            if (h->isElfMsb()) {
                sh_name_ = SwapBytes(sh->sh_name);
                sh_type_ = SwapBytes(sh->sh_type);
                sh_flags_ = SwapBytes(sh->sh_flags);
                sh_addr_ = SwapBytes(sh->sh_addr);
                sh_offset_ = SwapBytes(sh->sh_offset);
                sh_size_ = SwapBytes(sh->sh_size);
                sh_entsize_ = SwapBytes(sh->sh_entsize);
            } else {
                sh_name_ = sh->sh_name;
                sh_type_ = sh->sh_type;
                sh_flags_ = sh->sh_flags;
                sh_addr_ = sh->sh_addr;
                sh_offset_ = sh->sh_offset;
                sh_size_ = sh->sh_size;
                sh_entsize_ = sh->sh_entsize;
            }
        } else {
            Elf64_Shdr *sh = reinterpret_cast<Elf64_Shdr *>(img);
            if (h->isElfMsb()) {
                sh_name_ = SwapBytes(sh->sh_name);
                sh_type_ = SwapBytes(sh->sh_type);
                sh_flags_ = SwapBytes(sh->sh_flags);
                sh_addr_ = SwapBytes(sh->sh_addr);
                sh_offset_ = SwapBytes(sh->sh_offset);
                sh_size_ = SwapBytes(sh->sh_size);
                sh_entsize_ = SwapBytes(sh->sh_entsize);
            } else {
                sh_name_ = sh->sh_name;
                sh_type_ = sh->sh_type;
                sh_flags_ = sh->sh_flags;
                sh_addr_ = sh->sh_addr;
                sh_offset_ = sh->sh_offset;
                sh_size_ = sh->sh_size;
                sh_entsize_ = sh->sh_entsize;
            }
        }
    }
    virtual ElfWord get_name() { return sh_name_; }
    virtual ElfWord get_type() { return sh_type_; }
    virtual uint64_t get_offset() { return sh_offset_; }
    virtual uint64_t get_size() { return sh_size_; }
    virtual uint64_t get_addr() { return sh_addr_; }
    virtual uint64_t get_flags() { return sh_flags_; }
    virtual uint64_t get_entsize() { return sh_entsize_; }
 protected:
    ElfWord sh_name_;
    ElfWord sh_type_;
    uint64_t sh_offset_;
    uint64_t sh_size_;
    uint64_t sh_addr_;
    uint64_t sh_flags_;
    uint64_t sh_entsize_;
};


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

struct Elf32_Sym {
    ElfWord    st_name;
 	ElfAddr32	st_value;
 	ElfWord	st_size;
 	unsigned char	st_info;
 	unsigned char	st_other;
 	ElfHalf	st_shndx;
};

struct Elf64_Sym {
    ElfWord    st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	ElfHalf	st_shndx;
	ElfAddr64	st_value;
	ElfDWord	st_size;
};

class SymbolTableType {
 public:
    SymbolTableType(uint8_t *img, ElfHeaderType *h) {
        if (h->isElf32()) {
            Elf32_Sym *st = reinterpret_cast<Elf32_Sym *>(img);
            if (h->isElfMsb()) {
                st_name_ = SwapBytes(st->st_name);
                st_value_ = SwapBytes(st->st_value);
                st_size_ = SwapBytes(st->st_size);
            } else {
                st_name_ = st->st_name;
                st_value_ = st->st_value;
                st_size_ = st->st_size;
            }
            st_info_ = st->st_info;
        } else {
            Elf64_Sym *st = reinterpret_cast<Elf64_Sym *>(img);
            if (h->isElfMsb()) {
                st_name_ = SwapBytes(st->st_name);
                st_value_ = SwapBytes(st->st_value);
                st_size_ = SwapBytes(st->st_size);
            } else {
                st_name_ = st->st_name;
                st_value_ = st->st_value;
                st_size_ = st->st_size;
            }
            st_info_ = st->st_info;
        }
    }
    virtual ~SymbolTableType() {}
    virtual ElfWord get_name() { return st_name_; }
    virtual uint64_t get_value() { return st_value_; }
    virtual uint64_t get_size() { return st_size_; }
    virtual unsigned char get_info() { return st_info_; }
 protected:
    ElfWord    st_name_;
    unsigned char st_info_;
    uint64_t st_value_;
    uint64_t st_size_;
};


//p_type:
static const ElfWord PT_NULL     = 0;
static const ElfWord PT_LOAD     = 1;
static const ElfWord PT_DYNAMIC  = 2;
static const ElfWord PT_INTERP   = 3;
static const ElfWord PT_NOTE     = 4;
static const ElfWord PT_SHLIB    = 5;
static const ElfWord PT_PHDR     = 6;
static const ElfWord PT_LOPROC   = 0x70000000;
static const ElfWord PT_HIPROC   = 0x7fffffff;

typedef struct ProgramHeaderType64 {
    uint32_t    p_type;
    uint32_t    p_offset;
    uint64_t    p_vaddr;
    uint64_t    p_paddr;
    ElfDWord	p_filesz;
    ElfDWord	p_memsz;
    ElfDWord	p_flags;
    ElfDWord	p_align;
} ProgramHeaderType64;

typedef struct ProgramHeaderType32 {
    uint32_t    p_type;
    uint32_t    p_offset;
    ElfAddr32    p_vaddr;
    ElfAddr32    p_paddr;
    ElfWord	p_filesz;
    ElfWord	p_memsz;
    ElfWord	p_flags;
    ElfWord	p_align;
} ProgramHeaderType32;

