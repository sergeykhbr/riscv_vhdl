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

module vip_sdcard_top #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_nrst,                                     // To avoid undefined states of registers (xxx)
    input logic i_sclk,
    inout logic io_cmd,
    inout logic io_dat0,
    inout logic io_dat1,
    inout logic io_dat2,
    inout logic io_cd_dat3
);

import vip_sdcard_top_pkg::*;

logic w_clk;
logic [7:0] wb_rdata;
logic w_cmd_in;
logic w_cmd_out;
vip_sdcard_top_registers r, rin;

iobuf_tech iobufcmd0 (
    .io(io_cmd),
    .o(w_cmd_in),
    .i(w_cmd_out),
    .t(r.cmd_dir)
);


always_comb
begin: comb_proc
    vip_sdcard_top_registers v;
    logic [47:0] vb_cmd_txshift;

    vb_cmd_txshift = 0;

    v = r;

    vb_cmd_txshift = {r.cmd_txshift[46: 0], 1'h1};

    case (r.cmd_state)
    CMDSTATE_IDLE: begin
        v.cmd_dir = 1'b1;
        if (r.cmd_rxshift[7: 6] == 2'h1) begin
            v.cmd_state = CMDSTATE_REQ_ARG;
            v.bitcnt = 6'h1f;
        end
    end
    CMDSTATE_REQ_ARG: begin
        if ((|r.bitcnt) == 1'b0) begin
            v.cmd_state = CMDSTATE_REQ_CRC7;
            v.bitcnt = 6'h06;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_REQ_CRC7: begin
        if ((|r.bitcnt) == 1'b0) begin
            v.cmd_state = CMDSTATE_REQ_STOPBIT;
            v.bitcnt = 6'h0a;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_REQ_STOPBIT: begin
        v.cmd_state = CMDSTATE_WAIT_RESP;
        v.cmd_dir = 1'b0;
    end
    CMDSTATE_WAIT_RESP: begin
        // Preparing output with some delay (several clocks):
        if ((|r.bitcnt) == 1'b0) begin
            v.cmd_state = CMDSTATE_RESP;
            v.bitcnt = 6'h2f;
            vb_cmd_txshift[47: 46] = 2'h0;
            vb_cmd_txshift[45: 40] = r.cmd_rxshift[45: 40];
            vb_cmd_txshift[39: 8] = 32'h55555555;
            vb_cmd_txshift[7: 0] = 8'h7d;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    CMDSTATE_RESP: begin
        if ((|r.bitcnt) == 1'b0) begin
            v.cmd_state = CMDSTATE_IDLE;
            v.cmd_dir = 1'b1;
        end else begin
            v.bitcnt = (r.bitcnt - 1);
        end
    end
    default: begin
    end
    endcase

    if (r.cmd_state < CMDSTATE_REQ_STOPBIT) begin
        // This will includes clock with the stopbit itself
        v.cmd_rxshift = {r.cmd_rxshift[46: 0], w_cmd_in};
        v.cmd_txshift = '1;
    end else begin
        if ((r.cmd_state == CMDSTATE_RESP) && ((|r.bitcnt) == 1'b0)) begin
            v.cmd_rxshift = '1;
        end
        v.cmd_txshift = vb_cmd_txshift;
    end

    w_cmd_out = r.cmd_txshift[47];

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_sclk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_sdcard_top_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_sclk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: vip_sdcard_top
