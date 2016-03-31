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
#include "coreservices/ihostio.h"

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
    virtual void riseLine();
    virtual void lowerLine() {}
    virtual void setLevel(bool level) {}

private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType mipi_;
    AttributeType hostio_;
    IHostIO *ihostio_;

    struct irqctrl_map {
        uint32_t irq_mask;     // [RW] 1=disable; 0=enable
        uint32_t irq_pending;  // [RW]
        uint32_t irq_clear;    // [WO]
        uint32_t irq_rise;     // [WO]
        uint64_t irq_handler;  // [RW]
        uint64_t dbg_cause;
        uint64_t dbg_epc;
    } regs_;
};

DECLARE_CLASS(IrqController)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_IRQCTRL_H__
