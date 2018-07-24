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

#include "cmd_status.h"

namespace debugger {

CmdStatus::CmdStatus(ITap *tap, ISocInfo *info) 
    : ICommand ("status", tap, info) {

    briefDescr_.make_string("Read target's status register");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read target's status register as a uint64_t value.\n"
        "    Status register bits:\n"
        "        [0]     - Halt bit. 0 = running; 1 = is halted.\n"
        "        [1]     - Stepping mode enable bit.\n"
        "        [2]     - Breakpoint hit signaling bit.\n"
        "        [19:4]  - Core ID hardwired value.\n"
        "Example:\n"
        "    status\n");
}

bool CmdStatus::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && args->size() == 1) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdStatus::exec(AttributeType *args, AttributeType *res) {
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }
    res->make_nil();

    Reg64Type t1;
    DsuMapType *pdsu = info_->getpDsu();
    uint64_t addr = reinterpret_cast<uint64_t>(&pdsu->udbg.v.control);
    if (tap_->read(addr, 8, t1.buf) == TAP_ERROR) {
        return;
    }
    res->make_uint64(t1.val);

#if 0
    // Instr trace info
    addr = reinterpret_cast<uint64_t>(pdsu->ureg.v.instr_buf);
    AttributeType t2;
    t2.make_data(4*8);
    if (tap_->read(addr, 4*8, t2.data()) == TAP_ERROR) {
        return;
    }
    Reg64Type *instr = reinterpret_cast<Reg64Type *>(t2.data());
    RISCV_printf(0, 0, "    3. [%08x] %08x", instr[3].buf32[1], instr[3].buf32[0]);
    RISCV_printf(0, 0, "    2. [%08x] %08x", instr[2].buf32[1], instr[2].buf32[0]);
    RISCV_printf(0, 0, "    1. [%08x] %08x", instr[1].buf32[1], instr[1].buf32[0]);
    RISCV_printf(0, 0, "    0. [%08x] %08x", instr[0].buf32[1], instr[0].buf32[0]);
#endif
}

}  // namespace debugger
