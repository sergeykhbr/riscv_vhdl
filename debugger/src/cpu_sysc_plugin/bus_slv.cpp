/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "bus_slv.h"

namespace debugger {

BusSlave::BusSlave(sc_module_name name) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    o_req_valid("o_req_valid"),
    i_req_ready("i_req_ready"),
    o_req_addr("o_req_addr"),
    o_req_write("o_req_write"),
    o_req_wdata("o_req_wdata"),
    i_slv_resp_valid("i_slv_resp_valid"),
    o_slv_resp_ready("o_slv_resp_ready"),
    i_slv_resp_rdata("i_slv_resp_rdata") {

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

ETransStatus BusSlave::b_transport(Axi4TransactionType *trans) {
    return TRANS_ERROR;
}

ETransStatus BusSlave::nb_transport(Axi4TransactionType *trans,
                                    IAxi4NbResponse *cb) {

    uint64_t off = trans->addr - getBaseAddress();

    // Only 4-bytes requests:
    if (trans->action == MemAction_Read) {
        readreg(off >> 2);
    } else {
        if (trans->wstrb & 0x00FF) {
            writereg(off >> 2, trans->wpayload.b32[0]);
        } else if (trans->wstrb & 0xFF00) {
            writereg((off + 4) >> 2, trans->wpayload.b32[1]);
        }
    }
    lasttrans_ = *trans;
    lastcb_ = cb;
    return TRANS_OK;
}

void BusSlave::readreg(uint64_t idx) {
    bus_req_addr_ = idx;
    bus_req_write_ = 0;
    bus_req_valid_ = 1;
}

void BusSlave::writereg(uint64_t idx, uint32_t w32) {
    bus_req_addr_ = idx;
    bus_req_wdata_ = w32;
    bus_req_write_ = 1;
    bus_req_valid_ = 1;
}


void BusSlave::registers() {
    o_req_valid = bus_req_valid_;
    o_req_addr = bus_req_addr_;
    o_req_write = bus_req_write_;
    o_req_wdata = bus_req_wdata_;
    o_slv_resp_ready = 1;

    if (bus_req_valid_ && i_req_ready.read()) {
        bus_req_valid_ = 0;
    }

    if (i_slv_resp_valid.read()) {
        bus_resp_data_ = i_slv_resp_rdata.read();
        // We cannot get here without valid request no need additional checks
        lasttrans_.rpayload.b32[0] = bus_resp_data_;
        lastcb_->nb_response(&lasttrans_);
    }
}


}  // namespace debugger

