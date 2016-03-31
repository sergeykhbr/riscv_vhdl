/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug Support Unit (DSU) functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_DSU_H__
#define __DEBUGGER_SOCSIM_PLUGIN_DSU_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iwire.h"
#include "coreservices/ihostio.h"

namespace debugger {

class DSU : public IService, 
                      public IMemoryOperation {
public:
    DSU(const char *name);
    ~DSU();

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


private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType hostio_;
    IHostIO *ihostio_;

};

DECLARE_CLASS(DSU)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_DSU_H__
