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
    parameter int CFG_SDCARD_POWERUP_DONE_DELAY = 700,      // Delay of busy bits in ACMD41 response
    parameter logic CFG_SDCARD_HCS = 1'h1,                  // High Capacity Support
    parameter logic [3:0] CFG_SDCARD_VHS = 4'h1,            // CMD8 Voltage supply mask
    parameter logic CFG_SDCARD_PCIE_1_2V = 1'h0,
    parameter logic CFG_SDCARD_PCIE_AVAIL = 1'h0,
    parameter logic [23:0] CFG_SDCARD_VDD_VOLTAGE_WINDOW = 24'hff8000
)
(
    input logic i_nrst,
    input logic i_clk,
    input logic i_spi_mode,
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
    output logic o_stat_illegal_cmd
);

import vip_sdcard_ctrl_pkg::*;

vip_sdcard_ctrl_registers r, rin;

always_comb
begin: comb_proc
    vip_sdcard_ctrl_registers v;
    logic v_resp_valid;
    logic [31:0] vb_resp_data32;
    logic v_idle_state;

    v_resp_valid = 0;
    vb_resp_data32 = 0;
    v_idle_state = 0;

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

    if (r.sdstate == SDSTATE_IDLE) begin
        v_idle_state = 1'b1;
    end

    if (i_cmd_req_valid == 1'b1) begin
        case (r.sdstate)
        SDSTATE_IDLE: begin
            case (i_cmd_req_cmd)
            6'h00: begin                                    // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = '0;
                v.delay_cnt = 32'h00000014;
            end
            6'h08: begin                                    // CMD8: SEND_IF_COND.
                // Send memory Card interface condition:
                // [21] PCIe 1.2V support
                // [20] PCIe availability
                // [19:16] Voltage supply
                // [15:8] check pattern
                v.cmd_resp_valid = 1'b1;
                v.cmd_resp_r7 = 1'b1;
                v.delay_cnt = 32'h00000014;
                vb_resp_data32[13] = (i_cmd_req_data[13] & CFG_SDCARD_PCIE_1_2V);
                vb_resp_data32[12] = (i_cmd_req_data[12] & CFG_SDCARD_PCIE_AVAIL);
                vb_resp_data32[11: 8] = (i_cmd_req_data[11: 8] & CFG_SDCARD_VHS);
                vb_resp_data32[7: 0] = i_cmd_req_data[7: 0];
            end
            6'h37: begin                                    // CMD55: APP_CMD.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = '0;
            end
            6'h29: begin                                    // ACMD41: SD_SEND_OP_COND.
                // Send host capacity info:
                // [39] BUSY, active LOW
                // [38] HCS (OCR[30]) Host Capacity
                // [36] XPC
                // [32] S18R
                // [31:8] VDD Voltage Window (OCR[23:0])
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'h00000014;
                vb_resp_data32[31] = r.powerup_done;
                vb_resp_data32[30] = i_cmd_req_data[30];
                vb_resp_data32[23: 0] = (i_cmd_req_data[23: 0] & CFG_SDCARD_VDD_VOLTAGE_WINDOW);
                if ((i_cmd_req_data[23: 0] & CFG_SDCARD_VDD_VOLTAGE_WINDOW) == 24'h000000) begin
                    // OCR check failed:
                    v.sdstate = SDSTATE_INA;
                end else if ((i_spi_mode == 1'b0) && (r.powerup_done == 1'b1)) begin
                    // SD mode only
                    v.sdstate = SDSTATE_READY;
                end
            end
            6'h3a: begin                                    // CMD58: READ_OCR.
                v.cmd_resp_valid = 1'b1;
                v.cmd_resp_r7 = 1'b1;
                v.delay_cnt = 32'h00000014;
                if (i_spi_mode == 1'b1) begin
                    vb_resp_data32 = '0;
                    vb_resp_data32[30] = CFG_SDCARD_HCS;
                    vb_resp_data32[23: 0] = CFG_SDCARD_VDD_VOLTAGE_WINDOW;
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
            6'h00: begin                                    // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = '0;
                v.delay_cnt = 32'h00000002;
                v.sdstate = SDSTATE_IDLE;
            end
            6'h02: begin                                    // CMD2: .
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'h00000001;
                v.sdstate = SDSTATE_IDENT;
            end
            6'h0b: begin                                    // CMD11: .
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'h00000001;
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
            6'h00: begin                                    // CMD0: GO_IDLE_STATE.
                v.cmd_resp_valid = 1'b1;
                vb_resp_data32 = '0;
                v.delay_cnt = 32'h00000002;
                v.sdstate = SDSTATE_IDLE;
            end
            6'h03: begin                                    // CMD3: .
                v.cmd_resp_valid = 1'b1;
                v.delay_cnt = 32'h00000001;
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
    o_stat_idle_state = v_idle_state;

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
