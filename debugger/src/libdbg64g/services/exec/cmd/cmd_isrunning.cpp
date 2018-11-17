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

#include "cmd_isrunning.h"

namespace debugger {

CmdIsRunning::CmdIsRunning(ITap *tap, ISocInfo *info) 
    : ICommand ("isrunning", tap, info) {

    briefDescr_.make_string("Check target's status");
    detailedDescr_.make_string(
        "Description:\n"
        "    Check target's status as a boolean value.\n"
        "Example:\n"
        "    isrunning\n");
}

bool CmdIsRunning::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && args->size() == 1) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdIsRunning::exec(AttributeType *args, AttributeType *res) {
    res->make_boolean(false);
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    Reg64Type t1;
    GenericCpuControlType ctrl;
    DsuMapType *pdsu = info_->getpDsu();
    uint64_t addr = reinterpret_cast<uint64_t>(&pdsu->udbg.v.control);
    tap_->read(addr, 8, t1.buf);
    ctrl.val = t1.val;
    if (ctrl.bits.halt) {
        res->make_boolean(false);
    } else {
        res->make_boolean(true);
    }
}

}  // namespace debugger
