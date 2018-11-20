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

#include "cmd_run.h"

namespace debugger {

CmdRun::CmdRun(ITap *tap, ISocInfo *info) 
    : ICommand ("run", tap, info) {

    briefDescr_.make_string("Run simulation for a specify number of steps\"");
    detailedDescr_.make_string(
        "Description:\n"
        "    Run simulation for a specified number of steps.\n"
        "Usage:\n"
        "    run <N steps>\n"
        "Example:\n"
        "    run\n"
        "    go 1000\n"
        "    c 1\n");
}


int CmdRun::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (!cmdName_.is_equal(name.to_string())
        && !name.is_equal("c")
        && !name.is_equal("go")) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || args->size() == 2) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdRun::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    DsuMapType *dsu = info_->getpDsu();
    Reg64Type runctrl;
    uint64_t addr_run_ctrl = reinterpret_cast<uint64_t>(&dsu->udbg.v.control);
    uint64_t addr_step_cnt = 
        reinterpret_cast<uint64_t>(&dsu->udbg.v.stepping_mode_steps);

    if (args->size() == 1) {
        runctrl.val = 0;
        tap_->write(addr_run_ctrl, 8, runctrl.buf);
    } else if (args->size() == 2) {
        runctrl.val = (*args)[1].to_uint64();
        tap_->write(addr_step_cnt, 8, runctrl.buf);

        GenericCpuControlType ctrl;
        ctrl.val = 0;
        ctrl.bits.stepping = 1;
        runctrl.val = ctrl.val;
        tap_->write(addr_run_ctrl, 8, runctrl.buf);
    }
}

}  // namespace debugger
