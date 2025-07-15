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
    parameter logic async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_pwrreset,                                 // Power-on reset, external button. Active HIGH
    input logic i_dmireset,                                 // Debug reset: system reset except DMI interface
    input logic i_sys_locked,
    input logic i_ddr_locked,
    input logic i_pcie_phy_rst,                             // PCIE user reset: active HIGH
    input logic i_pcie_phy_clk,                             // PCIE user clock: 62.5 MHz (default)
    input logic i_pcie_phy_lnk_up,                          // PCIE PHY link status up
    output logic o_sys_rst,                                 // System reset except DMI. Active HIGH
    output logic o_sys_nrst,                                // System reset except DMI. Active LOW
    output logic o_dbg_nrst,                                // Reset DMI. Active LOW
    output logic o_pcie_nrst,                               // Reset PCIE DMA. Active LOW. Reset until link is up.
    output logic o_hdmi_nrst,                               // Reset HDMI. Reset until DDR link up
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
logic r_sys_rst;
logic r_sys_nrst;
logic r_dbg_nrst;
logic [1:0] rb_pcie_nrst;
logic [1:0] rb_hdmi_nrst;
logic r_sys_locked;
logic [1:0] rb_ddr_locked;
logic [1:0] rb_pcie_lnk_up;
apb_prci_registers r;
apb_prci_registers rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_PRCI)
) pslv0 (
    .i_clk(i_clk),
    .i_nrst(r_sys_nrst),
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
    apb_prci_registers v;
    logic [31:0] vb_rdata;

    v = r;
    vb_rdata = '0;


    // Registers access:
    case (wb_req_addr[11: 2])
    10'd0: begin                                            // 0x00: pll statuses
        vb_rdata[0] = r_sys_locked;
        vb_rdata[1] = rb_ddr_locked[1];
        vb_rdata[2] = rb_pcie_lnk_up[1];
    end
    10'd1: begin                                            // 0x04: reset status
        vb_rdata[0] = r_sys_nrst;
        vb_rdata[1] = r_dbg_nrst;
        if (w_req_valid == 1'b1) begin
            if (w_req_write == 1'b1) begin
                // todo:
            end
        end
    end
    default: begin
    end
    endcase

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 1'b0;

    if ((~async_reset) && (r_sys_nrst == 1'b0)) begin
        v = apb_prci_r_reset;
    end

    o_sys_rst = r_sys_rst;
    o_sys_nrst = r_sys_nrst;
    o_dbg_nrst = r_dbg_nrst;
    o_pcie_nrst = rb_pcie_nrst[1];
    o_hdmi_nrst = rb_hdmi_nrst[1];

    rin = v;
end: comb_proc


always_ff @(posedge i_clk, posedge i_pwrreset) begin: reqff_proc
    if (i_pwrreset == 1'b1) begin
        r_sys_locked <= 1'b0;
        rb_ddr_locked <= '0;
        rb_pcie_lnk_up <= '0;
        r_sys_rst <= 1'b1;
        r_sys_nrst <= 1'b0;
        r_dbg_nrst <= 1'b0;
        rb_pcie_nrst <= '0;
        rb_hdmi_nrst <= '0;
    end else begin
        r_sys_locked <= i_sys_locked;
        rb_ddr_locked <= {rb_ddr_locked[0], i_ddr_locked};
        rb_pcie_lnk_up <= {rb_pcie_lnk_up[0], i_pcie_phy_lnk_up};
        r_sys_rst <= ((~i_sys_locked) || i_dmireset);
        r_sys_nrst <= (i_sys_locked & (~i_dmireset));
        r_dbg_nrst <= i_sys_locked;
        rb_pcie_nrst <= {rb_pcie_nrst[0], (i_pcie_phy_lnk_up & (~i_pcie_phy_rst))};
        rb_hdmi_nrst <= {rb_hdmi_nrst[0], (rb_ddr_locked[1] & r_sys_nrst)};
    end
end: reqff_proc

generate
    if (async_reset) begin: async_r_en

        always_ff @(posedge i_clk, negedge r_sys_nrst) begin
            if (r_sys_nrst == 1'b0) begin
                r <= apb_prci_r_reset;
            end else begin
                r <= rin;
            end
        end

    end: async_r_en
    else begin: async_r_dis

        always_ff @(posedge i_clk) begin
            r <= rin;
        end

    end: async_r_dis
endgenerate

endmodule: apb_prci
