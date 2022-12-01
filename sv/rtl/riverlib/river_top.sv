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

module RiverTop #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned hartid = 0,
    parameter bit fpu_ena = 1'b1,
    parameter bit coherence_ena = 1'b0,
    parameter bit tracer_ena = 1'b1,
    parameter int unsigned ilog2_nways = 2,                 // I$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned ilog2_lines_per_way = 7,         // I$ Cache length: 7=16KB; 8=32KB; ..
    parameter int unsigned dlog2_nways = 2,                 // D$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned dlog2_lines_per_way = 7          // D$ Cache length: 7=16KB; 8=32KB; ..
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [63:0] i_mtimer,                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    // Memory interface:
    input logic i_req_mem_ready,                            // System Bus is ready to accept memory operation request
    output logic o_req_mem_path,                            // 0=ctrl; 1=data path
    output logic o_req_mem_valid,                           // AXI memory request is valid
    output logic [river_cfg_pkg::REQ_MEM_TYPE_BITS-1:0] o_req_mem_type,// AXI memory request type
    output logic [2:0] o_req_mem_size,                      // request size: 0=1 B;...; 7=128 B
    output logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] o_req_mem_addr,// AXI memory request address
    output logic [river_cfg_pkg::L1CACHE_BYTES_PER_LINE-1:0] o_req_mem_strob,// Writing strob. 1 bit per Byte (uncached only)
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_req_mem_data,// Writing data
    input logic i_resp_mem_valid,                           // AXI response is valid
    input logic i_resp_mem_path,                            // 0=ctrl; 1=data path
    input logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] i_resp_mem_data,// Read data
    input logic i_resp_mem_load_fault,                      // data load error
    input logic i_resp_mem_store_fault,                     // data store error
    // $D Snoop interface:
    input logic i_req_snoop_valid,
    input logic [river_cfg_pkg::SNOOP_REQ_TYPE_BITS-1:0] i_req_snoop_type,
    output logic o_req_snoop_ready,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_req_snoop_addr,
    input logic i_resp_snoop_ready,
    output logic o_resp_snoop_valid,
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_resp_snoop_data,
    output logic [river_cfg_pkg::DTAG_FL_TOTAL-1:0] o_resp_snoop_flags,
    output logic o_flush_l2,                                // Flush L2 after D$ has been finished
    // Interrupt lines:
    input logic [river_cfg_pkg::IRQ_TOTAL-1:0] i_irq_pending,// Per Hart pending interrupts pins
    // Debug interface:
    input logic i_haltreq,                                  // DMI: halt request from debug unit
    input logic i_resumereq,                                // DMI: resume request from debug unit
    input logic i_dport_req_valid,                          // Debug access from DSU is valid
    input logic [river_cfg_pkg::DPortReq_Total-1:0] i_dport_type,// Debug access type
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_addr,// dport address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_wdata,// Write value
    input logic [2:0] i_dport_size,                         // reg/mem access size:0=1B;...,4=128B;
    output logic o_dport_req_ready,
    input logic i_dport_resp_ready,                         // ready to accepd response
    output logic o_dport_resp_valid,                        // Response is valid
    output logic o_dport_resp_error,                        // Something wrong during command execution
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_dport_rdata,// Response value
    input logic [(32 * river_cfg_pkg::CFG_PROGBUF_REG_TOTAL)-1:0] i_progbuf,// progam buffer
    output logic o_halted                                   // CPU halted via debug interface
);

import river_cfg_pkg::*;
import river_top_pkg::*;

