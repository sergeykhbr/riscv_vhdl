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
