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

#include "river_amba.h"

namespace debugger {

RiverAmba::RiverAmba(sc_module_name name_, uint32_t hartid, bool async_reset,
    bool fpu_ena, bool tracer_ena) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_msti("i_msti"),
    o_msto("o_msto"),
    i_ext_irq("i_ext_irq"),
    o_time("o_time"),
    o_exec_cnt("o_exec_cnt"),
    i_dport_valid("i_dport_valid"),
    i_dport_write("i_dport_write"),
    i_dport_region("i_dport_region"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_ready("o_dport_ready"),
    o_dport_rdata("o_dport_rdata"),
    o_halted("o_halted") {

    river0 = new RiverTop("river0", hartid, async_reset, fpu_ena, tracer_ena);
    river0->i_clk(i_clk);
    river0->i_nrst(i_nrst);
    river0->i_req_mem_ready(req_mem_ready_i);
    river0->o_req_mem_path(req_mem_path_o);
    river0->o_req_mem_valid(req_mem_valid_o);
    river0->o_req_mem_write(req_mem_write_o);
    river0->o_req_mem_cached(req_mem_cached_o);
    river0->o_req_mem_addr(req_mem_addr_o);
    river0->o_req_mem_strob(req_mem_strob_o);
    river0->o_req_mem_data(req_mem_data_o);
    river0->i_resp_mem_valid(resp_mem_valid_i);
    river0->i_resp_mem_path(r.resp_path);
    river0->i_resp_mem_data(wb_msti_r_data);
    river0->i_resp_mem_load_fault(resp_mem_load_fault_i);
    river0->i_resp_mem_store_fault(resp_mem_store_fault_i);
    river0->i_resp_mem_store_fault_addr(r.b_addr);
    river0->i_ext_irq(i_ext_irq);
    river0->o_time(o_time);
    river0->o_exec_cnt(o_exec_cnt);
    river0->i_dport_valid(i_dport_valid);
    river0->i_dport_write(i_dport_write);
    river0->i_dport_region(i_dport_region);
    river0->i_dport_addr(i_dport_addr);
    river0->i_dport_wdata(i_dport_wdata);
    river0->o_dport_ready(o_dport_ready);
    river0->o_dport_rdata(o_dport_rdata);
    river0->o_halted(o_halted);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_msti;
    sensitive << req_mem_path_o;
    sensitive << req_mem_valid_o;
    sensitive << req_mem_write_o;
    sensitive << req_mem_cached_o;
    sensitive << req_mem_addr_o;
    sensitive << req_mem_strob_o;
    sensitive << req_mem_data_o;
    sensitive << r.state;
    sensitive << r.resp_path;
    sensitive << r.w_addr;
    sensitive << r.b_addr;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

RiverAmba::~RiverAmba() {
    delete river0;
}

void RiverAmba::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_time, o_time.name());
        sc_trace(o_vcd, o_halted, o_halted.name());
        sc_trace(o_vcd, o_dport_ready, o_dport_ready.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());

        sc_trace(o_vcd, i_msti, i_msti.name());
        sc_trace(o_vcd, o_msto, o_msto.name());

        std::string pn(name());
        sc_trace(o_vcd, req_mem_path_o, pn + ".req_mem_path_o");
        sc_trace(o_vcd, req_mem_valid_o, pn + ".req_mem_valid_o");
        sc_trace(o_vcd, req_mem_write_o, pn + ".req_mem_write_o");
        sc_trace(o_vcd, req_mem_cached_o, pn + ".req_mem_cached_o");
        sc_trace(o_vcd, req_mem_addr_o, pn + ".req_mem_addr_o");
        sc_trace(o_vcd, req_mem_strob_o, pn + ".req_mem_strob_o");
        sc_trace(o_vcd, req_mem_data_o, pn + ".req_mem_data_o");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        
    }

    river0->generateVCD(i_vcd, o_vcd);
}

