#pragma once

#include <inttypes.h>
#include <vector>
#include "elftypes.h"

class SrcElement {
 public:
    //uint32 adr;
    uint32_t val;
    uint8_t *pDataName;    // 0=no name
    uint8_t *pFuncName;
    uint8_t *pSectionName;
    uint8_t *pFileName;
    bool  bInit;
    char disas[128];
};

class SrcImage {
 public:
    uint32_t entry;
    uint32_t iSizeWords;
    SrcElement *arr;
};

class ElfReader {
 public:
    ElfReader(const char *file_name);
    ~ElfReader();

    bool isOpened() { return iElfFileSize != 0; }
    void writeRawImage(const char *file_name, uint32_t fixed_size = 0);
    void writeRomHexArray(const char *file_name, uint64_t base_addr,
                          uint32_t bytes_per_line, uint32_t fixed_size = 0);

 private:
    int iElfFileSize;
    uint8_t *elf_img;

    ElfHeaderType *Elf32_Ehdr;
    std::vector<SectionHeaderType> section;
    std::vector<SymbolTableType> debug_symbols;
    std::vector<ProgramHeaderType> program;

    char *pSectionNames;  // .shstrtab
    char *pSymbolName;    // .strtab
    uint32_t uiRawImageBytes;
    SrcImage src_img;


    void readElfHeader();
    void readSections();
    void createRawImage();
    void attachSymbolsToRawImage();
    void readProgramHeader(); // doesn't used information

    void SwapBytes(Elf32_Half &);
    void SwapBytes(Elf32_Word &);
    void SwapBytes(uint64_t &);
    uint32_t read32(uint32_t off);
};
