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
package execute_pkg;

import river_cfg_pkg::*;

localparam int Res_Zero = 0;
localparam int Res_Reg2 = 1;
localparam int Res_Npc = 2;
localparam int Res_Ra = 3;
localparam int Res_Csr = 4;
localparam int Res_Alu = 5;
localparam int Res_AddSub = 6;
localparam int Res_Shifter = 7;
localparam int Res_IMul = 8;
localparam int Res_IDiv = 9;
localparam int Res_FPU = 10;
localparam int Res_Total = 11;

localparam bit [3:0] State_Idle = 4'h0;
localparam bit [3:0] State_WaitMemAcces = 4'h1;
localparam bit [3:0] State_WaitMulti = 4'h2;
localparam bit [3:0] State_Amo = 4'h5;
localparam bit [3:0] State_Csr = 4'h6;
localparam bit [3:0] State_Halted = 4'h7;
localparam bit [3:0] State_DebugMemRequest = 4'h8;
localparam bit [3:0] State_DebugMemError = 4'h9;
localparam bit [3:0] State_Wfi = 4'hf;

localparam bit [1:0] CsrState_Idle = 2'h0;
localparam bit [1:0] CsrState_Req = 2'h1;
localparam bit [1:0] CsrState_Resp = 2'h2;

localparam bit [1:0] AmoState_WaitMemAccess = 2'h0;
localparam bit [1:0] AmoState_Read = 2'h1;
localparam bit [1:0] AmoState_Modify = 2'h2;
localparam bit [1:0] AmoState_Write = 2'h3;

typedef struct {
    logic ena;
    logic valid;
    logic [RISCV_ARCH-1:0] res;
} select_type;

typedef struct {
    logic [5:0] radr1;
    logic [5:0] radr2;
    logic [5:0] waddr;
    logic [RISCV_ARCH-1:0] imm;
    logic [RISCV_ARCH-1:0] pc;
    logic [31:0] instr;
    logic [MemopType_Total-1:0] memop_type;
    logic memop_sign_ext;
    logic [1:0] memop_size;
    logic unsigned_op;
    logic rv32;
    logic compressed;
    logic f64;
    logic [Instr_Total-1:0] ivec;
    logic [ISA_Total-1:0] isa_type;
} input_mux_type;


typedef struct {
    logic [3:0] state;
    logic [1:0] csrstate;
    logic [1:0] amostate;
    logic [RISCV_ARCH-1:0] pc;
    logic [RISCV_ARCH-1:0] npc;
    logic [RISCV_ARCH-1:0] dnpc;
    logic [5:0] radr1;
    logic [5:0] radr2;
    logic [5:0] waddr;
    logic [RISCV_ARCH-1:0] rdata1;
    logic [RISCV_ARCH-1:0] rdata2;
    logic [RISCV_ARCH-1:0] rdata1_amo;
    logic [RISCV_ARCH-1:0] rdata2_amo;
    logic [Instr_Total-1:0] ivec;
    logic [ISA_Total-1:0] isa_type;
    logic [RISCV_ARCH-1:0] imm;
    logic [31:0] instr;
    logic [(CFG_REG_TAG_WIDTH * REGS_TOTAL)-1:0] tagcnt;    // N-bits tag per register (expected)
    logic reg_write;
    logic [5:0] reg_waddr;
    logic [CFG_REG_TAG_WIDTH-1:0] reg_wtag;
    logic csr_req_rmw;                                      // csr read-modify-write request
    logic [CsrReq_TotalBits-1:0] csr_req_type;
    logic [11:0] csr_req_addr;
    logic [RISCV_ARCH-1:0] csr_req_data;
    logic memop_valid;
    logic memop_debug;                                      // request from dport
    logic memop_halted;                                     // 0=return to Idle; 1=return to halt state
    logic [MemopType_Total-1:0] memop_type;
    logic memop_sign_ext;
    logic [1:0] memop_size;
    logic [RISCV_ARCH-1:0] memop_memaddr;
    logic [RISCV_ARCH-1:0] memop_wdata;
    logic unsigned_op;
    logic rv32;
    logic compressed;
    logic f64;
    logic stack_overflow;
    logic stack_underflow;
    logic mem_ex_load_fault;
    logic mem_ex_store_fault;
    logic page_fault_r;
    logic page_fault_w;
    logic [RISCV_ARCH-1:0] mem_ex_addr;
    logic [RISCV_ARCH-1:0] res_npc;
    logic [RISCV_ARCH-1:0] res_ra;
    logic [RISCV_ARCH-1:0] res_csr;
    logic [Res_Total-1:0] select;
    logic valid;
    logic call;
    logic ret;
    logic jmp;
    logic stepdone;
} InstrExecute_registers;

const InstrExecute_registers InstrExecute_r_reset = '{
    State_Idle,                         // state
    CsrState_Idle,                      // csrstate
    AmoState_WaitMemAccess,             // amostate
    '0,                                 // pc
    CFG_RESET_VECTOR,                   // npc
    '0,                                 // dnpc
    '0,                                 // radr1
    '0,                                 // radr2
    '0,                                 // waddr
    '0,                                 // rdata1
    '0,                                 // rdata2
    '0,                                 // rdata1_amo
    '0,                                 // rdata2_amo
    '0,                                 // ivec
    '0,                                 // isa_type
    '0,                                 // imm
    '0,                                 // instr
    '0,                                 // tagcnt
    1'b0,                               // reg_write
    '0,                                 // reg_waddr
    '0,                                 // reg_wtag
    1'b0,                               // csr_req_rmw
    '0,                                 // csr_req_type
    '0,                                 // csr_req_addr
    '0,                                 // csr_req_data
    1'b0,                               // memop_valid
    1'b0,                               // memop_debug
    1'b0,                               // memop_halted
    '0,                                 // memop_type
    1'b0,                               // memop_sign_ext
    '0,                                 // memop_size
    '0,                                 // memop_memaddr
    '0,                                 // memop_wdata
    1'b0,                               // unsigned_op
    1'b0,                               // rv32
    1'b0,                               // compressed
    1'b0,                               // f64
    1'b0,                               // stack_overflow
    1'b0,                               // stack_underflow
    1'b0,                               // mem_ex_load_fault
    1'b0,                               // mem_ex_store_fault
    1'b0,                               // page_fault_r
    1'b0,                               // page_fault_w
    '0,                                 // mem_ex_addr
    '0,                                 // res_npc
    '0,                                 // res_ra
    '0,                                 // res_csr
    '0,                                 // select
    1'b0,                               // valid
    1'b0,                               // call
    1'b0,                               // ret
    1'b0,                               // jmp
    1'b0                                // stepdone
};

endpackage: execute_pkg
