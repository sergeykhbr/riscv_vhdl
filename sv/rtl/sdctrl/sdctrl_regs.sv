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

module sdctrl_regs #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_pmapinfo,          // APB interconnect slot information
    output types_pnp_pkg::dev_config_type o_pcfg,           // APB sd-controller configuration registers descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    output logic o_sck,                                     // SD-card clock usually upto 50 MHz
    output logic o_sck_posedge,                             // Strob just before positive edge
    output logic o_sck_negedge,                             // Strob just before negative edge
    output logic [15:0] o_watchdog,                         // Number of sclk to detect no response
    output logic o_clear_cmderr,                            // Clear cmderr from FW
    // Debug command state machine
    input logic [3:0] i_cmd_state,
    input logic [3:0] i_cmd_err,
    input logic i_cmd_req_valid,
    input logic [5:0] i_cmd_req_cmd,
    input logic i_cmd_resp_valid,
    input logic [5:0] i_cmd_resp_cmd,
    input logic [31:0] i_cmd_resp_reg,
    input logic [6:0] i_cmd_resp_crc7_rx,
    input logic [6:0] i_cmd_resp_crc7_calc
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
import sdctrl_regs_pkg::*;

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;
sdctrl_regs_registers r, rin;

apb_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_SDCTRL_REG)
) pslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_pmapinfo),
    .o_cfg(o_pcfg),
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
    sdctrl_regs_registers v;
    logic v_posedge;
    logic v_negedge;
    logic [31:0] vb_rdata;

    v_posedge = 0;
    v_negedge = 0;
    vb_rdata = 0;

    v = r;

    v.clear_cmderr = 1'b0;
    if (i_cmd_req_valid == 1'b1) begin
        v.last_req_cmd = i_cmd_req_cmd;
    end
    if (i_cmd_resp_valid == 1'b1) begin
        v.last_resp_cmd = i_cmd_resp_cmd;
        v.last_resp_crc7_rx = i_cmd_resp_crc7_rx;
        v.last_resp_crc7_calc = i_cmd_resp_crc7_calc;
        v.last_resp_reg = i_cmd_resp_reg;
    end

    // system bus clock scaler to baudrate:
    if (r.sclk_ena == 1'b1) begin
        if (r.scaler_cnt == r.scaler) begin
            v.scaler_cnt = '0;
            v.level = (~r.level);
            v_posedge = (~r.level);
            v_negedge = r.level;
        end else begin
            v.scaler_cnt = (r.scaler_cnt + 1);
        end
    end
    // Registers access:
    case (wb_req_addr[11: 2])
    10'h000: begin                                          // {0x00, 'RW', 'sckdiv', 'Clock Divivder'}
        vb_rdata = r.scaler;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scaler = wb_req_wdata[30: 0];
            v.scaler_cnt = '0;
        end
    end
    10'h001: begin                                          // {0x04, 'RW', 'control', 'Global Control register'}
        vb_rdata[0] = r.sclk_ena;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.sclk_ena = wb_req_wdata[0];
            v.clear_cmderr = wb_req_wdata[1];
        end
    end
    10'h002: begin                                          // {0x08, 'RW', 'watchdog', 'Watchdog'}
        vb_rdata[15: 0] = r.wdog;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.wdog = wb_req_wdata[15: 0];
        end
    end
    10'h004: begin                                          // {0x10, 'RO', 'cmd_status', 'CMD state machine status'}
        vb_rdata[3: 0] = i_cmd_err;
        vb_rdata[7: 4] = i_cmd_state;
    end
    10'h005: begin                                          // {0x14, 'RO', 'last_cmd_response', 'Last CMD response data'}
        vb_rdata[5: 0] = r.last_req_cmd;
        vb_rdata[13: 8] = r.last_resp_cmd;
        vb_rdata[22: 16] = r.last_resp_crc7_rx;
        vb_rdata[30: 24] = r.last_resp_crc7_calc;
    end
    10'h006: begin                                          // {0x18, 'RO', 'last_cmd_resp_arg'}
        vb_rdata = r.last_resp_reg;
    end
    10'h011: begin                                          // 0x44: reserved 4 (txctrl)
    end
    10'h012: begin                                          // 0x48: Tx FIFO Data
    end
    10'h013: begin                                          // 0x4C: Rx FIFO Data
    end
    10'h014: begin                                          // 0x50: Tx FIFO Watermark
    end
    10'h015: begin                                          // 0x54: Rx FIFO Watermark
    end
    10'h016: begin                                          // 0x58: CRC16 value (reserved FU740)
    end
    default: begin
    end
    endcase

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 1'b0;

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_regs_r_reset;
    end

    o_sck = r.level;
    o_sck_posedge = v_posedge;
    o_sck_negedge = v_negedge;
    o_watchdog = r.wdog;
    o_clear_cmderr = r.clear_cmderr;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= sdctrl_regs_r_reset;
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

endmodule: sdctrl_regs
