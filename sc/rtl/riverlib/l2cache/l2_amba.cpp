// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 

#include "l2_amba.h"
#include "api_core.h"

namespace debugger {

L2Amba::L2Amba(sc_module_name name,
               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    o_req_ready("o_req_ready"),
    i_req_valid("i_req_valid"),
    i_req_type("i_req_type"),
    i_req_size("i_req_size"),
    i_req_prot("i_req_prot"),
    i_req_addr("i_req_addr"),
    i_req_strob("i_req_strob"),
    i_req_data("i_req_data"),
    o_resp_data("o_resp_data"),
    o_resp_valid("o_resp_valid"),
    o_resp_ack("o_resp_ack"),
    o_resp_load_fault("o_resp_load_fault"),
    o_resp_store_fault("o_resp_store_fault"),
    i_msti("i_msti"),
    o_msto("o_msto") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_valid;
    sensitive << i_req_type;
    sensitive << i_req_size;
    sensitive << i_req_prot;
    sensitive << i_req_addr;
    sensitive << i_req_strob;
    sensitive << i_req_data;
    sensitive << i_msti;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void L2Amba::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, i_req_type, i_req_type.name());
        sc_trace(o_vcd, i_req_size, i_req_size.name());
        sc_trace(o_vcd, i_req_prot, i_req_prot.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, i_req_strob, i_req_strob.name());
        sc_trace(o_vcd, i_req_data, i_req_data.name());
        sc_trace(o_vcd, o_resp_data, o_resp_data.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, o_resp_ack, o_resp_ack.name());
        sc_trace(o_vcd, o_resp_load_fault, o_resp_load_fault.name());
        sc_trace(o_vcd, o_resp_store_fault, o_resp_store_fault.name());
        sc_trace(o_vcd, i_msti, i_msti.name());
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
    }

}

void L2Amba::comb() {
    bool v_req_mem_ready;
    bool v_resp_mem_valid;
    bool v_resp_mem_ack;
    bool v_mem_er_load_fault;
    bool v_mem_er_store_fault;
    bool v_next_ready;
    axi4_l2_out_type vmsto;

    v_req_mem_ready = 0;
    v_resp_mem_valid = 0;
    v_resp_mem_ack = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = 0;
    v_next_ready = 0;
    vmsto = axi4_l2_out_none;

    v = r;

    vmsto.r_ready = 1;
    vmsto.w_valid = 0;
    vmsto.w_last = 0;
    vmsto.ar_valid = 0;
    vmsto.aw_valid = 0;

    switch (r.state.read()) {
    case idle:
        v_next_ready = 1;
        break;
    case reading:
        vmsto.r_ready = 1;
        v_resp_mem_valid = i_msti.read().r_valid;
        // r_valid and r_last always should be in the same time
        if ((i_msti.read().r_valid == 1) && (i_msti.read().r_last == 1)) {
            v_mem_er_load_fault = i_msti.read().r_resp[1];
            v_next_ready = 1;
            v_resp_mem_ack = 1;
            v.state = idle;
        }
        break;
    case writing:
        vmsto.w_valid = 1;
        vmsto.w_last = 1;
        // Write full line without burst transactions:
        if (i_msti.read().w_ready == 1) {
            v.state = wack;
        }
        break;
    case wack:
        vmsto.b_ready = 1;
        if (i_msti.read().b_valid == 1) {
            v_resp_mem_valid = 1;
            v_mem_er_store_fault = i_msti.read().b_resp[1];
            v_next_ready = 1;
            v_resp_mem_ack = 1;
            v.state = idle;
        }
        break;
    default:
        break;
    }

    if ((v_next_ready == 1) && (i_req_valid.read() == 1)) {
        if (i_req_type.read()[REQ_MEM_TYPE_WRITE] == 0) {
            vmsto.ar_valid = 1;
            if (i_msti.read().ar_ready == 1) {
                v_req_mem_ready = 1;
                v.state = reading;
            }
        } else {
            vmsto.aw_valid = 1;
            if (i_msti.read().aw_ready == 1) {
                v_req_mem_ready = 1;
                v.state = writing;
            }
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        L2Amba_r_reset(v);
    }

    o_resp_data = i_msti.read().r_data;                     // can't directly pass to lower level

    // o_msto_aw_valid = vmsto_aw_valid;
    vmsto.aw_bits.addr = i_req_addr;
    vmsto.aw_bits.len = 0;
    vmsto.aw_bits.size = i_req_size;                        // 0=1B; 1=2B; 2=4B; 3=8B; 4=16B; 5=32B; 6=64B; 7=128B
    vmsto.aw_bits.burst = 0x1;                              // 00=FIX; 01=INCR; 10=WRAP
    vmsto.aw_bits.lock = 0;
    vmsto.aw_bits.cache = i_req_type.read()[REQ_MEM_TYPE_CACHED];
    vmsto.aw_bits.prot = i_req_prot;
    vmsto.aw_bits.qos = 0;
    vmsto.aw_bits.region = 0;
    vmsto.aw_id = 0;
    vmsto.aw_user = 0;
    // vmsto.w_valid = vmsto_w_valid;
    vmsto.w_data = i_req_data;
    // vmsto.w_last = vmsto_w_last;
    vmsto.w_strb = i_req_strob;
    vmsto.w_user = 0;
    vmsto.b_ready = 1;

    // vmsto.ar_valid = vmsto_ar_valid;
    vmsto.ar_bits.addr = i_req_addr;
    vmsto.ar_bits.len = 0;
    vmsto.ar_bits.size = i_req_size;                        // 0=1B; 1=2B; 2=4B; 3=8B; ...
    vmsto.ar_bits.burst = 0x1;                              // INCR
    vmsto.ar_bits.lock = 0;
    vmsto.ar_bits.cache = i_req_type.read()[REQ_MEM_TYPE_CACHED];
    vmsto.ar_bits.prot = i_req_prot;
    vmsto.ar_bits.qos = 0;
    vmsto.ar_bits.region = 0;
    vmsto.ar_id = 0;
    vmsto.ar_user = 0;
    // vmsto.r_ready = vmsto_r_ready;

    o_msto = vmsto;

    o_req_ready = v_req_mem_ready;
    o_resp_valid = v_resp_mem_valid;
    o_resp_ack = v_resp_mem_ack;
    o_resp_load_fault = v_mem_er_load_fault;
    o_resp_store_fault = v_mem_er_store_fault;
}

void L2Amba::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        L2Amba_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

