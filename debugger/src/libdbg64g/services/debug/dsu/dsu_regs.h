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
#include "debug/dmi_regs.h"

namespace debugger {

class DsuRegisters {
 public:
    explicit DsuRegisters(IService *parent);

 protected:
    class DPORT_REGION_BANK64 : public GenericReg64Bank {
     public:
        DPORT_REGION_BANK64(IService *parent, const char *name,
            uint64_t addr, int len) :
            GenericReg64Bank(parent, name, addr, len) {
            }

        /** IMemoryOperation methods */
        virtual ETransStatus b_transport(Axi4TransactionType *trans) override;
        virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                  IAxi4NbResponse *cb) override;


     protected:
        IFace *getInterface(const char *name) {
            if (strcmp(name, IFACE_SERVICE) == 0) {
                return parent_->getInterface(name);
            }
            return 0;
        }

     private:

        struct nb_trans_type {
            Axi4TransactionType *p_axi_trans;
            IAxi4NbResponse *iaxi_cb;
        } nb_trans_;
    };

    DPORT_REGION_BANK64 dport_region_;

    DMCONTROL_TYPE dmcontrol_;
    DMSTATUS_TYPE dmstatus_;
    HALTSUM0_TYPE haltsum0_;
    GenericReg64Bank bus_util_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_COMMON_DEBUG_DSU_REGS_H__
