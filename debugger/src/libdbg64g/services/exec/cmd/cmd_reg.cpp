/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read/write register value.
 */

#include "cmd_reg.h"

namespace debugger {

CmdReg::CmdReg(ITap *tap, ISocInfo *info) 
    : ICommand ("reg", tap, info) {

    briefDescr_.make_string("Read/write register value");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read or modify the specific CPU's register.\n"
        "Usage:\n"
        "    reg name\n"
        "    reg name wrvalue\n"
        "Example:\n"
        "    reg\n"
        "    reg pc\n"
        "    reg sp 0x10007fc0\n");
}

bool CmdReg::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
     && (args->size() == 2 || args->size() == 3)) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdReg::exec(AttributeType *args, AttributeType *res) {
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }
    res->make_nil();

    uint64_t val;
    const char *reg_name = (*args)[1].to_string();
    uint64_t addr = info_->reg2addr(reg_name);
    if (addr == REG_ADDR_ERROR) {
        char tstr[128];
        RISCV_sprintf(tstr, sizeof(tstr), "%s not found", reg_name);
        generateError(res, tstr);
        return;
    }

    if (args->size() == 2) {
        int err = tap_->read(addr, 8, reinterpret_cast<uint8_t *>(&val));
        if (err == TAP_ERROR) {
            return;
        }
        res->make_uint64(val);
    } else {
        val = (*args)[2].to_uint64();
        tap_->write(addr, 8, reinterpret_cast<uint8_t *>(&val));
    }
}

}  // namespace debugger
