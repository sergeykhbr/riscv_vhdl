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

#include "cmd_csr.h"

namespace debugger {

CmdCsr::CmdCsr(ITap *tap, ISocInfo *info) 
    : ICommand ("csr", tap, info) {

    briefDescr_.make_string("Access to CSR registers");
    detailedDescr_.make_string(
        "Description:\n"
        "    Access to CSRs registers of the CPU.\n"
        "Usage:\n"
        "    READ:  csr <addr|name>\n"
        "    WRITE: csr <addr|name> <value>\n"
        "Example:\n"
        "    csr MISA\n"
        "    csr 0xf10 1\n");
}

bool CmdCsr::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
     && (args->size() == 2 || args->size() == 3)) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdCsr::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    AttributeType &arg1 = (*args)[1];
    uint64_t val;
    int bytes = 8;
    uint64_t addr;
    if (arg1.is_string()) {
        const char *csrname = arg1.to_string();
        addr = 0;//!!!info_->csr2addr(csrname);
        if (addr == REG_ADDR_ERROR) {
            char tstr[128];
            RISCV_sprintf(tstr, sizeof(tstr), "%s not found", csrname);
            generateError(res, tstr);
            return;
        }
    } else {
        addr = arg1.to_uint64();
    }

    if (args->size() == 2) {
        tap_->read(addr, bytes, reinterpret_cast<uint8_t *>(&val));
        res->make_uint64(val);
    } else {
        val = (*args)[2].to_uint64();
        tap_->write(addr, bytes, reinterpret_cast<uint8_t *>(&val));
    }
}

void CmdCsr::to_string(AttributeType *args, AttributeType *res, AttributeType *out) {
    if (args->size() != 2) {
        out->make_nil();
        return;
    }

    char tstr[256];
    int tstrsz;
    uint64_t csr = res->to_uint64();
    uint64_t addr = 0;//info_->csr2addr((*args)[1].to_string());

    tstr[0] = '\0';
    if (addr == static_cast<uint64_t>(-1)) {
        RISCV_sprintf(tstr, sizeof(tstr), 
            "Unknown CSR '%s'\n", (*args)[1].to_string());
        out->make_string(tstr);
        return;
    }

    tstrsz = RISCV_sprintf(tstr, sizeof(tstr),
        "CSR[%03x] => %016" RV_PRI64 "x\n",
        static_cast<uint32_t>((addr >> 4) & 0xfff), res->to_uint64());
   
    if (!(*args)[1].is_string()) {
        return;
    }
    if ((*args)[1].is_equal("MISA")) {
        static const char *MISA_BASE[4] = {
            "RV32I", "RV32E", "RV64I", "RV128I"
        };
        tstrsz += RISCV_sprintf(&tstr[tstrsz], sizeof(tstr) - tstrsz,
            "    Base: %s", MISA_BASE[(csr >> 62) & 0x3]);
        // total 26 extensions
        char extenstion[2] = {0};
        for (int i = 0; i < 26; i++) {
            if (csr & (1LL << i)) {
                extenstion[0] = 'A' + i;
                if (extenstion[0] == 'I') {
                    continue;
                }
                tstrsz += RISCV_sprintf(&tstr[tstrsz], sizeof(tstr) - tstrsz,
                                        "%s", extenstion);
            }
        }
        tstrsz += RISCV_sprintf(&tstr[tstrsz], sizeof(tstr) - tstrsz, "\n");
    } else if ((*args)[1].is_equal("MTIME")) {
        tstrsz += RISCV_sprintf(&tstr[tstrsz], sizeof(tstr) - tstrsz,
            "    Clock: %" RV_PRI64 "d; ms: %.1f\n",
            csr, static_cast<double>(csr)/60000.0);
    }
    out->make_string(tstr);
    return;
}

}  // namespace debugger
