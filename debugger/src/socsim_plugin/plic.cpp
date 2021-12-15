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
#include "plic.h"
#include <riscv-isa.h>
#include "coreservices/icpuriscv.h"

namespace debugger {

PLIC::PLIC(const char *name) : RegMemBankGeneric(name),
    src_priority(static_cast<IService *>(this), "src_priority", 0x00, 1024),
    pending(static_cast<IService *>(this), "pending", 0x001000, 1024) {
    registerInterface(static_cast<IClockListener *>(this));
    registerInterface(static_cast<IIrqController *>(this));
    registerAttribute("Clock", &clock_);
    registerAttribute("ContextList", &contextList_);

    processing_ = NOT_PROCESSING;
    contextList_.make_list(0);
}

PLIC::~PLIC() {
    if (contextList_.size()) {
        for (unsigned i = 0; i < contextList_.size(); i++) {
            delete ctx_enable[i];
            delete ctx_priority_th[i];
            delete ctx_claim[i];
        }
        delete [] ctx_enable;
        delete [] ctx_priority_th;
        delete [] ctx_claim;
    }
}

void PLIC::postinitService() {
    char tstr[1024];
    unsigned ctx_total = contextList_.size();
    if (ctx_total) {
        ctx_enable = new GenericReg32Bank* [ctx_total];
        ctx_priority_th = new MappedReg32Type* [ctx_total];
        ctx_claim = new PLIC_CLAIM_COMPLETE_TYPE* [ctx_total];

        for (unsigned i = 0; i < contextList_.to_uint32(); i++) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s::enable",
                contextList_[i].to_string());
            ctx_enable[i] = new GenericReg32Bank(
                static_cast<IService *>(this), tstr, 0x2000 + 0x80*i, 1024/32);

            RISCV_sprintf(tstr, sizeof(tstr), "%s::priority_th",
                contextList_[i].to_string());
            ctx_priority_th[i] = new MappedReg32Type(
                static_cast<IService *>(this), tstr, 0x200000 + 0x1000*i);

            RISCV_sprintf(tstr, sizeof(tstr), "%s::claim",
                contextList_[i].to_string());
            ctx_claim[i] = new PLIC_CLAIM_COMPLETE_TYPE(
                static_cast<IService *>(this), tstr, 0x200004 + 0x1000*i);
        }
    }

    RegMemBankGeneric::postinitService();

    iclk_ = static_cast<IClock *>(
        RISCV_get_service_iface(clock_.to_string(), IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Can't find IClock interface %s", clock_.to_string());
        return;
    }
}

int PLIC::requestInterrupt(IFace *isrc, int idx) {
    
    return 0;
}

void PLIC::stepCallback(uint64_t t) {
}

void PLIC::setPendingBit(int idx) {
    pending.getpR32()[idx >> 5] |= 1ul << (idx & 0x1f);
    RISCV_info("request Interrupt %d", idx);
}

void PLIC::clearPendingBit(int idx) {
    pending.getpR32()[idx >> 5] &= ~(1ul << (idx & 0x1f));
}

uint32_t PLIC::claim() {
    processing_ = getNextInterrupt();
    return processing_;
}

void PLIC::complete(uint32_t idx) {
    if (idx == processing_) {
        processing_ = NOT_PROCESSING;
    } else {
        RISCV_error("Incorrect complete value %d != %d",    
                    processing_, idx);
    }
}

uint32_t PLIC::PLIC_CLAIM_COMPLETE_TYPE::aboutToRead(uint32_t prv_val) {
    PLIC *p = static_cast<PLIC *>(parent_);
    return p->claim();
}

uint32_t PLIC::PLIC_CLAIM_COMPLETE_TYPE::aboutToWrite(uint32_t nxt_val) {
    PLIC *p = static_cast<PLIC *>(parent_);
    p->complete(nxt_val);
    return nxt_val;
}

}  // namespace debugger

