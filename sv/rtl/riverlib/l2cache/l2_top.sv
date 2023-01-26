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

module L2Top #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned waybits = 4,                     // Log2 of number of ways. Default 4: 16 ways
    parameter int unsigned ibits = 9                        // Log2 of number of lines per way: 9=64KB (if bytes per line = 32 B)
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_river_pkg::axi4_l1_out_vector i_l1o,
    output types_river_pkg::axi4_l1_in_vector o_l1i,
    input types_river_pkg::axi4_l2_in_type i_l2i,
    output types_river_pkg::axi4_l2_out_type o_l2o,
    input logic i_flush_valid
);

import types_amba_pkg::*;
import river_cfg_pkg::*;
import types_river_pkg::*;
import l2_top_pkg::*;

logic w_req_ready;
logic w_req_valid;
logic [L2_REQ_TYPE_BITS-1:0] wb_req_type;
logic [CFG_CPU_ADDR_BITS-1:0] wb_req_addr;
logic [2:0] wb_req_size;
logic [2:0] wb_req_prot;
logic [L1CACHE_LINE_BITS-1:0] wb_req_wdata;
logic [L1CACHE_BYTES_PER_LINE-1:0] wb_req_wstrb;
logic w_cache_valid;
logic [L1CACHE_LINE_BITS-1:0] wb_cache_rdata;
logic [1:0] wb_cache_status;
// Memory interface:
logic w_req_mem_ready;
logic w_req_mem_valid;
logic [REQ_MEM_TYPE_BITS-1:0] wb_req_mem_type;
logic [2:0] wb_req_mem_size;
logic [2:0] wb_req_mem_prot;
logic [CFG_CPU_ADDR_BITS-1:0] wb_req_mem_addr;
logic [L2CACHE_BYTES_PER_LINE-1:0] wb_req_mem_strob;
logic [L2CACHE_LINE_BITS-1:0] wb_req_mem_data;
logic w_mem_data_valid;
logic w_mem_data_ack;
logic [L2CACHE_LINE_BITS-1:0] wb_mem_data;
logic w_mem_load_fault;
logic w_mem_store_fault;
// Flush interface
logic [CFG_CPU_ADDR_BITS-1:0] wb_flush_address;
logic w_flush_end;

assign wb_flush_address = '1;



L2Destination #(
    .async_reset(async_reset)
) dst0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_resp_valid(w_cache_valid),
    .i_resp_rdata(wb_cache_rdata),
    .i_resp_status(wb_cache_status),
    .i_l1o(i_l1o),
    .o_l1i(o_l1i),
    .i_req_ready(w_req_ready),
    .o_req_valid(w_req_valid),
    .o_req_type(wb_req_type),
    .o_req_addr(wb_req_addr),
    .o_req_size(wb_req_size),
    .o_req_prot(wb_req_prot),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb)
);


L2CacheLru #(
    .async_reset(async_reset),
    .waybits(waybits),
    .ibits(ibits)
) cache0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_req_valid(w_req_valid),
    .i_req_type(wb_req_type),
    .i_req_size(wb_req_size),
    .i_req_prot(wb_req_prot),
    .i_req_addr(wb_req_addr),
    .i_req_wdata(wb_req_wdata),
    .i_req_wstrb(wb_req_wstrb),
    .o_req_ready(w_req_ready),
    .o_resp_valid(w_cache_valid),
    .o_resp_rdata(wb_cache_rdata),
    .o_resp_status(wb_cache_status),
    .i_req_mem_ready(w_req_mem_ready),
    .o_req_mem_valid(w_req_mem_valid),
    .o_req_mem_type(wb_req_mem_type),
    .o_req_mem_size(wb_req_mem_size),
    .o_req_mem_prot(wb_req_mem_prot),
    .o_req_mem_addr(wb_req_mem_addr),
    .o_req_mem_strob(wb_req_mem_strob),
    .o_req_mem_data(wb_req_mem_data),
    .i_mem_data_valid(w_mem_data_valid),
    .i_mem_data(wb_mem_data),
    .i_mem_data_ack(w_mem_data_ack),
    .i_mem_load_fault(w_mem_load_fault),
    .i_mem_store_fault(w_mem_store_fault),
    .i_flush_address(wb_flush_address),
    .i_flush_valid(i_flush_valid),
    .o_flush_end(w_flush_end)
);


L2Amba #(
    .async_reset(async_reset)
) amba0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .o_req_ready(w_req_mem_ready),
    .i_req_valid(w_req_mem_valid),
    .i_req_type(wb_req_mem_type),
    .i_req_size(wb_req_mem_size),
    .i_req_prot(wb_req_mem_prot),
    .i_req_addr(wb_req_mem_addr),
    .i_req_strob(wb_req_mem_strob),
    .i_req_data(wb_req_mem_data),
    .o_resp_data(wb_mem_data),
    .o_resp_valid(w_mem_data_valid),
    .o_resp_ack(w_mem_data_ack),
    .o_resp_load_fault(w_mem_load_fault),
    .o_resp_store_fault(w_mem_store_fault),
    .i_msti(i_l2i),
    .o_msto(o_l2o)
);


endmodule: L2Top
