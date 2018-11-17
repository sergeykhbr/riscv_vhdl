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

#include "cmd_halt.h"

namespace debugger {

CmdHalt::CmdHalt(ITap *tap, ISocInfo *info) 
    : ICommand ("halt", tap, info) {

    briefDescr_.make_string("Stop simulation");
    detailedDescr_.make_string(
        "Description:\n"
        "    Stop simulation.\n"
        "Example:\n"
        "    halt\n"
        "    stop\n"
        "    s\n");
}


bool CmdHalt::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (name.is_equal("halt") || name.is_equal("break") 
        || name.is_equal("stop") || name.is_equal("s")) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdHalt::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    Reg64Type t1;
    DsuMapType *dsu = info_->getpDsu();
    GenericCpuControlType ctrl;
    uint64_t addr_run_ctrl = reinterpret_cast<uint64_t>(&dsu->udbg.v.control);
    ctrl.val = 0;
    ctrl.bits.halt = 1;
    t1.val = ctrl.val;
    tap_->write(addr_run_ctrl, 8, t1.buf);
}

}  // namespace debugger
