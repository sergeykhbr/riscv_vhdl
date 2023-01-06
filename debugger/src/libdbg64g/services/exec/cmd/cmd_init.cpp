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

#include "cmd_init.h"

namespace debugger {

CmdInit::CmdInit(IJtag *itap) 
    : ICommand ("init", 0, 0),
    jtag_(itap) {

    briefDescr_.make_string("Read platform configuration and capabilities");
    detailedDescr_.make_string(
        "Description:\n"
        "    See RISC-V Debug specification to clarify OpenOCD initialization sequence\n"
        "Output format:\n"
        "Example:\n"
        "    init\n");

}

int CmdInit::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() != 1) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdInit::exec(AttributeType *args, AttributeType *res) {
    jtag_->resetAsync();
    jtag_->resetSync();
    jtag_->scanIdCode();
}

}  // namespace debugger
