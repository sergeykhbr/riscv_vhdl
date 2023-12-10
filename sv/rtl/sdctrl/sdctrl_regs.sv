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
    output logic o_err_clear,                               // Clear err from FW
    // Configuration parameters:
    output logic o_spi_mode,                                // SPI mode was selected from FW
    output logic o_pcie_12V_support,                        // 0b: not asking 1.2V support
    output logic o_pcie_available,                          // 0b: not asking PCIe availability
    output logic [3:0] o_voltage_supply,                    // 0=not defined; 1=2.7-3.6V; 2=reserved for Low Voltage Range
    output logic [7:0] o_check_pattern,                     // Check pattern in CMD8 request
    input logic i_400khz_ena,                               // Default frequency enabled in identification mode
    input logic [2:0] i_sdtype,                             // Ver1X or Ver2X standard or Ver2X high/extended capacity
    // Debug command state machine
    input logic i_sd_cmd,
    input logic i_sd_dat0,
    input logic i_sd_dat1,
    input logic i_sd_dat2,
    input logic i_sd_dat3,
    input logic [3:0] i_err_code,
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

    v_posedge = 1'b0;
    v_negedge = 1'b0;
    vb_rdata = '0;

    v = r;

    v.err_clear = 1'b0;
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
        if (((i_400khz_ena == 1'b1) && (r.scaler_cnt >= r.scaler_400khz))
                || ((i_400khz_ena == 1'b0) && (r.scaler_cnt >= r.scaler_data))) begin
            v.scaler_cnt = 32'd0;
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
        vb_rdata = {r.scaler_data, r.scaler_400khz};
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.scaler_data = wb_req_wdata[31: 24];
            v.scaler_400khz = wb_req_wdata[23: 0];
            v.scaler_cnt = 32'd0;
        end
    end
    10'h001: begin                                          // {0x04, 'RW', 'control', 'Global Control register'}
        vb_rdata[0] = r.sclk_ena;
        vb_rdata[3] = r.spi_mode;
        vb_rdata[4] = i_sd_dat0;
        vb_rdata[5] = i_sd_dat1;
        vb_rdata[6] = i_sd_dat2;
        vb_rdata[7] = i_sd_dat3;
        vb_rdata[8] = i_sd_cmd;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.sclk_ena = wb_req_wdata[0];
            v.err_clear = wb_req_wdata[1];
            v.spi_mode = wb_req_wdata[3];
        end
    end
    10'h002: begin                                          // {0x08, 'RW', 'watchdog', 'Watchdog'}
        vb_rdata[15: 0] = r.wdog;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.wdog = wb_req_wdata[15: 0];
        end
    end
    10'h004: begin                                          // {0x10, 'RO', 'status', 'state machines status'}
        vb_rdata[3: 0] = i_err_code;                        // the latest error code
        vb_rdata[14: 12] = i_sdtype;                        // detected card type
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
    10'h008: begin                                          // {0x20, 'RW', 'interface_condition', 'CMD8 parameters'}
        vb_rdata[7: 0] = r.check_pattern;
        vb_rdata[11: 8] = r.voltage_supply;
        vb_rdata[12] = r.pcie_available;
        vb_rdata[13] = r.pcie_12V_support;
        if ((w_req_valid == 1'b1) && (w_req_write == 1'b1)) begin
            v.check_pattern = wb_req_wdata[7: 0];
            v.voltage_supply = wb_req_wdata[11: 8];
            v.pcie_available = wb_req_wdata[12];
            v.pcie_12V_support = wb_req_wdata[13];
        end
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

    o_spi_mode = r.spi_mode;
    o_pcie_12V_support = r.pcie_12V_support;
    o_pcie_available = r.pcie_available;
    o_voltage_supply = r.voltage_supply;
    o_check_pattern = r.check_pattern;

    o_sck = r.level;
    o_sck_posedge = v_posedge;
    o_sck_negedge = v_negedge;
    o_watchdog = r.wdog;
    o_err_clear = r.err_clear;

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
