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

module CacheTop #(
    parameter bit async_reset = 1'b0,
    parameter bit coherence_ena = 1'b0,
    parameter int unsigned ilog2_nways = 2,                 // I$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned ilog2_lines_per_way = 7,         // I$ Cache length: 7=16KB; 8=32KB; ..
    parameter int unsigned dlog2_nways = 2,                 // D$ Cache associativity. Default bits width = 2, means 4 ways
    parameter int unsigned dlog2_lines_per_way = 7          // D$ Cache length: 7=16KB; 8=32KB; ..
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // Control path:
    input logic i_req_ctrl_valid,                           // Control request from CPU Core is valid
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_req_ctrl_addr,// Control request address
    output logic o_req_ctrl_ready,                          // Control request from CPU Core is accepted
    output logic o_resp_ctrl_valid,                         // ICache response is valid and can be accepted
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_resp_ctrl_addr,// ICache response address
    output logic [63:0] o_resp_ctrl_data,                   // ICache read data
    output logic o_resp_ctrl_load_fault,                    // Bus response ERRSLV or ERRDEC on read
    input logic i_resp_ctrl_ready,                          // CPU Core is ready to accept ICache response
    // Data path:
    input logic i_req_data_valid,                           // Data path request from CPU Core is valid
    input logic [river_cfg_pkg::MemopType_Total-1:0] i_req_data_type,// Data write memopy operation flag
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_req_data_addr,// Memory operation address
    input logic [63:0] i_req_data_wdata,                    // Memory operation write value
    input logic [7:0] i_req_data_wstrb,                     // 8-bytes aligned strob
    input logic [1:0] i_req_data_size,
    output logic o_req_data_ready,                          // Memory operation request accepted by DCache
    output logic o_resp_data_valid,                         // DCache response is ready
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_resp_data_addr,// DCache response address
    output logic [63:0] o_resp_data_data,                   // DCache response read data
    output logic o_resp_data_load_fault,                    // Bus response ERRSLV or ERRDEC on read
    output logic o_resp_data_store_fault,                   // Bus response ERRSLV or ERRDEC on write
    input logic i_resp_data_ready,                          // CPU Core is ready to accept DCache repsonse
    // Memory interface:
    input logic i_req_mem_ready,                            // System Bus is ready to accept memory operation request
    output logic o_req_mem_path,                            // 1=ctrl; 0=data path
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
    // PMP interface:
    input logic i_pmp_ena,                                  // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    input logic i_pmp_we,                                   // write enable into PMP
    input logic [river_cfg_pkg::CFG_PMP_TBL_WIDTH-1:0] i_pmp_region,// selected PMP region
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_pmp_start_addr,// PMP region start address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_pmp_end_addr,// PMP region end address (inclusive)
    input logic [river_cfg_pkg::CFG_PMP_FL_TOTAL-1:0] i_pmp_flags,// {ena, lock, r, w, x}
    // $D Snoop interface:
    input logic i_req_snoop_valid,
    input logic [river_cfg_pkg::SNOOP_REQ_TYPE_BITS-1:0] i_req_snoop_type,
    output logic o_req_snoop_ready,
    input logic [river_cfg_pkg::CFG_CPU_ADDR_BITS-1:0] i_req_snoop_addr,
    input logic i_resp_snoop_ready,
    output logic o_resp_snoop_valid,
    output logic [river_cfg_pkg::L1CACHE_LINE_BITS-1:0] o_resp_snoop_data,
    output logic [river_cfg_pkg::DTAG_FL_TOTAL-1:0] o_resp_snoop_flags,
    // Debug signals:
    input logic i_flushi_valid,                             // address to clear icache is valid
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_flushi_addr,// clear ICache address from debug interface
    input logic i_flushd_valid,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_flushd_addr,
    output logic o_flushd_end
);

import river_cfg_pkg::*;
import cache_top_pkg::*;

logic [CFG_CPU_ADDR_BITS-1:0] wb_i_req_ctrl_addr;
logic [CFG_CPU_ADDR_BITS-1:0] wb_i_req_data_addr;
logic [CFG_CPU_ADDR_BITS-1:0] wb_i_flushi_addr;
logic [CFG_CPU_ADDR_BITS-1:0] wb_i_flushd_addr;
CacheOutputType i;
CacheOutputType d;
// Memory Control interface:
logic w_ctrl_resp_mem_data_valid;
logic [L1CACHE_LINE_BITS-1:0] wb_ctrl_resp_mem_data;
logic w_ctrl_resp_mem_load_fault;
logic w_ctrl_req_ready;
// Memory Data interface:
logic w_data_resp_mem_data_valid;
logic [L1CACHE_LINE_BITS-1:0] wb_data_resp_mem_data;
logic w_data_resp_mem_load_fault;
logic w_data_req_ready;
logic w_pma_icached;
logic w_pma_dcached;
logic w_pmp_r;
logic w_pmp_w;
logic w_pmp_x;
// Queue interface
logic queue_re_i;
logic queue_we_i;
logic [QUEUE_WIDTH-1:0] queue_wdata_i;
logic [QUEUE_WIDTH-1:0] queue_rdata_o;
logic queue_full_o;
logic queue_nempty_o;

