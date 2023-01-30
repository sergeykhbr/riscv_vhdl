/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "cmd_status.h"

namespace debugger {

CmdStatus::CmdStatus(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "status", ijtag) {

    briefDescr_.make_string("Read target's status register throught the DSU registers");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read Harts halt summary register as a uint64_t value.\n"
        "    Status register bits:\n"
        "        [n-1:0]     - Halt bits. 0 = running; 1 = is halted.\n"
        "Example:\n"
        "    status\n");
}

int CmdStatus::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdStatus::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    res->make_uint64(read_dmi(IJtag::DMI_HALTSUM0));
}

}  // namespace debugger
