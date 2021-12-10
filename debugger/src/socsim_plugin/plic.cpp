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
    pending(static_cast<IService *>(this), "pending", 0x001000, 1024),
    enable_ctx0(static_cast<IService *>(this), "enable_ctx0", 0x002000, 1024/32),
    enable_ctx1(static_cast<IService *>(this), "enable_ctx1", 0x002080, 1024/32),
    enable_ctx2(static_cast<IService *>(this), "enable_ctx2", 0x002100, 1024/32),
    enable_ctx3(static_cast<IService *>(this), "enable_ctx3", 0x002180, 1024/32),
    priority_th0(static_cast<IService *>(this), "priority_th0", 0x200000),
    claim_compl0(static_cast<IService *>(this), "claim_compl0", 0x200004),
    priority_th1(static_cast<IService *>(this), "priority_th1", 0x201000),
    claim_compl1(static_cast<IService *>(this), "claim_compl1", 0x201004),
    priority_th2(static_cast<IService *>(this), "priority_th2", 0x202000),
    claim_compl2(static_cast<IService *>(this), "claim_compl2", 0x202004),
    priority_th3(static_cast<IService *>(this), "priority_th3", 0x203000),
    claim_compl3(static_cast<IService *>(this), "claim_compl3", 0x203004) {
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("Clock", &clock_);

    processing_ = NOT_PROCESSING;
}

PLIC::~PLIC() {
}

void PLIC::postinitService() {
    RegMemBankGeneric::postinitService();

    iclk_ = static_cast<IClock *>(
        RISCV_get_service_iface(clock_.to_string(), IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Can't find IClock interface %s", clock_.to_string());
        return;
    }
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

