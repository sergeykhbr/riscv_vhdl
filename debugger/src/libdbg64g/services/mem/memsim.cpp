/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      ROM functional model implementation.
 */

#include "api_core.h"
#include "memsim.h"
#include <iostream>
#include <string.h>

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(MemorySim)

MemorySim::MemorySim(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("InitFile", &initFile_);
    registerAttribute("ReadOnly", &readOnly_);
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);

    initFile_.make_string("");
    readOnly_.make_boolean(false);
    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    mem_ = NULL;
}

MemorySim::~MemorySim() {
    if (mem_) {
        delete mem_;
    }
}

void MemorySim::postinitService() {
    // Get global settings:
    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["SimEnable"].to_bool() == false) {
        return;
    }

    mem_ = new uint8_t[static_cast<unsigned>(length_.to_uint64())];
    if (initFile_.size() == 0) {
        return;
    }

    std::string filename(initFile_.to_string());
    if (strstr(initFile_.to_string(), "..") != NULL ||
        strstr(initFile_.to_string(), "/") == NULL) {
        char path[1024];
        RISCV_get_core_folder(path, sizeof(path));
        std::string spath(path);
        filename = spath + std::string(initFile_.to_string());
    }

    FILE *fp = fopen(initFile_.to_string(), "r");
    if (fp == NULL) {
        for (uint64_t i = 0; i < length_.to_uint64()/4; i++) {
            // NOP isntruction
            reinterpret_cast<uint32_t *>(mem_)[i] = 0x00000013; 
        }
        RISCV_error("Can't open '%s' file", initFile_.to_string());
        return;
    }

    bool bhalf = false;
    int rd_symb;
    uint8_t symb;
    int linecnt = 0;
    int symbinline = SYMB_IN_LINE - 1;
    while (!feof(fp)) {
        rd_symb = fgetc(fp);
        if (!chishex(rd_symb))
            continue;

        if (!bhalf) {
            bhalf = true;
            symb = chtohex(rd_symb) << 4;
            continue;
        } 

        bhalf = false;
        symb |= chtohex(rd_symb) & 0xf;

        if ((SYMB_IN_LINE * linecnt + symbinline) >= 
                    static_cast<int>(length_.to_uint64())) {
            RISCV_error("HEX file tries to write out "
                        "of allocated array\n", NULL);
            break;
        } 

        mem_[SYMB_IN_LINE * linecnt + symbinline] = symb;
        if (--symbinline < 0) {
            linecnt++;
            symbinline = SYMB_IN_LINE - 1;
        }
    }
    fclose(fp);
}

void MemorySim::predeleteService() {
    
}

void MemorySim::transaction(Axi4TransactionType *payload) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = (payload->addr - getBaseAddress()) & mask;
    if (payload->rw) {
        if (readOnly_.to_bool()) {
            RISCV_error("Write to READ ONLY memory", NULL);
        } else {
            for (uint64_t i = 0; i < payload->xsize; i++) {
                if (((payload->wstrb >> i) & 0x1) == 0) {
                    continue;
                }
                mem_[off + i] = (payload->wpayload[i >> 2] >> 8*(i & 0x3));
            }
        }
    } else {
        memcpy(payload->rpayload, &mem_[off], payload->xsize);
    }

    const char *rw_str[2] = {"=>", "<="};
    uint32_t *pdata[2] = {payload->rpayload, payload->wpayload};
    RISCV_debug("[%08" RV_PRI64 "x] %s [%08x %08x %08x %08x]",
        payload->addr,
        rw_str[payload->rw],
        pdata[payload->rw][3], pdata[payload->rw][2], 
        pdata[payload->rw][1], pdata[payload->rw][0]);
}

bool MemorySim::chishex(int s) {
    bool ret = false;
    if (s >= '0' && s <= '9') {
        ret = true;
    } else if (s >= 'A' && s <= 'F') {
        ret = true;
    } else if (s >= 'a' && s <= 'f') {
        ret = true;
    }
    return ret;
}

uint8_t MemorySim::chtohex(int s) {
    if (s >= '0' && s <= '9') {
        s -= '0';
    } else if (s >= 'A' && s <= 'F') {
        s = s - 'A' + 10;
    } else if (s >= 'a' && s <= 'f') {
        s = s - 'a' + 10;
    }
    return static_cast<uint8_t>(s);
}

}  // namespace debugger

