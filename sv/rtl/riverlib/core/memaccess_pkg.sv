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
package memaccess_pkg;

import river_cfg_pkg::*;

localparam bit [1:0] State_Idle = 2'h0;
localparam bit [1:0] State_WaitReqAccept = 2'h1;
localparam bit [1:0] State_WaitResponse = 2'h2;
localparam bit [1:0] State_Hold = 2'h3;
localparam int QUEUE_WIDTH = (1  // memop_debug
        + 1  // i_flushd_valid
        + 1  // i_mmu_ena
        + 1  // i_mmu_sv39
        + 1  // i_mmu_sv48
        + CFG_REG_TAG_WIDTH  // vb_res_wtag
        + 64  // vb_mem_wdata
        + 8  // vb_mem_wstrb
        + RISCV_ARCH  // vb_res_data
        + 6  // vb_res_addr
        + 32  // vb_e_instr
        + RISCV_ARCH  // vb_e_pc
        + 2  // vb_mem_sz
        + 1  // v_mem_sign_ext
        + MemopType_Total  // vb_mem_type
        + RISCV_ARCH  // vb_mem_addr
);

typedef struct {
    logic [1:0] state;
    logic mmu_ena;
    logic mmu_sv39;
    logic mmu_sv48;
    logic [MemopType_Total-1:0] memop_type;
    logic [RISCV_ARCH-1:0] memop_addr;
    logic [63:0] memop_wdata;
    logic [7:0] memop_wstrb;
    logic memop_sign_ext;
    logic [1:0] memop_size;
    logic memop_debug;
    logic [RISCV_ARCH-1:0] memop_res_pc;
    logic [31:0] memop_res_instr;
    logic [5:0] memop_res_addr;
    logic [CFG_REG_TAG_WIDTH-1:0] memop_res_wtag;
    logic [RISCV_ARCH-1:0] memop_res_data;
    logic memop_res_wena;
    logic [RISCV_ARCH-1:0] hold_rdata;
    logic [RISCV_ARCH-1:0] pc;
    logic valid;
} MemAccess_registers;

const MemAccess_registers MemAccess_r_reset = '{
    State_Idle,                         // state
    1'b0,                               // mmu_ena
    1'b0,                               // mmu_sv39
    1'b0,                               // mmu_sv48
    '0,                                 // memop_type
    '0,                                 // memop_addr
    '0,                                 // memop_wdata
    '0,                                 // memop_wstrb
    1'b0,                               // memop_sign_ext
    '0,                                 // memop_size
    1'b0,                               // memop_debug
    '0,                                 // memop_res_pc
    '0,                                 // memop_res_instr
    '0,                                 // memop_res_addr
    '0,                                 // memop_res_wtag
    '0,                                 // memop_res_data
    1'b0,                               // memop_res_wena
    '0,                                 // hold_rdata
    '0,                                 // pc
    1'b0                                // valid
};

endpackage: memaccess_pkg
