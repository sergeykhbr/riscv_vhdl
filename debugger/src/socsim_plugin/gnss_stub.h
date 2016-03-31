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
#include "coreservices/iclklistener.h"
#include "coreservices/iclock.h"
#include "coreservices/iwire.h"

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
    virtual void transaction(Axi4TransactionType *payload);
    
    virtual uint64_t getBaseAddress() {
        return baseAddress_.to_uint64();
    }
    virtual uint64_t getLength() {
        return length_.to_uint64();
    }

    /** IClockListener */
    virtual void stepCallback(uint64_t t);

private:
    uint64_t OFFSET(void *addr) {
        return reinterpret_cast<uint64_t>(addr)
             - reinterpret_cast<uint64_t>(&regs_);
    }

private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType irqctrl_;
    AttributeType clksrc_;
    IWire *iwire_;
    IClock *iclk_;

    typedef struct TimerType {
        uint32_t rw_MsLength;
        uint32_t r_MsCnt;
        int32_t  rw_tow;
        int32_t  rw_tod;
        uint32_t unused[12];
    } TimerType;

    struct gnss_map {
        uint8_t    rsrv1[64];
        TimerType   tmr;
    } regs_;
};

DECLARE_CLASS(GNSSStub)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_GNSS_STUB_H__
