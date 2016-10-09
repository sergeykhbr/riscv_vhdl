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

bool CmdReg::exec(AttributeType *args, AttributeType *res) {
    res->make_uint64(~0);
    if (!isValid(args)) {
        return CMD_FAILED;
    }

    uint64_t val;
    uint64_t addr = info_->reg2addr((*args)[1].to_string());
    if (args->size() == 2) {
        tap_->read(addr, 8, reinterpret_cast<uint8_t *>(&val));
        res->make_uint64(val);
    } else {
        val = (*args)[2].to_uint64();
        tap_->write(addr, 8, reinterpret_cast<uint8_t *>(&val));
    }
    return CMD_SUCCESS;
}

}  // namespace debugger
