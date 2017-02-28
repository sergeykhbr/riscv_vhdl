/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      elf-file loader class implementation.
 */

#include "elfreader.h"
#include <iostream>

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(ElfReaderService)

ElfReaderService::ElfReaderService(const char *name) : IService(name) {
    registerInterface(static_cast<IElfReader *>(this));
    image_ = NULL;
    sectionNames_ = NULL;
    symbolList_.make_list(0);
    loadSectionList_.make_list(0);
}

ElfReaderService::~ElfReaderService() {
    if (image_) {
        delete image_;
    }
}

void ElfReaderService::postinitService() {
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
    }
    image_ = new uint8_t[sz];
    fread(image_, 1, sz, fp);

    if (readElfHeader() != 0) {
        fclose(fp);
        return 0;
    }

    if (!header_->e_shoff) {
        fclose(fp);
        return 0;
    }

    /** Init names */
    SectionHeaderType *sh;
    sh_tbl_ =
        reinterpret_cast<SectionHeaderType *>(&image_[header_->e_shoff]);
    for (int i = 0; i < header_->e_shnum; i++) {
        sh = &sh_tbl_[i];
        if (sh->sh_type == SHT_STRTAB) {
            processStringTable(sh);
        }
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
    return static_cast<int>(total_bytes);
}

void ElfReaderService::processStringTable(SectionHeaderType *sh) {
    if (sectionNames_ == NULL) {
        sectionNames_ = reinterpret_cast<char *>(&image_[sh->sh_offset]);
        if (strcmp(sectionNames_ + sh->sh_name, ".shstrtab") != 0) {
            /** This section holds section names. */
            printf("err: undefined .shstrtab section\n");
            sectionNames_ = NULL;
        }
    } else if (strcmp(sectionNames_ + sh->sh_name, ".strtab") == 0) {
        /** 
         * This section holds strings, most commonly the strings that
         * represent the names associated with symbol table entries. 
         * If the file has a loadable segment that includes the symbol
         * string table, the section's attributes will include the
         * SHF_ALLOC bit; otherwise, that bit will be turned off.
         */
        symbolNames_ = reinterpret_cast<char *>(&image_[sh->sh_offset]);
    } else {
        RISCV_error("Unsupported string section %s",
                            sectionNames_ + sh->sh_name);
    }
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

void ElfReaderService::addressToSymbol(uint64_t addr, AttributeType *name) {
    if (symbolList_.size() == 0) {
        name->make_string("no symbols");
        return;
    }
    // todo: search in sorted linst
    name->make_string("not found");
    for (unsigned i = 0; i < symbolList_.size(); i++) {
        AttributeType &symb = symbolList_[i][Symbol_Addr];
        if (symb.to_uint64() == addr) {
            *name = symbolList_[i][Symbol_Name];
            return;
        }
    }
}

}  // namespace debugger
