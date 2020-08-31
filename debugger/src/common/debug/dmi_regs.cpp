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

#include <api_core.h>
#include <generic-isa.h>
#include "dmi_regs.h"
#include "dsu.h"

namespace debugger {

uint64_t DMCONTROL_TYPE::aboutToWrite(uint64_t new_val) {
    DSU *p = static_cast<DSU *>(parent_);
    ValueType tnew;
    ValueType tprv;
    CrGenericRuncontrolType runcotnrol;
    tprv.val = value_.val;
    tnew.val = new_val;
    uint64_t hartid = (tnew.bits.hartselhi << 10) | tnew.bits.hartsello;

    if (tnew.bits.ndmreset != tprv.bits.ndmreset) {
        p->softReset(tnew.bits.ndmreset ? true: false);
    }
    runcotnrol.val = 0;
    if (tnew.bits.haltreq) {
        runcotnrol.bits.req_halt = 1;
        p->nb_debug_write(static_cast<uint32_t>(hartid),
                         CSR_runcontrol,
                         runcotnrol.val);
    } else if (tnew.bits.resumereq) {
        runcotnrol.bits.req_resume = 1;
        p->nb_debug_write(static_cast<uint32_t>(hartid),
                         CSR_runcontrol,
                         runcotnrol.val);
    }
    return new_val;
}

uint64_t DMCONTROL_TYPE::aboutToRead(uint64_t cur_val) {
    DSU *p = static_cast<DSU *>(parent_);
    ValueType t;
    t.val = cur_val;
    t.bits.hartsello = p->getCpuContext();
    t.bits.dmactive = 1;
    return t.val;
}

uint64_t DMSTATUS_TYPE::aboutToRead(uint64_t cur_val) {
    DSU *p = static_cast<DSU *>(parent_);
    ValueType t;
    bool halted = p->isCpuHalted(p->getCpuContext());
    t.val = 0;
    t.bits.allhalted = halted;
    t.bits.anyhalted = halted;
    t.bits.allrunning = !halted;
    t.bits.anyrunning = !halted;
    t.bits.authenticated = 1;
    t.bits.version = 2;
    return t.val;
}

uint64_t HALTSUM_TYPE::aboutToRead(uint64_t cur_val) {
    DSU *p = static_cast<DSU *>(parent_);
    uint64_t ret = 0;
    for (unsigned i = 0; i < p->getCpuTotal(); i++) {
        if (p->isCpuHalted(i)) {
            ret |= 1ull << i;
        }
    }
    return ret;
}

}  // namespace debugger

