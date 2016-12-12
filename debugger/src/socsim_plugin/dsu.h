/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug Support Unit (DSU) functional model.
 *
 * @details    DSU supports both types of transaction: blocking and
 *             non-blocking it allows to interact with SystemC in the
 *             same manner as with the Functional model.
 * @note       CPU Functional model must implement non-blocking interface
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_DSU_H__
#define __DEBUGGER_SOCSIM_PLUGIN_DSU_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iwire.h"
#include "coreservices/icpuriscv.h"

namespace debugger {

class DSU : public IService, 
            public IMemoryOperation,
            public IDbgNbResponse {
public:
    DSU(const char *name);
    ~DSU();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual void b_transport(Axi4TransactionType *trans);
    virtual void nb_transport(Axi4TransactionType *trans,
                              IAxi4NbResponse *cb);

    
    virtual uint64_t getBaseAddress() {
        return baseAddress_.to_uint64();
    }
    virtual uint64_t getLength() {
        return length_.to_uint64();
    }

    /** IDbgNbResponse */
    virtual void nb_response_debug_port(DebugPortTransactionType *trans);


private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType cpu_;
    ICpuRiscV *icpu_;
    uint64_t shifter32_;

    struct nb_trans_type {
        Axi4TransactionType *p_axi_trans;
        IAxi4NbResponse *iaxi_cb;
        DebugPortTransactionType dbg_trans;
    } nb_trans_;
};

DECLARE_CLASS(DSU)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_DSU_H__
