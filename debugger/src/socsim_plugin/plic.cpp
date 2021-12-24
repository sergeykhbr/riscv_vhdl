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
    registerInterface(static_cast<IIrqController *>(this));
    registerAttribute("ContextList", &contextList_);

    contextList_.make_list(0);
    pendingList_.make_list(0);
    ctx_enable = 0;
    ctx_priority_th = 0;
    ctx_claim = 0;
}

PLIC::~PLIC() {
    if (contextList_.size() && ctx_enable) {
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
    AttributeType tmap;
    char tstr[1024];
    unsigned ctx_total = contextList_.size();
    if (ctx_total) {
        tmap.make_list(2);  // to register Port Interface
        tmap[0u].make_string(getObjName());
        ctx_enable = new PLIC_ENABLE_TYPE* [ctx_total];
        ctx_priority_th = new PLIC_CONTEXT_PRIOIRTY_TYPE* [ctx_total];
        ctx_claim = new PLIC_CLAIM_COMPLETE_TYPE* [ctx_total];

        for (unsigned i = 0; i < contextList_.size(); i++) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s::enable",
                contextList_[i].to_string());
            ctx_enable[i] = new PLIC_ENABLE_TYPE(
                static_cast<IService *>(this), tstr, 0x2000 + 0x80*i, i);
            tmap[1].make_string(tstr);
            listMap_.add_to_list(&tmap);

            RISCV_sprintf(tstr, sizeof(tstr), "%s::priority_th",
                contextList_[i].to_string());
            ctx_priority_th[i] = new PLIC_CONTEXT_PRIOIRTY_TYPE(
                static_cast<IService *>(this), tstr, 0x200000 + 0x1000*i, i);
            tmap[1].make_string(tstr);
            listMap_.add_to_list(&tmap);

            RISCV_sprintf(tstr, sizeof(tstr), "%s::claim",
                contextList_[i].to_string());
            ctx_claim[i] = new PLIC_CLAIM_COMPLETE_TYPE(
                static_cast<IService *>(this), tstr, 0x200004 + 0x1000*i, i);
            tmap[1].make_string(tstr);
            listMap_.add_to_list(&tmap);
        }
    }

    RegMemBankGeneric::postinitService();
}

int PLIC::requestInterrupt(IFace *isrc, int idx) {
    setPendingBit(idx);
    return 0;
}

int PLIC::getPendingRequest(int ctxid) {
    uint32_t prio = 0;
    uint32_t irqidx = 0;

    // Select the highest priority request;
    uint32_t tidx;
    for (unsigned i = 0; i < pendingList_.size(); i++) {
        tidx = pendingList_[i].to_uint32();
        if (!isEnabled(tidx)) {
            continue;
        }
        if (!isUnmasked(ctxid, tidx)) {
            continue;
        }

        if (src_priority.getpR32()[tidx] >= prio) {
            prio = src_priority.getpR32()[i];
            irqidx = pendingList_[0u].to_uint32();
        }
    }

    if (irqidx != IRQ_REQUEST_NONE) {
        // Lower interrupt index has higher priority
        for (unsigned i = 0; i < pendingList_.size(); i++) {
            tidx = pendingList_[i].to_uint32();

            if (src_priority.getpR32()[i] != prio) {
                continue;
            }

            if (!isEnabled(tidx)) {
                continue;
            }
            if (!isUnmasked(ctxid, tidx)) {
                continue;
            }

            // Check only the highest prio interrupts:
            if (pendingList_[i].to_uint32() <  irqidx) {
                irqidx = pendingList_[i].to_uint32();
            }
        }
    }

    return irqidx;
}

bool PLIC::isEnabled(uint32_t irqidx) {
    // Check bits [2:0]
    // A priority value of 0 is
    // reserved to mean "never interrupt" and effectively disables the interrupt.
    // Priority 1 is the lowest active priority, and priority 7 is the highest.
    // Ties between global interrupts of the same priority
    // are broken by the Interrupt ID; interrupts with the lowest ID have the 
    // highest effective priority.
    if (src_priority.getpR32()[irqidx] == 0) {
        return false;
    }
    return true;
}

bool PLIC::isUnmasked(uint32_t ctxid, uint32_t irqidx) {
    uint32_t ie;
    ie = ctx_enable[ctxid]->getpR32()[irqidx / 32];
    ie >>= (irqidx & 0x1f);
    ie &= 0x1;
    if (!ie) {
        return false;
    }
    // Check priority masking for the context.
    // 0 = Enable all non-zero interrupts
    // 7 = Disable all interrupts
    if (src_priority.getpR32()[irqidx] <= 
        ctx_priority_th[ctxid]->getContextPrioiry()) {
        return false;
    }
    return true;
}

void PLIC::setPendingBit(int idx) {
    pending.getpR32()[idx >> 5] |= 1ul << (idx & 0x1f);
    bool add = true;
    for (unsigned i = 0; i < pendingList_.size(); i++) {
        if (pendingList_[i].to_int() == idx) {
            // already in a list
            add = false;
            break;
        }
    }
    if (add) {
        pendingList_.new_list_item().make_int64(idx);
    }
    RISCV_info("request Interrupt %d", idx);
}

void PLIC::clearPendingBit(int idx) {
    if (idx == 0) {
        return;
    }
    pending.getpR32()[idx >> 5] &= ~(1ul << (idx & 0x1f));
    for (unsigned i = 0; i < pendingList_.size(); i++) {
        if (pendingList_[i].to_int() == idx) {
            pendingList_.remove_from_list(i);
            break;
        }
    }
}

void PLIC::enableInterrupt(uint32_t ctxid, int idx) {
    RISCV_debug("Enable irq: context %d, irq=%d", ctxid, idx);
}

void PLIC::disableInterrupt(uint32_t ctxid, int idx) {
    RISCV_debug("Disable irq: context %d, irq=%d", ctxid, idx);
}

uint32_t PLIC::claim(unsigned ctxid) {
    uint32_t irqidx = getPendingRequest(ctxid);
    clearPendingBit(irqidx);
    return irqidx;
}

void PLIC::complete(unsigned ctxid, uint32_t idx) {
    // If the idx wasn't claimed silently ignored. No side effects needed
}

void PLIC::PLIC_ENABLE_TYPE::write(int idx, uint32_t val) {
    PLIC *p = static_cast<PLIC *>(parent_);
    uint32_t prev = getpR32()[idx];
    GenericReg32Bank::write(idx, val);

    uint32_t msk;
    for (int i = 0; i < 32; i++) {
        msk = 1ul << i;
        if ((prev & msk) && !(val & msk)) {
            p->disableInterrupt(contextid_, 32*idx + i);
        } else if (!(prev & msk) && (val & msk)) {
            p->enableInterrupt(contextid_, 32*idx + i);
        }
    }
}

uint32_t PLIC::PLIC_CLAIM_COMPLETE_TYPE::aboutToRead(uint32_t prv_val) {
    PLIC *p = static_cast<PLIC *>(parent_);
    return p->claim(contextid_);
}

uint32_t PLIC::PLIC_CLAIM_COMPLETE_TYPE::aboutToWrite(uint32_t nxt_val) {
    PLIC *p = static_cast<PLIC *>(parent_);
    p->complete(contextid_, nxt_val);
    return nxt_val;
}

}  // namespace debugger

