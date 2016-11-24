/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Interrupt controller functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_IRQCTRL_H__
#define __DEBUGGER_SOCSIM_PLUGIN_IRQCTRL_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iwire.h"
#include "coreservices/icpuriscv.h"

namespace debugger {

class IrqController : public IService, 
                      public IMemoryOperation,
                      public IWire {
public:
    IrqController(const char *name);
    ~IrqController();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual void transaction(Axi4TransactionType *payload);
    
    virtual uint64_t getBaseAddress() {
        return baseAddress_.to_uint64();
    }
    virtual uint64_t getLength() {
        return length_.to_uint64();
    }

    /** IWire */
    virtual void raiseLine(int idx);
    virtual void lowerLine() {}
    virtual void setLevel(bool level) {}

private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType mipi_;
    AttributeType irqTotal_;
    AttributeType cpu_;
    ICpuRiscV *icpu_;

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
