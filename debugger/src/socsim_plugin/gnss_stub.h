/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      GNSS stub module functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_GNSS_STUB_H__
#define __DEBUGGER_SOCSIM_PLUGIN_GNSS_STUB_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/iirq.h"

namespace debugger {

class GNSSStub : public IService, 
                 public IMemoryOperation,
                 public IClockListener {
public:
    GNSSStub(const char *name);
    ~GNSSStub();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    
    /** IClockListener */
    virtual void stepCallback(uint64_t t);

private:
    uint64_t OFFSET(void *addr) {
        return reinterpret_cast<uint64_t>(addr)
             - reinterpret_cast<uint64_t>(&regs_);
    }

private:
    AttributeType irqctrl_;
    AttributeType irqid_;
    AttributeType clksrc_;

    IIrqController *iirq_;
    IClock *iclk_;

    typedef struct MiscType {
        volatile uint32_t Date;
        volatile uint32_t GenericChanCfg;
        uint64_t hide[7];
    } MiscType;

    typedef struct TimerType {
        uint32_t rw_MsLength;
        uint32_t r_MsCnt;
        int32_t  rw_tow;
        int32_t  rw_tod;
        uint32_t unused[12];
    } TimerType;

    struct gnss_map {
        MiscType misc;
        TimerType   tmr;
        uint64_t noise[8];
        uint64_t chan[256*8];
    } regs_;
};

DECLARE_CLASS(GNSSStub)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_GNSS_STUB_H__