// Control path:
logic w_req_ctrl_ready;
logic w_req_ctrl_valid;
logic [RISCV_ARCH-1:0] wb_req_ctrl_addr;
logic w_resp_ctrl_valid;
logic [RISCV_ARCH-1:0] wb_resp_ctrl_addr;
logic [63:0] wb_resp_ctrl_data;
logic w_resp_ctrl_load_fault;
logic w_resp_ctrl_ready;
// Data path:
logic w_req_data_ready;
logic w_req_data_valid;
logic [MemopType_Total-1:0] wb_req_data_type;
logic [RISCV_ARCH-1:0] wb_req_data_addr;
logic [63:0] wb_req_data_wdata;
logic [7:0] wb_req_data_wstrb;
logic [1:0] wb_req_data_size;
logic w_resp_data_valid;
logic [RISCV_ARCH-1:0] wb_resp_data_addr;
logic [63:0] wb_resp_data_data;
logic w_resp_data_load_fault;
logic w_resp_data_store_fault;
logic w_resp_data_ready;
logic w_pmp_ena;
logic w_pmp_we;
logic [CFG_PMP_TBL_WIDTH-1:0] wb_pmp_region;
logic [RISCV_ARCH-1:0] wb_pmp_start_addr;
logic [RISCV_ARCH-1:0] wb_pmp_end_addr;
logic [CFG_PMP_FL_TOTAL-1:0] wb_pmp_flags;
logic w_flushi_valid;
logic [RISCV_ARCH-1:0] wb_flushi_addr;
logic w_flushd_valid;
logic [RISCV_ARCH-1:0] wb_flushd_addr;
logic w_flushd_end;

Processor #(
    .async_reset(async_reset),
    .hartid(hartid),
    .fpu_ena(fpu_ena),
    .tracer_ena(tracer_ena)
) proc0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mtimer(i_mtimer),
    .i_req_ctrl_ready(w_req_ctrl_ready),
    .o_req_ctrl_valid(w_req_ctrl_valid),
    .o_req_ctrl_addr(wb_req_ctrl_addr),
    .i_resp_ctrl_valid(w_resp_ctrl_valid),
    .i_resp_ctrl_addr(wb_resp_ctrl_addr),
    .i_resp_ctrl_data(wb_resp_ctrl_data),
    .i_resp_ctrl_load_fault(w_resp_ctrl_load_fault),
    .o_resp_ctrl_ready(w_resp_ctrl_ready),
    .i_req_data_ready(w_req_data_ready),
    .o_req_data_valid(w_req_data_valid),
    .o_req_data_type(wb_req_data_type),
    .o_req_data_addr(wb_req_data_addr),
    .o_req_data_wdata(wb_req_data_wdata),
    .o_req_data_wstrb(wb_req_data_wstrb),
    .o_req_data_size(wb_req_data_size),
    .i_resp_data_valid(w_resp_data_valid),
    .i_resp_data_addr(wb_resp_data_addr),
    .i_resp_data_data(wb_resp_data_data),
    .i_resp_data_load_fault(w_resp_data_load_fault),
    .i_resp_data_store_fault(w_resp_data_store_fault),
    .o_resp_data_ready(w_resp_data_ready),
    .i_irq_pending(i_irq_pending),
    .o_pmp_ena(w_pmp_ena),
    .o_pmp_we(w_pmp_we),
    .o_pmp_region(wb_pmp_region),
    .o_pmp_start_addr(wb_pmp_start_addr),
    .o_pmp_end_addr(wb_pmp_end_addr),
    .o_pmp_flags(wb_pmp_flags),
    .i_haltreq(i_haltreq),
    .i_resumereq(i_resumereq),
    .i_dport_req_valid(i_dport_req_valid),
    .i_dport_type(i_dport_type),
    .i_dport_addr(i_dport_addr),
    .i_dport_wdata(i_dport_wdata),
    .i_dport_size(i_dport_size),
    .o_dport_req_ready(o_dport_req_ready),
    .i_dport_resp_ready(i_dport_resp_ready),
    .o_dport_resp_valid(o_dport_resp_valid),
    .o_dport_resp_error(o_dport_resp_error),
    .o_dport_rdata(o_dport_rdata),
    .i_progbuf(i_progbuf),
    .o_halted(o_halted),
    .o_flushi_valid(w_flushi_valid),
    .o_flushi_addr(wb_flushi_addr),
    .o_flushd_valid(w_flushd_valid),
    .o_flushd_addr(wb_flushd_addr),
    .i_flushd_end(w_flushd_end)
);


