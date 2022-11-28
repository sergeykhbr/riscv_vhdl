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

module InstrFetch #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_bp_valid,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_bp_pc,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_requested_pc,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_fetching_pc,
    input logic i_mem_req_ready,
    output logic o_mem_addr_valid,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_mem_addr,
    input logic i_mem_data_valid,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_mem_data_addr,
    input logic [63:0] i_mem_data,
    input logic i_mem_load_fault,
    input logic i_mem_page_fault_x,
    output logic o_mem_resp_ready,
    input logic i_flush_pipeline,                           // reset pipeline and cache
    input logic i_progbuf_ena,                              // executing from prog buffer
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_progbuf_pc,// progbuf counter
    input logic [63:0] i_progbuf_instr,                     // progbuf instruction
    output logic o_instr_load_fault,
    output logic o_instr_page_fault_x,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pc,
    output logic [63:0] o_instr
);

import river_cfg_pkg::*;
import fetch_pkg::*;

InstrFetch_registers r, rin;

always_comb
begin: comb_proc
    InstrFetch_registers v;
    v = r;

    case (r.state)
    Idle: begin
        v.req_valid = 1'b0;
        v.resp_ready = 1'b0;
        v.progbuf_ena = 1'b0;
        if (i_progbuf_ena == 1'b1) begin
            // Execution from buffer
            v.progbuf_ena = 1'b1;
            v.pc = i_progbuf_pc;
            v.instr = i_progbuf_instr;
            v.instr_load_fault = 1'b0;
            v.instr_page_fault_x = 1'b0;
        end else if (i_bp_valid == 1'b1) begin
            v.state = WaitReqAccept;
            v.req_addr = i_bp_pc;
            v.req_valid = 1'b1;
        end
    end
    WaitReqAccept: begin
        if (i_mem_req_ready == 1'b1) begin
            v.req_valid = (i_bp_valid && (~i_progbuf_ena));
            v.req_addr = i_bp_pc;
            v.mem_resp_shadow = r.req_addr;
            v.resp_ready = 1'b1;
            v.state = WaitResp;
        end else if (i_bp_valid == 1'b1) begin
            // re-write requested address (while it wasn't accepted)
            v.req_addr = i_bp_pc;
        end
    end
    WaitResp: begin
        if (i_mem_data_valid == 1'b1) begin
            v.pc = i_mem_data_addr;
            v.instr = i_mem_data;
            v.instr_load_fault = i_mem_load_fault;
            v.instr_page_fault_x = i_mem_page_fault_x;
            v.req_valid = (i_bp_valid && (~i_progbuf_ena));

            if (r.req_valid == 1'b1) begin
                if (i_mem_req_ready == 1'b1) begin
                    v.req_addr = i_bp_pc;
                    v.mem_resp_shadow = r.req_addr;
                end else begin
                    v.state = WaitReqAccept;
                end
            end else if ((i_bp_valid == 1'b1) && (i_progbuf_ena == 1'b0)) begin
                v.req_addr = i_bp_pc;
                v.state = WaitReqAccept;
            end else begin
                v.req_addr = '1;
                v.state = Idle;
            end
        end
    end
    default: begin
    end
    endcase

    if (i_flush_pipeline == 1'b1) begin
        // Clear pipeline stage
        v.req_valid = 1'b0;
        v.pc = '1;
        v.instr = '0;
        v.instr_load_fault = 1'b0;
        v.instr_page_fault_x = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = InstrFetch_r_reset;
    end

    o_mem_addr_valid = r.req_valid;
    o_mem_addr = r.req_addr;
    o_mem_resp_ready = r.resp_ready;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_page_fault_x = r.instr_page_fault_x;
    o_requested_pc = r.req_addr;
    o_fetching_pc = r.mem_resp_shadow;
    o_pc = r.pc;
    o_instr = r.instr;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= InstrFetch_r_reset;
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

endmodule: InstrFetch
