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

module DecoderRvc #(
    parameter bit async_reset = 1'b0
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
import dec_rvc_pkg::*;

DecoderRvc_registers r, rin;

always_comb
begin: comb_proc
    DecoderRvc_registers v;
    logic v_error;
    logic [15:0] vb_instr;
    logic [4:0] vb_opcode1;
    logic [2:0] vb_opcode2;
    logic [Instr_Total-1:0] vb_dec;
    logic [ISA_Total-1:0] vb_isa_type;
    logic [5:0] vb_radr1;
    logic [5:0] vb_radr2;
    logic [5:0] vb_waddr;
    logic [RISCV_ARCH-1:0] vb_imm;
    logic v_memop_store;
    logic v_memop_load;
    logic v_memop_sign_ext;
    logic [1:0] vb_memop_size;
    logic v_rv32;

    v_error = 0;
    vb_instr = 0;
    vb_opcode1 = 0;
    vb_opcode2 = 0;
    vb_dec = 0;
    vb_isa_type = 0;
    vb_radr1 = 0;
    vb_radr2 = 0;
    vb_waddr = 0;
    vb_imm = 0;
    v_memop_store = 0;
    v_memop_load = 0;
    v_memop_sign_ext = 0;
    vb_memop_size = 0;
    v_rv32 = 0;

    v = r;

    vb_instr = i_f_instr;

    vb_opcode1 = {vb_instr[15: 13], vb_instr[1: 0]};
    case (vb_opcode1)
    OPCODE_C_ADDI4SPN: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_ADDI] = 1'b1;
        vb_radr1 = 6'h02;                                   // rs1 = sp
        vb_waddr = {3'h1, vb_instr[4: 2]};                  // rd
        vb_imm[9: 2] = {vb_instr[10: 7], vb_instr[12: 11], vb_instr[5], vb_instr[6]};
    end
    OPCODE_C_NOP_ADDI: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_ADDI] = 1'b1;
        vb_radr1 = vb_instr[11: 7];                         // rs1
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm[4: 0] = vb_instr[6: 2];
        if (vb_instr[12] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 5] = '1;
        end
    end
    OPCODE_C_SLLI: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_SLLI] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[11: 7]};                 // rs1
        vb_waddr = {1'h0, vb_instr[11: 7]};                 // rd
        vb_imm[5: 0] = {vb_instr[12], vb_instr[6: 2]};
    end
    OPCODE_C_JAL_ADDIW: begin
        // JAL is the RV32C only instruction
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_ADDIW] = 1'b1;
        vb_radr1 = {1'h0, vb_instr[11: 7]};                 // rs1
        vb_waddr = {1'h0, vb_instr[11: 7]};                 // rd
        vb_imm[4: 0] = vb_instr[6: 2];
        if (vb_instr[12] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 5] = '1;
        end
    end
    OPCODE_C_LW: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_LW] = 1'b1;
        vb_radr1 = {3'h1, vb_instr[9: 7]};                  // rs1
        vb_waddr = {3'h1, vb_instr[4: 2]};                  // rd
        vb_imm[6: 2] = {vb_instr[5], vb_instr[12: 10], vb_instr[6]};
    end
    OPCODE_C_LI: begin                                      // ADDI rd = r0 + imm
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_ADDI] = 1'b1;
        vb_waddr = {1'h0, vb_instr[11: 7]};                 // rd
        vb_imm[4: 0] = vb_instr[6: 2];
        if (vb_instr[12] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 5] = '1;
        end
    end
    OPCODE_C_LWSP: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_LW] = 1'b1;
        vb_radr1 = 6'h02;                                   // rs1 = sp
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm[7: 2] = {vb_instr[3: 2], vb_instr[12], vb_instr[6: 4]};
    end
    OPCODE_C_LD: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_LD] = 1'b1;
        vb_radr1 = {3'h1, vb_instr[9: 7]};
        vb_waddr = {3'h1, vb_instr[4: 2]};                  // rd
        vb_imm[7: 3] = {vb_instr[6], vb_instr[5], vb_instr[12: 10]};
    end
    OPCODE_C_ADDI16SP_LUI: begin
        if (vb_instr[11: 7] == 5'h02) begin
            vb_isa_type[ISA_I_type] = 1'b1;
            vb_dec[Instr_ADDI] = 1'b1;
            vb_radr1 = 6'h02;                               // rs1 = sp
            vb_waddr = 6'h02;                               // rd = sp
            vb_imm[8: 4] = {vb_instr[4: 3], vb_instr[5], vb_instr[2], vb_instr[6]};
            if (vb_instr[12] == 1'b1) begin
                vb_imm[(RISCV_ARCH - 1): 9] = '1;
            end
        end else begin
            vb_isa_type[ISA_U_type] = 1'b1;
            vb_dec[Instr_LUI] = 1'b1;
            vb_waddr = {1'h0, vb_instr[11: 7]};             // rd
            vb_imm[16: 12] = vb_instr[6: 2];
            if (vb_instr[12] == 1'b1) begin
                vb_imm[(RISCV_ARCH - 1): 17] = '1;
            end
        end
    end
    OPCODE_C_LDSP: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        vb_dec[Instr_LD] = 1'b1;
        vb_radr1 = 6'h02;                                   // rs1 = sp
        vb_waddr = vb_instr[11: 7];                         // rd
        vb_imm[8: 3] = {vb_instr[4: 2], vb_instr[12], vb_instr[6: 5]};
    end
    OPCODE_C_MATH: begin
        if (vb_instr[11: 10] == 2'h0) begin
            vb_isa_type[ISA_I_type] = 1'b1;
            vb_dec[Instr_SRLI] = 1'b1;
            vb_radr1 = {3'h1, vb_instr[9: 7]};              // rs1
            vb_waddr = {3'h1, vb_instr[9: 7]};              // rd
            vb_imm[5: 0] = {vb_instr[12], vb_instr[6: 2]};
        end else if (vb_instr[11: 10] == 2'h1) begin
            vb_isa_type[ISA_I_type] = 1'b1;
            vb_dec[Instr_SRAI] = 1'b1;
            vb_radr1 = {3'h1, vb_instr[9: 7]};              // rs1
            vb_waddr = {3'h1, vb_instr[9: 7]};              // rd
            vb_imm[5: 0] = {vb_instr[12], vb_instr[6: 2]};
        end else if (vb_instr[11: 10] == 2'h2) begin
            vb_isa_type[ISA_I_type] = 1'b1;
            vb_dec[Instr_ANDI] = 1'b1;
            vb_radr1 = {3'h1, vb_instr[9: 7]};              // rs1
            vb_waddr = {3'h1, vb_instr[9: 7]};              // rd
            vb_imm[4: 0] = vb_instr[6: 2];
            if (vb_instr[12] == 1'b1) begin
                vb_imm[(RISCV_ARCH - 1): 5] = '1;
            end
        end else if (vb_instr[12] == 1'b0) begin
            vb_isa_type[ISA_R_type] = 1'b1;
            vb_radr1 = {3'h1, vb_instr[9: 7]};              // rs1
            vb_radr2 = {3'h1, vb_instr[4: 2]};              // rs2
            vb_waddr = {3'h1, vb_instr[9: 7]};              // rd
            case (vb_instr[6: 5])
            2'h0: begin
                vb_dec[Instr_SUB] = 1'b1;
            end
            2'h1: begin
                vb_dec[Instr_XOR] = 1'b1;
            end
            2'h2: begin
                vb_dec[Instr_OR] = 1'b1;
            end
            default: begin
                vb_dec[Instr_AND] = 1'b1;
            end
            endcase
        end else begin
            vb_isa_type[ISA_R_type] = 1'b1;
            vb_radr1 = {3'h1, vb_instr[9: 7]};              // rs1
            vb_radr2 = {3'h1, vb_instr[4: 2]};              // rs2
            vb_waddr = {3'h1, vb_instr[9: 7]};              // rd
            case (vb_instr[6: 5])
            2'h0: begin
                vb_dec[Instr_SUBW] = 1'b1;
            end
            2'h1: begin
                vb_dec[Instr_ADDW] = 1'b1;
            end
            default: begin
                v_error = 1'b1;
            end
            endcase
        end
    end
    OPCODE_C_JR_MV_EBREAK_JALR_ADD: begin
        vb_isa_type[ISA_I_type] = 1'b1;
        if (vb_instr[12] == 1'b0) begin
            if ((|vb_instr[6: 2]) == 1'b0) begin
                vb_dec[Instr_JALR] = 1'b1;
                vb_radr1 = {1'h0, vb_instr[11: 7]};         // rs1
            end else begin
                vb_dec[Instr_ADDI] = 1'b1;
                vb_radr1 = {1'h0, vb_instr[6: 2]};          // rs1
                vb_waddr = {1'h0, vb_instr[11: 7]};         // rd
            end
        end else begin
            if (((|vb_instr[11: 7]) == 1'b0) && ((|vb_instr[6: 2]) == 1'b0)) begin
                vb_dec[Instr_EBREAK] = 1'b1;
            end else if ((|vb_instr[6: 2]) == 1'b0) begin
                vb_dec[Instr_JALR] = 1'b1;
                vb_radr1 = {1'h0, vb_instr[11: 7]};         // rs1
                vb_waddr = 6'h01;
            end else begin
                vb_dec[Instr_ADD] = 1'b1;
                vb_isa_type[ISA_R_type] = 1'b1;
                vb_radr1 = {1'h0, vb_instr[11: 7]};         // rs1
                vb_radr2 = {1'h0, vb_instr[6: 2]};          // rs2
                vb_waddr = {1'h0, vb_instr[11: 7]};         // rd
            end
        end
    end
    OPCODE_C_J: begin                                       // JAL with rd = 0
        vb_isa_type[ISA_UJ_type] = 1'b1;
        vb_dec[Instr_JAL] = 1'b1;
        vb_imm[10: 1] = {vb_instr[8],
                vb_instr[10: 9],
                vb_instr[6],
                vb_instr[7],
                vb_instr[2],
                vb_instr[11],
                vb_instr[5: 3]};
        if (vb_instr[12] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 11] = '1;
        end
    end
    OPCODE_C_SW: begin
        vb_isa_type[ISA_S_type] = 1'b1;
        vb_dec[Instr_SW] = 1'b1;
        vb_radr1 = {3'h1, vb_instr[9: 7]};                  // rs1
        vb_radr2 = {3'h1, vb_instr[4: 2]};                  // rs2
        vb_imm[6: 2] = {vb_instr[5], vb_instr[12], vb_instr[11: 10], vb_instr[6]};
    end
    OPCODE_C_BEQZ: begin
        vb_isa_type[ISA_SB_type] = 1'b1;
        vb_dec[Instr_BEQ] = 1'b1;
        vb_radr1 = {3'h1, vb_instr[9: 7]};                  // rs1
        vb_imm[7: 1] = {vb_instr[6: 5], vb_instr[2], vb_instr[11: 10], vb_instr[4: 3]};
        if (vb_instr[12] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 8] = '1;
        end
    end
    OPCODE_C_SWSP: begin
        vb_isa_type[ISA_S_type] = 1'b1;
        vb_dec[Instr_SW] = 1'b1;
        vb_radr1 = 6'h02;                                   // rs1 = sp
        vb_radr2 = {1'h0, vb_instr[6: 2]};                  // rs2
        vb_imm[7: 2] = {vb_instr[8: 7], vb_instr[12], vb_instr[11: 9]};
    end
    OPCODE_C_SD: begin
        vb_isa_type[ISA_S_type] = 1'b1;
        vb_dec[Instr_SD] = 1'b1;
        vb_radr1 = {3'h1, vb_instr[9: 7]};                  // rs1
        vb_radr2 = {3'h1, vb_instr[4: 2]};                  // rs2
        vb_imm[7: 3] = {vb_instr[6: 5], vb_instr[12], vb_instr[11: 10]};
    end
    OPCODE_C_BNEZ: begin
        vb_isa_type[ISA_SB_type] = 1'b1;
        vb_dec[Instr_BNE] = 1'b1;
        vb_radr1 = {3'h1, vb_instr[9: 7]};                  // rs1
        vb_imm[7: 1] = {vb_instr[6: 5], vb_instr[2], vb_instr[11: 10], vb_instr[4: 3]};
        if (vb_instr[12] == 1'b1) begin
            vb_imm[(RISCV_ARCH - 1): 8] = '1;
        end
    end
    OPCODE_C_SDSP: begin
        vb_isa_type[ISA_S_type] = 1'b1;
        vb_dec[Instr_SD] = 1'b1;
        vb_radr1 = 6'h02;                                   // rs1 = sp
        vb_radr2 = {1'h0, vb_instr[6: 2]};                  // rs2
        vb_imm[8: 3] = {vb_instr[9: 7], vb_instr[12], vb_instr[11: 10]};
    end
    default: begin
        v_error = 1'b1;
    end
    endcase

    v_memop_store = (vb_dec[Instr_SD] || vb_dec[Instr_SW]);
    v_memop_load = (vb_dec[Instr_LD] || vb_dec[Instr_LW]);
    v_memop_sign_ext = (vb_dec[Instr_LD] || vb_dec[Instr_LW]);
    if ((vb_dec[Instr_LD] || vb_dec[Instr_SD]) == 1'b1) begin
        vb_memop_size = MEMOP_8B;
    end else if ((vb_dec[Instr_LW] || vb_dec[Instr_SW]) == 1'b1) begin
        vb_memop_size = MEMOP_4B;
    end else begin
        vb_memop_size = MEMOP_8B;
    end
    v_rv32 = (vb_dec[Instr_ADDW] || vb_dec[Instr_ADDIW] || vb_dec[Instr_SUBW]);

    v.pc = i_f_pc;
    v.isa_type = vb_isa_type;
    v.instr_vec = vb_dec;
    v.instr = i_f_instr[15: 0];
    v.memop_store = v_memop_store;
    v.memop_load = v_memop_load;
    v.memop_sign_ext = v_memop_sign_ext;
    v.memop_size = vb_memop_size;
    v.rv32 = v_rv32;
    v.instr_load_fault = i_instr_load_fault;
    v.instr_page_fault_x = i_instr_page_fault_x;
    v.instr_unimplemented = v_error;
    v.radr1 = vb_radr1;
    v.radr2 = vb_radr2;
    v.waddr = vb_waddr;
    v.imm = vb_imm;
    v.progbuf_ena = i_progbuf_ena;

    if ((~async_reset && i_nrst == 1'b0) || (i_flush_pipeline == 1'b1)) begin
        v = DecoderRvc_r_reset;
    end

    o_pc = r.pc;
    o_instr = {16'h0000, r.instr};
    o_memop_load = r.memop_load;
    o_memop_store = r.memop_store;
    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_size = r.memop_size;
    o_unsigned_op = 1'b0;
    o_rv32 = r.rv32;
    o_f64 = 1'b0;
    o_compressed = 1'b1;
    o_amo = 1'b0;
    o_isa_type = r.isa_type;
    o_instr_vec = r.instr_vec;
    o_exception = r.instr_unimplemented;
    o_instr_load_fault = r.instr_load_fault;
    o_instr_page_fault_x = r.instr_page_fault_x;
    o_radr1 = r.radr1;
    o_radr2 = r.radr2;
    o_waddr = r.waddr;
    o_csr_addr = '0;
    o_imm = r.imm;
    o_progbuf_ena = r.progbuf_ena;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= DecoderRvc_r_reset;
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

endmodule: DecoderRvc
