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

    if (binaryFile_.to_bool()) {
        readBinFile(initFile_.to_string(), mem_, length_.to_int());
    } else if (strstr(initFile_.to_string(), ".hex")) {
        readHexFile(initFile_.to_string(), mem_, length_.to_int());
    } else {
        uint8_t *tbuf = new uint8_t[length_.to_int()];
        std::string lo = std::string(initFile_.to_string()) + "_lo.hex";
        std::string hi = std::string(initFile_.to_string()) + "_hi.hex";
        int sz = readHexFile(lo.c_str(), tbuf, length_.to_int());
        readHexFile(hi.c_str(), &tbuf[sz], length_.to_int() - sz);

        // Swap 32-bits words
        uint32_t *lsb = reinterpret_cast<uint32_t *>(tbuf);
        uint32_t *msb = reinterpret_cast<uint32_t *>(&tbuf[sz]);
        uint32_t *dst = reinterpret_cast<uint32_t *>(mem_);
        for (int i = 0; i < sz/sizeof(uint32_t); i++) {
            *dst++ = *lsb++;
            *dst++ = *msb++;
        }
        delete [] tbuf;
    }
}

int MemorySim::readHexFile(const char *filename, uint8_t *buf, int bufsz) {
    int ret = 0;
    int rd_symb;
    int linecnt = 0;
    uint64_t lineval = 0;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        RISCV_error("Can't open '%s' file", filename);
        return ret;
    }

    while (!feof(fp)) {
        rd_symb = fgetc(fp);
        if (chishex(rd_symb)) {
            lineval <<= 4;
            lineval |=  chtohex(rd_symb);
            linecnt++;
            continue;
        }

        if ((ret + linecnt/2) > bufsz) {
            RISCV_error("HEX file tries to write out "
                        "of allocated array\n", NULL);
            break;
        } 

        memcpy(&buf[ret], &lineval, linecnt/2);
        ret += linecnt/2;
        linecnt = 0;
        lineval = 0;
    }
    fclose(fp);
    return ret;
}

int MemorySim::readBinFile(const char *filename, uint8_t *buf, int bufsz) {
    int ret = 0;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        RISCV_error("Can't open '%s' file", filename);
        return ret;
    }
    fseek(fp, 0, SEEK_END);
    uint64_t fsz = ftell(fp);
    if (fsz > bufsz) {
        RISCV_error("File '%s' was trimmed", filename);
        fsz = bufsz;
    }
    fseek(fp, 0, SEEK_SET);
    ret = fread(mem_, 1, static_cast<size_t>(fsz), fp);
    fclose(fp);
    return ret;
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

