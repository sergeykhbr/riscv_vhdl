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
#include "dmi_regs.h"

namespace debugger {

IDebug *DebugRegisterType::getpIDebug() {
    if (!idbg_) {
        idbg_ = static_cast<IDebug *>(parent_->getInterface(IFACE_DEBUG));
    }
    return idbg_;
}

uint64_t DMCONTROL_TYPE::aboutToWrite(uint64_t new_val) {
    IDebug *p = getpIDebug();
    if (!p) {
        return new_val;
    }
    ValueType tnew;
    ValueType tprv;
    DMCONTROL_TYPE::ValueType runcotnrol;
    tprv.val = value_.val;
    tnew.val = new_val;
    int hartid = static_cast<int>(
        (tnew.bits.hartselhi << 10) | tnew.bits.hartsello);

    if (tnew.bits.ndmreset != tprv.bits.ndmreset) {
        p->setResetPin(tnew.bits.ndmreset);
    }
    runcotnrol.val = 0;
    if (tnew.bits.haltreq) {
        runcotnrol.bits.haltreq = 1;
        p->reqHalt(hartid);
        //p->nb_debug_write(static_cast<uint32_t>(hartid),
        //                 CSR_runcontrol,
        //                 runcotnrol.val);
    } else if (tnew.bits.resumereq) {
        p->reqResume(hartid);
        //runcotnrol.bits.resumereq = 1;
        //p->nb_debug_write(static_cast<uint32_t>(hartid),
        //                 CSR_runcontrol,
        //                 runcotnrol.val);
    }
    return new_val;
}

uint64_t DMCONTROL_TYPE::aboutToRead(uint64_t cur_val) {
    IDebug *p = getpIDebug();
    if (!p) {
        return cur_val;
    }
    ValueType t;
    t.val = cur_val;
    t.bits.hartsello = p->getHartSelected();
    t.bits.dmactive = 1;
    return t.val;
}

uint64_t DMSTATUS_TYPE::aboutToRead(uint64_t cur_val) {
    IDebug *p = getpIDebug();
    if (!p) {
        return cur_val;
    }
    ValueType t;
    int hartsel = p->getHartSelected();
    bool halted = p->isHalted(hartsel);
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
    IDebug *p = getpIDebug();
    if (!p) {
        return cur_val;
    }
    uint64_t ret = 0;
    for (int i = 0; i < p->hartTotal(); i++) {
        if (p->isHalted(i)) {
            ret |= 1ull << i;
        }
    }
    return ret;
}

}  // namespace debugger

