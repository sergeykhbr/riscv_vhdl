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

IDmi *DebugRegisterType::getpIDmi() {
    if (!idmi_) {
        idmi_ = static_cast<IDmi *>(parent_->getInterface(IFACE_DMI));
    }
    return idmi_;
}

uint32_t DMDATAx_TYPE::aboutToWrite(uint32_t new_val) {
    value_.val = new_val;
    IDmi *p = getpIDmi();
    if (p && p->isDataAutoexec(idx_)) {
        p->executeCommand();
    }
    return new_val;
}

uint32_t DMCONTROL_TYPE::aboutToWrite(uint32_t new_val) {
    IDmi *p = getpIDmi();
    if (!p) {
        return new_val;
    }
    ValueType tnew;
    ValueType tprv;
    tprv.val = value_.val;
    tnew.val = new_val;
    uint32_t hartsel = (tnew.bits.hartselhi << 10) | tnew.bits.hartsello;
    hartsel &= (p->getCpuMax() - 1);

    tnew.bits.hartsello = hartsel;
    tnew.bits.hartselhi = hartsel >> 10;

    if (tnew.bits.ndmreset != tprv.bits.ndmreset) {
        if (tnew.bits.ndmreset) {
            p->set_ndmreset();
        } else {
            p->clear_ndmreset();
        }
    }
    if (tnew.bits.setresethaltreq) {
        p->set_resethaltreq(hartsel);
    } else if (tnew.bits.clearresethaltreq) {
        p->clear_resethaltreq(hartsel);
    }

    if (tnew.bits.hartreset != tprv.bits.hartreset) {
        if (tnew.bits.hartreset) {
            p->set_hartreset(hartsel);
        } else {
            p->clear_hartreset(hartsel);
        }
    }

    if (tnew.bits.haltreq) {
        if (p->isHalted(hartsel)) {
            p->set_cmderr(CMDERR_WRONGSTATE);
        } else {
            p->set_haltreq(hartsel);
        }
    } else if (tnew.bits.resumereq) {
        if (!p->isHalted(hartsel)) {
            p->set_cmderr(CMDERR_WRONGSTATE);
        } else {
            p->set_resumereq(hartsel);
        }
    }

    return tnew.val;
}

uint32_t DMSTATUS_TYPE::aboutToRead(uint32_t cur_val) {
    IDmi *p = getpIDmi();
    if (!p) {
        return cur_val;
    }
    ValueType t;
    int hartsel = p->getHartSelected();
    bool resumeack = p->get_resumeack(hartsel);
    bool available = p->isAvailable(hartsel);
    bool halted = p->isHalted(hartsel);
    t.val = 0;
    t.bits.allresumeack = resumeack;
    t.bits.anyresumeack = resumeack;
    t.bits.allnonexistent = !available;
    t.bits.anynonexistent = !available;
    t.bits.allunavail = !available;
    t.bits.anyunavail = !available;
    t.bits.allrunning = !halted && available;
    t.bits.anyrunning = !halted && available;
    t.bits.allhalted = halted && available;
    t.bits.anyhalted = halted && available;
    t.bits.authenticated = 1;
    t.bits.hasresethaltreq = 1;
    t.bits.version = 2;
    return t.val;
}

uint32_t ABSTRACTCS_TYPE::aboutToWrite(uint32_t new_val) {
    IDmi *p = getpIDmi();
    if (!p) {
        return new_val;
    }
    ValueType tnew;
    tnew.val = new_val;
    if (tnew.bits.cmderr != 0) {
        tnew.bits.cmderr = 0;
    }
    return tnew.val;
}

uint32_t ABSTRACTCS_TYPE::aboutToRead(uint32_t cur_val) {
    IDmi *p = getpIDmi();
    if (!p) {
        return cur_val;
    }
    ValueType t;
    t.val = cur_val;
    t.bits.datacount = p->getRegTotal();
    t.bits.busy = 0;    // let's suppose command executed immediate in functional model
    t.bits.progbufsize = p->getProgbufTotal();
    return t.val;
}

uint32_t HALTSUM0_TYPE::aboutToRead(uint32_t cur_val) {
    IDmi *p = getpIDmi();
    if (!p) {
        return cur_val;
    }
    uint32_t ret = 0;
    for (int i = 0; i < p->getCpuMax(); i++) {
        if (p->isHalted(i) && p->isAvailable(i)) {
            ret |= 1ul << i;
        }
    }
    return ret;
}

void COMMAND_TYPE::execute() {
    IDmi *p = getpIDmi();
    if (!p) {
        return;
    }
    if (p->get_cmderr() != CMDERR_NONE) {
        return;
    }
    ValueType t;
    t.val = getValue().val;
    switch (t.bits.cmdtype) {
    case 0:     // Register Access
        if (t.bits.transfer) {
            if (t.bits.write) {
                p->writeTransfer(t.bits.regno, t.bits.aarsize);
            } else {
                p->readTransfer(t.bits.regno, t.bits.aarsize);
            }
        } 
        if (t.bits.postexec) {
        }
        if (t.bits.aarpostincrement) {
            t.bits.regno++;
            setValue(t.val);
        }
        break;
    default:;
    }
}

uint32_t COMMAND_TYPE::aboutToWrite(uint32_t nxt_val) {
    setValue(nxt_val);
    execute();
    return nxt_val;
}

}  // namespace debugger

