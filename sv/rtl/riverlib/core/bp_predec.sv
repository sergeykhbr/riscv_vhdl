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

module BpPreDecoder(
    input logic i_c_valid,                                  // Use compressed for prediction
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_addr,     // Memory response address
    input logic [31:0] i_data,                              // Memory response value
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_ra,       // Return address register value
    output logic o_jmp,                                     // Jump detected
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pc,      // Fetching instruction pointer
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_npc      // Fetching instruction pointer
);

import river_cfg_pkg::*;
import bp_predec_pkg::*;

logic [RISCV_ARCH-1:0] vb_npc;
logic v_jal;                                                // JAL instruction
logic v_branch;                                             // One of branch instructions (only negative offset)
logic v_c_j;                                                // compressed J instruction
logic v_c_ret;                                              // compressed RET pseudo-instruction

always_comb
begin: comb_proc
    logic [31:0] vb_tmp;
    logic [RISCV_ARCH-1:0] vb_npc;
    logic [RISCV_ARCH-1:0] vb_pc;
    logic [RISCV_ARCH-1:0] vb_jal_off;
    logic [RISCV_ARCH-1:0] vb_jal_addr;
    logic [RISCV_ARCH-1:0] vb_branch_off;
    logic [RISCV_ARCH-1:0] vb_branch_addr;
    logic [RISCV_ARCH-1:0] vb_c_j_off;
    logic [RISCV_ARCH-1:0] vb_c_j_addr;

    vb_tmp = 0;
    vb_npc = 0;
    vb_pc = 0;
    vb_jal_off = 0;
    vb_jal_addr = 0;
    vb_branch_off = 0;
    vb_branch_addr = 0;
    vb_c_j_off = 0;
    vb_c_j_addr = 0;

    vb_pc = i_addr;
    vb_tmp = i_data;

    // Unconditional jump "J"
    if (vb_tmp[31] == 1'b1) begin
        vb_jal_off[(RISCV_ARCH - 1): 20] = '1;
    end else begin
        vb_jal_off[(RISCV_ARCH - 1): 20] = '0;
    end
    vb_jal_off[19: 12] = vb_tmp[19: 12];
    vb_jal_off[11] = vb_tmp[20];
    vb_jal_off[10: 1] = vb_tmp[30: 21];
    vb_jal_off[0] = '0;
    vb_jal_addr = (vb_pc + vb_jal_off);

    v_jal = 1'b0;
    if (vb_tmp[6: 0] == 7'h6f) begin
        v_jal = 1'b1;
    end

    // Conditional branches "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU"
    // Only negative offset leads to predicted jumps
    if (vb_tmp[31] == 1'b1) begin
        vb_branch_off[(RISCV_ARCH - 1): 12] = '1;
    end else begin
        vb_branch_off[(RISCV_ARCH - 1): 12] = '0;
    end
    vb_branch_off[11] = vb_tmp[7];
    vb_branch_off[10: 5] = vb_tmp[30: 25];
    vb_branch_off[4: 1] = vb_tmp[11: 8];
    vb_branch_off[0] = '0;
    vb_branch_addr = (vb_pc + vb_branch_off);

    v_branch = 1'b0;
    if (((vb_tmp[6: 0] == 7'h63) && vb_tmp[31]) == 1'b1) begin
        v_branch = 1'b1;
    end

    // Check Compressed "C_J" unconditional jump
    if (vb_tmp[12] == 1'b1) begin
        vb_c_j_off[(RISCV_ARCH - 1): 11] = '1;
    end else begin
        vb_c_j_off[(RISCV_ARCH - 1): 11] = '0;
    end
    vb_c_j_off[10] = vb_tmp[8];
    vb_c_j_off[9: 8] = vb_tmp[10: 9];
    vb_c_j_off[7] = vb_tmp[6];
    vb_c_j_off[6] = vb_tmp[7];
    vb_c_j_off[5] = vb_tmp[2];
    vb_c_j_off[4] = vb_tmp[11];
    vb_c_j_off[3: 1] = vb_tmp[5: 3];
    vb_c_j_off[0] = '0;
    vb_c_j_addr = (vb_pc + vb_c_j_off);

    v_c_j = 1'b0;
    if ((vb_tmp[15: 13] == 3'h5) && (vb_tmp[1: 0] == 2'h1)) begin
        v_c_j = i_c_valid;
    end

    // Compressed RET pseudo-instruction
    v_c_ret = 1'b0;
    if (vb_tmp[15: 0] == 16'h8082) begin
        v_c_ret = i_c_valid;
    end

    if (v_jal == 1'b1) begin
        vb_npc = vb_jal_addr;
    end else if (v_branch == 1'b1) begin
        vb_npc = vb_branch_addr;
    end else if (v_c_j == 1'b1) begin
        vb_npc = vb_c_j_addr;
    end else if (v_c_ret == 1'b1) begin
        vb_npc = i_ra[(RISCV_ARCH - 1): 0];
    end else begin
        vb_npc = (vb_pc + 4);
    end

    o_jmp = (v_jal || v_branch || v_c_j || v_c_ret);
    o_pc = vb_pc;
    o_npc = vb_npc;
end: comb_proc

endmodule: BpPreDecoder
