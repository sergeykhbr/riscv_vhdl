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

module sdctrl_sdmode #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_posedge,                                  // SPI clock posedge pulse
    input logic i_dat0,                                     // Data Line[0] input; DI in SPI mode
    output logic o_dat0,                                    // Data Line[0] output
    output logic o_dat0_dir,                                // Direction bit: 1=input; 0=output
    input logic i_dat1,                                     // Data Line[1] input
    output logic o_dat1,                                    // Data Line[1] output
    output logic o_dat1_dir,                                // Direction bit: 1=input; 0=output
    input logic i_dat2,                                     // Data Line[2] input
    output logic o_dat2,                                    // Data Line[2] output
    output logic o_dat2_dir,                                // Direction bit: 1=input; 0=output
    input logic i_cd_dat3,                                  // Card Detect / Data Line[3] input
    output logic o_dat3,                                    // Data Line[3] output; CS output in SPI mode
    output logic o_dat3_dir,                                // Direction bit: 1=input; 0=output
    input logic i_detected,
    input logic i_protect,
    input logic i_cfg_pcie_12V_support,
    input logic i_cfg_pcie_available,
    input logic [3:0] i_cfg_voltage_supply,
    input logic [7:0] i_cfg_check_pattern,
    input logic i_cmd_req_ready,
    output logic o_cmd_req_valid,
    output logic [5:0] o_cmd_req_cmd,
    output logic [31:0] o_cmd_req_arg,
    output logic [2:0] o_cmd_req_rn,
    input logic i_cmd_resp_valid,
    input logic [5:0] i_cmd_resp_cmd,
    input logic [31:0] i_cmd_resp_arg32,
    output logic o_data_req_ready,
    input logic i_data_req_valid,
    input logic i_data_req_write,
    input logic [sdctrl_cfg_pkg::CFG_SDCACHE_ADDR_BITS-1:0] i_data_req_addr,
    input logic [511:0] i_data_req_wdata,
    output logic o_data_resp_valid,
    output logic [511:0] o_data_resp_rdata,
    input logic [15:0] i_crc16_0,
    input logic [15:0] i_crc16_1,
    input logic [15:0] i_crc16_2,
    input logic [15:0] i_crc16_3,
    output logic o_crc16_clear,
    output logic o_crc16_next,
    output logic o_wdog_ena,
    input logic i_wdog_trigger,
    input logic [3:0] i_err_code,
    output logic o_err_valid,
    output logic o_err_clear,
    output logic [3:0] o_err_code,
    output logic o_400khz_ena,
    output logic [2:0] o_sdtype
);

import sdctrl_cfg_pkg::*;
import sdctrl_sdmode_pkg::*;

sdctrl_sdmode_registers r, rin;


