/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <ihap.h>
#include "cmd_cpucontext.h"
#include "debug/dsumap.h"
#include "debug/dmi_regs.h"

namespace debugger {

CmdCpuContext::CmdCpuContext(uint64_t dmibar, ITap *tap) :
    ICommand ("cpucontext", dmibar, tap) {
    briefDescr_.make_string("Switch CPU context in multicore configuration.");
    detailedDescr_.make_string(
        "Description:\n"
        "    This command switches the default debug interface used by DSU\n"
        "    module on access to the CPU regions.\n"
        "Response:\n"
        "    integer: Current CPU context index\n"
        "Usage:\n"
        "    cpucontext 0\n"
        "    cpucontext 1");
}

int CmdCpuContext::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || (args->size() == 2 && (*args)[1].is_integer())) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdCpuContext::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();

    uint64_t addr = DSUREGBASE(ulocal.v.dmcontrol);
    DMCONTROL_TYPE::ValueType dmcontrol;
    uint64_t hartsel;

    if (args->size() == 1) {
        tap_->read(addr, 8, dmcontrol.u8);
        hartsel = dmcontrol.bits.hartselhi;
        hartsel = (hartsel << 10) | dmcontrol.bits.hartsello;
        res->make_uint64(hartsel);
        return;
    }
    hartsel = (*args)[1].to_uint64();
    
    dmcontrol.val =0;
    dmcontrol.bits.hartsello = hartsel;
    dmcontrol.bits.hartselhi = hartsel >> 10;
    tap_->write(addr, 8, dmcontrol.u8);

    RISCV_trigger_hap(HAP_CpuContextChanged, hartsel, "CPU context changed");
}

}  // namespace debugger
