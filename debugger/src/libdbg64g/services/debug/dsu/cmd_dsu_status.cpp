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

#include "cmd_dsu_status.h"
#include "debug/dsumap.h"

namespace debugger {

CmdDsuStatus::CmdDsuStatus(uint64_t dmibar, ITap *tap)
    : ICommand("status", dmibar, tap) {

    briefDescr_.make_string("Read target's status register throught the DSU registers");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read Harts halt summary register as a uint64_t value.\n"
        "    Status register bits:\n"
        "        [n-1:0]     - Halt bits. 0 = running; 1 = is halted.\n"
        "Example:\n"
        "    status\n");
}

int CmdDsuStatus::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdDsuStatus::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    Reg64Type t1;
    uint64_t addr = DSUREGBASE(ulocal.v.haltsum0);
    if (tap_->read(addr, 8, t1.buf) == TAP_ERROR) {
        return;
    }
    res->make_uint64(t1.val);
}

}  // namespace debugger