void RiverAmba::comb() {
    bool v_req_mem_ready;
    bool v_resp_mem_valid;
    bool v_mem_er_load_fault;
    bool v_mem_er_store_fault;
    bool v_next_ready;
    axi4_l1_out_type vmsto;
    sc_uint<3> vmsto_size;
    sc_uint<3> vmsto_prot;

    v = r;
    
    wb_msti_r_data = i_msti.read().r_data;  // can't directly pass to lower level

    v_req_mem_ready = 0;
    v_resp_mem_valid = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = i_msti.read().b_valid && i_msti.read().b_resp[1]
                           && r.b_wait.read();
    v_next_ready = 0;

    vmsto.r_ready = 1;
    vmsto.w_valid = 0;
    vmsto.w_last = 0;
    vmsto.ar_valid = 0;
    vmsto.aw_valid = 0;

    if (i_msti.read().b_valid == 1) {
        v.b_wait = 0;
    }

    switch (r.state.read()) {
    case idle:
        v_next_ready = 1;
        break;

    case reading:
        vmsto.r_ready = 1;
        v_mem_er_load_fault = i_msti.read().r_valid && i_msti.read().r_resp[1];
        v_resp_mem_valid = i_msti.read().r_valid;
        // r_valid and r_last always should be in the same time
        if (i_msti.read().r_valid == 1 && i_msti.read().r_last == 1) {
            v_next_ready = 1;
            v.state = idle;
        }
        break;

    case writing:
        vmsto.w_valid = 1;
        vmsto.w_last = 1;
        v_resp_mem_valid = i_msti.read().w_ready;
        // Write full line without burst transactions:
        if (i_msti.read().w_ready == 1) {
            v_next_ready = 1;
            v.state = idle;
            v.b_addr = r.w_addr;
            v.b_wait = 1;
        }
        break;

    default:;
    }

    if (v_next_ready == 1) {
        if (req_mem_valid_o.read() == 1) {
            v.resp_path = req_mem_path_o.read();
            if (req_mem_write_o.read() == 0) {
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
                    v.w_addr = req_mem_addr_o.read();
                }
            }
        }
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    if (req_mem_cached_o.read() == 1) {
        vmsto_size = 0x5;   // 32 Bytes
    } else if (req_mem_path_o.read() == 1) {
        vmsto_size = 0x4;   // 16 Bytes: Uncached Instruction
    } else {
        vmsto_size = 0x3;   // 8 Bytes: Uncached Data
    }

    vmsto_prot[0] = 0;                      // 0=Unpriviledge; 1=Priviledge access
    vmsto_prot[1] = 0;                      // 0=Secure access; 1=Non-secure access
    vmsto_prot[2] = req_mem_path_o.read();  // 0=Data; 1=Instruction

    //o_msto_aw_valid = vmsto_aw_valid;
    vmsto.aw_bits.addr = req_mem_addr_o;
    vmsto.aw_bits.len = 0;
    vmsto.aw_bits.size = vmsto_size;           // 0=1B; 1=2B; 2=4B; 3=8B; 4=16B; 5=32B; 6=64B; 7=128B
    vmsto.aw_bits.burst = 0x1;                 // 00=FIX; 01=INCR; 10=WRAP
    vmsto.aw_bits.lock = 0;
    vmsto.aw_bits.cache = req_mem_cached_o.read();
    vmsto.aw_bits.prot = vmsto_prot;
    vmsto.aw_bits.qos = 0;
    vmsto.aw_bits.region = 0;
    vmsto.aw_id = 0;
    vmsto.aw_user = 0;
    //vmsto.w_valid = vmsto_w_valid;
    vmsto.w_data = req_mem_data_o;
    //vmsto.w_last = vmsto_w_last;
    vmsto.w_strb = req_mem_strob_o;
    vmsto.w_user = 0;
    vmsto.b_ready = 1;

    //vmsto.ar_valid = vmsto_ar_valid;
    vmsto.ar_bits.addr = req_mem_addr_o;
    vmsto.ar_bits.len = 0;
    vmsto.ar_bits.size = vmsto_size;           // 0=1B; 1=2B; 2=4B; 3=8B; ...
    vmsto.ar_bits.burst = 0x1;                 // INCR
    vmsto.ar_bits.lock = 0;
    vmsto.ar_bits.cache = req_mem_cached_o.read();
    vmsto.ar_bits.prot = vmsto_prot;
    vmsto.ar_bits.qos = 0;
    vmsto.ar_bits.region = 0;
    vmsto.ar_id = 0;
    vmsto.ar_user = 0;
    //vmsto.r_ready = vmsto_r_ready;

    o_msto = vmsto;

    req_mem_ready_i = v_req_mem_ready;  
    resp_mem_valid_i = v_resp_mem_valid;
    resp_mem_load_fault_i = v_mem_er_load_fault;
    resp_mem_store_fault_i = v_mem_er_store_fault;
}

void RiverAmba::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

