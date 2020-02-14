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
    river0->i_resp_mem_path(r.req_path);
    river0->i_resp_mem_data(resp_mem_data_i);
    river0->i_resp_mem_load_fault(resp_mem_load_fault_i);
    river0->i_resp_mem_store_fault(resp_mem_store_fault_i);
    river0->i_req_snoop_valid(req_snoop_valid_i);
    river0->i_req_snoop_getdata(req_snoop_getdata_i);
    river0->o_req_snoop_ready(req_snoop_ready_o);
    river0->i_req_snoop_addr(req_snoop_addr_i);
    river0->i_resp_snoop_ready(resp_snoop_ready_i);
    river0->o_resp_snoop_valid(resp_snoop_valid_o);
    river0->o_resp_snoop_data(resp_snoop_data_o);
    river0->o_resp_snoop_flags(resp_snoop_flags_o);
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
    sensitive << r.req_addr;
    sensitive << r.req_path;
    sensitive << r.req_cached;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.req_size;
    sensitive << r.req_prot;

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
    bool v_resp_mem_valid;
    bool v_mem_er_load_fault;
    bool v_mem_er_store_fault;
    bool v_next_ready;
    axi4_l1_out_type vmsto;

    v = r;
    
    v_resp_mem_valid = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = 0;
    v_next_ready = 0;

    vmsto = axi4_l1_out_none;
    vmsto.ar_bits.burst = 0x1;  // INCR (possible any value actually)
    vmsto.aw_bits.burst = 0x1;  // INCR (possible any value actually)

    switch (r.state.read()) {
    case state_idle:
        v_next_ready = 1;
        if (req_mem_valid_o.read() == 1) {
            v.req_path = req_mem_path_o.read();
            v.req_addr = req_mem_addr_o;
            if (req_mem_cached_o.read() == 1) {
                v.req_size = 0x5;   // 32 Bytes
            } else if (req_mem_path_o.read() == 1) {
                v.req_size = 0x4;   // 16 Bytes: Uncached Instruction
            } else {
                v.req_size = 0x3;   // 8 Bytes: Uncached Data
            }
            // [0] 0=Unpriv/1=Priv;
            // [1] 0=Secure/1=Non-secure;
            // [2]  0=Data/1=Instruction
            v.req_prot = req_mem_path_o.read() << 2;
            if (req_mem_write_o.read() == 0) {
                v.state = state_ar;
                v.req_wdata = 0;
                v.req_wstrb = 0;
                if (req_mem_cached_o.read() == 1) {
                    v.req_cached = ARCACHE_WRBACK_READ_ALLOCATE;
                } else {
                    v.req_cached = ARCACHE_DEVICE_NON_BUFFERABLE;
                }
            } else {
                v.state = state_aw;
                v.req_wdata = req_mem_data_o;
                v.req_wstrb = req_mem_strob_o;
                if (req_mem_cached_o.read() == 1) {
                    v.req_cached = AWCACHE_WRBACK_WRITE_ALLOCATE;
                } else {
                    v.req_cached = AWCACHE_DEVICE_NON_BUFFERABLE;
                }
            }
        }
        break;

    case state_ar:
        vmsto.ar_valid = 1;
        vmsto.ar_bits.addr = r.req_addr;
        vmsto.ar_bits.cache = r.req_cached;
        vmsto.ar_bits.size = r.req_size;
        vmsto.ar_bits.prot = r.req_prot;
        if (i_msti.read().ar_ready == 1) {
            v.state = state_r;
        }
        break;
    case state_r:
        vmsto.r_ready = 1;
        v_mem_er_load_fault = i_msti.read().r_resp[1];
        v_resp_mem_valid = i_msti.read().r_valid;
        // r_valid and r_last always should be in the same time
        if (i_msti.read().r_valid == 1 && i_msti.read().r_last == 1) {
            v.state = state_idle;
        }
        break;

    case state_aw:
        vmsto.aw_valid = 1;
        vmsto.aw_bits.addr = r.req_addr;
        vmsto.aw_bits.cache = r.req_cached;
        vmsto.aw_bits.size = r.req_size;
        vmsto.aw_bits.prot = r.req_prot;
        // axi lite to simplify L2-cache
        vmsto.w_valid = 1;
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
        if (i_msti.read().aw_ready == 1) {
            if (i_msti.read().w_ready == 1) {
                v.state = state_b;
            } else {
                v.state = state_w;
            }
        }
        break;
    case state_w:
        // Shoudln't get here because of Lite interface:
        vmsto.w_valid = 1;
        vmsto.w_data = r.req_wdata;
        vmsto.w_strb = r.req_wstrb;
        if (i_msti.read().w_ready == 1) {
            v.state = state_b;
        }
        break;
    case state_b:
        vmsto.b_ready = 1;
        v_resp_mem_valid = i_msti.read().b_valid;
        v_mem_er_store_fault = i_msti.read().b_resp[1];
        if (i_msti.read().b_valid == 1) {
            v.state = state_idle;
        }
        break;
    default:;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }


    o_msto = vmsto;

    req_mem_ready_i = v_next_ready;  
    resp_mem_valid_i = v_resp_mem_valid;
    resp_mem_data_i = i_msti.read().r_data;
    resp_mem_load_fault_i = v_mem_er_load_fault;
    resp_mem_store_fault_i = v_mem_er_store_fault;

    req_snoop_valid_i = 0;
    req_snoop_getdata_i = 0;      // 0=check availability; 1=read line
    req_snoop_addr_i = 0;
    resp_snoop_ready_i = 1;
}

void RiverAmba::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

