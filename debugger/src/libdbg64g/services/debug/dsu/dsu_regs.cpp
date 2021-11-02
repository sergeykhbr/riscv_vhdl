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
 */

#include <api_core.h>
#include <generic-isa.h>
#include "dsu_regs.h"
#include "dsu.h"

namespace debugger {

DsuRegisters::DsuRegisters(IService *parent) :
    dport_region_(parent, "dport_region", 0x00000, 3*4096),
    dmcontrol_(parent, "dmcontrol", 0x18000 + 0x10*8),
    dmstatus_(parent, "dmstatus", 0x18000 + 0x11*8),
    haltsum0_(parent, "haltsum0", 0x18000 + 0x40*8),
    bus_util_(parent, "bus_util", 0x18040, 2*64) {
}

ETransStatus
DsuRegisters::DPORT_REGION_BANK64::b_transport(Axi4TransactionType *trans) {
    RISCV_error("b_transport() to debug port NOT SUPPORTED", NULL);
    trans->response = MemResp_Error;
    return TRANS_ERROR;
}

ETransStatus
DsuRegisters::DPORT_REGION_BANK64::nb_transport(Axi4TransactionType *trans,
                                              IAxi4NbResponse *cb) {
    if (!icpu_) {
        trans->response = MemResp_Error;
        cb->nb_response(trans);
        return TRANS_ERROR;
    }

    nb_trans_.p_axi_trans = trans;
    nb_trans_.iaxi_cb = cb;

    ETransStatus ret = TRANS_OK;
    return ret;
}

}  // namespace debugger

