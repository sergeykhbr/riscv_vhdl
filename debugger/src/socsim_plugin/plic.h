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
#include "coreservices/iclock.h"
#include "coreservices/imemop.h"
#include "coreservices/icpugen.h"
#include "coreservices/iirq.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {


class PLIC : public RegMemBankGeneric,
             public IClockListener,
             public IIrqController {
 public:
    explicit PLIC(const char *name);
    ~PLIC();

    /** IService interface */
    virtual void postinitService() override;

    /** IClockListener interface */
    virtual void stepCallback(uint64_t t);

    /** IIrqController */
    virtual int requestInterrupt(IFace *isrc, int idx);

    /** Controller specific methods visible for ports */
    uint32_t claim();
    void complete(uint32_t idx);
    void setPendingBit(int idx);
    void clearPendingBit(int idx);
 private:
    uint32_t getNextInterrupt() { return 0; }

 private:

    class PLIC_CLAIM_COMPLETE_TYPE : public MappedReg32Type {
     public:
        PLIC_CLAIM_COMPLETE_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
        }

     protected:
        virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
        virtual uint32_t aboutToRead(uint32_t prv_val) override;
    };

    static const int NOT_PROCESSING = 0;

    AttributeType clock_;
    AttributeType contextList_;     // List of context names: [MCore0, MCore1, SCore1, MCore2, ...]

    IClock *iclk_;

    GenericReg32Bank src_priority;          // [000000..000FFC] doens't exists, 1..1023
    GenericReg32Bank pending;               // [001000..00107C] 0..1023 1 bit per interrupt
    GenericReg32Bank **ctx_enable;          // [002000 + 0x80*n] 0..1023 1 bit per interrupt for context N
    MappedReg32Type **ctx_priority_th;      // [200000 + 0x1000*N] priority threshold for context N
    PLIC_CLAIM_COMPLETE_TYPE **ctx_claim;   // [200004 + 0x1000*N] claim/complete for context N

    int processing_;
};

DECLARE_CLASS(PLIC)

}  // namespace debugger
