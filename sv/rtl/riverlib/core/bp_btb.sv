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

module BpBTB #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_flush_pipeline,                           // sync reset BTB
    input logic i_e,                                        // executed jump
    input logic i_we,                                       // Write enable into BTB
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_we_pc,    // Jump start instruction address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_we_npc,   // Jump target address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_bp_pc,    // Start address of the prediction sequence
    output logic [(river_cfg_pkg::CFG_BP_DEPTH * river_cfg_pkg::RISCV_ARCH)-1:0] o_bp_npc,// Predicted sequence
    output logic [river_cfg_pkg::CFG_BP_DEPTH-1:0] o_bp_exec// Predicted value was jump-executed before
);

import river_cfg_pkg::*;
import bp_btb_pkg::*;

logic [RISCV_ARCH-1:0] dbg_npc[0: CFG_BP_DEPTH - 1];
BpBTB_registers r, rin;

always_comb
begin: comb_proc
    BpBTB_registers v;
    logic [(CFG_BP_DEPTH * RISCV_ARCH)-1:0] vb_addr;
    logic [CFG_BP_DEPTH-1:0] vb_hit;
    logic [RISCV_ARCH-1:0] t_addr;
    logic [CFG_BTB_SIZE-1:0] vb_pc_equal;
    logic [CFG_BTB_SIZE-1:0] vb_pc_nshift;
    logic [CFG_BP_DEPTH-1:0] vb_bp_exec;
    logic v_dont_update;

    vb_addr = 0;
    vb_hit = 0;
    t_addr = 0;
    vb_pc_equal = 0;
    vb_pc_nshift = 0;
    vb_bp_exec = 0;
    v_dont_update = 0;

    for (int i = 0; i < CFG_BTB_SIZE; i++) begin
        v.btb[i].pc = r.btb[i].pc;
        v.btb[i].npc = r.btb[i].npc;
        v.btb[i].exec = r.btb[i].exec;
    end

    vb_addr[(RISCV_ARCH - 1): 0] = i_bp_pc;
    vb_bp_exec[0] = i_e;

    for (int i = 1; i < CFG_BP_DEPTH; i++) begin
        t_addr = vb_addr[((i - 1) * RISCV_ARCH) +: RISCV_ARCH];
        for (int n = (CFG_BTB_SIZE - 1); n >= 0; n--) begin
            if (t_addr == r.btb[n].pc) begin
                vb_addr[(i * RISCV_ARCH) +: RISCV_ARCH] = r.btb[n].npc;
                vb_hit[i] = 1'h1;
                vb_bp_exec[i] = r.btb[n].exec;              // Used for: Do not override by pre-decoded jumps
            end else if (vb_hit[i] == 1'b0) begin
                vb_addr[(i * RISCV_ARCH) +: RISCV_ARCH] = (t_addr + 4);
            end
        end
    end

    v_dont_update = '0;
    vb_pc_equal = '0;
    for (int i = 0; i < CFG_BTB_SIZE; i++) begin
        if (r.btb[i].pc == i_we_pc) begin
            vb_pc_equal[i] = 1;
            v_dont_update = (r.btb[i].exec && (~i_e));
        end
    end
    vb_pc_nshift = '0;
    for (int i = 1; i < CFG_BTB_SIZE; i++) begin
        vb_pc_nshift[i] = (vb_pc_equal[(i - 1)] || vb_pc_nshift[(i - 1)]);
    end

    if ((i_we && (~v_dont_update)) == 1'b1) begin
        v.btb[0].exec = i_e;
        v.btb[0].pc = i_we_pc;
        v.btb[0].npc = i_we_npc;
        for (int i = 1; i < CFG_BTB_SIZE; i++) begin
            if (vb_pc_nshift[i] == 1'b0) begin
                v.btb[i] = r.btb[(i - 1)];
            end else begin
                v.btb[i] = r.btb[i];
            end
        end
    end

    if ((~async_reset && i_nrst == 1'b0) || i_flush_pipeline) begin
        for (int i = 0; i < CFG_BTB_SIZE; i++) begin
            v.btb[i].pc = '1;
            v.btb[i].npc = 64'h0000000000000000;
            v.btb[i].exec = 1'h0;
        end
    end

    for (int i = 0; i < CFG_BP_DEPTH; i++) begin
        dbg_npc[i] = vb_addr[(i * RISCV_ARCH) +: RISCV_ARCH];
    end
    o_bp_npc = vb_addr;
    o_bp_exec = vb_bp_exec;

    for (int i = 0; i < CFG_BTB_SIZE; i++) begin
        rin.btb[i].pc = v.btb[i].pc;
        rin.btb[i].npc = v.btb[i].npc;
        rin.btb[i].exec = v.btb[i].exec;
    end
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                for (int i = 0; i < CFG_BTB_SIZE; i++) begin
                    r.btb[i].pc <= '1;
                    r.btb[i].npc <= 64'h0000000000000000;
                    r.btb[i].exec <= 1'h0;
                end
            end else begin
                for (int i = 0; i < CFG_BTB_SIZE; i++) begin
                    r.btb[i].pc <= rin.btb[i].pc;
                    r.btb[i].npc <= rin.btb[i].npc;
                    r.btb[i].exec <= rin.btb[i].exec;
                end
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            for (int i = 0; i < CFG_BTB_SIZE; i++) begin
                r.btb[i].pc <= rin.btb[i].pc;
                r.btb[i].npc <= rin.btb[i].npc;
                r.btb[i].exec <= rin.btb[i].exec;
            end
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: BpBTB
