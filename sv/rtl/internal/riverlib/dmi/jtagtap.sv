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

module jtagtap #(
    parameter int abits = 7,
    parameter int irlen = 5,
    parameter logic [31:0] idcode = 32'h10E31913
)
(
    input logic i_trst,                                     // Must be open-train, pullup
    input logic i_tck,
    input logic i_tms,
    input logic i_tdi,
    output logic o_tdo,
    output logic o_dmi_req_valid,
    output logic o_dmi_req_write,
    output logic [6:0] o_dmi_req_addr,
    output logic [31:0] o_dmi_req_data,
    input logic [31:0] i_dmi_resp_data,
    input logic i_dmi_busy,
    input logic i_dmi_error,
    output logic o_dmi_hardreset
);

localparam int drlen = ((abits + 32) + 2);

localparam bit [irlen-1:0] IR_IDCODE = 5'h01;
localparam bit [irlen-1:0] IR_DTMCONTROL = 5'h10;
localparam bit [irlen-1:0] IR_DBUS = 5'h11;
localparam bit [irlen-1:0] IR_BYPASS = 5'h1F;

localparam bit [1:0] DMISTAT_SUCCESS = 2'h0;
localparam bit [1:0] DMISTAT_RESERVED = 2'h1;
localparam bit [1:0] DMISTAT_FAILED = 2'h2;
localparam bit [1:0] DMISTAT_BUSY = 2'h3;

// DTMCONTROL register bits
localparam int DTMCONTROL_DMIRESET = 16;
localparam int DTMCONTROL_DMIHARDRESET = 17;

// JTAG states:
localparam bit [3:0] RESET_TAP = 4'd0;
localparam bit [3:0] IDLE = 4'd1;
localparam bit [3:0] SELECT_DR_SCAN = 4'd2;
localparam bit [3:0] CAPTURE_DR = 4'd3;
localparam bit [3:0] SHIFT_DR = 4'd4;
localparam bit [3:0] EXIT1_DR = 4'd5;
localparam bit [3:0] PAUSE_DR = 4'd6;
localparam bit [3:0] EXIT2_DR = 4'd7;
localparam bit [3:0] UPDATE_DR = 4'd8;
localparam bit [3:0] SELECT_IR_SCAN = 4'd9;
localparam bit [3:0] CAPTURE_IR = 4'd10;
localparam bit [3:0] SHIFT_IR = 4'd11;
localparam bit [3:0] EXIT1_IR = 4'd12;
localparam bit [3:0] PAUSE_IR = 4'd13;
localparam bit [3:0] EXIT2_IR = 4'd14;
localparam bit [3:0] UPDATE_IR = 4'd15;

typedef struct {
    logic [3:0] state;
    logic [6:0] dr_length;
    logic [drlen-1:0] dr;
    logic bypass;
    logic [31:0] datacnt;
    logic dmi_busy;
    logic [1:0] err_sticky;
} jtagtap_rhegisters;

const jtagtap_rhegisters jtagtap_rh_reset = '{
    RESET_TAP,                          // state
    '0,                                 // dr_length
    idcode,                             // dr
    1'b0,                               // bypass
    '0,                                 // datacnt
    1'b0,                               // dmi_busy
    '0                                  // err_sticky
};
typedef struct {
    logic [irlen-1:0] ir;
    logic [abits-1:0] dmi_addr;
} jtagtap_rnhegisters;

const jtagtap_rnhegisters jtagtap_rnh_reset = '{
    IR_IDCODE,                          // ir
    '0                                  // dmi_addr
};
jtagtap_rhegisters rh;
jtagtap_rhegisters rhin;
jtagtap_rnhegisters rnh;
jtagtap_rnhegisters rnhin;


