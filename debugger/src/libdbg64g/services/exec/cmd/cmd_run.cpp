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
#include "debug/dsumap.h"
#include <generic-isa.h>
#include <ihap.h>

namespace debugger {

CmdRun::CmdRun(ITap *tap) : ICommand ("run", tap) {

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
    CrGenericRuncontrolType runctrl;
    uint64_t addr_runcontrol = DSUREGBASE(csr[CSR_runcontrol]);

    RISCV_trigger_hap(HAP_Resume, 0, "Resume command received");

    if (args->size() == 1) {
        runctrl.val = 0;
        runctrl.bits.req_resume = 1;
        tap_->write(addr_runcontrol, 8, runctrl.u8);
    } else if (args->size() == 2) {
        uint64_t addr_dcsr = DSUREGBASE(csr[CSR_dcsr]);
        uint64_t addr_step_cnt = DSUREGBASE(csr[CSR_insperstep]);
        Reg64Type t1;
        t1.val = (*args)[1].to_uint64();
        tap_->write(addr_step_cnt, 8, t1.buf);
        t1.val = 0;
        t1.bits.b2 = 1;     // step
        tap_->write(addr_dcsr, 8, t1.buf);
    }
}

}  // namespace debugger
