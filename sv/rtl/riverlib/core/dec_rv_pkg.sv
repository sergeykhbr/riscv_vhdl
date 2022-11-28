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
package dec_rv_pkg;

import river_cfg_pkg::*;

localparam bit [4:0] OPCODE_LB = 5'h00;                     // 00000: LB, LH, LW, LD, LBU, LHU, LWU
localparam bit [4:0] OPCODE_FPU_LD = 5'h01;                 // 00001: FLD
localparam bit [4:0] OPCODE_FENCE = 5'h03;                  // 00011: FENCE, FENCE_I
localparam bit [4:0] OPCODE_ADDI = 5'h04;                   // 00100: ADDI, ANDI, ORI, SLLI, SLTI, SLTIU, SRAI, SRLI, XORI
localparam bit [4:0] OPCODE_AUIPC = 5'h05;                  // 00101: AUIPC
localparam bit [4:0] OPCODE_ADDIW = 5'h06;                  // 00110: ADDIW, SLLIW, SRAIW, SRLIW
localparam bit [4:0] OPCODE_SB = 5'h08;                     // 01000: SB, SH, SW, SD
localparam bit [4:0] OPCODE_FPU_SD = 5'h09;                 // 01001: FSD
localparam bit [4:0] OPCODE_AMO = 5'h0b;                    // 01011: Atomic opcode (AMO)
localparam bit [4:0] OPCODE_ADD = 5'h0c;                    // 01100: ADD, AND, OR, SLT, SLTU, SLL, SRA, SRL, SUB, XOR, DIV, DIVU, MUL, REM, REMU
localparam bit [4:0] OPCODE_LUI = 5'h0d;                    // 01101: LUI
localparam bit [4:0] OPCODE_ADDW = 5'h0e;                   // 01110: ADDW, SLLW, SRAW, SRLW, SUBW, DIVW, DIVUW, MULW, REMW, REMUW
localparam bit [4:0] OPCODE_FPU_OP = 5'h14;                 // 10100: FPU operation
localparam bit [4:0] OPCODE_BEQ = 5'h18;                    // 11000: BEQ, BNE, BLT, BGE, BLTU, BGEU
localparam bit [4:0] OPCODE_JALR = 5'h19;                   // 11001: JALR
localparam bit [4:0] OPCODE_JAL = 5'h1b;                    // 11011: JAL
localparam bit [4:0] OPCODE_CSRR = 5'h1c;                   // 11100: CSRRC, CSRRCI, CSRRS, CSRRSI, CSRRW, CSRRWI, URET, SRET, HRET, MRET

typedef struct {
    logic [RISCV_ARCH-1:0] pc;
    logic [ISA_Total-1:0] isa_type;
    logic [Instr_Total-1:0] instr_vec;
    logic [31:0] instr;
    logic memop_store;
    logic memop_load;
    logic memop_sign_ext;
    logic [1:0] memop_size;
    logic unsigned_op;
    logic rv32;
    logic f64;
    logic compressed;
    logic amo;
    logic instr_load_fault;
    logic instr_page_fault_x;
    logic instr_unimplemented;
    logic [5:0] radr1;
    logic [5:0] radr2;
    logic [5:0] waddr;
    logic [11:0] csr_addr;
    logic [RISCV_ARCH-1:0] imm;
    logic progbuf_ena;
} DecoderRv_registers;

const DecoderRv_registers DecoderRv_r_reset = '{
    '1,                                 // pc
    '0,                                 // isa_type
    '0,                                 // instr_vec
    '1,                                 // instr
    1'b0,                               // memop_store
    1'b0,                               // memop_load
    1'b0,                               // memop_sign_ext
    MEMOP_1B,                           // memop_size
    1'b0,                               // unsigned_op
    1'b0,                               // rv32
    1'b0,                               // f64
    1'b0,                               // compressed
    1'b0,                               // amo
    1'b0,                               // instr_load_fault
    1'b0,                               // instr_page_fault_x
    1'b0,                               // instr_unimplemented
    '0,                                 // radr1
    '0,                                 // radr2
    '0,                                 // waddr
    '0,                                 // csr_addr
    '0,                                 // imm
    1'b0                                // progbuf_ena
};

endpackage: dec_rv_pkg
