/**
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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
 */

#ifndef __DEBUGGER_SRC_COMMON_DEBUG_DSU_REGS_H__
#define __DEBUGGER_SRC_COMMON_DEBUG_DSU_REGS_H__

#include <iservice.h>
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"
#include "coreservices/icpugen.h"

namespace debugger {

class DsuRegisters {
 public:
    explicit DsuRegisters(IService *parent);

 protected:
    class SOFT_RESET_TYPE : public MappedReg64Type {
     public:
        SOFT_RESET_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr) {}
     protected:
        virtual uint64_t aboutToWrite(uint64_t new_val) override;
    };

    class CPU_CONTEXT_TYPE : public MappedReg64Type {
     public:
        CPU_CONTEXT_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr) {}
     protected:
        virtual uint64_t aboutToWrite(uint64_t new_val) override;
    };

    class DSU_REGION_BANK64 : public GenericReg64Bank,
                              public IDbgNbResponse {
     public:
        DSU_REGION_BANK64(IService *parent, const char *name, uint8_t regid,
            uint64_t addr, int len) :
            GenericReg64Bank(parent, name, addr, len), region_id_(regid) {
                icpu_ = 0;
            }

        /** IMemoryOperation methods */
        virtual ETransStatus b_transport(Axi4TransactionType *trans) override;
        virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                  IAxi4NbResponse *cb) override;

        /** IDbgNbResponse */
        virtual void nb_response_debug_port(DebugPortTransactionType *trans);

        /** Switch CPU context methods: */
        void setCpu(ICpuGeneric *icpu) { icpu_ = icpu; }

     protected:
        IFace *getInterface(const char *name) {
            if (strcmp(name, IFACE_SERVICE) == 0) {
                return parent_->getInterface(name);
            }
            return 0;
        }

     private:
        uint8_t region_id_;
        ICpuGeneric *icpu_;

        struct nb_trans_type {
            Axi4TransactionType *p_axi_trans;
            IAxi4NbResponse *iaxi_cb;
            DebugPortTransactionType dbg_trans;
        } nb_trans_;
    };

    DSU_REGION_BANK64 csr_region_;
    DSU_REGION_BANK64 reg_region_;
    DSU_REGION_BANK64 dbg_region_;

    SOFT_RESET_TYPE soft_reset_;
    CPU_CONTEXT_TYPE cpu_context_;
    GenericReg64Bank bus_util_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_COMMON_DEBUG_DSU_REGS_H__
