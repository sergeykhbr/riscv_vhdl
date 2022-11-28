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
package dec_rvc_pkg;

import river_cfg_pkg::*;

localparam bit [4:0] OPCODE_C_ADDI4SPN = 5'h00;
localparam bit [4:0] OPCODE_C_NOP_ADDI = 5'h01;
localparam bit [4:0] OPCODE_C_SLLI = 5'h02;
localparam bit [4:0] OPCODE_C_JAL_ADDIW = 5'h05;
localparam bit [4:0] OPCODE_C_LW = 5'h08;
localparam bit [4:0] OPCODE_C_LI = 5'h09;
localparam bit [4:0] OPCODE_C_LWSP = 5'h0a;
localparam bit [4:0] OPCODE_C_LD = 5'h0c;
localparam bit [4:0] OPCODE_C_ADDI16SP_LUI = 5'h0d;
localparam bit [4:0] OPCODE_C_LDSP = 5'h0e;
localparam bit [4:0] OPCODE_C_MATH = 5'h11;
localparam bit [4:0] OPCODE_C_JR_MV_EBREAK_JALR_ADD = 5'h12;
localparam bit [4:0] OPCODE_C_J = 5'h15;
localparam bit [4:0] OPCODE_C_SW = 5'h18;
localparam bit [4:0] OPCODE_C_BEQZ = 5'h19;
localparam bit [4:0] OPCODE_C_SWSP = 5'h1a;
localparam bit [4:0] OPCODE_C_SD = 5'h1c;
localparam bit [4:0] OPCODE_C_BNEZ = 5'h1d;
localparam bit [4:0] OPCODE_C_SDSP = 5'h1e;

typedef struct {
    logic [RISCV_ARCH-1:0] pc;
    logic [ISA_Total-1:0] isa_type;
    logic [Instr_Total-1:0] instr_vec;
    logic [15:0] instr;
    logic memop_store;
    logic memop_load;
    logic memop_sign_ext;
    logic [1:0] memop_size;
    logic rv32;
    logic instr_load_fault;
    logic instr_page_fault_x;
    logic instr_unimplemented;
    logic [5:0] radr1;
    logic [5:0] radr2;
    logic [5:0] waddr;
    logic [RISCV_ARCH-1:0] imm;
    logic progbuf_ena;
} DecoderRvc_registers;

const DecoderRvc_registers DecoderRvc_r_reset = '{
    '1,                                 // pc
    '0,                                 // isa_type
    '0,                                 // instr_vec
    '1,                                 // instr
    1'b0,                               // memop_store
    1'b0,                               // memop_load
    1'b0,                               // memop_sign_ext
    MEMOP_1B,                           // memop_size
    1'b0,                               // rv32
    1'b0,                               // instr_load_fault
    1'b0,                               // instr_page_fault_x
    1'b0,                               // instr_unimplemented
    '0,                                 // radr1
    '0,                                 // radr2
    '0,                                 // waddr
    '0,                                 // imm
    1'b0                                // progbuf_ena
};

endpackage: dec_rvc_pkg
