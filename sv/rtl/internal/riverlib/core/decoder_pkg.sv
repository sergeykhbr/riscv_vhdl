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
package decoder_pkg;

import river_cfg_pkg::*;

localparam int DEC_NUM = 2;
localparam int DEC_BLOCK = (2 * DEC_NUM);                   // 2 rv + 2 rvc
// shift registers depth to store previous decoded data
localparam int FULL_DEC_DEPTH = (DEC_BLOCK * ((CFG_DEC_DEPTH - 1) + CFG_BP_DEPTH));

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
} DecoderDataType;


typedef struct {
    DecoderDataType d[0: FULL_DEC_DEPTH - 1];
} InstrDecoder_registers;

endpackage: decoder_pkg
