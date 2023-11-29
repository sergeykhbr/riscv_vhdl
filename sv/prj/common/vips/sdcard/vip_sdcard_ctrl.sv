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

module vip_sdcard_ctrl #(
    parameter bit async_reset = 1'b0,
    parameter int CFG_SDCARD_POWERUP_DONE_DELAY = 450,      // Delay of busy bits in ACMD41 response
    parameter logic CFG_SDCARD_HCS = 1'b1,                  // High Capacity Support
    parameter logic [3:0] CFG_SDCARD_VHS = 4'h1,            // CMD8 Voltage supply mask
    parameter logic CFG_SDCARD_PCIE_1_2V = 1'b0,
    parameter logic CFG_SDCARD_PCIE_AVAIL = 1'b0,
    parameter logic [23:0] CFG_SDCARD_VDD_VOLTAGE_WINDOW = 24'hff8000
)
(
    input logic i_nrst,
    input logic i_clk,
    input logic i_spi_mode,
    input logic i_cs,
    input logic i_cmd_req_valid,
    input logic [5:0] i_cmd_req_cmd,
    input logic [31:0] i_cmd_req_data,
    output logic o_cmd_req_ready,
    output logic o_cmd_resp_valid,
    output logic [31:0] o_cmd_resp_data32,
    input logic i_cmd_resp_ready,
    output logic o_cmd_resp_r1b,
    output logic o_cmd_resp_r2,
    output logic o_cmd_resp_r3,
    output logic o_cmd_resp_r7,
    output logic o_stat_idle_state,
    output logic o_stat_illegal_cmd,
    output logic [40:0] o_mem_addr,
    input logic [7:0] i_mem_rdata,
    output logic o_crc16_clear,
    output logic o_crc16_next,
    input logic [15:0] i_crc16,
    output logic o_dat_trans,
    output logic [3:0] o_dat,
    input logic i_cmdio_busy
);

import vip_sdcard_ctrl_pkg::*;

vip_sdcard_ctrl_registers r, rin;


