#include "elfreader.h"
#include <fstream>
#include <cstring>

ElfReader::ElfReader(const char *file_name)
{
    src_img.arr = NULL;
    pSectionNames = NULL;

    std::ifstream *pisElf = new std::ifstream(file_name, std::ios::binary);
    iElfFileSize = 0;
    if (pisElf->is_open()) {
        pisElf->seekg(0, std::ios::end);
        iElfFileSize = (int)pisElf->tellg();
        if(iElfFileSize) {
            elf_img = new uint8 [iElfFileSize];
            pisElf->seekg(0, std::ios::beg);
            pisElf->read((char *)elf_img, iElfFileSize);
        }
        pisElf->close();
    }
    delete pisElf;

    if (isOpened()) {
        readElfHeader();
        if (Elf32_Ehdr->e_shoff) {
            readSections();
            createRawImage();
            //attachSymbolsToRawImage();
        }
        if (Elf32_Ehdr->e_phoff) {
            readProgramHeader();
        }

    }
}

ElfReader::~ElfReader()
{
    if (iElfFileSize) delete [] elf_img;
    if (src_img.arr) delete [] src_img.arr;
}

//****************************************************************************
void ElfReader::SwapBytes(Elf32_Half& v)
{
    if (Elf32_Ehdr->e_ident[EI_DATA] == ELFDATA2MSB) {
          v = ((v>>8)&0xff) | ((v&0xFF)<<8);
    }
}

//****************************************************************************
void ElfReader::SwapBytes(Elf32_Word& v)
{
    if (Elf32_Ehdr->e_ident[EI_DATA] == ELFDATA2MSB) {
        v = ((v>>24)&0xff) | ((v>>8)&0xff00) | ((v<<8)&0xff0000) | ((v&0xFF)<<24);
    }
}

//****************************************************************************
void ElfReader::SwapBytes(uint64_t& v)
{
    if (Elf32_Ehdr->e_ident[EI_DATA] == ELFDATA2MSB) {
        v = ((v>>24)&0xff) | ((v>>8)&0xff00) | ((v<<8)&0xff0000) | ((v&0xFF)<<24);
    }
}

//****************************************************************************
uint32 ElfReader::read32(uint32 off)
{
    uint32 word;
    if(Elf32_Ehdr->e_ident[EI_DATA] == ELFDATA2MSB)  {
        word = uint32(elf_img[off+0])<<24;
        word |= uint32(elf_img[off+1])<<16;
        word |= uint32(elf_img[off+2])<<8;
        word |= uint32(elf_img[off+3])<<0;
    } else {
        word = uint32(elf_img[off+3])<<24;
        word |= uint32(elf_img[off+2])<<16;
        word |= uint32(elf_img[off+1])<<8;
        word |= uint32(elf_img[off+0])<<0;
    }
    return word;
}

//****************************************************************************
void ElfReader::readElfHeader()
{
    Elf32_Ehdr = (ElfHeaderType *)elf_img;

    for (int i=0; i<4; i++) {
        if(Elf32_Ehdr->e_ident[i] != MAGIC_BYTES[i]) {
            printf("err: wrong file format\n");
            return;
        }
    }

    SwapBytes(Elf32_Ehdr->e_type);
    SwapBytes(Elf32_Ehdr->e_machine);
    SwapBytes(Elf32_Ehdr->e_entry);
    SwapBytes(Elf32_Ehdr->e_phoff);
    SwapBytes(Elf32_Ehdr->e_shoff);
    SwapBytes(Elf32_Ehdr->e_flags);
    SwapBytes(Elf32_Ehdr->e_ehsize);   
    SwapBytes(Elf32_Ehdr->e_phentsize);
    SwapBytes(Elf32_Ehdr->e_phnum);    
    SwapBytes(Elf32_Ehdr->e_shentsize);
    SwapBytes(Elf32_Ehdr->e_shnum);    
    SwapBytes(Elf32_Ehdr->e_shstrndx);
}


