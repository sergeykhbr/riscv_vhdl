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

module StackTraceBuffer(
    input logic i_clk,                                      // CPU clock
    input logic [river_cfg_pkg::CFG_LOG2_STACK_TRACE_ADDR-1:0] i_raddr,
    output logic [(2 * river_cfg_pkg::RISCV_ARCH)-1:0] o_rdata,
    input logic i_we,
    input logic [river_cfg_pkg::CFG_LOG2_STACK_TRACE_ADDR-1:0] i_waddr,
    input logic [(2 * river_cfg_pkg::RISCV_ARCH)-1:0] i_wdata
);

import river_cfg_pkg::*;
import stacktrbuf_pkg::*;

StackTraceBuffer_rxegisters rx, rxin;


always_comb
begin: comb_proc
    StackTraceBuffer_rxegisters vx;
    vx.raddr = rx.raddr;
    for (int i = 0; i < STACK_TRACE_BUF_SIZE; i++) begin
        vx.stackbuf[i] = rx.stackbuf[i];
    end

    vx.raddr = i_raddr;
    if (i_we == 1'b1) begin
        vx.stackbuf[int'(i_waddr)] = i_wdata;
    end

    o_rdata = rx.stackbuf[int'(rx.raddr)];

    rxin.raddr = vx.raddr;
    for (int i = 0; i < STACK_TRACE_BUF_SIZE; i++) begin
        rxin.stackbuf[i] = vx.stackbuf[i];
    end
end: comb_proc

always_ff @(posedge i_clk) begin: rxg_proc
    rx.raddr <= rxin.raddr;
    for (int i = 0; i < STACK_TRACE_BUF_SIZE; i++) begin
        rx.stackbuf[i] <= rxin.stackbuf[i];
    end
end: rxg_proc

endmodule: StackTraceBuffer
