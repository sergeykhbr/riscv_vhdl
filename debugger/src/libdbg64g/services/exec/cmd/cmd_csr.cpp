/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read/write CSR register value.
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
        "    csr MCPUID\n"
        "    csr 0x762 1\n");
}

bool CmdCsr::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
     && (args->size() == 2 || args->size() == 3)) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

bool CmdCsr::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        return CMD_FAILED;
    }

    uint64_t val;
    uint64_t addr = info_->csr2addr((*args)[1].to_string());
    int bytes = 8;
    if (args->size() == 2) {
        tap_->read(addr, bytes, reinterpret_cast<uint8_t *>(&val));
        res->make_uint64(val);
    } else {
        val = (*args)[2].to_uint64();
        tap_->write(addr, bytes, reinterpret_cast<uint8_t *>(&val));
    }
    return CMD_SUCCESS;
}

bool CmdCsr::format(AttributeType *args, AttributeType *res, AttributeType *out) {
    if (args->size() != 2) {
        out->make_nil();
        return CMD_NO_OUTPUT;
    }

    char tstr[256];
    int tstrsz;
    uint64_t csr = res->to_uint64();
    uint64_t addr = info_->csr2addr((*args)[1].to_string());

    tstr[0] = '\0';
    if (addr == static_cast<uint64_t>(-1)) {
        RISCV_sprintf(tstr, sizeof(tstr), 
            "Unknown CSR '%s'\n", (*args)[1].to_string());
        out->make_string(tstr);
        return CMD_IS_OUTPUT;
    }

    tstrsz = RISCV_sprintf(tstr, sizeof(tstr),
        "CSR[%03x] => %016" RV_PRI64 "x\n",
        static_cast<uint32_t>((addr >> 4) & 0xfff), res->to_uint64());
   
    if (!(*args)[1].is_string()) {
        return CMD_IS_OUTPUT;
    }
    if ((*args)[1].is_equal("MCPUID")) {
        static const char *MCPUID_BASE[4] = {
            "RV32I", "RV32E", "RV64I", "RV128I"
        };
        tstrsz += RISCV_sprintf(&tstr[tstrsz], sizeof(tstr) - tstrsz,
            "    Base: %s", MCPUID_BASE[(csr >> 62) & 0x3]);
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
    return CMD_IS_OUTPUT;
}

}  // namespace debugger