always_comb
begin: comb_proc
    vip_sdcard_ctrl_registers v;
    logic v_resp_valid;
    logic [31:0] vb_resp_data32;

    v_resp_valid = 1'b0;
    vb_resp_data32 = '0;

    v = r;

    vb_resp_data32 = r.cmd_resp_data32;

    if ((r.cmd_resp_valid_delayed == 1'b1) && (i_cmd_resp_ready == 1'b1)) begin
        v.cmd_resp_valid_delayed = 1'b0;
        v.cmd_resp_r1b = 1'b0;
        v.cmd_resp_r2 = 1'b0;
        v.cmd_resp_r3 = 1'b0;
        v.cmd_resp_r7 = 1'b0;
        v.illegal_cmd = 1'b0;
    end
    // Power-up counter emulates 'busy' bit in ACMD41 response:
    if ((r.powerup_done == 1'b0) && (r.powerup_cnt < CFG_SDCARD_POWERUP_DONE_DELAY)) begin
        v.powerup_cnt = (r.powerup_cnt + 1);
    end else begin
        v.powerup_done = 1'b1;
    end

    if (i_cmd_req_valid == 1'b1) begin
        case (r.sdstate)
        SDSTATE_IDLE: begin
            case (i_cmd_req_cmd)
            6'd0: begin                                     // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = 32'd0;
                v.delay_cnt = 32'd20;
            end
            6'd8: begin                                     // CMD8: SEND_IF_COND.
                // Send memory Card interface condition:
                // [21] PCIe 1.2V support
                // [20] PCIe availability
                // [19:16] Voltage supply
                // [15:8] check pattern
                v.cmd_resp_valid = 1'b1;
                v.cmd_resp_r7 = 1'b1;
                v.delay_cnt = 32'd20;
                vb_resp_data32[13] = (i_cmd_req_data[13] & CFG_SDCARD_PCIE_1_2V);
                vb_resp_data32[12] = (i_cmd_req_data[12] & CFG_SDCARD_PCIE_AVAIL);
                vb_resp_data32[11: 8] = (i_cmd_req_data[11: 8] & CFG_SDCARD_VHS);
                vb_resp_data32[7: 0] = i_cmd_req_data[7: 0];
            end
            6'd55: begin                                    // CMD55: APP_CMD.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = 32'd0;
            end
            6'd41: begin                                    // ACMD41: SD_SEND_OP_COND.
                // Send host capacity info:
                // [39] BUSY, active LOW
                // [38] HCS (OCR[30]) Host Capacity
                // [36] XPC
                // [32] S18R
                // [31:8] VDD Voltage Window (OCR[23:0])
                v.ocr_hcs = (i_cmd_req_data[30] & CFG_SDCARD_HCS);
                v.ocr_vdd_window = (i_cmd_req_data[23: 0] & CFG_SDCARD_VDD_VOLTAGE_WINDOW);
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'd20;
                vb_resp_data32[31] = r.powerup_done;
                vb_resp_data32[30] = (i_cmd_req_data[30] & CFG_SDCARD_HCS);
                vb_resp_data32[23: 0] = (i_cmd_req_data[23: 0] & CFG_SDCARD_VDD_VOLTAGE_WINDOW);
                if ((i_cmd_req_data[23: 0] & CFG_SDCARD_VDD_VOLTAGE_WINDOW) == 24'd0) begin
                    // OCR check failed:
                    v.sdstate = SDSTATE_INA;
                end else if ((i_spi_mode == 1'b0) && (r.powerup_done == 1'b1)) begin
                    // SD mode only
                    v.sdstate = SDSTATE_READY;
                end
            end
            6'd58: begin                                    // CMD58: READ_OCR.
                v.cmd_resp_valid = 1'b1;
                v.cmd_resp_r7 = 1'b1;
                v.delay_cnt = 32'd20;
                if (i_spi_mode == 1'b1) begin
                    vb_resp_data32 = 32'd0;
                    vb_resp_data32[31] = r.powerup_done;
                    vb_resp_data32[30] = r.ocr_hcs;
                    vb_resp_data32[23: 0] = r.ocr_vdd_window;
                end else begin
                    v.illegal_cmd = 1'b1;
                end
            end
            6'd17: begin                                    // CMD17: READ_SINGLE_BLOCK.
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'd20;
                if (i_spi_mode == 1'b1) begin
                    v.req_mem_valid = 1'b1;
                    v.req_mem_addr = {i_cmd_req_data, 9'd0};
                    vb_resp_data32 = 32'd0;
                end else begin
                    v.illegal_cmd = 1'b1;
                end
            end
            default: begin
                // Illegal commands in 'idle' state:
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = '1;
                v.illegal_cmd = 1'b1;
            end
            endcase
        end
        SDSTATE_READY: begin
            case (i_cmd_req_cmd)
            6'd0: begin                                     // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = 32'd0;
                v.delay_cnt = 32'd2;
                v.sdstate = SDSTATE_IDLE;
            end
            6'd2: begin                                     // CMD2: .
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'd1;
                v.sdstate = SDSTATE_IDENT;
            end
            6'd11: begin                                    // CMD11: .
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'd1;
            end
            default: begin
                // Illegal commands in 'ready' state:
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = '1;
                v.illegal_cmd = 1'b1;
            end
            endcase
        end
        SDSTATE_IDENT: begin
            case (i_cmd_req_cmd)
            6'd0: begin                                     // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = 32'd0;
                v.delay_cnt = 32'd2;
                v.sdstate = SDSTATE_IDLE;
            end
            6'd3: begin                                     // CMD3: .
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'd1;
                v.sdstate = SDSTATE_STBY;
            end
            default: begin
                // Illegal commands in 'stby' state:
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = '1;
                v.illegal_cmd = 1'b1;
            end
            endcase
        end
        SDSTATE_STBY: begin
        end
        SDSTATE_TRAN: begin
        end
        SDSTATE_DATA: begin
        end
        SDSTATE_RCV: begin
        end
        SDSTATE_PRG: begin
        end
        SDSTATE_DIS: begin
        end
        SDSTATE_INA: begin
        end
        default: begin
        end
        endcase
    end

    v.shiftdat = {r.shiftdat[14: 0], 1'b1};
    case (r.datastate)
    DATASTATE_IDLE: begin
        v.crc16_clear = 1'b1;
        v.crc16_next = 1'b0;
        v.dat_trans = 1'b0;
        if ((r.req_mem_valid == 1'b1) && (i_cmdio_busy == 1'b0) && (i_cs == 1'b0)) begin
            v.req_mem_valid = 1'b0;
            v.datastate = DATASTATE_START;
            v.shiftdat = 16'hfe00;
            v.bitcnt = 13'd0;
            v.dat_trans = 1'b1;
        end
    end
    DATASTATE_START: begin
        v.bitcnt = (r.bitcnt + 1);
        if ((&r.bitcnt[2: 0]) == 1'b1) begin
            v.crc16_clear = 1'b0;
            v.crc16_next = 1'b1;
            if (r.bitcnt[12: 3] == 10'd512) begin
                v.datastate = DATASTATE_CRC15;
                v.shiftdat = i_crc16;
                v.bitcnt = 13'd0;
                v.crc16_next = 1'b0;
            end else begin
                // Read memory byte:
                v.shiftdat = {i_mem_rdata, r.shiftdat[7: 0]};
                v.req_mem_addr = (r.req_mem_addr + 1);
            end
        end
    end
    DATASTATE_CRC15: begin
        v.bitcnt = (r.bitcnt + 1);
        if ((&r.bitcnt[3: 0]) == 1'b1) begin
            v.datastate = DATASTATE_IDLE;
            v.dat_trans = 1'b0;
        end
    end
    default: begin
    end
    endcase

    v.cmd_resp_data32 = vb_resp_data32;
    v.cmd_req_ready = (~(|r.delay_cnt));
    if (r.cmd_resp_valid == 1'b1) begin
        if ((|r.delay_cnt) == 1'b0) begin
            v.cmd_resp_valid_delayed = r.cmd_resp_valid;
            v.cmd_resp_valid = 1'b0;
        end else begin
            v.delay_cnt = (r.delay_cnt - 1);
        end
    end

    o_cmd_req_ready = r.cmd_req_ready;
    o_cmd_resp_valid = r.cmd_resp_valid_delayed;
    o_cmd_resp_data32 = r.cmd_resp_data32;
    o_cmd_resp_r1b = r.cmd_resp_r1b;
    o_cmd_resp_r2 = r.cmd_resp_r2;
    o_cmd_resp_r3 = r.cmd_resp_r3;
    o_cmd_resp_r7 = r.cmd_resp_r7;
    o_stat_illegal_cmd = r.illegal_cmd;
    o_stat_idle_state = r.powerup_done;
    o_mem_addr = r.req_mem_addr;
    o_crc16_clear = r.crc16_clear;
    o_crc16_next = r.crc16_next;
    o_dat_trans = r.dat_trans;
    o_dat = r.shiftdat[15: 12];

    rin = v;
end: comb_proc


generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_sdcard_ctrl_r_reset;
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

endmodule: vip_sdcard_ctrl
