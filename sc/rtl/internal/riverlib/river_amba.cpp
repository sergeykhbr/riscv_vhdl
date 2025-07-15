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

#include "river_amba.h"
#include "api_core.h"

namespace debugger {

RiverAmba::RiverAmba(sc_module_name name,
                     bool async_reset,
                     uint32_t hartid,
                     bool fpu_ena,
                     bool coherence_ena,
                     bool tracer_ena)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mtimer("i_mtimer"),
    i_msti("i_msti"),
    o_msto("o_msto"),
    i_dport("i_dport"),
    o_dport("o_dport"),
    i_irq_pending("i_irq_pending"),
    o_flush_l2("o_flush_l2"),
    o_halted("o_halted"),
    o_available("o_available"),
    i_progbuf("i_progbuf") {

    async_reset_ = async_reset;
    hartid_ = hartid;
    fpu_ena_ = fpu_ena;
    coherence_ena_ = coherence_ena;
    tracer_ena_ = tracer_ena;
    river0 = 0;
    l1dma0 = 0;

    river0 = new RiverTop("river0",
                           async_reset,
                           hartid,
                           fpu_ena,
                           coherence_ena,
                           tracer_ena);
    river0->i_clk(i_clk);
    river0->i_nrst(i_nrst);
    river0->i_mtimer(i_mtimer);
    river0->i_req_mem_ready(req_mem_ready_i);
    river0->o_req_mem_path(req_mem_path_o);
    river0->o_req_mem_valid(req_mem_valid_o);
    river0->o_req_mem_type(req_mem_type_o);
    river0->o_req_mem_size(req_mem_size_o);
    river0->o_req_mem_addr(req_mem_addr_o);
    river0->o_req_mem_strob(req_mem_strob_o);
    river0->o_req_mem_data(req_mem_data_o);
    river0->i_resp_mem_valid(resp_mem_valid_i);
    river0->i_resp_mem_path(resp_mem_path_i);
    river0->i_resp_mem_data(resp_mem_data_i);
    river0->i_resp_mem_load_fault(resp_mem_load_fault_i);
    river0->i_resp_mem_store_fault(resp_mem_store_fault_i);
    river0->i_req_snoop_valid(req_snoop_valid_i);
    river0->i_req_snoop_type(req_snoop_type_i);
    river0->o_req_snoop_ready(req_snoop_ready_o);
    river0->i_req_snoop_addr(req_snoop_addr_i);
    river0->i_resp_snoop_ready(resp_snoop_ready_i);
    river0->o_resp_snoop_valid(resp_snoop_valid_o);
    river0->o_resp_snoop_data(resp_snoop_data_o);
    river0->o_resp_snoop_flags(resp_snoop_flags_o);
    river0->o_flush_l2(o_flush_l2);
    river0->i_irq_pending(i_irq_pending);
    river0->i_haltreq(w_dporti_haltreq);
    river0->i_resumereq(w_dporti_resumereq);
    river0->i_dport_req_valid(w_dporti_req_valid);
    river0->i_dport_type(wb_dporti_dtype);
    river0->i_dport_addr(wb_dporti_addr);
    river0->i_dport_wdata(wb_dporti_wdata);
    river0->i_dport_size(wb_dporti_size);
    river0->o_dport_req_ready(w_dporto_req_ready);
    river0->i_dport_resp_ready(w_dporti_resp_ready);
    river0->o_dport_resp_valid(w_dporto_resp_valid);
    river0->o_dport_resp_error(w_dporto_resp_error);
    river0->o_dport_rdata(wb_dporto_rdata);
    river0->i_progbuf(i_progbuf);
    river0->o_halted(o_halted);

    l1dma0 = new l1_dma_snoop<CFG_CPU_ADDR_BITS>("l1dma0",
                                                 async_reset,
                                                 1,
                                                 0x000000000000,
                                                 coherence_ena);
    l1dma0->i_nrst(i_nrst);
    l1dma0->i_clk(i_clk);
    l1dma0->o_req_mem_ready(req_mem_ready_i);
    l1dma0->i_req_mem_path(req_mem_path_o);
    l1dma0->i_req_mem_valid(req_mem_valid_o);
    l1dma0->i_req_mem_type(req_mem_type_o);
    l1dma0->i_req_mem_size(req_mem_size_o);
    l1dma0->i_req_mem_addr(req_mem_addr_o);
    l1dma0->i_req_mem_strob(req_mem_strob_o);
    l1dma0->i_req_mem_data(req_mem_data_o);
    l1dma0->o_resp_mem_path(resp_mem_path_i);
    l1dma0->o_resp_mem_valid(resp_mem_valid_i);
    l1dma0->o_resp_mem_load_fault(resp_mem_load_fault_i);
    l1dma0->o_resp_mem_store_fault(resp_mem_store_fault_i);
    l1dma0->o_resp_mem_data(resp_mem_data_i);
    l1dma0->o_req_snoop_valid(req_snoop_valid_i);
    l1dma0->o_req_snoop_type(req_snoop_type_i);
    l1dma0->i_req_snoop_ready(req_snoop_ready_o);
    l1dma0->o_req_snoop_addr(req_snoop_addr_i);
    l1dma0->o_resp_snoop_ready(resp_snoop_ready_i);
    l1dma0->i_resp_snoop_valid(resp_snoop_valid_o);
    l1dma0->i_resp_snoop_data(resp_snoop_data_o);
    l1dma0->i_resp_snoop_flags(resp_snoop_flags_o);
    l1dma0->i_msti(i_msti);
    l1dma0->o_msto(o_msto);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mtimer;
    sensitive << i_msti;
    sensitive << i_dport;
    sensitive << i_irq_pending;
    sensitive << i_progbuf;
    sensitive << req_mem_ready_i;
    sensitive << req_mem_path_o;
    sensitive << req_mem_valid_o;
    sensitive << req_mem_type_o;
    sensitive << req_mem_size_o;
    sensitive << req_mem_addr_o;
    sensitive << req_mem_strob_o;
    sensitive << req_mem_data_o;
    sensitive << resp_mem_data_i;
    sensitive << resp_mem_path_i;
    sensitive << resp_mem_valid_i;
    sensitive << resp_mem_load_fault_i;
    sensitive << resp_mem_store_fault_i;
    sensitive << req_snoop_valid_i;
    sensitive << req_snoop_type_i;
    sensitive << req_snoop_ready_o;
    sensitive << req_snoop_addr_i;
    sensitive << resp_snoop_ready_i;
    sensitive << resp_snoop_valid_o;
    sensitive << resp_snoop_data_o;
    sensitive << resp_snoop_flags_o;
    sensitive << w_dporti_haltreq;
    sensitive << w_dporti_resumereq;
    sensitive << w_dporti_resethaltreq;
    sensitive << w_dporti_hartreset;
    sensitive << w_dporti_req_valid;
    sensitive << wb_dporti_dtype;
    sensitive << wb_dporti_addr;
    sensitive << wb_dporti_wdata;
    sensitive << wb_dporti_size;
    sensitive << w_dporti_resp_ready;
    sensitive << w_dporto_req_ready;
    sensitive << w_dporto_resp_valid;
    sensitive << w_dporto_resp_error;
    sensitive << wb_dporto_rdata;
}

RiverAmba::~RiverAmba() {
    if (river0) {
        delete river0;
    }
    if (l1dma0) {
        delete l1dma0;
    }
}

void RiverAmba::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_mtimer, i_mtimer.name());
        sc_trace(o_vcd, i_msti, i_msti.name());
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, i_dport, i_dport.name());
        sc_trace(o_vcd, o_dport, o_dport.name());
        sc_trace(o_vcd, i_irq_pending, i_irq_pending.name());
        sc_trace(o_vcd, o_flush_l2, o_flush_l2.name());
        sc_trace(o_vcd, o_halted, o_halted.name());
        sc_trace(o_vcd, o_available, o_available.name());
        sc_trace(o_vcd, i_progbuf, i_progbuf.name());
    }

    if (river0) {
        river0->generateVCD(i_vcd, o_vcd);
    }
    if (l1dma0) {
        l1dma0->generateVCD(i_vcd, o_vcd);
    }
}

