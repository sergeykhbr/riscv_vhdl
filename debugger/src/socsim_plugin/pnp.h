/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plug'n'Play device functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_PNP_H__
#define __DEBUGGER_SOCSIM_PLUGIN_PNP_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/isocinfo.h"

namespace debugger {

class PNP : public IService, 
            public IMemoryOperation {
public:
    PNP(const char *name);
    ~PNP();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual void b_transport(Axi4TransactionType *trans);
    
    virtual uint64_t getBaseAddress() {
        return baseAddress_.to_uint64();
    }
    virtual uint64_t getLength() {
        return length_.to_uint64();
    }

private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType tech_;
    AttributeType adc_detector_;

    PnpMapType regs_;
};

DECLARE_CLASS(PNP)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_PNP_H__
