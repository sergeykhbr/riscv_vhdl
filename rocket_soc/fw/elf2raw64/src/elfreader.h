#pragma once

#include "elftypes.h"
#include <vector>


class SrcElement
{
  friend class dbg;
  public:
    //uint32 adr;
    uint32 val;
    uint8 *pDataName;    // 0=no name
    uint8 *pFuncName;
    uint8 *pSectionName;
    uint8 *pFileName;
    bool  bInit;
    char disas[128];
};

class SrcImage
{
  friend class dbg;
  public:
    uint32 entry;
    uint32 iSizeWords;
    SrcElement *arr;
};


class ElfReader
{
    int iElfFileSize;
    uint8 *elf_img;

    ElfHeaderType *Elf32_Ehdr;
    std::vector<SectionHeaderType> section;
    std::vector<SymbolTableType> debug_symbols;
    std::vector<ProgramHeaderType> program;

    char *pSectionNames;  // .shstrtab
    char *pSymbolName;    // .strtab
    uint32 uiRawImageBytes;
    SrcImage src_img;


    void readElfHeader();
    void readSections();
    void createRawImage();
    void attachSymbolsToRawImage();
    void readProgramHeader(); // doesn't used information

    void SwapBytes(Elf32_Half &);
    void SwapBytes(Elf32_Word &);
    void SwapBytes(uint64_t &);
    uint32 read32(uint32 off);


public:
    ElfReader(const char *file_name);
    ~ElfReader();

    bool isOpened() { return iElfFileSize != 0; }
    void writeRawImage(const char *file_name, uint32 fixed_size=0);
    void writeRomHexArray(const char *file_name, uint32 bytes_per_line, uint32 fixed_size=0);

};
