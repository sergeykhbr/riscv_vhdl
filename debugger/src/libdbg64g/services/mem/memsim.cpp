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

#include "api_core.h"
#include "memsim.h"
#include <iostream>
#include <string.h>

namespace debugger {

MemorySim::MemorySim(const char *name)  : MemoryGeneric(name) {
    registerAttribute("InitFile", &initFile_);
    registerAttribute("BinaryFile", &binaryFile_);

    initFile_.make_string("");
    binaryFile_.make_boolean(false);
    mem_ = NULL;
}

void MemorySim::postinitService() {
    MemoryGeneric::postinitService();

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
            reinterpret_cast<uint32_t *>(mem_)[i] = 0x00000013;  // intialize by NOPs
        }
        RISCV_error("Can't open '%s' file", initFile_.to_string());
        return;
    }

    if (binaryFile_.to_bool()) {
        fseek(fp, 0, SEEK_END);
        uint64_t fsz = ftell(fp);
        if (fsz > length_.to_uint64()) {
            fsz = length_.to_uint64();
        }
        fseek(fp, 0, SEEK_SET);
        fread(mem_, 1, static_cast<size_t>(fsz), fp);
    } else {
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
    }
    fclose(fp);
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