//****************************************************************************
void ElfReader::readSections()
{
    SectionHeaderType *sh;
    SymbolTableType *st;
    uiRawImageBytes = Elf32_Ehdr->e_entry; // Compute relative Base Address

    uint32 symbol_off;
    uint32 section_off = Elf32_Ehdr->e_shoff;

    for(int i=0; i<Elf32_Ehdr->e_shnum; i++) {
        sh = (SectionHeaderType *)&elf_img[section_off];

        SwapBytes(sh->sh_name);//Index in a header section table (gives name of section)
        SwapBytes(sh->sh_type);
        SwapBytes(sh->sh_flags);
        SwapBytes(sh->sh_addr);
        SwapBytes(sh->sh_offset);
        SwapBytes(sh->sh_size);
        SwapBytes(sh->sh_link);
        SwapBytes(sh->sh_info);
        SwapBytes(sh->sh_addralign);
        SwapBytes(sh->sh_entsize);

        switch (sh->sh_type) {
        // string names:
        case SHT_STRTAB:
            if (pSectionNames == NULL) {
                pSectionNames = (char *)&elf_img[sh->sh_offset];
                if (strcmp(pSectionNames+sh->sh_name, ".shstrtab") != 0) {
                    printf("err: undefined .shstrtab section\n");
                    pSectionNames = NULL;
                }
            } else if (strcmp(pSectionNames+sh->sh_name, ".strtab") == 0) {
                pSymbolName  = (char *)&elf_img[sh->sh_offset];
            } else {
                printf("err: unsupported string section %s\n", pSectionNames+sh->sh_name);
            }
            break;

        case SHT_PROGBITS:  // instructions or other processor's information
        case SHT_NOBITS:    // initialized data
            if (sh->sh_flags & SHF_ALLOC) {
                if ((sh->sh_addr + sh->sh_size) > uiRawImageBytes)
                    uiRawImageBytes = sh->sh_addr + sh->sh_size;
            }
            break;

        // Debug symbols:
        case SHT_SYMTAB:
        case SHT_DYNSYM:
            symbol_off = 0;
            while (symbol_off < sh->sh_size) {
                st = (SymbolTableType *)&elf_img[sh->sh_offset+symbol_off];
                SwapBytes(st->st_name);
                SwapBytes(st->st_value);
                SwapBytes(st->st_size);
                SwapBytes(st->st_shndx);

                if(sh->sh_entsize)   symbol_off += sh->sh_entsize;    // section with elements of fixed size
                else if(st->st_size) symbol_off += st->st_size;
                else                 symbol_off += sizeof(SymbolTableType);

                debug_symbols.push_back(*st);
            }
            break;
        default:;
        }

        section.push_back(*sh);
        section_off += sizeof(SectionHeaderType);
    }

    uiRawImageBytes -= Elf32_Ehdr->e_entry;
}

//****************************************************************************
void ElfReader::createRawImage()
{
    src_img.entry = Elf32_Ehdr->e_entry;
    src_img.iSizeWords = (uiRawImageBytes+3)/4;
    src_img.arr = new SrcElement[src_img.iSizeWords];
    memset(src_img.arr, 0, src_img.iSizeWords*sizeof(SrcElement));

    // print Program defined section:
    uint32 adr;
    uint32 word;
    SectionHeaderType *sh;
    for (unsigned i=0; i<section.size(); i++) {

        sh = &section[i];

        if (sh->sh_size == 0) continue;
        if ((sh->sh_flags & SHF_ALLOC) == 0) continue;

        adr = (sh->sh_addr - src_img.entry)/4;
        if ((adr + sh->sh_size/4) > src_img.iSizeWords) {
            printf("err: section address %08x is out of range of the allocated image\n", adr);
            continue;
        }

        src_img.arr[adr].pSectionName = (uint8 *)(pSectionNames + sh->sh_name);
        if ((sh->sh_type == SHT_PROGBITS) || (sh->sh_type == SHT_NOBITS)) {

            for (uint32 n=0; n<sh->sh_size; n+=4) {
                if (sh->sh_type == SHT_PROGBITS) {
                    word = read32(sh->sh_offset+n);
                } else if(sh->sh_type == SHT_NOBITS) {
                    word = 0;
                }

                src_img.arr[adr++].val = word;
            }
        }
    }
}

