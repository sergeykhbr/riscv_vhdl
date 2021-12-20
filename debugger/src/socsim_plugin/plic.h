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

#pragma once

#include <iclass.h>
#include <iservice.h>
#include "coreservices/imemop.h"
#include "coreservices/iirq.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

static const int PLIC_GLOBAL_IRQ_MAX = 1024;

class PLIC : public RegMemBankGeneric,
             public IIrqController {
 public:
    explicit PLIC(const char *name);
    virtual ~PLIC();

    /** IService interface */
    virtual void postinitService() override;

    /** IIrqController */
    virtual int requestInterrupt(IFace *isrc, int idx);
    virtual int getPendingRequest(int ctxid);

    /** Controller specific methods visible for ports */
    void enableInterrupt(uint32_t ctxid, int idx);
    void disableInterrupt(uint32_t ctxid, int idx);
    uint32_t claim(uint32_t ctxid);
    void complete(uint32_t ctxid, uint32_t idx);
    void setPendingBit(int idx);
    void clearPendingBit(int idx);

 private:
    bool isEnabled(uint32_t irqidx);
    bool isUnmasked(uint32_t ctxid, uint32_t irqidx);

 private:

    class PLIC_ENABLE_TYPE : public GenericReg32Bank {
     public:
        PLIC_ENABLE_TYPE(IService *parent, const char *name, uint64_t addr, unsigned ctxid)
            : GenericReg32Bank(parent, name, addr, PLIC_GLOBAL_IRQ_MAX/32),
            contextid_(ctxid) {}

        virtual void write(int idx, uint32_t val) override;
     protected:
        unsigned contextid_;
    };

    class PLIC_SRC_PRIORITY_TYPE : public GenericReg32Bank {
     public:
        PLIC_SRC_PRIORITY_TYPE(IService *parent, const char *name, uint64_t addr, int len)
            : GenericReg32Bank(parent, name, addr, len) {}

        virtual void write(int idx, uint32_t val) override {
            GenericReg32Bank::write(idx, val & 0x7);
        }
    };

    class PLIC_CONTEXT_PRIOIRTY_TYPE : public MappedReg32Type {
     public:
        PLIC_CONTEXT_PRIOIRTY_TYPE(IService *parent, const char *name,
            uint64_t addr, unsigned ctxid)
            : MappedReg32Type(parent, name, addr), contextid_(ctxid) {
        }

        uint32_t getContextPrioiry() { return getValue().val & 0x7; }
     protected:
        unsigned contextid_;
    };

    class PLIC_CLAIM_COMPLETE_TYPE : public MappedReg32Type {
     public:
        PLIC_CLAIM_COMPLETE_TYPE(IService *parent, const char *name,
            uint64_t addr, unsigned ctxid)
            : MappedReg32Type(parent, name, addr), contextid_(ctxid) {
        }

     protected:
        virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
        virtual uint32_t aboutToRead(uint32_t prv_val) override;
     protected:
        unsigned contextid_;
    };

    AttributeType contextList_;     // List of context names: [MCore0, MCore1, SCore1, MCore2, ...]
    AttributeType pendingList_;     // requested interrupt packed into attribute for better performance

    PLIC_SRC_PRIORITY_TYPE src_priority;            // [000000..000FFC] 0 doens't exists, 1..1023
    GenericReg32Bank pending;                       // [001000..00107C] 0..1023 1 bit per interrupt
    PLIC_ENABLE_TYPE **ctx_enable;                  // [002000 + 0x80*n] 0..1023 1 bit per interrupt for context N
    PLIC_CONTEXT_PRIOIRTY_TYPE **ctx_priority_th;   // [200000 + 0x1000*N] priority threshold for context N
    PLIC_CLAIM_COMPLETE_TYPE **ctx_claim;           // [200004 + 0x1000*N] claim/complete for context N
};

DECLARE_CLASS(PLIC)

}  // namespace debugger
