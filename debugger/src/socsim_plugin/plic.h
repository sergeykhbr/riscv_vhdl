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
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {


class PLIC : public RegMemBankGeneric,
             public IClockListener {
 public:
    explicit PLIC(const char *name);
    ~PLIC();

    /** IService interface */
    virtual void postinitService();

    /** IClockListener interface */
    virtual void stepCallback(uint64_t t);

    /** Controller specific methods visible for ports */
    void setPendingBit(int idx);
    void clearPendingBit(int idx);

 private:
    class IRQ_MASK_TYPE : public MappedReg32Type {
     public:
        IRQ_MASK_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x1e;
            value_.val = hard_reset_value_;
        }
    };

    class IRQ_PENDING_TYPE : public MappedReg32Type {
     public:
        IRQ_PENDING_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
        }

     protected:
        virtual uint32_t aboutToWrite(uint32_t nxt_val) override {
            return getValue().val;      // read only
        }
    };

    class IRQ_CLEAR_TYPE : public MappedReg32Type {
     public:
        IRQ_CLEAR_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
        }

     protected:
        virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
    };

    class IRQ_LOCK_TYPE : public MappedReg32Type {
     public:
        IRQ_LOCK_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x1;
            value_.val = hard_reset_value_;
        }
    };

    AttributeType irqTotal_;
    AttributeType cpu_;
    ICpuGeneric *icpu_;
    IClock *iclk_;
    static const int IRQ_MAX = 32;

    IRQ_MASK_TYPE irq_mask;         // 0x00: [RW] 1=disable; 0=enable
    IRQ_PENDING_TYPE irq_pending;   // 0x04: [RO]
    IRQ_CLEAR_TYPE irq_clear;       // 0x08: [WO]
    MappedReg32Type irq_raise;      // 0x0c: [WO]
    MappedReg32Type isr_table_l;    // 0x10: [RW]
    MappedReg32Type isr_table_m;    // 0x14: [RW]
    MappedReg32Type dbg_cause_l;    // 0x18: [RW]
    MappedReg32Type dbg_cause_m;    // 0x1c: [RW]
    MappedReg32Type dbg_epc_l;      // 0x20: [RW]
    MappedReg32Type dbg_epc_m;      // 0x24: [RW]
    IRQ_LOCK_TYPE irq_lock;         // 0x28: [RW]
    MappedReg32Type irq_cause;      // 0x2c: [RW]
};

DECLARE_CLASS(PLIC)

}  // namespace debugger
