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

#include "cmd_cpi.h"
#include "coreservices/icpuriscv.h"

namespace debugger {

CmdCpi::CmdCpi(IService *parent, IJtag *ijtag) 
    : ICommandRiscv(parent, "cpi", ijtag) {

    briefDescr_.make_string("Compute Clocks Per Instruction (CPI) rate");
    detailedDescr_.make_string(
        "Description:\n"
        "    This commands reads CSR registers: 'cycle counter' and\n"
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

int CmdCpi::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() != 1) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdCpi::exec(AttributeType *args, AttributeType *res) {
    Reg64Type user_cycle;
    Reg64Type user_insret;
    uint64_t d1, d2;
    uint32_t err;

    res->make_list(5);
    (*res)[0u].make_uint64(0);
    (*res)[1].make_uint64(0);

    err = get_reg("cycle", &user_cycle);
    if (err) {
        generateError(res, "cannot read CSR_cycle register");
        return;
    }
    err = get_reg("insret", &user_insret);
    if (err) {
        generateError(res, "cannot read CSR_insret register");
        return;
    }

    d1 = user_cycle.val - clockCnt_z;
    d2 = user_insret.val - stepCnt_z;

    (*res)[0u].make_uint64(user_cycle.val);
    (*res)[1].make_uint64(user_insret.val);
    (*res)[2].make_uint64(d1);
    (*res)[3].make_uint64(d2);
    if (d2 == 0) {
        if (user_insret.val == 0) {
            (*res)[4].make_floating(0);
        } else {
            (*res)[4].make_floating(1.0);
        }
    } else {
        (*res)[4].make_floating(
            static_cast<double>(d1) / static_cast<double>(d2));
    }

    clockCnt_z = user_cycle.val;
    stepCnt_z = user_insret.val;
}

}  // namespace debugger
