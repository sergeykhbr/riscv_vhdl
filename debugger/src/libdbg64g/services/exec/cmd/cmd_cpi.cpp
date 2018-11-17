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

#include "cmd_cpi.h"

namespace debugger {

CmdCpi::CmdCpi(ITap *tap, ISocInfo *info) 
    : ICommand ("cpi", tap, info) {

    briefDescr_.make_string("Compute Clocks Per Instruction (CPI) rate");
    detailedDescr_.make_string(
        "Description:\n"
        "    This commands reads CPU registers: 'clock counter' and.\n"
        "    'step counter' and computes Clocks Per Instruction (CPI) rate\n"
        "    in run-time.\n"
        "Output format:\n"
        "    [i,i,i,i,d]\n"
        "         i - Current CPU clock counter value (int64_t).\n"
        "         i - Current CPU step counter value (int64_t).\n"
        "         i - (Current - Previous) CPU clock counter value (int64_t).\n"
        "         i - (Current - Previous) CPU step counter value (int64_t).\n"
        "         d - CPI rate (double) = (delta clocks)/(delta steps).\n"
        "Example:\n"
        "    cpi\n");

    stepCnt_z = 0;
    clockCnt_z = 0;
}

bool CmdCpi::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && args->size() == 1) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdCpi::exec(AttributeType *args, AttributeType *res) {
    res->make_list(5);
    (*res)[0u].make_uint64(0);
    (*res)[1].make_uint64(0);
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    struct CpiRegsType {
        Reg64Type clock_cnt;
        Reg64Type executed_cnt;
    };
    union CpiRegionType {
        CpiRegsType regs;
        uint8_t buf[sizeof(CpiRegsType)];
    } t1;
    DsuMapType *dsu = info_->getpDsu();
    uint64_t addr = reinterpret_cast<uint64_t>(&dsu->udbg.v.clock_cnt);
    uint64_t d1, d2;
    tap_->read(addr, 16, t1.buf);

    d1 = t1.regs.clock_cnt.val - clockCnt_z;
    d2 = t1.regs.executed_cnt.val - stepCnt_z;

    (*res)[0u].make_uint64(t1.regs.clock_cnt.val);
    (*res)[1].make_uint64(t1.regs.executed_cnt.val);
    (*res)[2].make_uint64(d1);
    (*res)[3].make_uint64(d2);
    if (d2 == 0) {
        if (t1.regs.executed_cnt.val == 0) {
            (*res)[4].make_floating(0);
        } else {
            (*res)[4].make_floating(1.0);
        }
    } else {
        (*res)[4].make_floating(
            static_cast<double>(d1) / static_cast<double>(d2));
    }

    clockCnt_z = t1.regs.clock_cnt.val;
    stepCnt_z = t1.regs.executed_cnt.val;
}

}  // namespace debugger
