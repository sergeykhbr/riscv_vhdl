/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Add or remove memory breakpoint.
 */

#include "cmd_br.h"

namespace debugger {

CmdBr::CmdBr(ITap *tap, ISocInfo *info) 
    : ICommand ("br", tap, info) {

    briefDescr_.make_string("Add or remove memory breakpoint.");
    detailedDescr_.make_string(
        "Description:\n"
        "    Add or remove memory breakpoint.\n"
        "Usage:\n"
        "    br add <addr>\n"
        "    br rm <addr>\n"
        "Example:\n"
        "    br add 0x10000000\n"
        "    br rm 0x10000000\n");
}

bool CmdBr::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
        && (args->size() == 3 && (*args)[1].is_string())) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdBr::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    Reg64Type r;
    r.val = (*args)[2].to_uint64();
    if ((*args)[1].is_equal("add")) {
        tap_->write(info_->addressBreakCreate(), 8, r.buf);
    } else if ((*args)[1].is_equal("rm")) {
        tap_->write(info_->addressBreakRemove(), 8, r.buf);
    }
}

}  // namespace debugger