//****************************************************************************
void ElfReader::readProgramHeader()
{
    ProgramHeaderType *pr;
    uint32 pr_offset = Elf32_Ehdr->e_phoff;
  
    for(uint32 i=0; i<Elf32_Ehdr->e_phnum; i++) {
        pr = (ProgramHeaderType *)&elf_img[pr_offset];
        SwapBytes(pr->p_type);
        SwapBytes(pr->p_offset);
        SwapBytes(pr->p_vaddr);
        SwapBytes(pr->p_paddr);
        SwapBytes(pr->p_filesz);
        SwapBytes(pr->p_memsz);
        SwapBytes(pr->p_flags);
        SwapBytes(pr->p_align);

        program.push_back(*pr);
    }
}


//****************************************************************************
void ElfReader::attachSymbolsToRawImage()
{
    SymbolTableType *st;
    uint32 offset;
    for (uint32 i=0; i<debug_symbols.size(); i++) {
        st = &debug_symbols[i];
        offset = (st->st_value - src_img.entry)/4;  // TODO: byte access

        if (offset > src_img.iSizeWords) {
            printf("err: Symbol has address %08x out of image range\n", offset);
            continue;
        }

        switch(ELF32_ST_TYPE(st->st_info)) {
        case STT_OBJECT:
            src_img.arr[offset].pDataName = (uint8 *)(pSymbolName + st->st_name);
            break;
        case STT_FUNC:
            src_img.arr[offset].pFuncName = (uint8 *)(pSymbolName + st->st_name);
            break;
        case STT_FILE:
            src_img.arr[offset].pFileName = (uint8 *)(pSymbolName + st->st_name);
            break;
        default:;
        }
    }
}

//****************************************************************************
void ElfReader::writeRawImage(const char *file_name, uint32 fixed_size)
{
    SrcElement *e;
    std::ofstream osraw(file_name, std::ios::binary);
    if (!osraw.is_open()) {
        printf("err: can't create output file %s\n", file_name);
        return;
    }
    if (fixed_size && fixed_size < 4*src_img.iSizeWords) {
        printf("Warning: defined size %d is less than actual size %d\n",
            fixed_size, 4*src_img.iSizeWords);
    }

    char ss;
    for(uint32 i=0; i<src_img.iSizeWords; i++) {
        e = &src_img.arr[i];

        for (int n=0; n<4; n++) {
            ss = (char)(e->val >> (24-8*n));
            osraw.write(&ss, 1);
        }
    }

    if (fixed_size) {
        uint32 fix_word = (fixed_size+3)/4;
        for (uint32 i=src_img.iSizeWords; i<fix_word; i++) {
            uint32 fix_zero = 0;
            osraw.write((char *)&fix_zero, 4);
        }
    }

#if 0
    // Warning! Bug in BOOT ROM was fixed when MBR was added, but incorrect
    //          could be output like "coping section ffff"
    if ((src_img.iSizeWords&0x1ff) == 0) {
        uint32 fix_zero = 0;
        osraw.write((char *)&fix_zero, 4);
    }
#endif
    osraw.flush();

    printf("Image %d Bytes was generated\n", 
        fixed_size ? fixed_size :uiRawImageBytes);
}

//****************************************************************************
void ElfReader::writeRomHexArray(const char *file_name, uint32 bytes_per_line, uint32 fixed_size)
{
    if (fixed_size && fixed_size < 4*src_img.iSizeWords) {
        printf("Warning: defined size %d is less than actual size %d\n",
            fixed_size, 4*src_img.iSizeWords);
    }

    // generate *.rom array
    char chRomFile[64];
    std::string rom_file_name(file_name);
    std::ofstream osRom(rom_file_name.c_str(), std::ios::out);
    uint32 words_per_line = bytes_per_line / 4;

    uint32 words_total;
    if (fixed_size == 0) {
        words_total = src_img.iSizeWords;
    } else {
        words_total = (fixed_size + (bytes_per_line - 1)) / bytes_per_line;
    }

    int idx;
    for (uint32 i = 0; i < words_total; i += words_per_line) {
        for (uint32 n = words_per_line; n > 0; n--) {
            idx = i + n - 1;
            if (idx < src_img.iSizeWords) {
                sprintf(chRomFile,"%08X", src_img.arr[idx].val);
            } else {
                sprintf(chRomFile,"%08X", 0);
            }
            osRom << chRomFile;
        }
        osRom << "\n";
    }


    osRom.close();

    printf("HexRom was generated: %dx32 words\n", words_total);
}