ICacheLru #(
    .async_reset(async_reset),
    .waybits(ilog2_nways),
    .ibits(ilog2_lines_per_way)
) i1 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_req_valid(i_req_ctrl_valid),
    .i_req_addr(wb_i_req_ctrl_addr),
    .o_req_ready(o_req_ctrl_ready),
    .o_resp_valid(o_resp_ctrl_valid),
    .o_resp_addr(i.resp_addr),
    .o_resp_data(o_resp_ctrl_data),
    .o_resp_load_fault(o_resp_ctrl_load_fault),
    .i_resp_ready(i_resp_ctrl_ready),
    .i_req_mem_ready(w_ctrl_req_ready),
    .o_req_mem_valid(i.req_mem_valid),
    .o_req_mem_type(i.req_mem_type),
    .o_req_mem_size(i.req_mem_size),
    .o_req_mem_addr(i.req_mem_addr),
    .o_req_mem_strob(i.req_mem_strob),
    .o_req_mem_data(i.req_mem_wdata),
    .i_mem_data_valid(w_ctrl_resp_mem_data_valid),
    .i_mem_data(wb_ctrl_resp_mem_data),
    .i_mem_load_fault(w_ctrl_resp_mem_load_fault),
    .o_mpu_addr(i.mpu_addr),
    .i_pma_cached(w_pma_icached),
    .i_pmp_x(w_pmp_x),
    .i_flush_address(wb_i_flushi_addr),
    .i_flush_valid(i_flushi_valid)
);


DCacheLru #(
    .async_reset(async_reset),
    .waybits(dlog2_nways),
    .ibits(dlog2_lines_per_way),
    .coherence_ena(coherence_ena)
) d0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_req_valid(i_req_data_valid),
    .i_req_type(i_req_data_type),
    .i_req_addr(wb_i_req_data_addr),
    .i_req_wdata(i_req_data_wdata),
    .i_req_wstrb(i_req_data_wstrb),
    .i_req_size(i_req_data_size),
    .o_req_ready(o_req_data_ready),
    .o_resp_valid(o_resp_data_valid),
    .o_resp_addr(d.resp_addr),
    .o_resp_data(o_resp_data_data),
    .o_resp_er_load_fault(o_resp_data_load_fault),
    .o_resp_er_store_fault(o_resp_data_store_fault),
    .i_resp_ready(i_resp_data_ready),
    .i_req_mem_ready(w_data_req_ready),
    .o_req_mem_valid(d.req_mem_valid),
    .o_req_mem_type(d.req_mem_type),
    .o_req_mem_size(d.req_mem_size),
    .o_req_mem_addr(d.req_mem_addr),
    .o_req_mem_strob(d.req_mem_strob),
    .o_req_mem_data(d.req_mem_wdata),
    .i_mem_data_valid(w_data_resp_mem_data_valid),
    .i_mem_data(wb_data_resp_mem_data),
    .i_mem_load_fault(w_data_resp_mem_load_fault),
    .i_mem_store_fault(i_resp_mem_store_fault),
    .o_mpu_addr(d.mpu_addr),
    .i_pma_cached(w_pma_dcached),
    .i_pmp_r(w_pmp_r),
    .i_pmp_w(w_pmp_w),
    .i_req_snoop_valid(i_req_snoop_valid),
    .i_req_snoop_type(i_req_snoop_type),
    .o_req_snoop_ready(o_req_snoop_ready),
    .i_req_snoop_addr(i_req_snoop_addr),
    .i_resp_snoop_ready(i_resp_snoop_ready),
    .o_resp_snoop_valid(o_resp_snoop_valid),
    .o_resp_snoop_data(o_resp_snoop_data),
    .o_resp_snoop_flags(o_resp_snoop_flags),
    .i_flush_address(wb_i_flushd_addr),
    .i_flush_valid(i_flushd_valid),
    .o_flush_end(o_flushd_end)
);


PMA pma0 (
    .i_clk(i_clk),
    .i_iaddr(i.mpu_addr),
    .i_daddr(d.mpu_addr),
    .o_icached(w_pma_icached),
    .o_dcached(w_pma_dcached)
);


PMP #(
    .async_reset(async_reset)
) pmp0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(i_pmp_ena),
    .i_iaddr(i.mpu_addr),
    .i_daddr(d.mpu_addr),
    .i_we(i_pmp_we),
    .i_region(i_pmp_region),
    .i_start_addr(i_pmp_start_addr),
    .i_end_addr(i_pmp_end_addr),
    .i_flags(i_pmp_flags),
    .o_r(w_pmp_r),
    .o_w(w_pmp_w),
    .o_x(w_pmp_x)
);


Queue #(
    .async_reset(async_reset),
    .abits(2),
    .dbits(QUEUE_WIDTH)
) queue0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_re(queue_re_i),
    .i_we(queue_we_i),
    .i_wdata(queue_wdata_i),
    .o_rdata(queue_rdata_o),
    .o_full(queue_full_o),
    .o_nempty(queue_nempty_o)
);


