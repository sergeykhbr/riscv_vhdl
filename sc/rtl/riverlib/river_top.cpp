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

#include "river_top.h"
#include "api_core.h"

namespace debugger {

RiverTop::RiverTop(sc_module_name name,
                   bool async_reset,
                   uint32_t hartid,
                   bool fpu_ena,
                   bool coherence_ena,
                   bool tracer_ena,
                   uint32_t ilog2_nways,
                   uint32_t ilog2_lines_per_way,
                   uint32_t dlog2_nways,
                   uint32_t dlog2_lines_per_way)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mtimer("i_mtimer"),
    i_req_mem_ready("i_req_mem_ready"),
    o_req_mem_path("o_req_mem_path"),
    o_req_mem_valid("o_req_mem_valid"),
    o_req_mem_type("o_req_mem_type"),
    o_req_mem_size("o_req_mem_size"),
    o_req_mem_addr("o_req_mem_addr"),
    o_req_mem_strob("o_req_mem_strob"),
    o_req_mem_data("o_req_mem_data"),
    i_resp_mem_valid("i_resp_mem_valid"),
    i_resp_mem_path("i_resp_mem_path"),
    i_resp_mem_data("i_resp_mem_data"),
    i_resp_mem_load_fault("i_resp_mem_load_fault"),
    i_resp_mem_store_fault("i_resp_mem_store_fault"),
    i_req_snoop_valid("i_req_snoop_valid"),
    i_req_snoop_type("i_req_snoop_type"),
    o_req_snoop_ready("o_req_snoop_ready"),
    i_req_snoop_addr("i_req_snoop_addr"),
    i_resp_snoop_ready("i_resp_snoop_ready"),
    o_resp_snoop_valid("o_resp_snoop_valid"),
    o_resp_snoop_data("o_resp_snoop_data"),
    o_resp_snoop_flags("o_resp_snoop_flags"),
    o_flush_l2("o_flush_l2"),
    i_irq_pending("i_irq_pending"),
    i_haltreq("i_haltreq"),
    i_resumereq("i_resumereq"),
    i_dport_req_valid("i_dport_req_valid"),
    i_dport_type("i_dport_type"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    i_dport_size("i_dport_size"),
    o_dport_req_ready("o_dport_req_ready"),
    i_dport_resp_ready("i_dport_resp_ready"),
    o_dport_resp_valid("o_dport_resp_valid"),
    o_dport_resp_error("o_dport_resp_error"),
    o_dport_rdata("o_dport_rdata"),
    i_progbuf("i_progbuf"),
    o_halted("o_halted") {

    async_reset_ = async_reset;
    hartid_ = hartid;
    fpu_ena_ = fpu_ena;
    coherence_ena_ = coherence_ena;
    tracer_ena_ = tracer_ena;
    ilog2_nways_ = ilog2_nways;
    ilog2_lines_per_way_ = ilog2_lines_per_way;
    dlog2_nways_ = dlog2_nways;
    dlog2_lines_per_way_ = dlog2_lines_per_way;
    proc0 = 0;
    cache0 = 0;

    proc0 = new Processor("proc0", async_reset,
                           hartid,
                           fpu_ena,
                           tracer_ena);
    proc0->i_clk(i_clk);
    proc0->i_nrst(i_nrst);
    proc0->i_mtimer(i_mtimer);
    proc0->i_req_ctrl_ready(w_req_ctrl_ready);
    proc0->o_req_ctrl_valid(w_req_ctrl_valid);
    proc0->o_req_ctrl_addr(wb_req_ctrl_addr);
    proc0->i_resp_ctrl_valid(w_resp_ctrl_valid);
    proc0->i_resp_ctrl_addr(wb_resp_ctrl_addr);
    proc0->i_resp_ctrl_data(wb_resp_ctrl_data);
    proc0->i_resp_ctrl_load_fault(w_resp_ctrl_load_fault);
    proc0->o_resp_ctrl_ready(w_resp_ctrl_ready);
    proc0->i_req_data_ready(w_req_data_ready);
    proc0->o_req_data_valid(w_req_data_valid);
    proc0->o_req_data_type(wb_req_data_type);
    proc0->o_req_data_addr(wb_req_data_addr);
    proc0->o_req_data_wdata(wb_req_data_wdata);
    proc0->o_req_data_wstrb(wb_req_data_wstrb);
    proc0->o_req_data_size(wb_req_data_size);
    proc0->i_resp_data_valid(w_resp_data_valid);
    proc0->i_resp_data_addr(wb_resp_data_addr);
    proc0->i_resp_data_data(wb_resp_data_data);
    proc0->i_resp_data_load_fault(w_resp_data_load_fault);
    proc0->i_resp_data_store_fault(w_resp_data_store_fault);
    proc0->o_resp_data_ready(w_resp_data_ready);
    proc0->i_irq_pending(i_irq_pending);
    proc0->o_pmp_ena(w_pmp_ena);
    proc0->o_pmp_we(w_pmp_we);
    proc0->o_pmp_region(wb_pmp_region);
    proc0->o_pmp_start_addr(wb_pmp_start_addr);
    proc0->o_pmp_end_addr(wb_pmp_end_addr);
    proc0->o_pmp_flags(wb_pmp_flags);
    proc0->i_haltreq(i_haltreq);
    proc0->i_resumereq(i_resumereq);
    proc0->i_dport_req_valid(i_dport_req_valid);
    proc0->i_dport_type(i_dport_type);
    proc0->i_dport_addr(i_dport_addr);
    proc0->i_dport_wdata(i_dport_wdata);
    proc0->i_dport_size(i_dport_size);
    proc0->o_dport_req_ready(o_dport_req_ready);
    proc0->i_dport_resp_ready(i_dport_resp_ready);
    proc0->o_dport_resp_valid(o_dport_resp_valid);
    proc0->o_dport_resp_error(o_dport_resp_error);
    proc0->o_dport_rdata(o_dport_rdata);
    proc0->i_progbuf(i_progbuf);
    proc0->o_halted(o_halted);
    proc0->o_flushi_valid(w_flushi_valid);
    proc0->o_flushi_addr(wb_flushi_addr);
    proc0->o_flushd_valid(w_flushd_valid);
    proc0->o_flushd_addr(wb_flushd_addr);
    proc0->i_flushd_end(w_flushd_end);


    cache0 = new CacheTop("cache0", async_reset,
                           coherence_ena,
                           ilog2_nways,
                           ilog2_lines_per_way,
                           dlog2_nways,
                           dlog2_lines_per_way);
    cache0->i_clk(i_clk);
    cache0->i_nrst(i_nrst);
    cache0->i_req_ctrl_valid(w_req_ctrl_valid);
    cache0->i_req_ctrl_addr(wb_req_ctrl_addr);
    cache0->o_req_ctrl_ready(w_req_ctrl_ready);
    cache0->o_resp_ctrl_valid(w_resp_ctrl_valid);
    cache0->o_resp_ctrl_addr(wb_resp_ctrl_addr);
    cache0->o_resp_ctrl_data(wb_resp_ctrl_data);
    cache0->o_resp_ctrl_load_fault(w_resp_ctrl_load_fault);
    cache0->i_resp_ctrl_ready(w_resp_ctrl_ready);
    cache0->i_req_data_valid(w_req_data_valid);
    cache0->i_req_data_type(wb_req_data_type);
    cache0->i_req_data_addr(wb_req_data_addr);
    cache0->i_req_data_wdata(wb_req_data_wdata);
    cache0->i_req_data_wstrb(wb_req_data_wstrb);
    cache0->i_req_data_size(wb_req_data_size);
    cache0->o_req_data_ready(w_req_data_ready);
    cache0->o_resp_data_valid(w_resp_data_valid);
    cache0->o_resp_data_addr(wb_resp_data_addr);
    cache0->o_resp_data_data(wb_resp_data_data);
    cache0->o_resp_data_load_fault(w_resp_data_load_fault);
    cache0->o_resp_data_store_fault(w_resp_data_store_fault);
    cache0->i_resp_data_ready(w_resp_data_ready);
    cache0->i_req_mem_ready(i_req_mem_ready);
    cache0->o_req_mem_path(o_req_mem_path);
    cache0->o_req_mem_valid(o_req_mem_valid);
    cache0->o_req_mem_type(o_req_mem_type);
    cache0->o_req_mem_size(o_req_mem_size);
    cache0->o_req_mem_addr(o_req_mem_addr);
    cache0->o_req_mem_strob(o_req_mem_strob);
    cache0->o_req_mem_data(o_req_mem_data);
    cache0->i_resp_mem_valid(i_resp_mem_valid);
    cache0->i_resp_mem_path(i_resp_mem_path);
    cache0->i_resp_mem_data(i_resp_mem_data);
    cache0->i_resp_mem_load_fault(i_resp_mem_load_fault);
    cache0->i_resp_mem_store_fault(i_resp_mem_store_fault);
    cache0->i_pmp_ena(w_pmp_ena);
    cache0->i_pmp_we(w_pmp_we);
    cache0->i_pmp_region(wb_pmp_region);
    cache0->i_pmp_start_addr(wb_pmp_start_addr);
    cache0->i_pmp_end_addr(wb_pmp_end_addr);
    cache0->i_pmp_flags(wb_pmp_flags);
    cache0->i_req_snoop_valid(i_req_snoop_valid);
    cache0->i_req_snoop_type(i_req_snoop_type);
    cache0->o_req_snoop_ready(o_req_snoop_ready);
    cache0->i_req_snoop_addr(i_req_snoop_addr);
    cache0->i_resp_snoop_ready(i_resp_snoop_ready);
    cache0->o_resp_snoop_valid(o_resp_snoop_valid);
    cache0->o_resp_snoop_data(o_resp_snoop_data);
    cache0->o_resp_snoop_flags(o_resp_snoop_flags);
    cache0->i_flushi_valid(w_flushi_valid);
    cache0->i_flushi_addr(wb_flushi_addr);
    cache0->i_flushd_valid(w_flushd_valid);
    cache0->i_flushd_addr(wb_flushd_addr);
    cache0->o_flushd_end(w_flushd_end);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mtimer;
    sensitive << i_req_mem_ready;
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_path;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_mem_load_fault;
    sensitive << i_resp_mem_store_fault;
    sensitive << i_req_snoop_valid;
    sensitive << i_req_snoop_type;
    sensitive << i_req_snoop_addr;
    sensitive << i_resp_snoop_ready;
    sensitive << i_irq_pending;
    sensitive << i_haltreq;
    sensitive << i_resumereq;
    sensitive << i_dport_req_valid;
    sensitive << i_dport_type;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << i_dport_size;
    sensitive << i_dport_resp_ready;
    sensitive << i_progbuf;
    sensitive << w_req_ctrl_ready;
    sensitive << w_req_ctrl_valid;
    sensitive << wb_req_ctrl_addr;
    sensitive << w_resp_ctrl_valid;
    sensitive << wb_resp_ctrl_addr;
    sensitive << wb_resp_ctrl_data;
    sensitive << w_resp_ctrl_load_fault;
    sensitive << w_resp_ctrl_ready;
    sensitive << w_req_data_ready;
    sensitive << w_req_data_valid;
    sensitive << wb_req_data_type;
    sensitive << wb_req_data_addr;
    sensitive << wb_req_data_wdata;
    sensitive << wb_req_data_wstrb;
    sensitive << wb_req_data_size;
    sensitive << w_resp_data_valid;
    sensitive << wb_resp_data_addr;
    sensitive << wb_resp_data_data;
    sensitive << w_resp_data_load_fault;
    sensitive << w_resp_data_store_fault;
    sensitive << w_resp_data_ready;
    sensitive << w_pmp_ena;
    sensitive << w_pmp_we;
    sensitive << wb_pmp_region;
    sensitive << wb_pmp_start_addr;
    sensitive << wb_pmp_end_addr;
    sensitive << wb_pmp_flags;
    sensitive << w_flushi_valid;
    sensitive << wb_flushi_addr;
    sensitive << w_flushd_valid;
    sensitive << wb_flushd_addr;
    sensitive << w_flushd_end;
}

RiverTop::~RiverTop() {
    if (proc0) {
        delete proc0;
    }
    if (cache0) {
        delete cache0;
    }
}

void RiverTop::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_mtimer, i_mtimer.name());
        sc_trace(o_vcd, i_req_mem_ready, i_req_mem_ready.name());
        sc_trace(o_vcd, o_req_mem_path, o_req_mem_path.name());
        sc_trace(o_vcd, o_req_mem_valid, o_req_mem_valid.name());
        sc_trace(o_vcd, o_req_mem_type, o_req_mem_type.name());
        sc_trace(o_vcd, o_req_mem_size, o_req_mem_size.name());
        sc_trace(o_vcd, o_req_mem_addr, o_req_mem_addr.name());
        sc_trace(o_vcd, o_req_mem_strob, o_req_mem_strob.name());
        sc_trace(o_vcd, o_req_mem_data, o_req_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_valid, i_resp_mem_valid.name());
        sc_trace(o_vcd, i_resp_mem_path, i_resp_mem_path.name());
        sc_trace(o_vcd, i_resp_mem_data, i_resp_mem_data.name());
        sc_trace(o_vcd, i_resp_mem_load_fault, i_resp_mem_load_fault.name());
        sc_trace(o_vcd, i_resp_mem_store_fault, i_resp_mem_store_fault.name());
        sc_trace(o_vcd, i_req_snoop_valid, i_req_snoop_valid.name());
        sc_trace(o_vcd, i_req_snoop_type, i_req_snoop_type.name());
        sc_trace(o_vcd, o_req_snoop_ready, o_req_snoop_ready.name());
        sc_trace(o_vcd, i_req_snoop_addr, i_req_snoop_addr.name());
        sc_trace(o_vcd, i_resp_snoop_ready, i_resp_snoop_ready.name());
        sc_trace(o_vcd, o_resp_snoop_valid, o_resp_snoop_valid.name());
        sc_trace(o_vcd, o_resp_snoop_data, o_resp_snoop_data.name());
        sc_trace(o_vcd, o_resp_snoop_flags, o_resp_snoop_flags.name());
        sc_trace(o_vcd, o_flush_l2, o_flush_l2.name());
        sc_trace(o_vcd, i_irq_pending, i_irq_pending.name());
        sc_trace(o_vcd, i_haltreq, i_haltreq.name());
        sc_trace(o_vcd, i_resumereq, i_resumereq.name());
        sc_trace(o_vcd, i_dport_req_valid, i_dport_req_valid.name());
        sc_trace(o_vcd, i_dport_type, i_dport_type.name());
        sc_trace(o_vcd, i_dport_addr, i_dport_addr.name());
        sc_trace(o_vcd, i_dport_wdata, i_dport_wdata.name());
        sc_trace(o_vcd, i_dport_size, i_dport_size.name());
        sc_trace(o_vcd, o_dport_req_ready, o_dport_req_ready.name());
        sc_trace(o_vcd, i_dport_resp_ready, i_dport_resp_ready.name());
        sc_trace(o_vcd, o_dport_resp_valid, o_dport_resp_valid.name());
        sc_trace(o_vcd, o_dport_resp_error, o_dport_resp_error.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());
        sc_trace(o_vcd, i_progbuf, i_progbuf.name());
        sc_trace(o_vcd, o_halted, o_halted.name());
    }

    if (proc0) {
        proc0->generateVCD(i_vcd, o_vcd);
    }
    if (cache0) {
        cache0->generateVCD(i_vcd, o_vcd);
    }
}

void RiverTop::comb() {
    o_flush_l2 = w_flushd_end;
}

}  // namespace debugger

