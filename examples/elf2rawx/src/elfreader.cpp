#include "elfreader.h"
#include <fstream>
#include <cstring>

ElfReader::ElfReader(const char *filename) {
    image_ = NULL;
    sectionNames_ = NULL;
    symbolList_.make_list(0);
    loadSectionList_.make_list(0);
    sourceProc_.make_string("");


    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("elf2rawx error: File '%s' not found\n", filename);
        return ;
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
        return;
    }

    symbolNames_ = 0;
    lastLoadAdr_ = header_->get_entry();
    if (!header_->get_shoff()) {
        fclose(fp);
        return;
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
        printf("elf2rawx error: section .shstrtab not found.\n");
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
        printf("elf2rawx error: section .strtab not found. No debug symbols.\n");
    }

    /** Direct loading via tap interface: */
    int bytes_loaded = loadSections();
    printf("elf2rawx: Loaded: %d B\n", bytes_loaded);

    if (header_->get_phoff()) {
        //readProgramHeader();
    }

    fclose(fp);
}

int ElfReader::readElfHeader() {
    header_ = new ElfHeaderType(image_);
    if (header_->isElf()) {
        return 0;
    }
    printf("elf2rawx error: File format is not ELF\n");
    return -1;
}

int ElfReader::loadSections() {
    SectionHeaderType *sh;
    uint64_t total_bytes = 0;
    AttributeType tsymb;

    for (int i = 0; i < header_->get_shnum(); i++) {
        sh = sh_tbl_[i];

        if (sh->get_size() == 0) {
            continue;
        }

#if 0
        if (sectionNames_ && (sh->get_flags() & SHF_ALLOC)) {
            printf("elf2rawx: Reading '%s' section\n",
                    &sectionNames_[sh->get_name()]);
        }
#endif

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
            uint64_t t1 = sh->get_addr() + sh->get_size();
            if (t1 > lastLoadAdr_) {
                lastLoadAdr_ = t1;
            }
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
            uint64_t t1 = sh->get_addr() + sh->get_size();
            if (t1 > lastLoadAdr_) {
                lastLoadAdr_ = t1;
            }
        } else if (sh->get_type() == SHT_SYMTAB || sh->get_type() == SHT_DYNSYM) {
            processDebugSymbol(sh);
        }
    }
    symbolList_.sort(LoadSh_name);
    return static_cast<int>(total_bytes);
}

void ElfReader::processDebugSymbol(SectionHeaderType *sh) {
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
            tsymb[Symbol_Addr].make_uint64(st->get_value());
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

void ElfReader::writeRawImage(AttributeType *ofiles, uint32_t fixed_size) {
#if 0
    SrcElement *e;
    std::ofstream osraw(file_name, std::ios::binary);
    if (!osraw.is_open()) {
        printf("elf2rawx error: can't create output file %s\n", file_name);
        return;
    }
    if (fixed_size && fixed_size < 4*src_img.iSizeWords) {
        printf("Warning: defined size %d is less than actual size %d\n",
            fixed_size, 4*src_img.iSizeWords);
    }

    char ss;
    for (uint32_t i = 0; i < src_img.iSizeWords; i++) {
        e = &src_img.arr[i];

        for (int n=0; n<4; n++) {
            ss = (char)(e->val >> (24-8*n));
            osraw.write(&ss, 1);
        }
    }

    if (fixed_size) {
        uint32_t fix_word = (fixed_size+3)/4;
        for (uint32_t i = src_img.iSizeWords; i < fix_word; i++) {
            uint32_t fix_zero = 0;
            osraw.write((char *)&fix_zero, 4);
        }
    }

    osraw.flush();

    printf("elf2rawx: Image %d Bytes was generated\n", 
        fixed_size ? fixed_size :uiRawImageBytes);
#endif
}

void ElfReader::writeRomHexArray(AttributeType *ofiles,
                                 int bytes_per_line,
                                 int fixed_size) {
    bool st = true;
    uint64_t entry = header_->get_entry();
    uint64_t sz = lastLoadAdr_ - entry;
    if (fixed_size) {
        if (sz >= fixed_size) {
            printf("elf2raw warning: trim image of Size %d B t %d B\n",
                    static_cast<int>(sz), fixed_size);
        }
        sz = fixed_size;
    }

    FILE *fp = fopen((*ofiles)[0u].to_string(), "wb");
    FILE *fplsb = 0;
    if (!fp) {
        printf("elf2rawx error: can't create '%s' file\n",
                (*ofiles)[0u].to_string());
        return ;
    }
    if (ofiles->size() == 2) {
        fplsb = fopen((*ofiles)[1].to_string(), "wb");
    }

    uint8_t *img = new uint8_t[static_cast<int>(sz)];
    memset(img, 0, (size_t)sz);

    uint64_t sec_addr;
    int sec_sz;
    for (unsigned i = 0; i < loadableSectionTotal(); i++) {
        sec_addr = sectionAddress(i) - entry;
        sec_sz = static_cast<int>(sectionSize(i));
        if (sec_addr + sec_sz <= sz) {
            memcpy(&img[sec_addr], sectionData(i), sec_sz);
        } else {
            memcpy(&img[sec_addr], sectionData(i), (size_t)(sz - sec_addr));
            break;
        }
    }

    char tstr[128];
    int tstr_sz;
    uint32_t byte_cnt = 0;
    int startidx, stopidx;
    for (uint64_t i = 0; i < sz/bytes_per_line; i++) {
        tstr_sz = 0;
        startidx = static_cast<int>((i + 1)*bytes_per_line - 1);
        stopidx = static_cast<int>(i*bytes_per_line);
        if (fplsb == 0) {
            /** Single file */
            for (int n = startidx; n >= stopidx; n--) {
                tstr_sz += sprintf(&tstr[tstr_sz], "%02X", img[n]);
            }
            tstr_sz += sprintf(&tstr[tstr_sz], "%s", "\r\n");
            fwrite(tstr, tstr_sz, 1, fp);
        } else {
            /** Couple of files */
            char tstr2[128];
            int tstr2_sz = 0;
            for (int n = startidx; n >= (stopidx+bytes_per_line/2); n--) {
                tstr_sz += sprintf(&tstr[tstr_sz], "%02X", img[n]);
                tstr2_sz += sprintf(&tstr2[tstr2_sz], "%02X", img[n - bytes_per_line/2]);
            }
            tstr_sz += sprintf(&tstr[tstr_sz], "%s", "\r\n");
            tstr2_sz += sprintf(&tstr2[tstr2_sz], "%s", "\r\n");
            fwrite(tstr, tstr_sz, 1, fp);
            fwrite(tstr2, tstr2_sz, 1, fplsb);
        }
        byte_cnt++;
    }
    
    fclose(fp);
    delete [] img;

    printf("elf2rawx: HexRom was generated: %dx%d lines\n", 
            static_cast<int>(sz/bytes_per_line), 8*bytes_per_line);
}
