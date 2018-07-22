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

#include "elfreader.h"
#include <iostream>

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(ElfReaderService)

ElfReaderService::ElfReaderService(const char *name) : IService(name) {
    registerInterface(static_cast<IElfReader *>(this));
    registerAttribute("SourceProc", &sourceProc_);
    image_ = NULL;
    sectionNames_ = NULL;
    symbolList_.make_list(0);
    loadSectionList_.make_list(0);
    sourceProc_.make_string("");
    isrc_ = 0;
}

ElfReaderService::~ElfReaderService() {
    if (image_) {
        delete image_;
    }
}

void ElfReaderService::postinitService() {
    isrc_ = static_cast<ISourceCode *>(
       RISCV_get_service_iface(sourceProc_.to_string(),
                               IFACE_SOURCE_CODE));
    if (!isrc_) {
        RISCV_error("SourceCode interface '%s' not found", 
                    sourceProc_.to_string());
    }
}

void ElfReaderService::SwapBytes(Elf32_Half& v) {
    if (header_->e_ident[EI_DATA] != ELFDATA2MSB) {
        return;
    }
     v = ((v>>8)&0xff) | ((v&0xFF)<<8);
}

void ElfReaderService::SwapBytes(Elf32_Word& v) {
    if (header_->e_ident[EI_DATA] != ELFDATA2MSB) {
        return;
    }
    v = ((v >> 24) & 0xff) | ((v >> 8) & 0xff00) 
      | ((v << 8) & 0xff0000) | ((v & 0xFF) << 24);
}

void ElfReaderService::SwapBytes(Elf32_DWord& v) {
    if (header_->e_ident[EI_DATA] != ELFDATA2MSB) {
        return;
    }
    v = ((v >> 56) & 0xffull) | ((v >> 48) & 0xff00ull) 
      | ((v >> 40) & 0xff0000ull) | ((v >> 32) & 0xff000000ull)
      | ((v << 8) & 0xff000000ull) | ((v << 16) & 0xff0000000000ull)
      | ((v << 24) & 0xff0000000000ull) | ((v << 32) & 0xff00000000000000ull);
}

void ElfReaderService::swap_elfheader(ElfHeaderType *h) {
    if (header_->e_ident[EI_DATA] != ELFDATA2MSB) {
        return;
    }
    SwapBytes(h->e_shoff);
    SwapBytes(h->e_shnum);
}

void ElfReaderService::swap_secheader(SectionHeaderType *sh) {
    if (header_->e_ident[EI_DATA] != ELFDATA2MSB) {
        return;
    }
    SwapBytes(sh->sh_name);
    SwapBytes(sh->sh_type);
    SwapBytes(sh->sh_flags);
    SwapBytes(sh->sh_addr);
    SwapBytes(sh->sh_offset);
    SwapBytes(sh->sh_size);
    SwapBytes(sh->sh_link);
    SwapBytes(sh->sh_info);
    SwapBytes(sh->sh_addralign);
    SwapBytes(sh->sh_entsize);
}

void ElfReaderService::swap_symbheader(SymbolTableType *symb) {
    if (header_->e_ident[EI_DATA] != ELFDATA2MSB) {
        return;
    }
    SwapBytes(symb->st_name);
    SwapBytes(symb->st_value);
    SwapBytes(symb->st_size);
    SwapBytes(symb->st_shndx);
}

int ElfReaderService::readFile(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        RISCV_error("File '%s' not found", filename);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);

    if (image_) {
        delete image_;
        sectionNames_ = NULL;
        sourceProc_.make_list(0);
        symbolList_.make_list(0);
        loadSectionList_.make_list(0);
    }
    image_ = new uint8_t[sz];
    fread(image_, 1, sz, fp);

    if (readElfHeader() != 0) {
        fclose(fp);
        return 0;
    }

    symbolNames_ = 0;
    swap_elfheader(header_);
    if (!header_->e_shoff) {
        fclose(fp);
        return 0;
    }

    SectionHeaderType *sh;
    /** Search .shstrtab section */
    sh_tbl_ =
        reinterpret_cast<SectionHeaderType *>(&image_[header_->e_shoff]);
    for (int i = 0; i < header_->e_shnum; i++) {
        sh = &sh_tbl_[i];
        swap_secheader(sh);
        sectionNames_ = reinterpret_cast<char *>(&image_[sh->sh_offset]);
        if (sh->sh_type == SHT_STRTAB && 
            strcmp(sectionNames_ + sh->sh_name, ".shstrtab") != 0) {
            sectionNames_ = NULL;
        }
    }
    if (!sectionNames_) {
        printf("err: section .shstrtab not found.\n");
    }

    /** Search ".strtab" section with Debug symbols */
    sh_tbl_ =
        reinterpret_cast<SectionHeaderType *>(&image_[header_->e_shoff]);
    for (int i = 0; i < header_->e_shnum; i++) {
        sh = &sh_tbl_[i];
        if (sectionNames_ == NULL || sh->sh_type != SHT_STRTAB) {
            continue;
        }
        if (strcmp(sectionNames_ + sh->sh_name, ".strtab")) {
            continue;
        }
        /** 
            * This section holds strings, most commonly the strings that
            * represent the names associated with symbol table entries. 
            * If the file has a loadable segment that includes the symbol
            * string table, the section's attributes will include the
            * SHF_ALLOC bit; otherwise, that bit will be turned off.
            */
        symbolNames_ = reinterpret_cast<char *>(&image_[sh->sh_offset]);
    }
    if (!symbolNames_) {
        printf("err: section .strtab not found. No debug symbols.\n");
    }

    /** Direct loading via tap interface: */
    int bytes_loaded = loadSections();
    RISCV_info("Loaded: %d B", bytes_loaded);

    if (header_->e_phoff) {
        //readProgramHeader();
    }

    fclose(fp);
    return 0;
}

