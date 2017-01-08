/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      ROM functional model declaration.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_ROM_H__
#define __DEBUGGER_SOCSIM_PLUGIN_ROM_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"

namespace debugger {

class MemorySim : public IService, 
                  public IMemoryOperation {
public:
    MemorySim(const char *name);
    ~MemorySim();

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
    static const int SYMB_IN_LINE = 16/2;
    bool chishex(int s);
    uint8_t chtohex(int s);

private:
    AttributeType initFile_;
    AttributeType readOnly_;
    AttributeType baseAddress_;
    AttributeType length_;
    uint8_t *mem_;
};

DECLARE_CLASS(MemorySim)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_ROM_H__
