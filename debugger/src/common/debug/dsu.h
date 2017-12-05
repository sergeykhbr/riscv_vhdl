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

#ifndef __COMMON_DEBUG_DSU_H__
#define __COMMON_DEBUG_DSU_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/imemop.h"
#include "coreservices/ireset.h"
#include "coreservices/icpugen.h"

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
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                      IAxi4NbResponse *cb);

    /** IDbgNbResponse */
    virtual void nb_response_debug_port(DebugPortTransactionType *trans);

 private:
    void readLocal(uint64_t off, Axi4TransactionType *trans);
    void writeLocal(uint64_t off, Axi4TransactionType *trans);

 private:
    AttributeType cpu_;
    AttributeType bus_;
    AttributeType reset_;
    ICpuGeneric *icpu_;
    IMemoryOperation *ibus_;
    IReset *irst_;
    uint64_t shifter32_;
    uint64_t wdata64_;
    uint64_t soft_reset_;

    struct nb_trans_type {
        Axi4TransactionType *p_axi_trans;
        IAxi4NbResponse *iaxi_cb;
        DebugPortTransactionType dbg_trans;
    } nb_trans_;
};

DECLARE_CLASS(DSU)

}  // namespace debugger

#endif  // __COMMON_DEBUG_DSU_H__
