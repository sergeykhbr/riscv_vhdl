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

#include "cmd_isrunning.h"

namespace debugger {

CmdIsRunning::CmdIsRunning(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "isrunning", ijtag) {

    briefDescr_.make_string("Check target's status");
    detailedDescr_.make_string(
        "Description:\n"
        "    Check target's status as a boolean value.\n"
        "Example:\n"
        "    isrunning\n");
}

int CmdIsRunning::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() != 1) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdIsRunning::exec(AttributeType *args, AttributeType *res) {
    IJtag::dmi_dmstatus_type dmstatus;
    res->make_boolean(false);

    dmstatus.u32 = read_dmi(IJtag::DMI_DMSTATUS);

    if (dmstatus.bits.allhalted) {
        res->make_boolean(false);
    } else {
        res->make_boolean(true);
    }
}

}  // namespace debugger