always_comb
begin: comb_proc
    jtagtap_rnhegisters vnh;
    jtagtap_rhegisters vh;
    logic [drlen-1:0] vb_dr;
    logic v_dmi_req_valid;
    logic v_dmi_req_write;
    logic [31:0] vb_dmi_req_data;
    logic [abits-1:0] vb_dmi_req_addr;
    logic [1:0] vb_err_sticky;
    logic v_dmi_hardreset;

    vnh = rnh;
    vh = rh;
    vb_dr = '0;
    v_dmi_req_valid = 1'b0;
    v_dmi_req_write = 1'b0;
    vb_dmi_req_data = '0;
    vb_dmi_req_addr = '0;
    vb_err_sticky = '0;
    v_dmi_hardreset = 1'b0;

    vb_dr = rh.dr;
    vb_err_sticky = rh.err_sticky;

    case (rh.state)
    RESET_TAP: begin
        vnh.ir = IR_IDCODE;
        if (i_tms == 1'b1) begin
            vh.state = RESET_TAP;
        end else begin
            vh.state = IDLE;
        end
    end
    IDLE: begin
        if (i_tms == 1'b1) begin
            vh.state = SELECT_DR_SCAN;
        end else begin
            vh.state = IDLE;
        end
    end
    SELECT_DR_SCAN: begin
        if (i_tms == 1'b1) begin
            vh.state = SELECT_IR_SCAN;
        end else begin
            vh.state = CAPTURE_DR;
        end
    end
    CAPTURE_DR: begin
        if (i_tms == 1'b1) begin
            vh.state = EXIT1_DR;
        end else begin
            vh.state = SHIFT_DR;
        end
        if (rnh.ir == IR_IDCODE) begin
            vb_dr = idcode;
            vh.dr_length = 7'd32;
        end else if (rnh.ir == IR_DTMCONTROL) begin
            vb_dr[31: 0] = '0;
            vb_dr[3: 0] = 4'h1;                             // version
            vb_dr[9: 4] = abits;                            // the size of the address
            vb_dr[11: 10] = rh.err_sticky;
            vh.dr_length = 7'd32;
        end else if (rnh.ir == IR_DBUS) begin
            if (i_dmi_error == 1'b1) begin
                vb_err_sticky = DMISTAT_FAILED;
                vb_dr[1: 0] = DMISTAT_FAILED;
            end else begin
                vb_dr[1: 0] = rh.err_sticky;
            end
            vb_dr[33: 2] = i_dmi_resp_data;
            vb_dr[((34 + abits) - 1): 34] = rnh.dmi_addr;
            vh.dr_length = (abits + 7'd34);
        end else if (rnh.ir == IR_BYPASS) begin
            vb_dr[0] = rh.bypass;
            vh.dr_length = 7'd1;
        end
        vh.datacnt = 32'd0;
    end
    SHIFT_DR: begin
        if (i_tms == 1'b1) begin
            vh.state = EXIT1_DR;
        end else begin
            vh.state = SHIFT_DR;
        end
        if (rh.dr_length > 7'd1) begin
            // For the bypass dr_length = 1
            vb_dr = {1'b0, rh.dr[(drlen - 1): 1]};
            vb_dr[(int'(rh.dr_length) - 1)] = i_tdi;
        end else begin
            vb_dr[0] = i_tdi;
        end
        vh.datacnt = (rh.datacnt + 1);                      // debug counter no need in rtl
    end
    EXIT1_DR: begin
        if (i_tms == 1'b1) begin
            vh.state = UPDATE_DR;
        end else begin
            vh.state = PAUSE_DR;
        end
    end
    PAUSE_DR: begin
        if (i_tms == 1'b1) begin
            vh.state = EXIT2_DR;
        end else begin
            vh.state = PAUSE_DR;
        end
    end
    EXIT2_DR: begin
        if (i_tms == 1'b1) begin
            vh.state = UPDATE_DR;
        end else begin
            vh.state = SHIFT_DR;
        end
    end
    UPDATE_DR: begin
        if (i_tms == 1'b1) begin
            vh.state = SELECT_DR_SCAN;
        end else begin
            vh.state = IDLE;
        end
        if (rnh.ir == IR_DTMCONTROL) begin
            v_dmi_hardreset = rh.dr[DTMCONTROL_DMIHARDRESET];
            if (rh.dr[DTMCONTROL_DMIRESET] == 1'b1) begin
                vb_err_sticky = DMISTAT_SUCCESS;
            end
        end else if (rnh.ir == IR_BYPASS) begin
            vh.bypass = rh.dr[0];
        end else if (rnh.ir == IR_DBUS) begin
            if (rh.err_sticky != DMISTAT_SUCCESS) begin
                // This operation should never result in a busy or error response.
            end else if (rh.dmi_busy == 1'b1) begin
                vb_err_sticky = DMISTAT_BUSY;
            end else begin
                v_dmi_req_valid = (|rh.dr[1: 0]);
            end
            v_dmi_req_write = rh.dr[1];
            vb_dmi_req_data = rh.dr[33: 2];
            vb_dmi_req_addr = rh.dr[((34 + abits) - 1): 34];

            vnh.dmi_addr = rh.dr[((34 + abits) - 1): 34];
        end
    end
    SELECT_IR_SCAN: begin
        if (i_tms == 1'b1) begin
            vh.state = RESET_TAP;
        end else begin
            vh.state = CAPTURE_IR;
        end
    end
    CAPTURE_IR: begin
        if (i_tms == 1'b1) begin
            vh.state = EXIT1_IR;
        end else begin
            vh.state = SHIFT_IR;
        end
        vb_dr[(irlen - 1): 2] = rnh.ir[(irlen - 1): 2];
        vb_dr[1: 0] = 2'h1;
    end
    SHIFT_IR: begin
        if (i_tms == 1'b1) begin
            vh.state = EXIT1_IR;
        end else begin
            vh.state = SHIFT_IR;
        end
        vb_dr[(irlen - 1)] = i_tdi;
        vb_dr[(irlen - 2): 0] = rh.dr[(irlen - 1): 1];
    end
    EXIT1_IR: begin
        if (i_tms == 1'b1) begin
            vh.state = UPDATE_IR;
        end else begin
            vh.state = PAUSE_IR;
        end
    end
    PAUSE_IR: begin
        if (i_tms == 1'b1) begin
            vh.state = EXIT2_IR;
        end else begin
            vh.state = PAUSE_IR;
        end
    end
    EXIT2_IR: begin
        if (i_tms == 1'b1) begin
            vh.state = UPDATE_IR;
        end else begin
            vh.state = SHIFT_IR;
        end
    end
    UPDATE_IR: begin
        if (i_tms == 1'b1) begin
            vh.state = SELECT_DR_SCAN;
        end else begin
            vh.state = IDLE;
        end
        vnh.ir = rh.dr[(irlen - 1): 0];
    end
    default: begin
    end
    endcase
    vh.dr = vb_dr;
    vh.dmi_busy = i_dmi_busy;
    vh.err_sticky = vb_err_sticky;

    o_tdo = rh.dr[0];
    o_dmi_req_valid = v_dmi_req_valid;
    o_dmi_req_write = v_dmi_req_write;
    o_dmi_req_data = vb_dmi_req_data;
    o_dmi_req_addr = vb_dmi_req_addr;
    o_dmi_hardreset = v_dmi_hardreset;

    rhin = vh;
    rnhin = vnh;
end: comb_proc

always_ff @(posedge i_tck, posedge i_trst) begin
    if (i_trst == 1'b1) begin
        rh <= jtagtap_rh_reset;
    end else begin
        rh <= rhin;
    end
end

always_ff @(negedge i_tck, posedge i_trst) begin
    if (i_trst == 1'b1) begin
        rnh <= jtagtap_rnh_reset;
    end else begin
        rnh <= rnhin;
    end
end

endmodule: jtagtap
