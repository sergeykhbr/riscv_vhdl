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
package mmu_pkg;

import river_cfg_pkg::*;

localparam int unsigned Idle = 0;
localparam int unsigned WaitRespNoMmu = 1;
localparam int unsigned WaitRespLast = 2;
localparam int unsigned CheckTlb = 3;
localparam int unsigned CacheReq = 4;
localparam int unsigned WaitResp = 5;
localparam int unsigned HandleResp = 6;
localparam int unsigned UpdateTlb = 7;
localparam int unsigned AcceptCore = 8;
localparam int unsigned FlushTlb = 9;
localparam int PTE_V = 0;                                   // Valid PTE entry bit
localparam int PTE_R = 1;                                   // Read access PTE entry bit
localparam int PTE_W = 2;                                   // Write Access PTE entry bit
localparam int PTE_X = 3;                                   // Execute Access PTE entry bit
localparam int PTE_U = 4;                                   // Accessible in U-mode
localparam int PTE_G = 5;                                   // Global mapping
localparam int PTE_A = 6;                                   // Accessed bit
localparam int PTE_D = 7;                                   // Dirty bit

typedef struct {
    logic [3:0] state;
    logic req_x;
    logic req_r;
    logic req_w;
    logic [RISCV_ARCH-1:0] req_pa;
    logic [MemopType_Total-1:0] req_type;
    logic [63:0] req_wdata;
    logic [7:0] req_wstrb;
    logic [1:0] req_size;
    logic req_flush;
    logic last_mmu_ena;
    logic [RISCV_ARCH-1:0] last_va;
    logic [51:0] last_pa;
    logic [7:0] last_permission;                            // Last permisison flags: DAGUXWRV
    logic [1:0] last_page_size;
    logic [RISCV_ARCH-1:0] resp_addr;
    logic [63:0] resp_data;
    logic resp_load_fault;
    logic resp_store_fault;
    logic ex_page_fault;
    logic tlb_hit;
    logic [3:0] tlb_level;
    logic [1:0] tlb_page_size;
    logic [CFG_MMU_PTE_DWIDTH-1:0] tlb_wdata;
    logic [CFG_MMU_TLB_AWIDTH-1:0] tlb_flush_cnt;
    logic [CFG_MMU_TLB_AWIDTH-1:0] tlb_flush_adr;
} Mmu_registers;

const Mmu_registers Mmu_r_reset = '{
    FlushTlb,                           // state
    1'b0,                               // req_x
    1'b0,                               // req_r
    1'b0,                               // req_w
    '0,                                 // req_pa
    '0,                                 // req_type
    '0,                                 // req_wdata
    '0,                                 // req_wstrb
    '0,                                 // req_size
    1'b0,                               // req_flush
    1'b0,                               // last_mmu_ena
    '1,                                 // last_va
    '1,                                 // last_pa
    '0,                                 // last_permission
    '0,                                 // last_page_size
    '0,                                 // resp_addr
    '0,                                 // resp_data
    1'b0,                               // resp_load_fault
    1'b0,                               // resp_store_fault
    1'b0,                               // ex_page_fault
    1'b0,                               // tlb_hit
    '0,                                 // tlb_level
    '0,                                 // tlb_page_size
    '0,                                 // tlb_wdata
    '1,                                 // tlb_flush_cnt
    '0                                  // tlb_flush_adr
};

endpackage: mmu_pkg
