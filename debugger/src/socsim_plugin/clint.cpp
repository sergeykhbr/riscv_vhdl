/**
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "api_core.h"
#include "clint.h"
#include <riscv-isa.h>
#include "coreservices/icpuriscv.h"

namespace debugger {

CLINT::CLINT(const char *name) : RegMemBankGeneric(name),
    msip(static_cast<IService *>(this), "msip", 0x00),
    mtimecmp(static_cast<IService *>(this), "mtimecmp", 0x004000),
    mtime(static_cast<IService *>(this), "mtime", 0x00bff8) {
    registerInterface(static_cast<IIrqController *>(this));
    registerAttribute("Clock", &clock_);
    update_time_ = 0;
}

void CLINT::postinitService() {
    RegMemBankGeneric::postinitService();

    iclk_ = static_cast<IClock *>(
            RISCV_get_service_iface(clock_.to_string(), 
                                    IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Can't get IClock interface %s",
                    clock_.to_string());
    }
}

void CLINT::setTimer(uint64_t v) {
    update_time_ = iclk_->getStepCounter();
    mtime.setValue(v);
}

void CLINT::updateTimer() {
    uint64_t cur_time = iclk_->getStepCounter();
    uint64_t dt = cur_time - update_time_;
    uint64_t t = mtime.getValue().val;

    update_time_ = cur_time;
    mtime.setValue(t + dt);
}

int CLINT::getPendingRequest(int ctxid) {
    int ret = 0;
    // Context id:
    //   hart0_sw = 0
    //   hart0_tmr = 1
    //   hart1_sw = 2
    //   hart1_tmr = 3
    //   ..
    uint32_t hartid = ctxid / 2;
    uint32_t sw = (~ctxid) & 0x1;
    uint32_t tmr = ctxid & 0x1;

    if (sw) {
        ret = msip.getp()[hartid].bits.b0;
    } else if (tmr) {
        updateTimer();
        if (mtime.getValue().val >= mtimecmp.getp()[hartid].val) {
            ret = 1;
        }
    }

    return ret;
}

uint64_t CLINT::CLINT_MTIME_TYPE::aboutToRead(uint64_t cur_val) {
    CLINT *p = static_cast<CLINT *>(parent_);
    p->updateTimer();
    cur_val = getValue().val;
    return cur_val;
}

uint64_t CLINT::CLINT_MTIME_TYPE::aboutToWrite(uint64_t new_val) {
    CLINT *p = static_cast<CLINT *>(parent_);
    p->setTimer(new_val);
    return new_val;
}

}  // namespace debugger

