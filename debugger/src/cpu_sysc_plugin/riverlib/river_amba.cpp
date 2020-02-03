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
    i_msti_aw_ready("i_msti_aw_ready"),
    i_msti_w_ready("i_msti_w_ready"),
    i_msti_b_valid("i_msti_b_valid"),
    i_msti_b_resp("i_msti_b_resp"),
    i_msti_b_id("i_msti_b_id"),
    i_msti_b_user("i_msti_b_user"),
    i_msti_ar_ready("i_msti_ar_ready"),
    i_msti_r_valid("i_msti_r_valid"),
    i_msti_r_resp("i_msti_r_resp"),
    i_msti_r_data("i_msti_r_data"),
    i_msti_r_last("i_msti_r_last"),
    i_msti_r_id("i_msti_r_id"),
    i_msti_r_user("i_msti_r_user"),
    o_msto_aw_valid("o_msto_aw_valid"),
    o_msto_aw_bits_addr("o_msto_aw_bits_addr"),
    o_msto_aw_bits_len("o_msto_aw_bits_len"),
    o_msto_aw_bits_size("o_msto_aw_bits_size"),
    o_msto_aw_bits_burst("o_msto_aw_bits_burst"),
    o_msto_aw_bits_lock("o_msto_aw_bits_lock"),
    o_msto_aw_bits_cache("o_msto_aw_bits_cache"),
    o_msto_aw_bits_prot("o_msto_aw_bits_prot"),
    o_msto_aw_bits_qos("o_msto_aw_bits_qos"),
    o_msto_aw_bits_region("o_msto_aw_bits_region"),
    o_msto_aw_id("o_msto_aw_id"),
    o_msto_aw_user("o_msto_aw_user"),
    o_msto_w_valid("o_msto_w_valid"),
    o_msto_w_data("o_msto_w_data"),
    o_msto_w_last("o_msto_w_last"),
    o_msto_w_strb("o_msto_w_strb"),
    o_msto_w_user("o_msto_w_user"),
    o_msto_b_ready("o_msto_b_ready"),
    o_msto_ar_valid("o_msto_ar_valid"),
    o_msto_ar_bits_addr("o_msto_ar_bits_addr"),
    o_msto_ar_bits_len("o_msto_ar_bits_len"),
    o_msto_ar_bits_size("o_msto_ar_bits_size"),
    o_msto_ar_bits_burst("o_msto_ar_bits_burst"),
    o_msto_ar_bits_lock("o_msto_ar_bits_lock"),
    o_msto_ar_bits_cache("o_msto_ar_bits_cache"),
    o_msto_ar_bits_prot("o_msto_ar_bits_prot"),
    o_msto_ar_bits_qos("o_msto_ar_bits_qos"),
    o_msto_ar_bits_region("o_msto_ar_bits_region"),
    o_msto_ar_id("o_msto_ar_id"),
    o_msto_ar_user("o_msto_ar_user"),
    o_msto_r_ready("o_msto_r_ready"),
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
    river0->o_req_mem_addr(req_mem_addr_o);
    river0->o_req_mem_strob(req_mem_strob_o);
    river0->o_req_mem_data(req_mem_data_o);
    river0->o_req_mem_len(req_mem_len_o);
    river0->o_req_mem_burst(req_mem_burst_o);
    river0->o_req_mem_last(req_mem_last_o);
    river0->i_resp_mem_valid(resp_mem_valid_i);
    river0->i_resp_mem_path(r.resp_path);
    river0->i_resp_mem_data(i_msti_r_data);
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
    sensitive << i_msti_aw_ready;
    sensitive << i_msti_w_ready;
    sensitive << i_msti_b_valid;
    sensitive << i_msti_b_resp;
    sensitive << i_msti_b_id;
    sensitive << i_msti_b_user;
    sensitive << i_msti_ar_ready;
    sensitive << i_msti_r_valid;
    sensitive << i_msti_r_resp;
    sensitive << i_msti_r_data;
    sensitive << i_msti_r_last;
    sensitive << i_msti_r_id;
    sensitive << i_msti_r_user;
    sensitive << req_mem_path_o;
    sensitive << req_mem_valid_o;
    sensitive << req_mem_write_o;
    sensitive << req_mem_addr_o;
    sensitive << req_mem_strob_o;
    sensitive << req_mem_data_o;
    sensitive << req_mem_len_o;
    sensitive << req_mem_burst_o;
    sensitive << req_mem_last_o;
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

        sc_trace(o_vcd, i_msti_aw_ready, i_msti_aw_ready.name());
        sc_trace(o_vcd, i_msti_w_ready, i_msti_w_ready.name());
        sc_trace(o_vcd, i_msti_b_valid, i_msti_b_valid.name());
        sc_trace(o_vcd, i_msti_b_resp, i_msti_b_resp.name());
        sc_trace(o_vcd, i_msti_ar_ready, i_msti_ar_ready.name());
        sc_trace(o_vcd, i_msti_r_valid, i_msti_r_valid.name());
        sc_trace(o_vcd, i_msti_r_resp, i_msti_r_resp.name());
        sc_trace(o_vcd, i_msti_r_data, i_msti_r_data.name());
        sc_trace(o_vcd, i_msti_r_last, i_msti_r_last.name());

        sc_trace(o_vcd, o_msto_aw_valid, o_msto_aw_valid.name());
        sc_trace(o_vcd, o_msto_aw_bits_addr, o_msto_aw_bits_addr.name());
        sc_trace(o_vcd, o_msto_aw_bits_len, o_msto_aw_bits_len.name());
        sc_trace(o_vcd, o_msto_aw_bits_size, o_msto_aw_bits_size.name());
        sc_trace(o_vcd, o_msto_aw_bits_burst, o_msto_aw_bits_burst.name());
        sc_trace(o_vcd, o_msto_aw_bits_lock, o_msto_aw_bits_lock.name());
        sc_trace(o_vcd, o_msto_aw_bits_cache, o_msto_aw_bits_cache.name());
        sc_trace(o_vcd, o_msto_w_valid, o_msto_w_valid.name());
        sc_trace(o_vcd, o_msto_w_data, o_msto_w_data.name());
        sc_trace(o_vcd, o_msto_w_last, o_msto_w_last.name());
        sc_trace(o_vcd, o_msto_w_strb, o_msto_w_strb.name());
        sc_trace(o_vcd, o_msto_b_ready, o_msto_b_ready.name());
        sc_trace(o_vcd, o_msto_ar_valid, o_msto_ar_valid.name());
        sc_trace(o_vcd, o_msto_ar_bits_addr, o_msto_ar_bits_addr.name());
        sc_trace(o_vcd, o_msto_ar_bits_len, o_msto_ar_bits_len.name());
        sc_trace(o_vcd, o_msto_ar_bits_size, o_msto_ar_bits_size.name());
        sc_trace(o_vcd, o_msto_ar_bits_burst, o_msto_ar_bits_burst.name());
        sc_trace(o_vcd, o_msto_ar_bits_lock, o_msto_ar_bits_lock.name());
        sc_trace(o_vcd, o_msto_ar_bits_cache, o_msto_ar_bits_cache.name());
        sc_trace(o_vcd, o_msto_r_ready, o_msto_r_ready.name());

        std::string pn(name());
        sc_trace(o_vcd, req_mem_path_o, pn + ".req_mem_path_o");
        sc_trace(o_vcd, req_mem_valid_o, pn + ".req_mem_valid_o");
        sc_trace(o_vcd, req_mem_write_o, pn + ".req_mem_write_o");
        sc_trace(o_vcd, req_mem_addr_o, pn + ".req_mem_addr_o");
        sc_trace(o_vcd, req_mem_strob_o, pn + ".req_mem_strob_o");
        sc_trace(o_vcd, req_mem_data_o, pn + ".req_mem_data_o");
        sc_trace(o_vcd, req_mem_len_o, pn + ".req_mem_len_o");
        sc_trace(o_vcd, req_mem_burst_o, pn + ".req_mem_burst_o");
        sc_trace(o_vcd, req_mem_last_o, pn + ".req_mem_last_o");
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
    bool vmsto_r_ready;
    bool vmsto_w_valid;
    bool vmsto_w_last;
    bool vmsto_ar_valid;
    bool vmsto_aw_valid;

    v = r;

    v_req_mem_ready = 0;
    v_resp_mem_valid = 0;
    v_mem_er_load_fault = 0;
    v_mem_er_store_fault = i_msti_b_valid.read() && i_msti_b_resp.read()[1];
    v_next_ready = 0;

    vmsto_r_ready = 1;
    vmsto_w_valid = 0;
    vmsto_w_last = 0;
    vmsto_ar_valid = 0;
    vmsto_aw_valid = 0;

    switch (r.state.read()) {
    case idle:
        v_next_ready = 1;
        break;

    case reading:
        vmsto_r_ready = 1;
        v_mem_er_load_fault = i_msti_r_valid.read() && i_msti_r_resp.read()[1];
        v_resp_mem_valid = i_msti_r_valid.read();
        if (i_msti_r_valid.read() == 1 && i_msti_r_last.read() == 1) {
            v_next_ready = 1;
            v.state = idle;
        }
        break;

    case writing:
        vmsto_w_valid = 1;
        vmsto_w_last = req_mem_last_o;
        v_resp_mem_valid = i_msti_w_ready.read();
        if (i_msti_w_ready.read() == 1 && req_mem_last_o.read() == 1) {
            v_next_ready = 1;
            v.state = idle;
            v.b_addr = r.w_addr;
        }
        break;

    default:;
    }

    if (v_next_ready == 1) {
        if (req_mem_valid_o.read() == 1) {
            v.resp_path = req_mem_path_o.read();
            if (req_mem_write_o.read() == 0) {
                vmsto_ar_valid = 1;
                if (i_msti_ar_ready.read() == 1) {
                    v_req_mem_ready = 1;
                    v.state = reading;
                }
            } else {
                vmsto_aw_valid = 1;
                if (i_msti_aw_ready.read() == 1) {
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


    o_msto_aw_valid = vmsto_aw_valid;
    o_msto_aw_bits_addr = req_mem_addr_o;
    o_msto_aw_bits_len = req_mem_len_o;
    o_msto_aw_bits_size = 0x3;                  // 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto_aw_bits_burst = req_mem_burst_o;
    o_msto_aw_bits_lock = 0;
    o_msto_aw_bits_cache = 0;
    o_msto_aw_bits_prot = 0;
    o_msto_aw_bits_qos = 0;
    o_msto_aw_bits_region = 0;
    o_msto_aw_id = 0;
    o_msto_aw_user = 0;
    o_msto_w_valid = vmsto_w_valid;
    o_msto_w_data = req_mem_data_o;
    o_msto_w_last = vmsto_w_last;
    o_msto_w_strb = req_mem_strob_o;
    o_msto_w_user = 0;
    o_msto_b_ready = 1;

    o_msto_ar_valid = vmsto_ar_valid;
    o_msto_ar_bits_addr = req_mem_addr_o;
    o_msto_ar_bits_len = req_mem_len_o;
    o_msto_ar_bits_size = 0x3;                  // 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto_ar_bits_burst = req_mem_burst_o;
    o_msto_ar_bits_lock = 0;
    o_msto_ar_bits_cache = 0;
    o_msto_ar_bits_prot = 0;
    o_msto_ar_bits_qos = 0;
    o_msto_ar_bits_region = 0;
    o_msto_ar_id = 0;
    o_msto_ar_user = 0;
    o_msto_r_ready = vmsto_r_ready;


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