always_comb
begin: comb_proc
    sdctrl_sdmode_registers v;
    logic v_dat0;
    logic v_dat1;
    logic v_dat2;
    logic v_dat3;
    logic [31:0] vb_cmd_req_arg;
    logic v_data_req_ready;
    logic v_crc16_next;

    v_dat0 = 1'b0;
    v_dat1 = 1'b0;
    v_dat2 = 1'b0;
    v_dat3 = 1'b0;
    vb_cmd_req_arg = '0;
    v_data_req_ready = 1'b0;
    v_crc16_next = 1'b0;

    v = r;

    v.err_clear = 1'b0;
    v.err_valid = 1'b0;
    v.err_code = 4'd0;
    v.data_resp_valid = 1'b0;
    vb_cmd_req_arg = r.cmd_req_arg;

    if (i_posedge) begin
        // Not a full block 4096 bits just a cache line (dat_csn is active LOW):
        if (r.dat_full_ena == 1'b0) begin
            v.data_data = {r.data_data[510: 0], (i_dat0 || r.dat_csn)};
        end else begin
            v.data_data = {r.data_data[507: 0], {i_dat0, i_dat1, i_dat2, i_cd_dat3}};
        end
        v.bitcnt = (r.bitcnt + 1);
    end

    if (r.cmd_req_valid == 1'b1) begin
        if (i_cmd_req_ready == 1'b1) begin
            v.cmd_req_valid = 1'b0;
            v.wait_cmd_resp = 1'b1;
        end
    end else if ((i_cmd_resp_valid == 1'b1) && (r.wait_cmd_resp == 1'b1)) begin
        // Parse Rx response:
        v.wait_cmd_resp = 1'b0;
        v.cmd_resp_cmd = i_cmd_resp_cmd;
        v.cmd_resp_arg32 = i_cmd_resp_arg32;
        case (r.cmd_req_rn)
        R3: begin
            // Table 5-1: OCR Register definition, page 246
            //     [23:0]  Voltage window can be requested by CMD58
            //     [24]    Switching to 1.8V accepted (S18A)
            //     [27]    Over 2TB support status (CO2T)
            //     [29]    UHS-II Card status
            //     [30]    Card Capacity Status (CCS)
            //     [31]    Card power-up status (busy is LOW if the card not finished the power-up routine)
            if (i_cmd_resp_arg32[31] == 1'b1) begin
                v.OCR_VoltageWindow = i_cmd_resp_arg32[23: 0];
                v.HCS = i_cmd_resp_arg32[30];
                if (i_cmd_resp_arg32[30] == 1'b1) begin
                    v.sdtype = SDCARD_VER2X_HC;
                end else if (r.sdtype == SDCARD_UNKNOWN) begin
                    v.sdtype = SDCARD_VER2X_SC;
                end
                v.S18 = i_cmd_resp_arg32[24];
            end
        end
        R6: begin
            v.data_addr = {i_cmd_resp_arg32[31: 16], 16'd0};
        end
        default: begin
        end
        endcase
    end else if (r.wait_cmd_resp == 1'b1) begin
        // do nothing
    end else if (r.sdstate == SDSTATE_IDLE) begin
        v.sck_400khz_ena = 1'b1;
        case (r.initstate)
        IDLESTATE_CMD0: begin
            v.sdtype = SDCARD_UNKNOWN;
            v.HCS = 1'b1;
            v.S18 = 1'b0;
            v.data_addr = 32'd0;
            v.cmd_req_valid = 1'b1;
            v.cmd_req_cmd = CMD0;
            v.cmd_req_rn = R1;
            vb_cmd_req_arg = 32'd0;
            v.initstate = IDLESTATE_CMD8;
        end
        IDLESTATE_CMD8: begin
            // See page 113. 4.3.13 Send Interface Condition Command
            //   [39:22] reserved 00000h
            //   [21]    PCIe 1.2V support 0
            //   [20]    PCIe availability 0
            //   [19:16] Voltage Supply (VHS) 0001b: 2.7-3.6V
            //   [15:8]  Check Pattern 55h
            if ((|i_err_code) == 1'b0) begin
                v.cmd_req_valid = 1'b1;
            end else begin
                v.initstate = IDLESTATE_CMD0;
                v.err_clear = 1'b1;
            end
            v.cmd_req_cmd = CMD8;
            v.cmd_req_rn = R7;
            vb_cmd_req_arg = 32'd0;
            vb_cmd_req_arg[13] = i_cfg_pcie_12V_support;
            vb_cmd_req_arg[12] = i_cfg_pcie_available;
            vb_cmd_req_arg[11: 8] = i_cfg_voltage_supply;
            vb_cmd_req_arg[7: 0] = i_cfg_check_pattern;
            v.initstate = IDLESTATE_CMD55;
        end
        IDLESTATE_CMD55: begin
            // Page 64: APP_CMD (CMD55) shall always precede ACMD41.
            //   [31:16] RCA (Relative Adrress should be set 0)
            //   [15:0] stuff bits
            if (i_err_code == CMDERR_NO_RESPONSE) begin
                v.sdtype = SDCARD_VER1X;
                v.HCS = 1'b0;                               // Standard Capacity only
                v.err_clear = 1'b1;
            end
            v.cmd_req_valid = 1'b1;
            v.cmd_req_cmd = CMD55;
            v.cmd_req_rn = R1;
            vb_cmd_req_arg = 32'd0;
            v.initstate = IDLESTATE_ACMD41;
        end
        IDLESTATE_ACMD41: begin
            // Page 131: SD_SEND_OP_COND. 
            //   [31] reserved bit
            //   [30] HCS (high capacity support)
            //   [29] reserved for eSD
            //   [28] XPC (maximum power in default speed)
            //   [27:25] reserved bits
            //   [24] S18R Send request to switch to 1.8V
            //   [23:0] VDD voltage window (OCR[23:0])
            v.cmd_req_valid = 1'b1;
            v.cmd_req_cmd = ACMD41;
            vb_cmd_req_arg = 32'd0;
            vb_cmd_req_arg[30] = r.HCS;
            vb_cmd_req_arg[23: 0] = r.OCR_VoltageWindow;
            vb_cmd_req_arg[24] = r.S18;
            v.cmd_req_rn = R3;
            v.initstate = IDLESTATE_CARD_IDENTIFICATION;
        end
        IDLESTATE_CARD_IDENTIFICATION: begin
            if (r.cmd_resp_arg32[31] == 1'b0) begin
                // LOW if the card has not finished power-up routine
                v.initstate = IDLESTATE_CMD55;
            end else if (r.S18 == 1'b1) begin
                // Voltage switch command to change 3.3V to 1.8V
                v.readystate = READYSTATE_CMD11;
            end else begin
                v.readystate = READYSTATE_CMD2;
            end
            v.sdstate = SDSTATE_READY;
        end
        default: begin
            v.initstate = IDLESTATE_CMD0;
        end
        endcase
    end else if (r.sdstate == SDSTATE_READY) begin
        case (r.readystate)
        READYSTATE_CMD11: begin
            // CMD11: VOLTAGE_SWITCH siwtch to 1.8V bus signaling.
            //   [31:0] reserved all zeros
            v.cmd_req_valid = 1'b1;
            v.cmd_req_cmd = CMD11;
            v.cmd_req_rn = R1;
            vb_cmd_req_arg = 32'd0;
            v.readystate = READYSTATE_CMD2;
        end
        READYSTATE_CMD2: begin
            // CMD2: ALL_SEND_CID ask to send CID number.
            //   [31:0] stuff bits
            v.cmd_req_valid = 1'b1;
            v.cmd_req_cmd = CMD2;
            v.cmd_req_rn = R2;
            vb_cmd_req_arg = 32'd0;
            v.readystate = READYSTATE_CHECK_CID;
        end
        READYSTATE_CHECK_CID: begin
            v.sdstate = SDSTATE_IDENT;
            v.identstate = IDENTSTATE_CMD3;
        end
        default: begin
        end
        endcase
    end else if (r.sdstate == SDSTATE_IDENT) begin
        case (r.identstate)
        IDENTSTATE_CMD3: begin
            // CMD3: SEND_RELATIVE_ADDR ask card to publish a new relative address (RCA).
            //   [31:0] stuff bits
            v.cmd_req_valid = 1'b1;
            v.cmd_req_cmd = CMD3;
            v.cmd_req_rn = R6;
            vb_cmd_req_arg = 32'd0;
            v.identstate = IDENTSTATE_CHECK_RCA;
        end
        IDENTSTATE_CHECK_RCA: begin
            v.sdstate = SDSTATE_STBY;
            v.sck_400khz_ena = 1'b0;
        end
        default: begin
        end
        endcase
    end else if (r.sdstate == SDSTATE_STBY) begin
    end else begin
    end

    v.cmd_req_arg = vb_cmd_req_arg;
    v_dat0 = r.data_data[511];
    v_dat1 = r.data_data[510];
    v_dat2 = r.data_data[509];
    v_dat3 = r.data_data[508];

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_sdmode_r_reset;
    end

    o_dat0 = v_dat0;
    o_dat1 = v_dat1;
    o_dat2 = v_dat2;
    o_dat3 = v_dat3;
    o_dat0_dir = r.dat_csn;
    o_dat1_dir = r.dat_csn;
    o_dat2_dir = r.dat_csn;
    o_dat3_dir = r.dat_csn;
    o_crc16_clear = r.crc16_clear;
    o_crc16_next = v_crc16_next;
    o_cmd_req_valid = r.cmd_req_valid;
    o_cmd_req_cmd = r.cmd_req_cmd;
    o_cmd_req_arg = r.cmd_req_arg;
    o_cmd_req_rn = r.cmd_req_rn;
    o_data_req_ready = v_data_req_ready;
    o_data_resp_valid = r.data_resp_valid;
    o_data_resp_rdata = r.data_data;
    o_wdog_ena = r.wdog_ena;
    o_err_valid = r.err_valid;
    o_err_clear = r.err_clear;
    o_err_code = r.err_code;
    o_400khz_ena = r.sck_400khz_ena;
    o_sdtype = r.sdtype;

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= sdctrl_sdmode_r_reset;
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

endmodule: sdctrl_sdmode
