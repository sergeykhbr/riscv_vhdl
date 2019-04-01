/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      General Purpose Timers model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_GPTIMERS_H__
#define __DEBUGGER_SOCSIM_PLUGIN_GPTIMERS_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/iwire.h"

namespace debugger {

class GPTimers : public IService, 
                 public IMemoryOperation,
                 public IClockListener {
public:
    GPTimers(const char *name);
    ~GPTimers();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IClockListener */
    virtual void stepCallback(uint64_t t);

private:
    AttributeType irqctrl_;
    AttributeType clksrc_;
    IWire *iwire_;
    IClock *iclk_;

    static const uint32_t TIMER_CONTROL_ENA = 1<<0;
    struct gptimers_map {
        uint64_t highcnt;
        uint32_t pending;
        uint32_t rsvr[13];
        typedef struct gptimer_type {
            volatile uint32_t control;
            volatile uint32_t rsv1;
            volatile uint64_t cur_value;
            volatile uint64_t init_value;
        } gptimer_type;
        gptimer_type timer[2];
    } regs_;

    int dbg_irq_cnt_;
};

DECLARE_CLASS(GPTimers)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_GPTIMERS_H__
