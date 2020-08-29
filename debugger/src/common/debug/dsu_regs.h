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
    class DMCONTROL_TYPE : public MappedReg64Type {
     public:
        DMCONTROL_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr) {}

        union ValueType {
            uint64_t val;
            struct {
                uint64_t dmactive : 1;          // [0] 1=module functioning normally
                uint64_t ndmreset : 1;          // [1] 1=system reset
                uint64_t rsrv5_2 : 4;           // [5:2]
                uint64_t hartselhi : 10;        // [15:6]
                uint64_t hartsello : 10;        // [25:16]
                uint64_t rsrv29_26  : 4;        // [29:26]
                uint64_t resumereq : 1;         // [30]
                uint64_t haltreq : 1;           // [31]
                uint64_t rsv      : 32;
            } bits;
        };

     protected:
        virtual uint64_t aboutToWrite(uint64_t new_val) override;
        virtual uint64_t aboutToRead(uint64_t cur_val) override;
    };

    class DMSTATUS_TYPE : public MappedReg64Type {
     public:
        DMSTATUS_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr) {}

        union ValueType {
            uint64_t val;
            struct {
                uint64_t version : 4;           // [3:0] 2=version 0.13
                uint64_t rsrv6_4 : 3;           // [6:4]
                uint64_t authenticated : 1;     // [7]
                uint64_t anyhalted : 1;         // [8]
                uint64_t allhalted : 1;         // [9]
                uint64_t anyrunning : 1;        // [10]
                uint64_t allrunning : 1;        // [11]
                uint64_t anyunavail : 1;        // [12]
                uint64_t allunavail : 1;        // [13]
                uint64_t anynonexistent: 1;     // [14]
                uint64_t allnonexistent: 1;     // [15]
                uint64_t anyresumeack: 1;       // [16]
                uint64_t allresumeack: 1;       // [17]
                uint64_t rsrv21_18  : 4;        // [21:18]
                uint64_t impebreak : 1;         // [22]
                uint64_t rsv      : 41;         // [63:23]
            } bits;
        };

     protected:
        virtual uint64_t aboutToRead(uint64_t cur_val) override;
    };

    class HALTSUM_TYPE : public MappedReg64Type {
     public:
        HALTSUM_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr) {}
     protected:
        virtual uint64_t aboutToRead(uint64_t cur_val) override;
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

    DMCONTROL_TYPE dmcontrol_;
    DMSTATUS_TYPE dmstatus_;
    HALTSUM_TYPE haltsum0_;
    GenericReg64Bank bus_util_;

    DebugPortTransactionType nb_trans_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_COMMON_DEBUG_DSU_REGS_H__
