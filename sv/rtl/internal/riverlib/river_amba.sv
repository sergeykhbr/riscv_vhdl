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

`timescale 1ns/10ps

module RiverAmba #(
    parameter logic async_reset = 1'b0,
    parameter int unsigned hartid = 0,
    parameter bit fpu_ena = 1,
    parameter bit coherence_ena = 0,
    parameter bit tracer_ena = 1
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [63:0] i_mtimer,                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    input types_river_pkg::axi4_l1_in_type i_msti,
    output types_river_pkg::axi4_l1_out_type o_msto,
    input types_river_pkg::dport_in_type i_dport,
    output types_river_pkg::dport_out_type o_dport,
    input logic [river_cfg_pkg::IRQ_TOTAL-1:0] i_irq_pending,// Per Hart pending interrupts pins
    output logic o_flush_l2,                                // Flush L2 after D$ has been finished
    output logic o_halted,                                  // CPU halted via debug interface
    output logic o_available,                               // CPU was instantitated of stubbed
    input logic [(32 * river_cfg_pkg::CFG_PROGBUF_REG_TOTAL)-1:0] i_progbuf// progam buffer
);

import types_amba_pkg::*;
import types_river_pkg::*;
import river_cfg_pkg::*;
logic req_mem_ready_i;
logic req_mem_path_o;
logic req_mem_valid_o;
logic [REQ_MEM_TYPE_BITS-1:0] req_mem_type_o;
logic [2:0] req_mem_size_o;
logic [CFG_CPU_ADDR_BITS-1:0] req_mem_addr_o;
logic [L1CACHE_BYTES_PER_LINE-1:0] req_mem_strob_o;
logic [L1CACHE_LINE_BITS-1:0] req_mem_data_o;
logic [L1CACHE_LINE_BITS-1:0] resp_mem_data_i;
logic resp_mem_path_i;
logic resp_mem_valid_i;
logic resp_mem_load_fault_i;
logic resp_mem_store_fault_i;
// D$ Snoop interface
logic req_snoop_valid_i;
logic [SNOOP_REQ_TYPE_BITS-1:0] req_snoop_type_i;
logic req_snoop_ready_o;
logic [CFG_CPU_ADDR_BITS-1:0] req_snoop_addr_i;
logic resp_snoop_ready_i;
logic resp_snoop_valid_o;
logic [L1CACHE_LINE_BITS-1:0] resp_snoop_data_o;
logic [DTAG_FL_TOTAL-1:0] resp_snoop_flags_o;
logic w_dporti_haltreq;
logic w_dporti_resumereq;
logic w_dporti_resethaltreq;
logic w_dporti_hartreset;
logic w_dporti_req_valid;
logic [DPortReq_Total-1:0] wb_dporti_dtype;
logic [RISCV_ARCH-1:0] wb_dporti_addr;
logic [RISCV_ARCH-1:0] wb_dporti_wdata;
logic [2:0] wb_dporti_size;
logic w_dporti_resp_ready;
logic w_dporto_req_ready;
logic w_dporto_resp_valid;
logic w_dporto_resp_error;
logic [RISCV_ARCH-1:0] wb_dporto_rdata;

RiverTop #(
    .async_reset(async_reset),
    .hartid(hartid),
    .fpu_ena(fpu_ena),
    .coherence_ena(coherence_ena),
    .tracer_ena(tracer_ena)
) river0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mtimer(i_mtimer),
    .i_req_mem_ready(req_mem_ready_i),
    .o_req_mem_path(req_mem_path_o),
    .o_req_mem_valid(req_mem_valid_o),
    .o_req_mem_type(req_mem_type_o),
    .o_req_mem_size(req_mem_size_o),
    .o_req_mem_addr(req_mem_addr_o),
    .o_req_mem_strob(req_mem_strob_o),
    .o_req_mem_data(req_mem_data_o),
    .i_resp_mem_valid(resp_mem_valid_i),
    .i_resp_mem_path(resp_mem_path_i),
    .i_resp_mem_data(resp_mem_data_i),
    .i_resp_mem_load_fault(resp_mem_load_fault_i),
    .i_resp_mem_store_fault(resp_mem_store_fault_i),
    .i_req_snoop_valid(req_snoop_valid_i),
    .i_req_snoop_type(req_snoop_type_i),
    .o_req_snoop_ready(req_snoop_ready_o),
    .i_req_snoop_addr(req_snoop_addr_i),
    .i_resp_snoop_ready(resp_snoop_ready_i),
    .o_resp_snoop_valid(resp_snoop_valid_o),
    .o_resp_snoop_data(resp_snoop_data_o),
    .o_resp_snoop_flags(resp_snoop_flags_o),
    .o_flush_l2(o_flush_l2),
    .i_irq_pending(i_irq_pending),
    .i_haltreq(w_dporti_haltreq),
    .i_resumereq(w_dporti_resumereq),
    .i_dport_req_valid(w_dporti_req_valid),
    .i_dport_type(wb_dporti_dtype),
    .i_dport_addr(wb_dporti_addr),
    .i_dport_wdata(wb_dporti_wdata),
    .i_dport_size(wb_dporti_size),
    .o_dport_req_ready(w_dporto_req_ready),
    .i_dport_resp_ready(w_dporti_resp_ready),
    .o_dport_resp_valid(w_dporto_resp_valid),
    .o_dport_resp_error(w_dporto_resp_error),
    .o_dport_rdata(wb_dporto_rdata),
    .i_progbuf(i_progbuf),
    .o_halted(o_halted)
);

l1_dma_snoop #(
    .abits(CFG_CPU_ADDR_BITS),
    .async_reset(async_reset),
    .userbits(1),
    .base_offset(48'h000000000000),
    .coherence_ena(coherence_ena)
) l1dma0 (
    .i_nrst(i_nrst),
    .i_clk(i_clk),
    .o_req_mem_ready(req_mem_ready_i),
    .i_req_mem_path(req_mem_path_o),
    .i_req_mem_valid(req_mem_valid_o),
    .i_req_mem_type(req_mem_type_o),
    .i_req_mem_size(req_mem_size_o),
    .i_req_mem_addr(req_mem_addr_o),
    .i_req_mem_strob(req_mem_strob_o),
    .i_req_mem_data(req_mem_data_o),
    .o_resp_mem_path(resp_mem_path_i),
    .o_resp_mem_valid(resp_mem_valid_i),
    .o_resp_mem_load_fault(resp_mem_load_fault_i),
    .o_resp_mem_store_fault(resp_mem_store_fault_i),
    .o_resp_mem_data(resp_mem_data_i),
    .o_req_snoop_valid(req_snoop_valid_i),
    .o_req_snoop_type(req_snoop_type_i),
    .i_req_snoop_ready(req_snoop_ready_o),
    .o_req_snoop_addr(req_snoop_addr_i),
    .o_resp_snoop_ready(resp_snoop_ready_i),
    .i_resp_snoop_valid(resp_snoop_valid_o),
    .i_resp_snoop_data(resp_snoop_data_o),
    .i_resp_snoop_flags(resp_snoop_flags_o),
    .i_msti(i_msti),
    .o_msto(o_msto)
);

always_comb
begin: comb_proc
    dport_out_type vdporto;

    vdporto = dport_out_none;


    w_dporti_haltreq = i_dport.haltreq;                     // systemc compatibility
    w_dporti_resumereq = i_dport.resumereq;                 // systemc compatibility
    w_dporti_resethaltreq = i_dport.resethaltreq;           // systemc compatibility
    w_dporti_hartreset = i_dport.hartreset;                 // systemc compatibility
    w_dporti_req_valid = i_dport.req_valid;                 // systemc compatibility
    wb_dporti_dtype = i_dport.dtype;                        // systemc compatibility
    wb_dporti_addr = i_dport.addr;                          // systemc compatibility
    wb_dporti_wdata = i_dport.wdata;                        // systemc compatibility
    wb_dporti_size = i_dport.size;                          // systemc compatibility
    w_dporti_resp_ready = i_dport.resp_ready;               // systemc compatibility

    vdporto.req_ready = w_dporto_req_ready;                 // systemc compatibility
    vdporto.resp_valid = w_dporto_resp_valid;               // systemc compatibility
    vdporto.resp_error = w_dporto_resp_error;               // systemc compatibility
    vdporto.rdata = wb_dporto_rdata;                        // systemc compatibility

    o_dport = vdporto;                                      // systemc compatibility
    o_available = 1'b1;
end: comb_proc

endmodule: RiverAmba
