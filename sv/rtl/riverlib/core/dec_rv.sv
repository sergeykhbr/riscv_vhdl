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

module DecoderRv #(
    parameter bit async_reset = 1'b0,
    parameter bit fpu_ena = 1'b1
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic i_flush_pipeline,                           // reset pipeline and cache
    input logic i_progbuf_ena,                              // executing from progbuf
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_f_pc,     // Fetched pc
    input logic [31:0] i_f_instr,                           // Fetched instruction value
    input logic i_instr_load_fault,                         // fault instruction's address
    input logic i_instr_page_fault_x,                       // IMMU page fault signal
    output logic [5:0] o_radr1,                             // register bank address 1 (rs1)
    output logic [5:0] o_radr2,                             // register bank address 2 (rs2)
    output logic [5:0] o_waddr,                             // register bank output (rd)
    output logic [11:0] o_csr_addr,                         // CSR bank output
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_imm,     // immediate constant decoded from instruction
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pc,      // Current instruction pointer value
    output logic [31:0] o_instr,                            // Current instruction value
    output logic o_memop_store,                             // Store to memory operation
    output logic o_memop_load,                              // Load from memoru operation
    output logic o_memop_sign_ext,                          // Load memory value with sign extending
    output logic [1:0] o_memop_size,                        // Memory transaction size
    output logic o_rv32,                                    // 32-bits instruction
    output logic o_compressed,                              // C-type instruction
    output logic o_amo,                                     // A-type instruction
    output logic o_f64,                                     // 64-bits FPU (D-extension)
    output logic o_unsigned_op,                             // Unsigned operands
    output logic [river_cfg_pkg::ISA_Total-1:0] o_isa_type, // Instruction format accordingly with ISA
    output logic [river_cfg_pkg::Instr_Total-1:0] o_instr_vec,// One bit per decoded instruction bus
    output logic o_exception,                               // Exception detected
    output logic o_instr_load_fault,                        // fault instruction's address
    output logic o_instr_page_fault_x,                      // IMMU page fault signal
    output logic o_progbuf_ena                              // Debug execution from progbuf
);

import river_cfg_pkg::*;
import dec_rv_pkg::*;

DecoderRv_registers r, rin;