void RiverAmba::comb() {
    dport_out_type vdporto;

    vdporto = dport_out_none;


    w_dporti_haltreq = i_dport.read().haltreq;              // systemc compatibility
    w_dporti_resumereq = i_dport.read().resumereq;          // systemc compatibility
    w_dporti_resethaltreq = i_dport.read().resethaltreq;    // systemc compatibility
    w_dporti_hartreset = i_dport.read().hartreset;          // systemc compatibility
    w_dporti_req_valid = i_dport.read().req_valid;          // systemc compatibility
    wb_dporti_dtype = i_dport.read().dtype;                 // systemc compatibility
    wb_dporti_addr = i_dport.read().addr;                   // systemc compatibility
    wb_dporti_wdata = i_dport.read().wdata;                 // systemc compatibility
    wb_dporti_size = i_dport.read().size;                   // systemc compatibility
    w_dporti_resp_ready = i_dport.read().resp_ready;        // systemc compatibility

    vdporto.req_ready = w_dporto_req_ready.read();          // systemc compatibility
    vdporto.resp_valid = w_dporto_resp_valid.read();        // systemc compatibility
    vdporto.resp_error = w_dporto_resp_error.read();        // systemc compatibility
    vdporto.rdata = wb_dporto_rdata.read();                 // systemc compatibility

    o_dport = vdporto;                                      // systemc compatibility
    o_available = 1;
}

}  // namespace debugger

