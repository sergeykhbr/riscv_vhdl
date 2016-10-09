/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Log file enable/disable.
 */

#include "cmd_log.h"

namespace debugger {

CmdLog::CmdLog(ITap *tap, ISocInfo *info) 
    : ICommand ("log", tap, info) {

    briefDescr_.make_string("Enable log-file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Write console output into specified file.\n"
        "Example:\n"
        "    log session.log\n"
        "    log /home/riscv/session.log\n");
}

bool CmdLog::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && args->size() == 2) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

bool CmdLog::exec(AttributeType *args, AttributeType *res) {
    res->make_dict();
    if (!isValid(args)) {
        return CMD_FAILED;
    }
    /** Do nothing, log file is enabled in Executor itself */
    return CMD_SUCCESS;
}

bool CmdLog::format(AttributeType *res, AttributeType *out) {
    return CMD_NO_OUTPUT;
}

}  // namespace debugger
