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

module BranchPredictor #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_flush_pipeline,                           // sync reset BTB
    input logic i_resp_mem_valid,                           // Memory response from ICache is valid
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_resp_mem_addr,// Memory response address
    input logic [63:0] i_resp_mem_data,                     // Memory response value
    input logic i_e_jmp,                                    // jump was executed
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_pc,     // Previous 'Executor' instruction
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_npc,    // Valid instruction value awaited by 'Executor'
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_ra,       // Return address register value
    output logic o_f_valid,                                 // Fetch request is valid
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_f_pc,    // Fetching instruction pointer
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_f_requested_pc,// already requested but not accepted address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_f_fetching_pc,// currently memory address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_f_fetched_pc,// already requested and fetched address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_d_pc      // decoded instructions
);

import river_cfg_pkg::*;
import bp_pkg::*;

PreDecType wb_pd[0: 2 - 1];
logic w_btb_e;
logic w_btb_we;
logic [RISCV_ARCH-1:0] wb_btb_we_pc;
logic [RISCV_ARCH-1:0] wb_btb_we_npc;
logic [RISCV_ARCH-1:0] wb_start_pc;
logic [(CFG_BP_DEPTH * RISCV_ARCH)-1:0] wb_npc;
logic [CFG_BP_DEPTH-1:0] wb_bp_exec;                        // Predicted value was jump-executed before

for (genvar i = 0; i < 2; i++) begin: predecx
    BpPreDecoder predec (
        .i_c_valid(wb_pd[i].c_valid),
        .i_addr(wb_pd[i].addr),
        .i_data(wb_pd[i].data),
        .i_ra(i_ra),
        .o_jmp(wb_pd[i].jmp),
        .o_pc(wb_pd[i].pc),
        .o_npc(wb_pd[i].npc)
    );

end: predecx

BpBTB #(
    .async_reset(async_reset)
) btb (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_flush_pipeline(i_flush_pipeline),
    .i_e(w_btb_e),
    .i_we(w_btb_we),
    .i_we_pc(wb_btb_we_pc),
    .i_we_npc(wb_btb_we_npc),
    .i_bp_pc(wb_start_pc),
    .o_bp_npc(wb_npc),
    .o_bp_exec(wb_bp_exec)
);


always_comb
begin: comb_proc
    logic [RISCV_ARCH-1:0] vb_addr[0: CFG_BP_DEPTH-1];
    logic [(RISCV_ARCH - 2)-1:0] vb_piped[0: 4-1];
    logic [RISCV_ARCH-1:0] vb_fetch_npc;
    logic v_btb_we;
    logic [RISCV_ARCH-1:0] vb_btb_we_pc;
    logic [RISCV_ARCH-1:0] vb_btb_we_npc;
    logic [3:0] vb_hit;
    logic [1:0] vb_ignore_pd;

    for (int i = 0; i < CFG_BP_DEPTH; i++) begin
        vb_addr[i] = 64'h0000000000000000;
    end
    for (int i = 0; i < 4; i++) begin
        vb_piped[i] = 62'h0000000000000000;
    end
    vb_fetch_npc = 0;
    v_btb_we = 0;
    vb_btb_we_pc = 0;
    vb_btb_we_npc = 0;
    vb_hit = 0;
    vb_ignore_pd = 0;

    // Transform address into 2-dimesional array for convinience
    for (int i = 0; i < CFG_BP_DEPTH; i++) begin
        vb_addr[i] = wb_npc[(i * RISCV_ARCH) +: RISCV_ARCH];
    end

    vb_piped[0] = i_d_pc[(RISCV_ARCH - 1): 2];
    vb_piped[1] = i_f_fetched_pc[(RISCV_ARCH - 1): 2];
    vb_piped[2] = i_f_fetching_pc[(RISCV_ARCH - 1): 2];
    vb_piped[3] = i_f_requested_pc[(RISCV_ARCH - 1): 2];
    // Check availablity of pc in pipeline
    vb_hit = '0;
    for (int n = 0; n < 4; n++) begin
        for (int i = n; i < 4; i++) begin
            if (vb_addr[n][(RISCV_ARCH - 1): 2] == vb_piped[i]) begin
                vb_hit[n] = 1'h1;
            end
        end
    end

    vb_fetch_npc = vb_addr[(CFG_BP_DEPTH - 1)];
    for (int i = 3; i >= 0; i--) begin
        if (vb_hit[i] == 1'b0) begin
            vb_fetch_npc = vb_addr[i];
        end
    end

    // Pre-decoder input signals (not used for now)
    for (int i = 0; i < 2; i++) begin
        wb_pd[i].c_valid = (~(&i_resp_mem_data[(16 * i) +: 2]));
        wb_pd[i].addr = (i_resp_mem_addr + (2 * i));
        wb_pd[i].data = i_resp_mem_data[(16 * i) +: 32];
    end
    vb_ignore_pd = '0;
    for (int i = 0; i < 4; i++) begin
        if (wb_pd[0].npc[(RISCV_ARCH - 1): 2] == vb_piped[i]) begin
            vb_ignore_pd[0] = 1'h1;
        end
        if (wb_pd[1].npc[(RISCV_ARCH - 1): 2] == vb_piped[i]) begin
            vb_ignore_pd[1] = 1'h1;
        end
    end

    v_btb_we = (i_e_jmp || wb_pd[0].jmp || wb_pd[1].jmp);
    if (i_e_jmp == 1'b1) begin
        vb_btb_we_pc = i_e_pc;
        vb_btb_we_npc = i_e_npc;
    end else if (wb_pd[0].jmp) begin
        vb_btb_we_pc = wb_pd[0].pc;
        vb_btb_we_npc = wb_pd[0].npc;
        if ((vb_hit[2: 0] == 3'h7) && (wb_bp_exec[2] == 1'b0) && (vb_ignore_pd[0] == 1'b0)) begin
            vb_fetch_npc = wb_pd[0].npc;
        end
    end else if (wb_pd[1].jmp) begin
        vb_btb_we_pc = wb_pd[1].pc;
        vb_btb_we_npc = wb_pd[1].npc;
        if ((vb_hit[2: 0] == 3'h7) && (wb_bp_exec[2] == 1'b0) && (vb_ignore_pd[1] == 1'b0)) begin
            vb_fetch_npc = wb_pd[1].npc;
        end
    end else begin
        vb_btb_we_pc = i_e_pc;
        vb_btb_we_npc = i_e_npc;
    end

    wb_start_pc = i_e_npc;
    w_btb_e = i_e_jmp;
    w_btb_we = v_btb_we;
    wb_btb_we_pc = vb_btb_we_pc;
    wb_btb_we_npc = vb_btb_we_npc;

    o_f_valid = 1'h1;
    o_f_pc = {vb_fetch_npc[(RISCV_ARCH - 1): 2], 2'h0};
end: comb_proc

endmodule: BranchPredictor
