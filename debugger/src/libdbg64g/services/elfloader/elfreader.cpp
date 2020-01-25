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
    if (!header_->get_shoff()) {
        fclose(fp);
        return 0;
    }

    sh_tbl_ = new SectionHeaderType *[header_->get_shnum()];

    /** Search .shstrtab section */
    uint8_t *psh = &image_[header_->get_shoff()];
    for (int i = 0; i < header_->get_shnum(); i++) {
        sh_tbl_[i] = new SectionHeaderType(psh, header_);

        sectionNames_ = reinterpret_cast<char *>(&image_[sh_tbl_[i]->get_offset()]);
        if (sh_tbl_[i]->get_type() == SHT_STRTAB && 
            strcmp(sectionNames_ + sh_tbl_[i]->get_name(), ".shstrtab") != 0) {
            sectionNames_ = NULL;
        }

        if (header_->isElf32()) {
            psh += sizeof(Elf32_Shdr);
        } else {
            psh += sizeof(Elf64_Shdr);
        }
    }
    if (!sectionNames_) {
        printf("err: section .shstrtab not found.\n");
    }

    /** Search ".strtab" section with Debug symbols */
    SectionHeaderType *sh;
    for (int i = 0; i < header_->get_shnum(); i++) {
        sh = sh_tbl_[i];
        if (sectionNames_ == NULL || sh->get_type() != SHT_STRTAB) {
            continue;
        }
        if (strcmp(sectionNames_ + sh->get_name(), ".strtab")) {
            continue;
        }
        /** 
            * This section holds strings, most commonly the strings that
            * represent the names associated with symbol table entries. 
            * If the file has a loadable segment that includes the symbol
            * string table, the section's attributes will include the
            * SHF_ALLOC bit; otherwise, that bit will be turned off.
            */
        symbolNames_ = reinterpret_cast<char *>(&image_[sh->get_offset()]);
    }
    if (!symbolNames_) {
        printf("err: section .strtab not found. No debug symbols.\n");
    }

    /** Direct loading via tap interface: */
    int bytes_loaded = loadSections();
    RISCV_info("Loaded: %d B", bytes_loaded);

    if (header_->get_phoff()) {
        //readProgramHeader();
    }

    fclose(fp);
    return 0;
}

int ElfReaderService::readElfHeader() {
    header_ = new ElfHeaderType(image_);
    if (header_->isElf()) {
        return 0;
    }
    RISCV_error("File format is not ELF", NULL);
    return -1;
}

int ElfReaderService::loadSections() {
    SectionHeaderType *sh;
    uint64_t total_bytes = 0;
    AttributeType tsymb;

    for (int i = 0; i < header_->get_shnum(); i++) {
        sh = sh_tbl_[i];

        if (sh->get_size() == 0) {
            continue;
        }

        if (sectionNames_ && (sh->get_flags() & SHF_ALLOC)) {
            RISCV_info("Reading '%s' section", &sectionNames_[sh->get_name()]);
        }

        if ((sh->get_type() == SHT_PROGBITS ||
             sh->get_type() == SHT_INIT_ARRAY ||
             sh->get_type() == SHT_FINI_ARRAY ||
             sh->get_type() == SHT_PREINIT_ARRAY) &&
             (sh->get_flags() & SHF_ALLOC) != 0) {
            /**
             * @brief   Instructions or other processor's information
             * @details This section holds information defined by the program, 
             *          whose format and meaning are determined solely by the
             *          program.
             */
            AttributeType loadsec;
            loadsec.make_list(LoadSh_Total);
            if (sectionNames_) {
                loadsec[LoadSh_name].make_string(&sectionNames_[sh->get_name()]);
            } else {
                loadsec[LoadSh_name].make_string("unknown");
            }
            loadsec[LoadSh_addr].make_uint64(sh->get_addr());
            loadsec[LoadSh_size].make_uint64(sh->get_size());
            loadsec[LoadSh_data].make_data(static_cast<unsigned>(sh->get_size()),
                                           &image_[sh->get_offset()]);
            loadSectionList_.add_to_list(&loadsec);
            total_bytes += sh->get_size();
        } else if (sh->get_type() == SHT_NOBITS
                    && (sh->get_flags() & SHF_ALLOC) != 0) {
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
                loadsec[LoadSh_name].make_string(&sectionNames_[sh->get_name()]);
            } else {
                loadsec[LoadSh_name].make_string("unknown");
            }
            loadsec[LoadSh_addr].make_uint64(sh->get_addr());
            loadsec[LoadSh_size].make_uint64(sh->get_size());
            loadsec[LoadSh_data].make_data(static_cast<unsigned>(sh->get_size()));
            memset(loadsec[LoadSh_data].data(), 
                        0, static_cast<size_t>(sh->get_size()));
            loadSectionList_.add_to_list(&loadsec);
            total_bytes += sh->get_size();
        } else if (sh->get_type() == SHT_SYMTAB || sh->get_type() == SHT_DYNSYM) {
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

    while (symbol_off < sh->get_size()) {
        st = new SymbolTableType(&image_[sh->get_offset() + symbol_off],
                                header_);
        
        symb_name = &symbolNames_[st->get_name()];

        st_type = st->get_info() & 0xF;
        if ((st_type == STT_OBJECT || st_type == STT_FUNC) && st->get_value()) {
            tsymb.make_list(Symbol_Total);
            tsymb[Symbol_Name].make_string(symb_name);
            tsymb[Symbol_Addr].make_uint64(st->get_value() & ~1ull);
            tsymb[Symbol_Size].make_uint64(st->get_size());
            if (st_type == STT_FUNC) {
                tsymb[Symbol_Type].make_uint64(SYMBOL_TYPE_FUNCTION);
            } else {
                tsymb[Symbol_Type].make_uint64(SYMBOL_TYPE_DATA);
            }
            symbolList_.add_to_list(&tsymb);
        } else if (st_type == STT_FILE) {
            //file_name = symb_name;
        }

        if (sh->get_entsize()) {
            // section with elements of fixed size
            symbol_off += sh->get_entsize(); 
        } else if (st->get_size()) {
            symbol_off += st->get_size();
        } else {
            if (header_->isElf32()) {
                symbol_off += sizeof(Elf32_Sym);
            } else {
                symbol_off += sizeof(Elf64_Sym);
            }
        }
        delete st;
    }
}

}  // namespace debugger
