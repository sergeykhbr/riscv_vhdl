/**
/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @details    DSU supports both types of transaction: blocking and
 *             non-blocking it allows to interact with SystemC in the
 *             same manner as with the Functional model.
 * @note       CPU Functional model must implement non-blocking interface
 */

#ifndef __DEBUGGER_COMMON_DEBUG_DSU_H__
#define __DEBUGGER_COMMON_DEBUG_DSU_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/imemop.h"
#include "coreservices/ireset.h"
#include "coreservices/icpugen.h"
#include "coreservices/idsugen.h"

namespace debugger {

class DSU : public IService,
            public IMemoryOperation,
            public IDbgNbResponse,
            public IDsuGeneric {
 public:
    explicit DSU(const char *name);
    ~DSU();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                      IAxi4NbResponse *cb);

    /** IDbgNbResponse */
    virtual void nb_response_debug_port(DebugPortTransactionType *trans);

    /** IDsuGeneric */
    virtual void incrementRdAccess(int mst_id);
    virtual void incrementWrAccess(int mst_id);

 private:
    void readLocal(uint64_t off, Axi4TransactionType *trans);
    void writeLocal(uint64_t off, Axi4TransactionType *trans);

 private:
    AttributeType cpu_;
    AttributeType bus_;
    ICpuGeneric *icpu_;
    IResetListener *icpurst_;
    IMemoryOperation *ibus_;
    uint64_t shifter32_;
    uint64_t wdata64_;
    uint64_t soft_reset_;

    struct nb_trans_type {
        Axi4TransactionType *p_axi_trans;
        IAxi4NbResponse *iaxi_cb;
        DebugPortTransactionType dbg_trans;
    } nb_trans_;

    static const int BUS_MASTERS_MAX = 64;
    struct BusUtilType {
        uint64_t w_cnt;
        uint64_t r_cnt;
    } info_[BUS_MASTERS_MAX];
};

DECLARE_CLASS(DSU)

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_DEBUG_DSU_H__
