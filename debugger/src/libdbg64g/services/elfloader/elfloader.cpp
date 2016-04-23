/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      elf-file loader class implementation.
 */

#include "elfloader.h"
#include <iostream>

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(ElfLoaderService)

ElfLoaderService::ElfLoaderService(const char *name) : IService(name) {
    registerInterface(static_cast<IElfLoader *>(this));
    registerAttribute("Tap", &tap_);
    registerAttribute("VerifyEna", &verify_ena_);
    tap_.make_string("");
    verify_ena_.make_boolean(false);
    itap_ = 0;
    image_ = NULL;
    sectionNames_ = NULL;
}

ElfLoaderService::~ElfLoaderService() {
    if (image_) {
        delete image_;
    }
}

void ElfLoaderService::postinitService() {
    IService *iserv = 
        static_cast<IService *>(RISCV_get_service(tap_.to_string()));
    if (!iserv) {
        RISCV_error("TAP service '%'s not found", tap_.to_string());
    }
    itap_ = static_cast<ITap *>(iserv->getInterface(IFACE_TAP));
    if (!itap_) {
        RISCV_error("ITap interface '%s' not found", tap_.to_string());
    }
}

int ElfLoaderService::loadFile(const char *filename) {
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

    readElfHeader();

    if (!header_->e_shoff) {
        fclose(fp);
        return 0;
    }

    /** Init names */
    SectionHeaderType *sh;
    sh_tbl_ = reinterpret_cast<SectionHeaderType *>
                                (&image_[header_->e_shoff]);
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

void ElfLoaderService::readElfHeader() {
    header_ = reinterpret_cast<ElfHeaderType *>(image_);
    for (int i = 0; i < 4; i++) {
        if (header_->e_ident[i] != MAGIC_BYTES[i]) {
            RISCV_error("File format is not ELF", NULL);
            return;
        }
    }
}

int ElfLoaderService::loadSections() {
    SectionHeaderType *sh;
    uint64_t total_bytes = 0;

    for (int i = 0; i < header_->e_shnum; i++) {
        sh = &sh_tbl_[i];

        if (sh->sh_size == 0) {
            continue;
        }
        if ((sh->sh_flags & SHF_ALLOC) == 0) {
            continue;
        }

        if (sectionNames_) {
            RISCV_info("Loading '%s' section", &sectionNames_[sh->sh_name]);
        }

        if (sh->sh_type == SHT_PROGBITS) {
            /**
             * @brief   Instructions or other processor's information
             * @details This section holds information defined by the program, 
             *          whose format and meaning are determined solely by the
             *          program.
             */
            total_bytes += 
                loadMemory(sh->sh_addr, &image_[sh->sh_offset], sh->sh_size);
        } else if (sh->sh_type == SHT_NOBITS) {
            /**
             * @brief   Initialized data
             * @details A section of this type occupies no space in  the file
             *          but otherwise resembles SHT_PROGBITS.  Although this
             *          section contains no bytes, the sh_offset member
             *          contains the conceptual file offset.
             */
             total_bytes += initMemory(sh->sh_addr, sh->sh_size);
        } else if (sh->sh_type == SHT_SYMTAB || sh->sh_type == SHT_DYNSYM) {
            processDebugSymbol(sh);
        }
    }
    return static_cast<int>(total_bytes);
}

void ElfLoaderService::processStringTable(SectionHeaderType *sh) {
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

void ElfLoaderService::processDebugSymbol(SectionHeaderType *sh) {
    uint64_t symbol_off = 0;
    SymbolTableType *st;

    while (symbol_off < sh->sh_size) {
        st = reinterpret_cast<SymbolTableType *>
                    (&image_[sh->sh_offset + symbol_off]);

        if (sh->sh_entsize) {
            // section with elements of fixed size
            symbol_off += sh->sh_entsize; 
        } else if (st->st_size) {
            symbol_off += st->st_size;
        } else {
            symbol_off += sizeof(SymbolTableType);
        }
        //debug_symbols.push_back(*st);
    }
}

uint64_t ElfLoaderService::loadMemory(uint64_t addr, 
                                      uint8_t *buf, uint64_t bufsz) {
    itap_->write(addr, static_cast<int>(bufsz), buf);

    if (verify_ena_.to_bool()) {
        uint8_t *chk = new uint8_t [static_cast<int>(bufsz) + 8];
        itap_->read(addr, static_cast<int>(bufsz), chk);
        for (uint64_t i = 0; i < bufsz; i++) {
            if (buf[i] != chk[i]) {
                RISCV_error("[%08" RV_PRI64 "x] verif. error %02x != %02x",
                            addr + i, buf[i], chk[i]);
            }
        }
        delete [] chk;
    }
    return bufsz;
}

uint64_t ElfLoaderService::initMemory(uint64_t addr, uint64_t bufsz) {
    uint32_t zero = 0;
    uint64_t cnt = 0;
    while (cnt < bufsz) {
        itap_->write(addr, 4, reinterpret_cast<uint8_t *>(&zero));
        addr += 4;
        cnt += 4;
    }
    return bufsz;
}

}  // namespace debugger
