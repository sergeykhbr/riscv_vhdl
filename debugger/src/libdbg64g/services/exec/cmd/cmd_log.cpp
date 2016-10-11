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
        "    Close log-file if the filename not specified.\n"
        "Example:\n"
        "    log session.log\n"
        "    log /home/riscv/session.log\n");
}

bool CmdLog::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) 
        && (args->size() == 1 || args->size() == 2)) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdLog::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }
    
    if (args->size() == 1) {
        RISCV_disable_log();
    } else {
        const char *filename = (*args)[1].to_string();
        if (RISCV_enable_log(filename)) {
            generateError(res, "Can't open file");
        }
    }
}

}  // namespace debugger
