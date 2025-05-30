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

module axi_sram #(
    parameter bit async_reset = 1'b0,
    parameter int abits = 17
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_pnp_pkg::dev_config_type o_cfg,            // Device descriptor
    input types_amba_pkg::axi4_slave_in_type i_xslvi,       // AXI Slave to Bridge interface
    output types_amba_pkg::axi4_slave_out_type o_xslvo      // AXI Bridge to Slave interface
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic [7:0] wb_req_size;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;
logic w_req_ready;
logic w_resp_valid;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_resp_rdata;
logic wb_resp_err;
logic [abits-1:0] wb_req_addr_abits;

axi_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_SRAM)
) xslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_xslvi(i_xslvi),
    .o_xslvo(o_xslvo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_size(wb_req_size),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb),
    .o_req_last(w_req_last),
    .i_req_ready(w_req_ready),
    .i_resp_valid(w_resp_valid),
    .i_resp_rdata(wb_resp_rdata),
    .i_resp_err(wb_resp_err)
);

ram_bytes_tech #(
    .abits(abits),
    .log2_dbytes(CFG_LOG2_SYSBUS_DATA_BYTES)
) tech0 (
    .i_clk(i_clk),
    .i_addr(wb_req_addr_abits),
    .i_wena(w_req_write),
    .i_wstrb(wb_req_wstrb),
    .i_wdata(wb_req_wdata),
    .o_rdata(wb_resp_rdata)
);

always_comb
begin: comb_proc

    wb_req_addr_abits = wb_req_addr[(abits - 1): 0];
    w_req_ready = 1'b1;
    w_resp_valid = 1'b1;
    wb_resp_err = 1'b0;
end: comb_proc

endmodule: axi_sram
