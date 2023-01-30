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

#include "cmd_memdump.h"
#include <string>

namespace debugger {

CmdMemDump::CmdMemDump(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "memdump", ijtag) {

    briefDescr_.make_string("Dump memory to file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Dump memory to file (default in Binary format).\n"
        "Usage:\n"
        "    memdump <addr> <bytes> [filepath] [bin|hex]\n"
        "Example:\n"
        "    memdump 0x0 8192 dump.bin\n"
        "    memdump 0x40000000 524288 dump.hex hex\n"
        "    memdump 0x10000000 128 \"c:/My Documents/dump.bin\"\n");
}

int CmdMemDump::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 4 || args->size() == 5) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdMemDump::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    const char *filename = (*args)[3].to_string();
    FILE *fd = fopen(filename, "wb");
    if (fd == NULL) {
        char tst[256];
        RISCV_sprintf(tst, sizeof(tst), "Can't open '%s' file", filename);
        generateError(res, tst);
        return;
    }
    uint64_t addr = (*args)[1].to_uint64();
    int len = static_cast<int>((*args)[2].to_uint64());
    res->make_data(len);

    read_memory(addr, len, res->data());
    uint8_t *dumpbuf = res->data();

    if (args->size() == 5 && (*args)[4].is_equal("hex")) {
        char t1[256];
        int t1_cnt = 0;
        int idx;
        for (int i = 0; i < ((len + 0xf) & ~0xf); i++) {
            idx = (i & ~0xf) | (0xf - (i & 0xf));
            if (idx > len) {
                t1[t1_cnt++] = ' ';
                t1[t1_cnt++] = ' ';
                continue;
            }
            t1_cnt += RISCV_sprintf(&t1[t1_cnt], sizeof(t1) - t1_cnt, "%02x",
                                    dumpbuf[idx]);
            if ((i & 0xf) != 0xf) {
                continue;
            }
            t1_cnt += RISCV_sprintf(&t1[t1_cnt], sizeof(t1) - t1_cnt, "\n");
            fwrite(t1, t1_cnt, 1, fd);
            t1_cnt = 0;
        }
    } else {
        fwrite(dumpbuf, len, 1, fd);
    }
    fclose(fd);
}

}  // namespace debugger
