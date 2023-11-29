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

module sdctrl_spimode #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_posedge,                                  // SPI clock posedge pulse
    input logic i_miso,                                     // SPI data input
    output logic o_mosi,                                    // SPI master output slave input
    output logic o_csn,                                     // Chip select active LOW
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
    input logic [14:0] i_cmd_resp_r1r2,
    input logic [31:0] i_cmd_resp_arg32,
    output logic o_data_req_ready,
    input logic i_data_req_valid,
    input logic i_data_req_write,
    input logic [sdctrl_cfg_pkg::CFG_SDCACHE_ADDR_BITS-1:0] i_data_req_addr,
    input logic [511:0] i_data_req_wdata,
    output logic o_data_resp_valid,
    output logic [511:0] o_data_resp_rdata,
    input logic [15:0] i_crc16_0,
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
import sdctrl_spimode_pkg::*;

sdctrl_spimode_registers r, rin;


always_comb
begin: comb_proc
    sdctrl_spimode_registers v;
    logic v_dat;
    logic [31:0] vb_cmd_req_arg;
    logic v_data_req_ready;
    logic v_crc16_next;

    v_dat = 1'b0;
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
        v.data_data = {r.data_data[510: 0], (i_miso || r.dat_csn)};
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
        v.cmd_resp_r1 = i_cmd_resp_r1r2[14: 8];
        v.cmd_resp_arg32 = i_cmd_resp_arg32;
        case (r.cmd_req_rn)
        R2: begin
            v.cmd_resp_r2 = i_cmd_resp_r1r2[7: 0];
        end
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
            // Relative Address (RCA):
            v.data_addr = {i_cmd_resp_arg32[31: 16], 16'd0};
        end
        default: begin
        end
        endcase
    end else if (r.wait_cmd_resp == 1'b1) begin
        // do nothing
    end else if (r.state == STATE_CMD0) begin
        v.sdtype = SDCARD_UNKNOWN;
        v.HCS = 1'b1;
        v.S18 = 1'b0;
        v.data_addr = 32'd0;
        v.OCR_VoltageWindow = 32'h00ff8000;
        v.cmd_req_valid = 1'b1;
        v.cmd_req_cmd = CMD0;
        v.cmd_req_rn = R1;
        vb_cmd_req_arg = 32'd0;
        v.state = STATE_CMD8;
    end else if (r.state == STATE_CMD8) begin
        // See page 113. 4.3.13 Send Interface Condition Command
        //   [39:22] reserved 00000h
        //   [21]    PCIe 1.2V support 0
        //   [20]    PCIe availability 0
        //   [19:16] Voltage Supply (VHS) 0001b: 2.7-3.6V
        //   [15:8]  Check Pattern 55h
        if ((|i_err_code) == 1'b0) begin
            v.cmd_req_valid = 1'b1;
        end else begin
            v.state = STATE_CMD0;
            v.err_clear = 1'b1;
        end
        v.cmd_req_cmd = CMD8;
        v.cmd_req_rn = R7;
        vb_cmd_req_arg = 32'd0;
        vb_cmd_req_arg[13] = i_cfg_pcie_12V_support;
        vb_cmd_req_arg[12] = i_cfg_pcie_available;
        vb_cmd_req_arg[11: 8] = i_cfg_voltage_supply;
        vb_cmd_req_arg[7: 0] = i_cfg_check_pattern;
        v.state = STATE_CMD55;
    end else if (r.state == STATE_CMD55) begin
        // Page 64: APP_CMD (CMD55) shall always precede ACMD41.
        //   [31:16] RCA (Relative Adrress should be set 0)
        //   [15:0] stuff bits
        if (i_err_code == CMDERR_NO_RESPONSE) begin
            v.sdtype = SDCARD_VER1X;
            v.HCS = 1'b0;                                   // Standard Capacity only
            v.err_clear = 1'b1;
        end
        v.cmd_req_valid = 1'b1;
        v.cmd_req_cmd = CMD55;
        v.cmd_req_rn = R1;
        vb_cmd_req_arg = 32'd0;
        v.state = STATE_ACMD41;
    end else if (r.state == STATE_ACMD41) begin
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
        v.cmd_req_rn = R1;
        v.state = STATE_CMD58;
    end else if (r.state == STATE_CMD58) begin
        // READ_OCR: Reads OCR register. Used in SPI mode only.
        //   [31] reserved bit
        //   [30] HCS (high capacity support)
        //   [29:0] reserved
        if (r.cmd_resp_r1 != 7'h01) begin
            // SD card not in idle state
            v.state = STATE_CMD55;
        end else begin
            v.cmd_req_valid = 1'b1;
            v.cmd_req_cmd = CMD58;
            vb_cmd_req_arg = 32'd0;
            v.cmd_req_rn = R3;
            v.state = STATE_WAIT_DATA_REQ;
            v.sck_400khz_ena = 1'b0;
        end
    end else if (r.state == STATE_WAIT_DATA_REQ) begin
        v.wdog_ena = 1'b0;
        v_data_req_ready = 1'b1;
        if (i_data_req_valid == 1'b1) begin
            v.data_addr = i_data_req_addr[(CFG_SDCACHE_ADDR_BITS - 1): 9];
            v.data_data = i_data_req_wdata;
            if (i_data_req_write == 1'b1) begin
                v.state = STATE_CMD24_WRITE_SINGLE_BLOCK;
            end else begin
                v.state = STATE_CMD17_READ_SINGLE_BLOCK;
            end
        end
    end else if (r.state == STATE_CMD17_READ_SINGLE_BLOCK) begin
        // CMD17: READ_SINGLE_BLOCK. Reads a block of the size SET_BLOCKLEN
        //   [31:0] data address
        v.cmd_req_valid = 1'b1;
        v.cmd_req_cmd = CMD17;
        v.cmd_req_rn = R1;
        vb_cmd_req_arg = r.data_addr;
        v.state = STATE_WAIT_DATA_START;
        v.bitcnt = 12'd0;
    end else if (r.state == STATE_CMD24_WRITE_SINGLE_BLOCK) begin
        // TODO:
    end else if (r.state == STATE_WAIT_DATA_START) begin
        v.dat_csn = 1'b0;
        v.dat_reading = 1'b1;
        v.crc16_clear = 1'b1;
        v.wdog_ena = 1'b1;
        if (i_err_code != CMDERR_NONE) begin
            v.state = STATE_WAIT_DATA_REQ;
        end else if (r.data_data[7: 0] == 8'hfe) begin
            v.state = STATE_READING_DATA;
            v.bitcnt = 12'd0;
            v.crc16_clear = 1'b0;
        end else if (i_wdog_trigger == 1'b1) begin
            v.state = STATE_WAIT_DATA_REQ;
            v.err_valid = 1'b1;
            v.err_code = CMDERR_NO_RESPONSE;
        end
    end else if (r.state == STATE_READING_DATA) begin
        if (i_posedge == 1'b1) begin
            v_crc16_next = 1'b1;
            if ((&r.bitcnt) == 1'b1) begin
                v.state = STATE_READING_CRC15;
                v.crc16_calc0 = i_crc16_0;
            end
            if ((&r.bitcnt[8: 0]) == 1'b1) begin
                v.data_resp_valid = 1'b1;
            end
        end
    end else if (r.state == STATE_READING_CRC15) begin
        if (i_posedge == 1'b1) begin
            if ((&r.bitcnt[3: 0]) == 1'b1) begin
                v.state = STATE_READING_END;
                v.dat_csn = 1'b1;
                v.dat_reading = 1'b0;
            end
        end
    end else if (r.state == STATE_READING_END) begin
        v.crc16_rx0 = r.data_data[15: 0];
        v.state = STATE_WAIT_DATA_REQ;
    end

    v.cmd_req_arg = vb_cmd_req_arg;
    v_dat = (r.dat_reading || r.data_data[511]);

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_spimode_r_reset;
    end

    o_mosi = v_dat;
    o_cmd_req_valid = r.cmd_req_valid;
    o_cmd_req_cmd = r.cmd_req_cmd;
    o_cmd_req_arg = r.cmd_req_arg;
    o_cmd_req_rn = r.cmd_req_rn;
    o_data_req_ready = v_data_req_ready;
    o_data_resp_valid = r.data_resp_valid;
    o_data_resp_rdata = r.data_data;
    o_csn = r.dat_csn;
    o_crc16_clear = r.crc16_clear;
    o_crc16_next = v_crc16_next;
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
                r <= sdctrl_spimode_r_reset;
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

endmodule: sdctrl_spimode