always_comb
begin: comb_proc
    logic [QUEUE_WIDTH-1:0] vb_ctrl_bus;
    logic [QUEUE_WIDTH-1:0] vb_data_bus;
    logic [QUEUE_WIDTH-1:0] vb_queue_bus;
    logic ctrl_path_id;
    logic data_path_id;
    logic v_queue_we;
    logic v_queue_re;
    logic v_req_mem_path_o;
    logic [REQ_MEM_TYPE_BITS-1:0] vb_req_mem_type_o;
    logic [2:0] vb_req_mem_size_o;
    logic [CFG_CPU_ADDR_BITS-1:0] vb_req_mem_addr_o;
    logic [RISCV_ARCH-1:0] vb_resp_ctrl_addr;
    logic [RISCV_ARCH-1:0] vb_resp_data_addr;

    vb_ctrl_bus = 55'h00000000000000;
    vb_data_bus = 55'h00000000000000;
    vb_queue_bus = 55'h00000000000000;
    ctrl_path_id = 0;
    data_path_id = 0;
    v_queue_we = 0;
    v_queue_re = 0;
    v_req_mem_path_o = 0;
    vb_req_mem_type_o = 0;
    vb_req_mem_size_o = 0;
    vb_req_mem_addr_o = 0;
    vb_resp_ctrl_addr = 0;
    vb_resp_data_addr = 0;

    wb_i_req_ctrl_addr = i_req_ctrl_addr[(CFG_CPU_ADDR_BITS - 1): 0];
    wb_i_req_data_addr = i_req_data_addr[(CFG_CPU_ADDR_BITS - 1): 0];
    wb_i_flushi_addr = i_flushi_addr[(CFG_CPU_ADDR_BITS - 1): 0];
    wb_i_flushd_addr = i_flushd_addr[(CFG_CPU_ADDR_BITS - 1): 0];
    v_queue_re = i_req_mem_ready;
    v_queue_we = (i.req_mem_valid || d.req_mem_valid);

    ctrl_path_id = CTRL_PATH;
    vb_ctrl_bus = {ctrl_path_id,
            i.req_mem_type,
            i.req_mem_size,
            i.req_mem_addr};

    data_path_id = DATA_PATH;
    vb_data_bus = {data_path_id,
            d.req_mem_type,
            d.req_mem_size,
            d.req_mem_addr};

    if (d.req_mem_valid == 1'b1) begin
        vb_queue_bus = vb_data_bus;
    end else begin
        vb_queue_bus = vb_ctrl_bus;
    end

    queue_wdata_i = vb_queue_bus;
    queue_we_i = v_queue_we;
    queue_re_i = v_queue_re;

    w_data_req_ready = 1'b1;
    w_ctrl_req_ready = (~d.req_mem_valid);
    if (i_resp_mem_path == CTRL_PATH) begin
        w_ctrl_resp_mem_data_valid = i_resp_mem_valid;
        w_ctrl_resp_mem_load_fault = i_resp_mem_load_fault;
        w_data_resp_mem_data_valid = 1'b0;
        w_data_resp_mem_load_fault = 1'b0;
    end else begin
        w_ctrl_resp_mem_data_valid = 1'b0;
        w_ctrl_resp_mem_load_fault = 1'b0;
        w_data_resp_mem_data_valid = i_resp_mem_valid;
        w_data_resp_mem_load_fault = i_resp_mem_load_fault;
    end

    wb_ctrl_resp_mem_data = i_resp_mem_data;
    wb_data_resp_mem_data = i_resp_mem_data;
    v_req_mem_path_o = queue_rdata_o[54];
    vb_req_mem_type_o = queue_rdata_o[53: 51];
    vb_req_mem_size_o = queue_rdata_o[50: 48];
    vb_req_mem_addr_o = queue_rdata_o[47: 0];

    vb_resp_ctrl_addr[(CFG_CPU_ADDR_BITS - 1): 0] = i.resp_addr;
    vb_resp_data_addr[(CFG_CPU_ADDR_BITS - 1): 0] = d.resp_addr;
    if (CFG_CPU_ADDR_BITS < RISCV_ARCH) begin
        if (i.resp_addr[(CFG_CPU_ADDR_BITS - 1)] == 1'b1) begin
            vb_resp_ctrl_addr[(RISCV_ARCH - 1): CFG_CPU_ADDR_BITS] = '1;
        end
        if (d.resp_addr[(CFG_CPU_ADDR_BITS - 1)] == 1'b1) begin
            vb_resp_data_addr[(RISCV_ARCH - 1): CFG_CPU_ADDR_BITS] = '1;
        end
    end

    o_req_mem_valid = queue_nempty_o;
    o_req_mem_path = v_req_mem_path_o;
    o_req_mem_type = vb_req_mem_type_o;
    o_req_mem_size = vb_req_mem_size_o;
    o_req_mem_addr = vb_req_mem_addr_o;
    o_req_mem_strob = d.req_mem_strob;
    o_req_mem_data = d.req_mem_wdata;
    o_resp_ctrl_addr = vb_resp_ctrl_addr;
    o_resp_data_addr = vb_resp_data_addr;
end: comb_proc

endmodule: CacheTop