CacheTop #(
    .async_reset(async_reset),
    .coherence_ena(coherence_ena),
    .ilog2_nways(ilog2_nways),
    .ilog2_lines_per_way(ilog2_lines_per_way),
    .dlog2_nways(dlog2_nways),
    .dlog2_lines_per_way(dlog2_lines_per_way)
) cache0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_req_ctrl_valid(w_req_ctrl_valid),
    .i_req_ctrl_addr(wb_req_ctrl_addr),
    .o_req_ctrl_ready(w_req_ctrl_ready),
    .o_resp_ctrl_valid(w_resp_ctrl_valid),
    .o_resp_ctrl_addr(wb_resp_ctrl_addr),
    .o_resp_ctrl_data(wb_resp_ctrl_data),
    .o_resp_ctrl_load_fault(w_resp_ctrl_load_fault),
    .i_resp_ctrl_ready(w_resp_ctrl_ready),
    .i_req_data_valid(w_req_data_valid),
    .i_req_data_type(wb_req_data_type),
    .i_req_data_addr(wb_req_data_addr),
    .i_req_data_wdata(wb_req_data_wdata),
    .i_req_data_wstrb(wb_req_data_wstrb),
    .i_req_data_size(wb_req_data_size),
    .o_req_data_ready(w_req_data_ready),
    .o_resp_data_valid(w_resp_data_valid),
    .o_resp_data_addr(wb_resp_data_addr),
    .o_resp_data_data(wb_resp_data_data),
    .o_resp_data_load_fault(w_resp_data_load_fault),
    .o_resp_data_store_fault(w_resp_data_store_fault),
    .i_resp_data_ready(w_resp_data_ready),
    .i_req_mem_ready(i_req_mem_ready),
    .o_req_mem_path(o_req_mem_path),
    .o_req_mem_valid(o_req_mem_valid),
    .o_req_mem_type(o_req_mem_type),
    .o_req_mem_size(o_req_mem_size),
    .o_req_mem_addr(o_req_mem_addr),
    .o_req_mem_strob(o_req_mem_strob),
    .o_req_mem_data(o_req_mem_data),
    .i_resp_mem_valid(i_resp_mem_valid),
    .i_resp_mem_path(i_resp_mem_path),
    .i_resp_mem_data(i_resp_mem_data),
    .i_resp_mem_load_fault(i_resp_mem_load_fault),
    .i_resp_mem_store_fault(i_resp_mem_store_fault),
    .i_pmp_ena(w_pmp_ena),
    .i_pmp_we(w_pmp_we),
    .i_pmp_region(wb_pmp_region),
    .i_pmp_start_addr(wb_pmp_start_addr),
    .i_pmp_end_addr(wb_pmp_end_addr),
    .i_pmp_flags(wb_pmp_flags),
    .i_req_snoop_valid(i_req_snoop_valid),
    .i_req_snoop_type(i_req_snoop_type),
    .o_req_snoop_ready(o_req_snoop_ready),
    .i_req_snoop_addr(i_req_snoop_addr),
    .i_resp_snoop_ready(i_resp_snoop_ready),
    .o_resp_snoop_valid(o_resp_snoop_valid),
    .o_resp_snoop_data(o_resp_snoop_data),
    .o_resp_snoop_flags(o_resp_snoop_flags),
    .i_flushi_valid(w_flushi_valid),
    .i_flushi_addr(wb_flushi_addr),
    .i_flushd_valid(w_flushd_valid),
    .i_flushd_addr(wb_flushd_addr),
    .o_flushd_end(w_flushd_end)
);


always_comb
begin: comb_proc
    o_flush_l2 = w_flushd_end;
end: comb_proc

endmodule: RiverTop
