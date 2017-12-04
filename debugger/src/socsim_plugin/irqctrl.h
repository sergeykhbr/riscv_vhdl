/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Interrupt controller functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_IRQCTRL_H__
#define __DEBUGGER_SOCSIM_PLUGIN_IRQCTRL_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/iclock.h"
#include "coreservices/imemop.h"
#include "coreservices/iwire.h"
#include "coreservices/icpugen.h"

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

class IrqController : public IService, 
                      public IMemoryOperation,
                      public IClockListener {
 public:
    IrqController(const char *name);
    ~IrqController();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *payload);

    /** IClockListener interface */
    virtual void stepCallback(uint64_t t);

    /** Controller specific methods visible for ports */
    void requestInterrupt(int idx);

 private:
    AttributeType mipi_;
    AttributeType irqTotal_;
    AttributeType cpu_;
    ICpuGeneric *icpu_;
    IClock *iclk_;
    static const int IRQ_MAX = 32;
    IrqPort *irqlines_[IRQ_MAX];

    struct irqctrl_map {
        uint32_t irq_mask;      // 0x00: [RW] 1=disable; 0=enable
        uint32_t irq_pending;   // 0x04: [RW]
        uint32_t irq_clear;     // 0x08: [WO]
        uint32_t irq_rise;      // 0x0c: [WO]
        uint64_t isr_table;     // 0x10: [RW]
        uint64_t dbg_cause;     // 0x18: [RW]
        uint64_t dbg_epc;       // 0x20: [RW]
        uint32_t irq_lock;      // 0x28: [RW]
        uint32_t irq_cause_idx; // 0x2c: [RW]
    } regs_;
};

DECLARE_CLASS(IrqController)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_IRQCTRL_H__