always_comb
begin: comb_proc
    DecoderRv_registers v;
    logic v_error;
    logic v_compressed;
    logic [31:0] vb_instr;
    logic [4:0] vb_opcode1;
    logic [2:0] vb_opcode2;
    logic [Instr_Total-1:0] vb_dec;
    logic [ISA_Total-1:0] vb_isa_type;
    logic [5:0] vb_radr1;
    logic [5:0] vb_radr2;
    logic [5:0] vb_waddr;
    logic [11:0] vb_csr_addr;
    logic [RISCV_ARCH-1:0] vb_imm;
    logic v_memop_store;
    logic v_memop_load;
    logic v_memop_sign_ext;
    logic [1:0] vb_memop_size;
    logic v_unsigned_op;
    logic v_rv32;
    logic v_f64;
    logic v_amo;

    v_error = 0;
    v_compressed = 0;
    vb_instr = 0;
    vb_opcode1 = 0;
    vb_opcode2 = 0;
    vb_dec = 0;
    vb_isa_type = 0;
    vb_radr1 = 0;
    vb_radr2 = 0;
    vb_waddr = 0;
    vb_csr_addr = 0;
    vb_imm = 0;
    v_memop_store = 0;
    v_memop_load = 0;
    v_memop_sign_ext = 0;
    vb_memop_size = 0;
    v_unsigned_op = 0;
    v_rv32 = 0;
    v_f64 = 0;
    v_amo = 0;

    v = r;

    vb_instr = i_f_instr;

    if (vb_instr[1: 0] != 2'h3) begin
        v_compressed = 1'b1;
    end

    vb_opcode1 = vb_instr[6: 2];
    vb_opcode2 = vb_instr[14: 12];
    case (vb_opcode1)
    OPCODE_AMO: begin
        vb_isa_type[ISA_R_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_radr2 = {1'h0, vb_instr[24: 20]};
        vb_waddr = vb_instr[11: 7];
        case (vb_instr[31: 27])
        5'h00: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOADD_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOADD_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h01: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOSWAP_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOSWAP_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h02: begin
            if ((vb_opcode2 == 3'h2) && ((|vb_instr[24: 20]) == 1'b0)) begin
                vb_dec[Instr_LR_W] = 1'b1;
            end else if ((vb_opcode2 == 3'h3) && ((|vb_instr[24: 20]) == 1'b0)) begin
                vb_dec[Instr_LR_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h03: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_SC_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_SC_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h04: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOXOR_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOXOR_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h08: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOOR_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOOR_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h0c: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOAND_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOAND_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h10: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOMIN_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOMIN_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h14: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOMAX_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOMAX_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h18: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOMINU_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOMINU_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        5'h1c: begin
            if (vb_opcode2 == 3'h2) begin
                vb_dec[Instr_AMOMAXU_W] = 1'b1;
            end else if (vb_opcode2 == 3'h3) begin
                vb_dec[Instr_AMOMAXU_D] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_ADD: begin
        vb_isa_type[ISA_R_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_radr2 = {1'h0, vb_instr[24: 20]};
        vb_waddr = vb_instr[11: 7];                         // rdc
        case (vb_opcode2)
        3'h0: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_ADD] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_MUL] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h20) begin
                vb_dec[Instr_SUB] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h1: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_SLL] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_MULH] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h2: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_SLT] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_MULHSU] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h3: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_SLTU] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_MULHU] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h4: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_XOR] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_DIV] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h5: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_SRL] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_DIVU] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h20) begin
                vb_dec[Instr_SRA] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h6: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_OR] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_REM] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h7: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_AND] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_REMU] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_ADDI: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm = vb_instr[31: 20];
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 12] = '1;
        end
        case (vb_opcode2)
        3'h0: begin
            vb_dec[Instr_ADDI] = 1'b1;
        end
        3'h1: begin
            vb_dec[Instr_SLLI] = 1'b1;
        end
        3'h2: begin
            vb_dec[Instr_SLTI] = 1'b1;
        end
        3'h3: begin
            vb_dec[Instr_SLTIU] = 1'b1;
        end
        3'h4: begin
            vb_dec[Instr_XORI] = 1'b1;
        end
        3'h5: begin
            if (vb_instr[31: 26] == 6'h00) begin
                vb_dec[Instr_SRLI] = 1'b1;
            end else if (vb_instr[31: 26] == 6'h10) begin
                vb_dec[Instr_SRAI] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h6: begin
            vb_dec[Instr_ORI] = 1'b1;
        end
        3'h7: begin
            vb_dec[Instr_ANDI] = 1'b1;
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_ADDIW: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm = vb_instr[31: 20];
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 12] = '1;
        end
        case (vb_opcode2)
        3'h0: begin
            vb_dec[Instr_ADDIW] = 1'b1;
        end
        3'h1: begin
            vb_dec[Instr_SLLIW] = 1'b1;
        end
        3'h5: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_SRLIW] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h20) begin
                vb_dec[Instr_SRAIW] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_ADDW: begin
        vb_isa_type[ISA_R_type] = 1'h1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_radr2 = {1'h0, vb_instr[24: 20]};
        vb_waddr = vb_instr[11: 7];                         // rd
        case (vb_opcode2)
        3'h0: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_ADDW] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_MULW] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h20) begin
                vb_dec[Instr_SUBW] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h1: begin
            vb_dec[Instr_SLLW] = 1'b1;
        end
        3'h4: begin
            if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_DIVW] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h5: begin
            if (vb_instr[31: 25] == 7'h00) begin
                vb_dec[Instr_SRLW] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_DIVUW] = 1'b1;
            end else if (vb_instr[31: 25] == 7'h20) begin
                vb_dec[Instr_SRAW] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h6: begin
            if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_REMW] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        3'h7: begin
            if (vb_instr[31: 25] == 7'h01) begin
                vb_dec[Instr_REMUW] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_AUIPC: begin
        vb_isa_type[ISA_U_type] = 1'b1;
        vb_dec[Instr_AUIPC] = 1'b1;
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm[31: 12] = vb_instr[31: 12];
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 32] = '1;
        end
    end
    OPCODE_BEQ: begin
        vb_isa_type[ISA_SB_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_radr2 = vb_instr[24: 20];
        vb_imm[11: 1] = {vb_instr[7], vb_instr[30: 25], vb_instr[11: 8]};
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 12] = '1;
        end
        case (vb_opcode2)
        3'h0: begin
            vb_dec[Instr_BEQ] = 1'b1;
        end
        3'h1: begin
            vb_dec[Instr_BNE] = 1'b1;
        end
        3'h4: begin
            vb_dec[Instr_BLT] = 1'b1;
        end
        3'h5: begin
            vb_dec[Instr_BGE] = 1'b1;
        end
        3'h6: begin
            vb_dec[Instr_BLTU] = 1'b1;
        end
        3'h7: begin
            vb_dec[Instr_BGEU] = 1'b1;
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_JAL: begin
        vb_isa_type[ISA_UJ_type] = 1'b1;
        vb_dec[Instr_JAL] = 1'b1;
        vb_waddr = {1'h0, vb_instr[11: 7]};                 // rd
        vb_imm[19: 1] = {vb_instr[19: 12], vb_instr[20], vb_instr[30: 21]};
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 20] = '1;
        end
    end
    OPCODE_JALR: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm[11: 0] = vb_instr[31: 20];
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 12] = '1;
        end
        case (vb_opcode2)
        3'h0: begin
            vb_dec[Instr_JALR] = 1'b1;
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_LB: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm[11: 0] = vb_instr[31: 20];
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 12] = '1;
        end
        case (vb_opcode2)
        3'h0: begin
            vb_dec[Instr_LB] = 1'b1;
        end
        3'h1: begin
            vb_dec[Instr_LH] = 1'b1;
        end
        3'h2: begin
            vb_dec[Instr_LW] = 1'b1;
        end
        3'h3: begin
            vb_dec[Instr_LD] = 1'b1;
        end
        3'h4: begin
            vb_dec[Instr_LBU] = 1'b1;
        end
        3'h5: begin
            vb_dec[Instr_LHU] = 1'b1;
        end
        3'h6: begin
            vb_dec[Instr_LWU] = 1'b1;
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_LUI: begin
        vb_isa_type[ISA_U_type] = 1'b1;
        vb_dec[Instr_LUI] = 1'b1;
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm[31: 12] = vb_instr[31: 12];
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 32] = '1;
        end
    end
    OPCODE_SB: begin
        vb_isa_type[ISA_S_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_radr2 = {1'h0, vb_instr[24: 20]};
        vb_imm[11: 0] = {vb_instr[31: 25], vb_instr[11: 7]};
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 12] = '1;
        end
        case (vb_opcode2)
        0: begin
            vb_dec[Instr_SB] = 1'b1;
        end
        1: begin
            vb_dec[Instr_SH] = 1'b1;
        end
        2: begin
            vb_dec[Instr_SW] = 1'b1;
        end
        3: begin
            vb_dec[Instr_SD] = 1'b1;
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_CSRR: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[19: 15]};
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_csr_addr = vb_instr[31: 20];
        vb_imm[11: 0] = vb_instr[31: 20];
        if (vb_instr[31] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 12] = '1;
        end
        case (vb_opcode2)
        0: begin
            if (vb_instr == 32'h00000073) begin
                vb_dec[Instr_ECALL] = 1'b1;
            end else if (vb_instr == 32'h00100073) begin
                vb_dec[Instr_EBREAK] = 1'b1;
            end else if (vb_instr == 32'h00200073) begin
                vb_dec[Instr_URET] = 1'b1;
            end else if (vb_instr == 32'h10200073) begin
                vb_dec[Instr_SRET] = 1'b1;
            end else if (vb_instr == 32'h10500073) begin
                vb_dec[Instr_WFI] = 1'b1;
            end else if (vb_instr == 32'h20200073) begin
                vb_dec[Instr_HRET] = 1'b1;
            end else if (vb_instr == 32'h30200073) begin
                vb_dec[Instr_MRET] = 1'b1;
            end else if ((vb_instr[31: 25] == 7'h09) && ((|vb_waddr) == 1'b0)) begin
                vb_dec[Instr_SFENCE_VMA] = 1'b1;
            end else begin
                v_error = 1'b1;
            end
        end
        1: begin
            vb_dec[Instr_CSRRW] = 1'b1;
        end
        2: begin
            vb_dec[Instr_CSRRS] = 1'b1;
        end
        3: begin
            vb_dec[Instr_CSRRC] = 1'b1;
        end
        5: begin
            vb_dec[Instr_CSRRWI] = 1'b1;
        end
        6: begin
            vb_dec[Instr_CSRRSI] = 1'b1;
        end
        7: begin
            vb_dec[Instr_CSRRCI] = 1'b1;
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    OPCODE_FENCE: begin
        case (vb_opcode2)
        3'h0: begin
            vb_dec[Instr_FENCE] = 1'b1;
        end
        3'h1: begin
            vb_dec[Instr_FENCE_I] = 1'b1;
        end
        default: begin
            v_error = 1'b1;
        end
        endcase
    end
    default: begin
        if (fpu_ena) begin
            case (vb_opcode1)
            OPCODE_FPU_LD: begin
                vb_isa_type[ISA_I_type] = 1'b1;
                vb_radr1 = {1'h0, vb_instr[19: 15]};
                vb_waddr = {1'h1, vb_instr[11: 7]};         // rd
                vb_imm[11: 0] = vb_instr[31: 20];
                if (vb_instr[31] == 1'b1) begin
                    vb_imm[(RISCV_ARCH - 1): 12] = '1;
                end
                if (vb_opcode2 == 3) begin
                    vb_dec[Instr_FLD] = 1'b1;
                end else begin
                    v_error = 1'b1;
                end
            end
            OPCODE_FPU_SD: begin
                vb_isa_type[ISA_S_type] = 1'b1;
                vb_radr1 = {1'h0, vb_instr[19: 15]};
                vb_radr2 = {1'h1, vb_instr[24: 20]};
                vb_imm[11: 0] = {vb_instr[31: 25], vb_instr[11: 7]};
                if (vb_instr[31] == 1'b1) begin
                    vb_imm[(RISCV_ARCH - 1): 12] = '1;
                end
                if (vb_opcode2 == 3'h3) begin
                    vb_dec[Instr_FSD] = 1'b1;
                end else begin
                    v_error = 1'b1;
                end
            end
            OPCODE_FPU_OP: begin
                vb_isa_type[ISA_R_type] = 1'b1;
                vb_radr1 = {1'h1, vb_instr[19: 15]};
                vb_radr2 = {1'h1, vb_instr[24: 20]};
                vb_waddr = {1'h1, vb_instr[11: 7]};         // rd
                case (vb_instr[31: 25])
                7'h01: begin
                    vb_dec[Instr_FADD_D] = 1'b1;
                end
                7'h05: begin
                    vb_dec[Instr_FSUB_D] = 1'b1;
                end
                7'h09: begin
                    vb_dec[Instr_FMUL_D] = 1'b1;
                end
                7'h0d: begin
                    vb_dec[Instr_FDIV_D] = 1'b1;
                end
                7'h15: begin
                    if (vb_opcode2 == 3'h0) begin
                        vb_dec[Instr_FMIN_D] = 1'b1;
                    end else if (vb_opcode2 == 3'h1) begin
                        vb_dec[Instr_FMAX_D] = 1'b1;
                    end else begin
                        v_error = 1'b1;
                    end
                end
                7'h51: begin
                    vb_waddr[5] = 1'b0;
                    if (vb_opcode2 == 3'h0) begin
                        vb_dec[Instr_FLE_D] = 1'b1;
                    end else if (vb_opcode2 == 3'h1) begin
                        vb_dec[Instr_FLT_D] = 1'b1;
                    end else if (vb_opcode2 == 3'h2) begin
                        vb_dec[Instr_FEQ_D] = 1'b1;
                    end else begin
                        v_error = 1'b1;
                    end
                end
                7'h61: begin
                    vb_waddr[5] = 1'b0;
                    if (vb_instr[24: 20] == 5'h00) begin
                        vb_dec[Instr_FCVT_W_D] = 1'b1;
                    end else if (vb_instr[24: 20] == 5'h01) begin
                        vb_dec[Instr_FCVT_WU_D] = 1'b1;
                    end else if (vb_instr[24: 20] == 5'h02) begin
                        vb_dec[Instr_FCVT_L_D] = 1'b1;
                    end else if (vb_instr[24: 20] == 5'h03) begin
                        vb_dec[Instr_FCVT_LU_D] = 1'b1;
                    end else begin
                        v_error = 1'b1;
                    end
                end
                7'h69: begin
                    vb_radr1[5] = 1'b0;
                    if (vb_instr[24: 20] == 5'h00) begin
                        vb_dec[Instr_FCVT_D_W] = 1'b1;
                    end else if (vb_instr[24: 20] == 5'h01) begin
                        vb_dec[Instr_FCVT_D_WU] = 1'b1;
                    end else if (vb_instr[24: 20] == 5'h02) begin
                        vb_dec[Instr_FCVT_D_L] = 1'b1;
                    end else if (vb_instr[24: 20] == 5'h03) begin
                        vb_dec[Instr_FCVT_D_LU] = 1'b1;
                    end else begin
                        v_error = 1'b1;
                    end
                end
                7'h71: begin
                    vb_waddr[5] = 1'b0;
                    if (((|vb_instr[24: 20]) == 1'b0) && ((|vb_opcode2) == 1'b0)) begin
                        vb_dec[Instr_FMOV_X_D] = 1'b1;
                    end else begin
                        v_error = 1'b1;
                    end
                end
                7'h79: begin
                    vb_radr1[5] = 1'b0;
                    if (((|vb_instr[24: 20]) == 1'b0) && ((|vb_opcode2) == 1'b0)) begin
                        vb_dec[Instr_FMOV_D_X] = 1'b1;
                    end else begin
                        v_error = 1'b1;
                    end
                end
                default: begin
                    v_error = 1'b1;
                end
                endcase
            end
            default: begin
                v_error = 1'b1;
            end
            endcase
        end else begin
            // FPU disabled
            v_error = 1'b1;
        end
    end
    endcase

    v_amo = (vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_AMOADD_D]
            || vb_dec[Instr_AMOXOR_D]
            || vb_dec[Instr_AMOOR_D]
            || vb_dec[Instr_AMOAND_D]
            || vb_dec[Instr_AMOMIN_D]
            || vb_dec[Instr_AMOMAX_D]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]
            || vb_dec[Instr_AMOSWAP_D]);

    v_memop_store = (vb_dec[Instr_SD]
            || vb_dec[Instr_SW]
            || vb_dec[Instr_SH]
            || vb_dec[Instr_SB]
            || vb_dec[Instr_FSD]
            || vb_dec[Instr_SC_W]
            || vb_dec[Instr_SC_D]);

    v_memop_load = (vb_dec[Instr_LD]
            || vb_dec[Instr_LW]
            || vb_dec[Instr_LH]
            || vb_dec[Instr_LB]
            || vb_dec[Instr_LWU]
            || vb_dec[Instr_LHU]
            || vb_dec[Instr_LBU]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_LR_W]
            || vb_dec[Instr_AMOADD_D]
            || vb_dec[Instr_AMOXOR_D]
            || vb_dec[Instr_AMOOR_D]
            || vb_dec[Instr_AMOAND_D]
            || vb_dec[Instr_AMOMIN_D]
            || vb_dec[Instr_AMOMAX_D]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]
            || vb_dec[Instr_AMOSWAP_D]
            || vb_dec[Instr_LR_D]);

    v_memop_sign_ext = (vb_dec[Instr_LD]
            || vb_dec[Instr_LW]
            || vb_dec[Instr_LH]
            || vb_dec[Instr_LB]
            || vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_LR_W]);

    v_f64 = (vb_dec[Instr_FADD_D]
            || vb_dec[Instr_FSUB_D]
            || vb_dec[Instr_FMUL_D]
            || vb_dec[Instr_FDIV_D]
            || vb_dec[Instr_FMIN_D]
            || vb_dec[Instr_FMAX_D]
            || vb_dec[Instr_FLE_D]
            || vb_dec[Instr_FLT_D]
            || vb_dec[Instr_FEQ_D]
            || vb_dec[Instr_FCVT_W_D]
            || vb_dec[Instr_FCVT_WU_D]
            || vb_dec[Instr_FCVT_L_D]
            || vb_dec[Instr_FCVT_LU_D]
            || vb_dec[Instr_FMOV_X_D]
            || vb_dec[Instr_FCVT_D_W]
            || vb_dec[Instr_FCVT_D_WU]
            || vb_dec[Instr_FCVT_D_L]
            || vb_dec[Instr_FCVT_D_LU]
            || vb_dec[Instr_FMOV_D_X]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_FSD]);

    if ((vb_dec[Instr_LD]
            || vb_dec[Instr_SD]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_FSD]
            || vb_dec[Instr_AMOADD_D]
            || vb_dec[Instr_AMOXOR_D]
            || vb_dec[Instr_AMOOR_D]
            || vb_dec[Instr_AMOAND_D]
            || vb_dec[Instr_AMOMIN_D]
            || vb_dec[Instr_AMOMAX_D]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]
            || vb_dec[Instr_AMOSWAP_D]
            || vb_dec[Instr_LR_D]
            || vb_dec[Instr_SC_D]) == 1'b1) begin
        vb_memop_size = MEMOP_8B;
    end else if ((vb_dec[Instr_LW]
                || vb_dec[Instr_LWU]
                || vb_dec[Instr_SW]
                || vb_dec[Instr_AMOADD_W]
                || vb_dec[Instr_AMOXOR_W]
                || vb_dec[Instr_AMOOR_W]
                || vb_dec[Instr_AMOAND_W]
                || vb_dec[Instr_AMOMIN_W]
                || vb_dec[Instr_AMOMAX_W]
                || vb_dec[Instr_AMOMINU_W]
                || vb_dec[Instr_AMOMAXU_W]
                || vb_dec[Instr_AMOSWAP_W]
                || vb_dec[Instr_LR_W]
                || vb_dec[Instr_SC_W]) == 1'b1) begin
        vb_memop_size = MEMOP_4B;
    end else if ((vb_dec[Instr_LH] || vb_dec[Instr_LHU] || vb_dec[Instr_SH]) == 1'b1) begin
        vb_memop_size = MEMOP_2B;
    end else begin
        vb_memop_size = MEMOP_1B;
    end

    v_unsigned_op = (vb_dec[Instr_DIVU]
            || vb_dec[Instr_REMU]
            || vb_dec[Instr_DIVUW]
            || vb_dec[Instr_REMUW]
            || vb_dec[Instr_MULHU]
            || vb_dec[Instr_FCVT_WU_D]
            || vb_dec[Instr_FCVT_LU_D]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOMINU_D]
            || vb_dec[Instr_AMOMAXU_D]);

    v_rv32 = (vb_dec[Instr_ADDW]
            || vb_dec[Instr_ADDIW]
            || vb_dec[Instr_SLLW]
            || vb_dec[Instr_SLLIW]
            || vb_dec[Instr_SRAW]
            || vb_dec[Instr_SRAIW]
            || vb_dec[Instr_SRLW]
            || vb_dec[Instr_SRLIW]
            || vb_dec[Instr_SUBW]
            || vb_dec[Instr_DIVW]
            || vb_dec[Instr_DIVUW]
            || vb_dec[Instr_MULW]
            || vb_dec[Instr_REMW]
            || vb_dec[Instr_REMUW]
            || vb_dec[Instr_AMOADD_W]
            || vb_dec[Instr_AMOXOR_W]
            || vb_dec[Instr_AMOOR_W]
            || vb_dec[Instr_AMOAND_W]
            || vb_dec[Instr_AMOMIN_W]
            || vb_dec[Instr_AMOMAX_W]
            || vb_dec[Instr_AMOMINU_W]
            || vb_dec[Instr_AMOMAXU_W]
            || vb_dec[Instr_AMOSWAP_W]
            || vb_dec[Instr_LR_W]
            || vb_dec[Instr_SC_W]);

    v_f64 = (vb_dec[Instr_FADD_D]
            || vb_dec[Instr_FSUB_D]
            || vb_dec[Instr_FMUL_D]
            || vb_dec[Instr_FDIV_D]
            || vb_dec[Instr_FMIN_D]
            || vb_dec[Instr_FMAX_D]
            || vb_dec[Instr_FLE_D]
            || vb_dec[Instr_FLT_D]
            || vb_dec[Instr_FEQ_D]
            || vb_dec[Instr_FCVT_W_D]
            || vb_dec[Instr_FCVT_WU_D]
            || vb_dec[Instr_FCVT_L_D]
            || vb_dec[Instr_FCVT_LU_D]
            || vb_dec[Instr_FMOV_X_D]
            || vb_dec[Instr_FCVT_D_W]
            || vb_dec[Instr_FCVT_D_WU]
            || vb_dec[Instr_FCVT_D_L]
            || vb_dec[Instr_FCVT_D_LU]
            || vb_dec[Instr_FMOV_D_X]
            || vb_dec[Instr_FLD]
            || vb_dec[Instr_FSD]);

    v.pc = i_f_pc;
    v.isa_type = vb_isa_type;
    v.instr_vec = vb_dec;
    v.instr = i_f_instr;
    v.memop_store = v_memop_store;
    v.memop_load = v_memop_load;
    v.memop_sign_ext = v_memop_sign_ext;
    v.memop_size = vb_memop_size;
    v.unsigned_op = v_unsigned_op;
    v.rv32 = v_rv32;
    v.f64 = v_f64;
    v.compressed = v_compressed;
    v.amo = v_amo;
    v.instr_load_fault = i_instr_load_fault;
    v.instr_page_fault_x = i_instr_page_fault_x;
    v.instr_unimplemented = v_error;
    v.radr1 = vb_radr1;
    v.radr2 = vb_radr2;
    v.waddr = vb_waddr;
    v.csr_addr = vb_csr_addr;
    v.imm = vb_imm;
    v.progbuf_ena = i_progbuf_ena;

    if ((~async_reset && i_nrst == 1'b0) || (i_flush_pipeline == 1'b1)) begin
        v = DecoderRv_r_reset;
    end

    o_pc = r.pc;
    o_instr = r.instr;
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_size = r.memop_size;
    o_unsigned_op = r.unsigned_op;
    o_rv32 = r.rv32;
    o_f64 = r.f64;
    o_compressed = r.compressed;
    o_amo = r.amo;
    o_isa_type = r.isa_type;
    o_instr_vec = r.instr_vec;
    o_exception = r.instr_unimplemented;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_page_fault_x = r.instr_page_fault_x;
    o_radr1 = r.radr1;
    o_radr2 = r.radr2;
    o_waddr = r.waddr;
    o_csr_addr = r.csr_addr;
    o_imm = r.imm;
    o_progbuf_ena = r.progbuf_ena;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= DecoderRv_r_reset;
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

endmodule: DecoderRv
