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

module apb_prci #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_pwrreset,                                 // Power-on reset, external button. Active HIGH
    input logic i_dmireset,                                 // Debug reset: system reset except DMI interface
    input logic i_sys_locked,
    input logic i_ddr_locked,
    output logic o_sys_rst,                                 // System reset except DMI. Active HIGH
    output logic o_sys_nrst,                                // System reset except DMI. Active LOW
    output logic o_dbg_nrst,                                // Reset DMI. Active LOW
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_pnp_pkg::dev_config_type o_cfg,            // Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB  Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo              // APB Bridge to Slave interface
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
import apb_prci_pkg::*;

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
apb_prci_rhegisters rh, rhin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_PRCI)
) pslv0 (
    .i_clk(i_clk),
    .i_nrst(rh.sys_nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(o_cfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .i_resp_valid(rh.resp_valid),
    .i_resp_rdata(rh.resp_rdata),
    .i_resp_err(rh.resp_err)
);

always_comb
begin: comb_proc
    apb_prci_rhegisters vh;
    logic [31:0] vb_rdata;

    vb_rdata = '0;

    vh = rh;

    vh.sys_rst = (i_pwrreset || (~i_sys_locked) || i_dmireset);
    vh.sys_nrst = (~(i_pwrreset || (~i_sys_locked) || i_dmireset));
    vh.dbg_nrst = (~(i_pwrreset || (~i_sys_locked)));

    // Registers access:
    case (wb_req_addr[11: 2])
    10'd0: begin                                            // 0x00: pll statuses
        vb_rdata[0] = i_sys_locked;
        vb_rdata[1] = i_ddr_locked;
    end
    10'd1: begin                                            // 0x04: reset status
        vb_rdata[0] = rh.sys_nrst;
        vb_rdata[1] = rh.dbg_nrst;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                // todo:
            end
        end
    end
    default: begin
    end
    endcase

    vh.resp_valid = w_req_valid;
    vh.resp_rdata = vb_rdata;
    vh.resp_err = 1'b0;

    if (~async_reset && i_pwrreset == 1'b1) begin
        vh = apb_prci_rh_reset;
    end

    o_sys_rst = rh.sys_rst;
    o_sys_nrst = rh.sys_nrst;
    o_dbg_nrst = rh.dbg_nrst;

    rhin = vh;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, posedge i_pwrreset) begin: rhg_proc
            if (i_pwrreset == 1'b1) begin
                rh <= apb_prci_rh_reset;
            end else begin
                rh <= rhin;
            end
        end: rhg_proc

    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rhg_proc
            rh <= rhin;
        end: rhg_proc

    end: no_rst_gen
endgenerate

endmodule: apb_prci
