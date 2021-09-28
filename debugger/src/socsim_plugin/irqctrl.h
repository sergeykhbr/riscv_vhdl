/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Interrupt controller functional model.
 */

#pragma once

#include <iclass.h>
#include <iservice.h>
#include "coreservices/iclock.h"
#include "coreservices/imemop.h"
#include "coreservices/iwire.h"
#include "coreservices/icpugen.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"
namespace debugger {

class IrqPort : public IWire {
 public:
    IrqPort(IService *parent, const char *portname, int idx);

    /** IWire interface */
    virtual void raiseLine();
    virtual void lowerLine() { level_ = false; }
    virtual void setLevel(bool level);
    virtual bool getLevel() { return level_; }

 protected:
    IService *parent_;
    int idx_;
    bool level_;
};

class IrqController : public RegMemBankGeneric,
                      public IClockListener {
 public:
    IrqController(const char *name);
    ~IrqController();

    /** IService interface */
    virtual void postinitService();

    /** IClockListener interface */
    virtual void stepCallback(uint64_t t);

    /** Controller specific methods visible for ports */
    void setPendingBit(int idx);
    void clearPendingBit(int idx);

 private:
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

    AttributeType irqTotal_;
    AttributeType cpu_;
    ICpuGeneric *icpu_;
    IClock *iclk_;
    static const int IRQ_MAX = 32;
    IrqPort *irqlines_[IRQ_MAX];

    MappedReg32Type irq_mask;       // 0x00: [RW] 1=disable; 0=enable
    IRQ_PENDING_TYPE irq_pending;   // 0x04: [RO]
    IRQ_CLEAR_TYPE irq_clear;       // 0x08: [WO]
    MappedReg32Type irq_raise;      // 0x0c: [WO]
    MappedReg32Type isr_table_l;    // 0x10: [RW]
    MappedReg32Type isr_table_m;    // 0x14: [RW]
    MappedReg32Type dbg_cause_l;    // 0x18: [RW]
    MappedReg32Type dbg_cause_m;    // 0x1c: [RW]
    MappedReg32Type dbg_epc_l;      // 0x20: [RW]
    MappedReg32Type dbg_epc_m;      // 0x24: [RW]
    MappedReg32Type irq_lock;       // 0x28: [RW]
    MappedReg32Type irq_cause;      // 0x2c: [RW]
};

DECLARE_CLASS(IrqController)

}  // namespace debugger
