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

module apb_ddr #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // APB clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_amba_pkg::dev_config_type o_cfg,           // Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB input interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB output interface
    input logic i_pll_locked,                               // PLL locked
    input logic i_init_calib_done,                          // DDR initialization done
    input logic [11:0] i_device_temp,                       // Temperature monitor value
    input logic i_sr_active,
    input logic i_ref_ack,
    input logic i_zq_ack
);

import types_amba_pkg::*;
import apb_ddr_pkg::*;

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
apb_ddr_registers r, rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_DDRCTRL)
) pslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .i_resp_valid(r.resp_valid),
    .i_resp_rdata(r.resp_rdata),
    .i_resp_err(r.resp_err)
);


always_comb
begin: comb_proc
    apb_ddr_registers v;
    logic [31:0] vb_rdata;

    vb_rdata = 0;

    v = r;

    v.pll_locked = i_pll_locked;
    v.init_calib_done = i_init_calib_done;
    v.device_temp = i_device_temp;
    v.sr_active = i_sr_active;
    v.ref_ack = i_ref_ack;
    v.zq_ack = i_zq_ack;

    v.resp_err = 1'b0;
    // Registers access:
    case (wb_req_addr[11: 2])
    10'h000: begin                                          // 0x00: clock status
        vb_rdata[0] = r.pll_locked;
        vb_rdata[1] = r.init_calib_done;
    end
    10'h001: begin                                          // 0x04: temperature
        vb_rdata[11: 0] = r.device_temp;
    end
    10'h002: begin                                          // 0x08: app bits
        vb_rdata[0] = r.sr_active;                          // [0] 
        vb_rdata[1] = r.ref_ack;                            // [1] 
        vb_rdata[2] = r.zq_ack;                             // [2] 
    end
    default: begin
    end
    endcase

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;

    if (~async_reset && i_nrst == 1'b0) begin
        v = apb_ddr_r_reset;
    end

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= apb_ddr_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: apb_ddr
