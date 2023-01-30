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

#include "cmd_read.h"

namespace debugger {

CmdRead::CmdRead(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "read", ijtag) {

    briefDescr_.make_string("Read memory");
    detailedDescr_.make_string(
        "Description:\n"
        "    32-bits aligned memory reading. Default bytes = 4 bytes.\n"
        "Usage:\n"
        "    read <addr> <bytes>\n"
        "Example:\n"
        "    read 0xfffff004 16\n"
        "    read 0xfffff004\n");

    rdData_.make_data(1024);
}

int CmdRead::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 2 || args->size() == 3) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdRead::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    uint64_t addr = (*args)[1].to_uint64();
    unsigned bytes = 4;
    if (args->size() == 3) {
        bytes = static_cast<unsigned>((*args)[2].to_uint64());
    }
    /** Use 4 x Buffer size to store formatted output */
    if (4 * bytes > rdData_.size()) {
        rdData_.make_data(4 * bytes);
    }

    if (read_memory(addr, bytes, rdData_.data())) {
        generateError(res, "Cannot read memory");
        return;
    }

    res->make_data(bytes, rdData_.data());
}

void CmdRead::to_string(AttributeType *args, AttributeType *res, AttributeType *out) {
    uint64_t addr = (*args)[1].to_uint64();
    int bytes = 4;
    if (args->size() == 3) {
        bytes = static_cast<int>((*args)[2].to_uint64());
    }

    const uint64_t MSK64 = 0x7ull;
    uint64_t addr_start, addr_end, inv_i;
    addr_start = addr & ~MSK64;
    addr_end = (addr + bytes + 7) & ~MSK64;

    int strsz = 0;
    int rdBufSz = static_cast<int>(rdData_.size());
    char *strbuf = reinterpret_cast<char *>(rdData_.data());
    for (uint64_t i = addr_start; i < addr_end; i++) {
        if ((i & MSK64) == 0) {
            // Output address:
            strsz += RISCV_sprintf(&strbuf[strsz], rdBufSz - strsz,
                                "[%016" RV_PRI64 "x]: ", i);
        }
        inv_i = (i & ~MSK64) | (MSK64 - (i & MSK64));
        if ((addr <= inv_i) && (inv_i < (addr + bytes))) {
            strsz += RISCV_sprintf(&strbuf[strsz], rdBufSz - strsz, " %02x",
                                    res->data()[inv_i - addr]);
        } else {
            strsz += RISCV_sprintf(&strbuf[strsz], rdBufSz - strsz, " ..");
        }
        if ((i & MSK64) == MSK64) {
            strsz += RISCV_sprintf(&strbuf[strsz], rdBufSz - strsz, "\n");
        }
    }
    out->make_string(strbuf);
}

}  // namespace debugger
