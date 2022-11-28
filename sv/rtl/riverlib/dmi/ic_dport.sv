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

module ic_dport #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    // DMI connection
    input logic [river_cfg_pkg::CFG_LOG2_CPU_MAX-1:0] i_hartsel,// Selected hart index
    input logic i_haltreq,
    input logic i_resumereq,
    input logic i_resethaltreq,                             // Halt core after reset request
    input logic i_hartreset,                                // Reset currently selected hart
    input logic i_dport_req_valid,                          // Debug access from DSU is valid
    input logic [river_cfg_pkg::DPortReq_Total-1:0] i_dport_req_type,// Debug access types
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_addr,// Register index
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_wdata,// Write value
    input logic [2:0] i_dport_size,                         // 0=1B;1=2B;2=4B;3=8B;4=128B
    output logic o_dport_req_ready,                         // Response is ready
    input logic i_dport_resp_ready,                         // ready to accept response
    output logic o_dport_resp_valid,                        // Response is valid
    output logic o_dport_resp_error,                        // Something goes wrong
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_dport_rdata,// Response value or error code
    // To Cores cluster
    output types_river_pkg::dport_in_vector o_dporti,
    input types_river_pkg::dport_out_vector i_dporto
);

import river_cfg_pkg::*;
import types_river_pkg::*;
import ic_dport_pkg::*;

ic_dport_registers r, rin;

always_comb
begin: comb_proc
    ic_dport_registers v;
    logic [CFG_LOG2_CPU_MAX-1:0] vb_hartsel;
    logic [CFG_CPU_MAX-1:0] vb_cpu_mask;
    logic [CFG_CPU_MAX-1:0] vb_req_ready_mask;
    logic [CFG_CPU_MAX-1:0] vb_req_valid_mask;
    logic [CFG_CPU_MAX-1:0] vb_haltreq;
    logic [CFG_CPU_MAX-1:0] vb_resumereq;
    logic [CFG_CPU_MAX-1:0] vb_resethaltreq;
    logic [CFG_CPU_MAX-1:0] vb_hartreset;
    logic [CFG_CPU_MAX-1:0] vb_req_valid;
    logic [CFG_CPU_MAX-1:0] vb_req_ready;
    dport_in_type vb_dporti[0: CFG_CPU_MAX-1];
    dport_out_type vb_dporto[0: CFG_CPU_MAX-1];
    logic v_req_accepted;

    vb_hartsel = 0;
    vb_cpu_mask = 0;
    vb_req_ready_mask = 0;
    vb_req_valid_mask = 0;
    vb_haltreq = 0;
    vb_resumereq = 0;
    vb_resethaltreq = 0;
    vb_hartreset = 0;
    vb_req_valid = 0;
    vb_req_ready = 0;
    for (int i = 0; i < CFG_CPU_MAX; i++) begin
        vb_dporti[i] = dport_in_none;
    end
    for (int i = 0; i < CFG_CPU_MAX; i++) begin
        vb_dporto[i] = dport_out_none;
    end
    v_req_accepted = 0;

    v = r;

    vb_cpu_mask[int'(i_hartsel)] = 1'h1;
    if (i_haltreq == 1'b1) begin
        vb_haltreq = ALL_CPU_MASK;
    end
    if (i_resumereq == 1'b1) begin
        vb_resumereq = ALL_CPU_MASK;
    end
    if (i_resethaltreq == 1'b1) begin
        vb_resethaltreq = ALL_CPU_MASK;
    end
    if (i_hartreset == 1'b1) begin
        vb_hartreset = ALL_CPU_MASK;
    end
    if (i_dport_req_valid == 1'b1) begin
        vb_req_valid = ALL_CPU_MASK;
    end
    for (int i = 0; i < CFG_CPU_MAX; i++) begin
        vb_req_ready[i] = i_dporto[i].req_ready;
    end

    vb_req_ready_mask = (vb_req_ready & vb_cpu_mask);
    vb_req_valid_mask = (vb_req_valid & vb_req_ready & vb_cpu_mask);
    v_req_accepted = (|vb_req_valid_mask);

    for (int i = 0; i < CFG_CPU_MAX; i++) begin
        vb_dporti[i].haltreq = (vb_haltreq[i] && vb_cpu_mask[i]);
        vb_dporti[i].resumereq = (vb_resumereq[i] && vb_cpu_mask[i]);
        vb_dporti[i].resethaltreq = (vb_resethaltreq[i] && vb_cpu_mask[i]);
        vb_dporti[i].hartreset = (vb_hartreset[i] && vb_cpu_mask[i]);
        vb_dporti[i].req_valid = (vb_req_valid[i] && vb_cpu_mask[i]);
        vb_dporti[i].dtype = i_dport_req_type;
        vb_dporti[i].addr = i_dport_addr;
        vb_dporti[i].wdata = i_dport_wdata;
        vb_dporti[i].size = i_dport_size;
        vb_dporti[i].resp_ready = i_dport_resp_ready;
    end

    if (v_req_accepted == 1'b1) begin
        vb_hartsel = i_hartsel;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = ic_dport_r_reset;
    end

    for (int i = 0; i < CFG_CPU_MAX; i++) begin
        o_dporti[i] = vb_dporti[i];
    end

    o_dport_req_ready = (|vb_req_ready_mask);
    o_dport_resp_valid = i_dporto[int'(r.hartsel)].resp_valid;
    o_dport_resp_error = i_dporto[int'(r.hartsel)].resp_error;
    o_dport_rdata = i_dporto[int'(r.hartsel)].rdata;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= ic_dport_r_reset;
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

endmodule: ic_dport
