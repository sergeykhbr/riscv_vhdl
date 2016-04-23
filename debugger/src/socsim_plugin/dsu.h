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
#include "coreservices/icpuriscv.h"

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
    void regionCsrRd(uint64_t off, Axi4TransactionType *payload);
    void regionCsrWr(uint64_t off, Axi4TransactionType *payload);
    void regionControlRd(uint64_t off, Axi4TransactionType *payload);
    void regionControlWr(uint64_t off, Axi4TransactionType *payload);

    void msb_of_64(uint64_t *val, uint32_t dw);
    void lsb_of_64(uint64_t *val, uint32_t dw);

private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType hostio_;
    IHostIO *ihostio_;

    uint64_t wdata_;
    struct RegType {
        uint64_t control;
        uint64_t step_cnt;
    } regs_;
};

DECLARE_CLASS(DSU)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_DSU_H__
