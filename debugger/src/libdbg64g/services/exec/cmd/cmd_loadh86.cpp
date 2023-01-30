/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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
#include "cmd_loadh86.h"
#include <iostream>

namespace debugger {

//char bindata[1 << 24] = {0};
//char flgdata[1 << 24] = {0};

CmdLoadH86::CmdLoadH86(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "loadh86", ijtag) {

    briefDescr_.make_string("Load Intel HEX file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Load H86-file (Intel Hex) to SOC target memory.\n"
        "Arguments: This command supports conversion of h86 to binary file\n"
        "           For this use the following argument list:"
        "    loadh86 [ifile] [osize] [ofile]"
        "Example:\n"
        "    loadh86 /home/c166/image.h86\n"
        "    loadh86 /home/c166/image.h86 34603008 image.bin\n");
    addr_msb_ = 0;
}

int CmdLoadH86::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 2) {
        return CMD_VALID;
    }
    if (args->size() == 4 && (*args)[2].is_integer()) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdLoadH86::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    const char *filename = (*args)[1].to_string();
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        generateError(res, "File not found");
        return;
    }
    fseek(fp, 0, SEEK_END);
    int sz = ftell(fp);
    rewind(fp);
    uint8_t *image = new uint8_t[sz];
    fread(image, 1, sz, fp);
    fclose(fp);

    int off = 0;
    uint64_t sec_addr;
    int sec_sz;
    uint8_t sec_data[1024];
    int code = 0;
    int maxadr = 0;

    uint8_t *binFileBuf = 0;
    unsigned binFileSz = 0;
    if (args->size() == 4) {
        // Writing to binary file
        binFileSz =(*args)[2].to_uint32();
        binFileBuf = new uint8_t[binFileSz];
    }

    IJtag::dmi_dmcontrol_type dmcontrol;
    dmcontrol.u32 = 0;
    dmcontrol.bits.ndmreset = 1;
    if (binFileBuf == 0) {
        write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);
    }

    while (code != -1) {
        code = readline(image, off, sec_addr, sec_sz, sec_data);
        switch (code) {
        case 0:
            if (binFileBuf == 0) {
                write_memory(sec_addr, sec_sz, sec_data);
                //memcpy(&bindata[sec_addr & 0xFFFFFF], sec_data, sec_sz);
                //memset(&flgdata[sec_addr & 0xFFFFFF], 0xff, sec_sz);
            } else if ((sec_addr + sec_sz) <= binFileSz) {
                memcpy(&binFileBuf[sec_addr], sec_data, sec_sz);
            } else {
                generateError(res, "Wrong file size");
                code = -1;
            }
            break;
        case 1:
            // End of file marker. Correct ending
            code = -1;
            break;
        case 4:
            addr_msb_ = (static_cast<int>(sec_data[0]) << 8) | sec_data[1];
            if (addr_msb_ > maxadr) {
                maxadr = addr_msb_;
            }
            //if (addr_msb_ >= 0x00c0 && addr_msb_ < 0x0100) {
                //addr_msb_ -= 0x00c0;
                //bool st = true;
            //}
            break;
        case 5:
            //generateError(res, "EIP not supported");
            break;
        default:
            generateError(res, "Wrong file format");
            code = -1;
        }
    }

    if (binFileBuf) {
        FILE *fw = fopen((*args)[3].to_string(), "wb");
        if (fw) {
            fwrite(binFileBuf, 1, binFileSz, fw);
            fclose(fw);
        }
    }

    delete [] image;
}

uint8_t CmdLoadH86::str2byte(uint8_t *pair) {
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

bool CmdLoadH86::check_crc(uint8_t *str, int sz) {
    uint8_t sum = 0;
    uint8_t *cur = str;
    for (int i = 0; i < sz; i++) {
        sum += str2byte(cur);
        cur += 2;
    }
    sum = ~sum + 1;
    uint8_t ctrl = str2byte(cur);
    return ctrl == sum;
}

int CmdLoadH86::readline(uint8_t *img, int &off,
                         uint64_t &addr, int &sz, uint8_t *out) {
    int retcode = -1;
    if (img[off++] != ':') {
        return retcode;
    }

    sz = str2byte(&img[off]);
    if (!check_crc(&img[off], sz + 4)) {
        return retcode;
    }
    off += 2;

    addr = addr_msb_;
    for (int i = 0; i < 2; i++) {
        addr <<= 8;
        addr += str2byte(&img[off]);
        off += 2;
    }

    retcode = str2byte(&img[off]);
    off += 2;

    for (int i = 0; i < sz; i++) {
        out[i] = static_cast<char>(str2byte(&img[off]));
        off += 2;
    }
    off += 2;  // skip checksum
    if (img[off] == '\r' && img[off + 1] == '\n') {
        off += 2;
    } else if (img[off] == '\n') {
        off += 1;
    } else {
        return -1;
    }
    return retcode;
}

}  // namespace debugger