int ElfReaderService::readElfHeader() {
    header_ = reinterpret_cast<ElfHeaderType *>(image_);
    for (int i = 0; i < 4; i++) {
        if (header_->e_ident[i] != MAGIC_BYTES[i]) {
            RISCV_error("File format is not ELF", NULL);
            return -1;
        }
    }
    return 0;
}

int ElfReaderService::loadSections() {
    SectionHeaderType *sh;
    uint64_t total_bytes = 0;
    AttributeType tsymb;

    for (int i = 0; i < header_->e_shnum; i++) {
        sh = &sh_tbl_[i];

        if (sh->sh_size == 0) {
            continue;
        }

        if (sectionNames_ && (sh->sh_flags & SHF_ALLOC)) {
            RISCV_info("Reading '%s' section", &sectionNames_[sh->sh_name]);
        }

        if (sh->sh_type == SHT_PROGBITS && (sh->sh_flags & SHF_ALLOC) != 0) {
            /**
             * @brief   Instructions or other processor's information
             * @details This section holds information defined by the program, 
             *          whose format and meaning are determined solely by the
             *          program.
             */
            AttributeType loadsec;
            loadsec.make_list(LoadSh_Total);
            if (sectionNames_) {
                loadsec[LoadSh_name].make_string(&sectionNames_[sh->sh_name]);
            } else {
                loadsec[LoadSh_name].make_string("unknown");
            }
            loadsec[LoadSh_addr].make_uint64(sh->sh_addr);
            loadsec[LoadSh_size].make_uint64(sh->sh_size);
            loadsec[LoadSh_data].make_data(static_cast<unsigned>(sh->sh_size),
                                           &image_[sh->sh_offset]);
            loadSectionList_.add_to_list(&loadsec);
            total_bytes += sh->sh_size;
        } else if (sh->sh_type == SHT_NOBITS
                    && (sh->sh_flags & SHF_ALLOC) != 0) {
            /**
             * @brief   Initialized data
             * @details A section of this type occupies no space in  the file
             *          but otherwise resembles SHT_PROGBITS.  Although this
             *          section contains no bytes, the sh_offset member
             *          contains the conceptual file offset.
             */
            AttributeType loadsec;
            loadsec.make_list(LoadSh_Total);
            if (sectionNames_) {
                loadsec[LoadSh_name].make_string(&sectionNames_[sh->sh_name]);
            } else {
                loadsec[LoadSh_name].make_string("unknown");
            }
            loadsec[LoadSh_addr].make_uint64(sh->sh_addr);
            loadsec[LoadSh_size].make_uint64(sh->sh_size);
            loadsec[LoadSh_data].make_data(static_cast<unsigned>(sh->sh_size));
            memset(loadsec[LoadSh_data].data(), 
                        0, static_cast<size_t>(sh->sh_size));
            loadSectionList_.add_to_list(&loadsec);
            total_bytes += sh->sh_size;
        } else if (sh->sh_type == SHT_SYMTAB || sh->sh_type == SHT_DYNSYM) {
            processDebugSymbol(sh);
        }
    }
    symbolList_.sort(LoadSh_name);
    if (isrc_) {
        isrc_->addSymbols(&symbolList_);
    }
    return static_cast<int>(total_bytes);
}

void ElfReaderService::processDebugSymbol(SectionHeaderType *sh) {
    uint64_t symbol_off = 0;
    SymbolTableType *st;
    AttributeType tsymb;
    uint8_t st_type;
    const char *symb_name;
    //const char *file_name = 0;

    if (!symbolNames_) {
        return;
    }

    while (symbol_off < sh->sh_size) {
        st = reinterpret_cast<SymbolTableType *>
                    (&image_[sh->sh_offset + symbol_off]);
        swap_symbheader(st);
        symb_name = &symbolNames_[st->st_name];

        if (sh->sh_entsize) {
            // section with elements of fixed size
            symbol_off += sh->sh_entsize; 
        } else if (st->st_size) {
            symbol_off += st->st_size;
        } else {
            symbol_off += sizeof(SymbolTableType);
        }

        st_type = st->st_info & 0xF;
        if ((st_type == STT_OBJECT || st_type == STT_FUNC) && st->st_value) {
            tsymb.make_list(Symbol_Total);
            tsymb[Symbol_Name].make_string(symb_name);
            tsymb[Symbol_Addr].make_uint64(st->st_value);
            tsymb[Symbol_Size].make_uint64(st->st_size);
            if (st_type == STT_FUNC) {
                tsymb[Symbol_Type].make_uint64(SYMBOL_TYPE_FUNCTION);
            } else {
                tsymb[Symbol_Type].make_uint64(SYMBOL_TYPE_DATA);
            }
            symbolList_.add_to_list(&tsymb);
        } else if (st_type == STT_FILE) {
            //file_name = symb_name;
        }
    }
}

}  // namespace debugger
