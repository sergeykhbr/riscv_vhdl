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

#include "iservice.h"
#include "cmd_loadsrec.h"
#include <iostream>

namespace debugger {

//#define SHOW_USAGE_INFO

#ifdef SHOW_USAGE_INFO
#define ADDR_SPACE  (1 << 16)
char mark_[ADDR_SPACE] = {0};

void mark_addr(uint64_t addr, int len) {
    for (int i = 0; i < len; i++) {
        if ((addr + i) >= ADDR_SPACE) {
            continue;
        }
        mark_[addr + i] = 1;
    }
}

bool is_flash(unsigned addr) {
    if (addr >= 0x0450 && addr < 0x0500) {
        return true;
    }
    if (addr >= 0x0580 && addr < 0x0600) {
        return true;
    }
    if (addr >= 0x0E00 && addr < 0xfe00) {
        return true;
    }
    return false;
}

void print_flash_usage() {
    unsigned start_addr = 0;
    int cnt = 0;
    int total_cnt = 0;

    RISCV_printf(NULL, 0, "!!! Free Flash regions:", NULL);
    for (unsigned i = 0; i < ADDR_SPACE; i++) {
        if (!is_flash(i)) {
            if (cnt != 0) {
                RISCV_printf(NULL, 0, "    [%04x..%04x], length %d B",
                            start_addr, (start_addr + cnt - 1), cnt);
                total_cnt += cnt;
                cnt = 0;
            }
            continue;
        }
        
        if (mark_[i]) {
            if (cnt != 0) {
                RISCV_printf(NULL, 0, "    [%04x..%04x], length %d B",
                            start_addr, (start_addr + cnt - 1), cnt);
                total_cnt += cnt;
                cnt = 0;
            }
            continue;
        }
        if (cnt == 0) {
            start_addr = i;
        }
        cnt++;
    }

    RISCV_printf(NULL, 0, "    =========================", NULL);
    RISCV_printf(NULL, 0, "    Total: %d B", total_cnt);
}
#endif

CmdLoadSrec::CmdLoadSrec(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "loadsrec", ijtag) {

    briefDescr_.make_string("Load SREC-file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Load SREC-file to SOC target memory.\n"
        "Example:\n"
        "    loadsrec /home/hc08/image.s19\n");
}

int CmdLoadSrec::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() != 2) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdLoadSrec::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    const char *filename = (*args)[1].to_string();
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        char tstr[1024];
        RISCV_sprintf(tstr, sizeof(tstr), "can't open file %s", filename);
        generateError(res, tstr);
        return;
    }
    fseek(fp, 0, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);
    uint8_t *image = new uint8_t[sz];
    fread(image, 1, sz, fp);
    fclose(fp);

    int off = check_header(image);

    IJtag::dmi_dmcontrol_type dmcontrol;
    dmcontrol.u32 = 0;
    dmcontrol.bits.ndmreset = 1;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);

    uint64_t sec_addr;
    int sec_sz;
    uint8_t sec_data[1024];
    while ((off = readline(image, off, sec_addr, sec_sz, sec_data)) != 0) {
        write_memory(sec_addr, sec_sz, sec_data);
#ifdef SHOW_USAGE_INFO
        mark_addr(sec_addr, sec_sz);
#endif
    }

//    soft_reset = 0;
//    tap_->write(addr, 8, reinterpret_cast<uint8_t *>(&soft_reset));
    delete [] image;

#ifdef SHOW_USAGE_INFO
    print_flash_usage();
#endif
}

uint8_t CmdLoadSrec::str2byte(uint8_t *pair) {
    uint8_t ret = 0;
    for (int i = 0; i < 2; i++) {
        ret <<= 4;
        if (pair[i] >= '0' && pair[i] <= '9') {
            ret |= pair[i] - '0';
        } else if (pair[i] >= 'A' && pair[i] <= 'F') {
            ret |= pair[i] - 'A' + 10;
        } else if (pair[i] >= 'a' && pair[i] <= 'f') {
            ret |= pair[i] - 'a' + 10;
        }
    }
    return ret;
}

bool CmdLoadSrec::check_crc(uint8_t *str, int sz) {
    uint8_t sum = 0;
    uint8_t *cur = str;
    for (int i = 0; i < sz; i++) {
        sum += str2byte(cur);
        cur += 2;
    }
    sum = ~sum;
    uint8_t ctrl = str2byte(cur);
    return ctrl == sum;
}

int CmdLoadSrec::check_header(uint8_t *img) {
    int off = 2;
    if (img[0] != 'S' || img[1] != '0') {
        return 0;
    }
    uint8_t sz = str2byte(&img[off]);
    if (!check_crc(&img[off], sz)) {
        return 0;
    }

    off += 2;
    uint16_t addr = str2byte(&img[off]);
    off += 2;
    addr = (addr << 8) + str2byte(&img[off]);
    off += 2;
    if (addr != 0) {
        return 0;
    }
    for (int i = 0; i < sz - 3; i++) {  // size (1) + addr (2) = 3
        header_data_[i] = static_cast<char>(str2byte(&img[off]));
        header_data_[i + 1] = 0;
        off += 2;
    }
    off += 2;  // skip checksum
    if (img[off] != '\r' || img[off + 1] != '\n') {
        return 0;
    }
    return off + 2;
}

int CmdLoadSrec::readline(uint8_t *img, int off,
                          uint64_t &addr, int &sz, uint8_t *out) {
    if (img[off++] != 'S') {
        return 0;
    }
    int bytes4addr = 0;
    switch (img[off++]) {    // 16-bits address only
    case '1':
        bytes4addr = 2; // 16-bits address
        break;
    case '2':
        bytes4addr = 3; // 24-bits address
        break;
    case '3':
        bytes4addr = 4; // 32-bits address
        break;
    default:
        return 0;
    }
    sz = str2byte(&img[off]);
    if (!check_crc(&img[off], sz)) {
        return 0;
    }
    sz -= 1;
    off += 2;

    addr = 0;
    for (int i = 0; i < bytes4addr; i++) {
        addr <<= 8;
        addr += str2byte(&img[off]);
        off += 2;
        sz--;
    }

    for (int i = 0; i < sz; i++) {
        out[i] = static_cast<char>(str2byte(&img[off]));
        off += 2;
    }
    off += 2;  // skip checksum
    if (img[off] != '\r' || img[off + 1] != '\n') {
        return 0;
    }
    return off + 2;
}

}  // namespace debugger
