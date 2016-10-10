/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Exit and close application.
 */

#include "cmd_exit.h"

namespace debugger {

CmdExit::CmdExit(ITap *tap, ISocInfo *info) 
    : ICommand ("exit", tap, info) {

    briefDescr_.make_string("Exit and close application");
    detailedDescr_.make_string(
        "Description:\n"
        "    Immediate close the application and exit.\n"
        "Example:\n"
        "    exit\n");
}

bool CmdExit::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string())) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

bool CmdExit::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        return CMD_FAILED;
    }
    RISCV_break_simulation();
    return CMD_SUCCESS;
}

}  // namespace debugger
