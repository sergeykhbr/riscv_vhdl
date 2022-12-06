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

module Mmu #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    output logic o_core_req_ready,
    input logic i_core_req_valid,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_core_req_addr,
    input logic i_core_req_fetch,                           // Memory request from 0=fetcher; 1=memaccess
    input logic [river_cfg_pkg::MemopType_Total-1:0] i_core_req_type,// Memory operation type
    input logic [63:0] i_core_req_wdata,                    // Data path requested data (write transaction)
    input logic [7:0] i_core_req_wstrb,                     // 8-bytes aligned strobs
    input logic [1:0] i_core_req_size,                      // 1,2,4 or 8-bytes operation for uncached access
    output logic o_core_resp_valid,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_core_resp_addr,
    output logic [63:0] o_core_resp_data,
    output logic o_core_resp_load_fault,                    // Ex.2./Ex.5. Instruction access fault when = 0 and fetch or Load access fault
    output logic o_core_resp_store_fault,                   // Ex.7. Store/AMO access fault
    output logic o_core_resp_page_x_fault,                  // Ex.12 Instruction page fault
    output logic o_core_resp_page_r_fault,                  // Ex.13 Load page fault
    output logic o_core_resp_page_w_fault,                  // Ex.15 Store/AMO page fault
    input logic i_core_resp_ready,
    input logic i_mem_req_ready,
    output logic o_mem_req_valid,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_mem_req_addr,
    output logic [river_cfg_pkg::MemopType_Total-1:0] o_mem_req_type,// Memory operation type
    output logic [63:0] o_mem_req_wdata,                    // Data path requested data (write transaction)
    output logic [7:0] o_mem_req_wstrb,                     // 8-bytes aligned strobs
    output logic [1:0] o_mem_req_size,                      // 1,2,4 or 8-bytes operation for uncached access
    input logic i_mem_resp_valid,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_mem_resp_addr,
    input logic [63:0] i_mem_resp_data,
    input logic i_mem_resp_load_fault,
    input logic i_mem_resp_store_fault,
    output logic o_mem_resp_ready,
    input logic i_mmu_ena,                                  // MMU enabled in U and S modes. Sv39 or Sv48 are implemented.
    input logic i_mmu_sv39,                                 // MMU sv39 is active
    input logic i_mmu_sv48,                                 // MMU sv48 is active
    input logic [43:0] i_mmu_ppn,                           // Physical Page Number from SATP CSR
    input logic i_mprv,                                     // modify priviledge flag can be active in m-mode
    input logic i_mxr,                                      // make executabale readable
    input logic i_sum,                                      // permit Supervisor User Mode access
    input logic i_fence,                                    // reset TBL entries at specific address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_fence_addr// Fence address: 0=clean all TBL
);

import river_cfg_pkg::*;
import mmu_pkg::*;

logic [CFG_MMU_TLB_AWIDTH-1:0] wb_tlb_adr;
logic w_tlb_wena;
logic [CFG_MMU_PTE_DWIDTH-1:0] wb_tlb_wdata;
logic [CFG_MMU_PTE_DWIDTH-1:0] wb_tlb_rdata;
Mmu_registers r, rin;

ram_mmu_tech #(
    .abits(CFG_MMU_TLB_AWIDTH),
    .dbits(CFG_MMU_PTE_DWIDTH)
) tlb (
    .i_clk(i_clk),
    .i_addr(wb_tlb_adr),
    .i_wena(w_tlb_wena),
    .i_wdata(wb_tlb_wdata),
    .o_rdata(wb_tlb_rdata)
);


always_comb
begin: comb_proc
    Mmu_registers v;
    logic v_core_req_x;
    logic v_core_req_r;
    logic v_core_req_w;
    logic last_page_fault_x;
    logic last_page_fault_r;
    logic last_page_fault_w;
    logic v_core_req_ready;
    logic v_core_resp_valid;
    logic [RISCV_ARCH-1:0] vb_core_resp_addr;
    logic [63:0] vb_core_resp_data;
    logic v_core_resp_load_fault;
    logic v_core_resp_store_fault;
    logic [RISCV_ARCH-1:0] vb_last_pa_req;
    logic v_mem_req_valid;
    logic [RISCV_ARCH-1:0] vb_mem_req_addr;
    logic [MemopType_Total-1:0] vb_mem_req_type;
    logic [63:0] vb_mem_req_wdata;
    logic [7:0] vb_mem_req_wstrb;
    logic [1:0] vb_mem_req_size;
    logic v_mem_resp_ready;
    logic v_tlb_wena;
    logic [CFG_MMU_TLB_AWIDTH-1:0] vb_tlb_adr;
    logic [(RISCV_ARCH - 12)-1:0] vb_pte_start_va;
    logic [(RISCV_ARCH - 12)-1:0] vb_resp_ppn;
    logic v_va_ena;
    logic [11:0] vb_level0_off;
    logic [11:0] vb_level1_off;
    logic [11:0] vb_level2_off;
    logic [11:0] vb_level3_off;
    logic v_last_valid;
    logic v_tlb_hit;
    logic [RISCV_ARCH-1:0] vb_tlb_pa0;                      // 4 KB page phys address
    logic [RISCV_ARCH-1:0] vb_tlb_pa1;                      // 8 MB page phys address
    logic [RISCV_ARCH-1:0] vb_tlb_pa2;                      // 16 GB page phys address
    logic [RISCV_ARCH-1:0] vb_tlb_pa3;                      // 32 TB page phys address
    logic [RISCV_ARCH-1:0] vb_tlb_pa_hit;
    logic [CFG_MMU_PTE_DWIDTH-1:0] t_tlb_wdata;
    int t_idx_lsb;

    v_core_req_x = 0;
    v_core_req_r = 0;
    v_core_req_w = 0;
    last_page_fault_x = 0;
    last_page_fault_r = 0;
    last_page_fault_w = 0;
    v_core_req_ready = 0;
    v_core_resp_valid = 0;
    vb_core_resp_addr = 0;
    vb_core_resp_data = 0;
    v_core_resp_load_fault = 0;
    v_core_resp_store_fault = 0;
    vb_last_pa_req = 0;
    v_mem_req_valid = 0;
    vb_mem_req_addr = 0;
    vb_mem_req_type = 0;
    vb_mem_req_wdata = 0;
    vb_mem_req_wstrb = 0;
    vb_mem_req_size = 0;
    v_mem_resp_ready = 0;
    v_tlb_wena = 0;
    vb_tlb_adr = 0;
    vb_pte_start_va = 0;
    vb_resp_ppn = 0;
    v_va_ena = 0;
    vb_level0_off = 0;
    vb_level1_off = 0;
    vb_level2_off = 0;
    vb_level3_off = 0;
    v_last_valid = 0;
    v_tlb_hit = 0;
    vb_tlb_pa0 = 64'h0000000000000000;
    vb_tlb_pa1 = 64'h0000000000000000;
    vb_tlb_pa2 = 64'h0000000000000000;
    vb_tlb_pa3 = 64'h0000000000000000;
    vb_tlb_pa_hit = 0;
    t_tlb_wdata = 0;
    t_idx_lsb = 0;

    v = r;

    t_idx_lsb = (12 + (9 * int'(r.last_page_size)));
    vb_tlb_adr = i_core_req_addr[t_idx_lsb +: CFG_MMU_TLB_AWIDTH];

    if (i_core_req_fetch == 1'b1) begin
        v_core_req_x = 1'b1;
    end else if (((|i_core_req_type) == 1'b0)
                || (i_core_req_type == MemopType_Reserve)) begin
        v_core_req_r = 1'b1;
    end else begin
        v_core_req_w = 1'b1;
    end
    if (r.last_permission[PTE_A] == 1'b0) begin
        last_page_fault_x = 1'b1;
        last_page_fault_r = 1'b1;
        last_page_fault_w = 1'b1;
    end
    if (r.last_permission[PTE_X] == 1'b0) begin
        last_page_fault_x = 1'b1;
    end
    if (r.last_permission[PTE_R] == 1'b0) begin
        last_page_fault_r = 1'b1;
    end
    if ((r.last_permission[PTE_W] == 1'b0) || (r.last_permission[PTE_D] == 1'b0)) begin
        last_page_fault_w = 1'b1;
    end

    // Start Page Physical Address
    vb_pte_start_va[43: 0] = i_mmu_ppn;
    if (i_mmu_ppn[43] == 1'b1) begin
        vb_pte_start_va[51: 44] = '1;
    end
    // Page walking base Physical Address
    vb_resp_ppn[43: 0] = r.resp_data[53: 10];
    v_va_ena = i_mmu_ena;
    // M-mode can opearate with physical and virtual addresses when MPRV=1
    if ((i_mprv == 1'b1) && ((&i_core_req_addr[63: 48]) == 1'b0)) begin
        // Use physical address
        v_va_ena = 1'b0;
    end
    vb_level0_off = {r.last_va[47: 39], 3'h0};
    vb_level1_off = {r.last_va[38: 30], 3'h0};
    vb_level2_off = {r.last_va[29: 21], 3'h0};
    vb_level3_off = {r.last_va[20: 12], 3'h0};

    // Pages: 4 KB, 8MB, 16 GB and 32 TB for sv48 only
    // Check the last hit depending page size:
    v_last_valid = 1'b0;
    if ((r.last_page_size == 2'h0)
            && (i_core_req_addr[63: 12] == r.last_va[63: 12])) begin
        v_last_valid = 1'b1;
        vb_last_pa_req = {r.last_pa, i_core_req_addr[11: 0]};
    end else if ((r.last_page_size == 2'h1)
                && (i_core_req_addr[63: 21] == r.last_va[63: 21])) begin
        v_last_valid = 1'b1;
        vb_last_pa_req = {r.last_pa[51: 9], i_core_req_addr[20: 0]};
    end else if ((r.last_page_size == 2'h2)
                && (i_core_req_addr[63: 30] == r.last_va[63: 30])) begin
        v_last_valid = 1'b1;
        vb_last_pa_req = {r.last_pa[51: 18], i_core_req_addr[29: 0]};
    end else if ((r.last_page_size == 2'h3)
                && (i_core_req_addr[63: 39] == r.last_va[63: 39])) begin
        v_last_valid = 1'b1;
        vb_last_pa_req = {r.last_pa[51: 27], i_core_req_addr[38: 0]};
    end

    // Check table hit depending page size:
    vb_tlb_pa0[63: 12] = wb_tlb_rdata[63: 12];
    vb_tlb_pa0[11: 0] = r.last_va[11: 0];
    vb_tlb_pa1[63: 21] = wb_tlb_rdata[63: 21];
    vb_tlb_pa1[20: 0] = r.last_va[20: 0];
    vb_tlb_pa2[63: 30] = wb_tlb_rdata[63: 30];
    vb_tlb_pa2[29: 0] = r.last_va[29: 0];
    vb_tlb_pa3[63: 39] = wb_tlb_rdata[63: 39];
    vb_tlb_pa3[38: 0] = r.last_va[38: 0];

    if (wb_tlb_rdata[PTE_V] == 1'b1) begin
        if (wb_tlb_rdata[9: 8] == 2'h3) begin
            // 32 TB pages:
            v_tlb_hit = (~(r.last_va[63: 39] ^ wb_tlb_rdata[115: 91]));
            vb_tlb_pa_hit = vb_tlb_pa3;
        end else if (wb_tlb_rdata[9: 8] == 2'h2) begin
            // 16 GB pages:
            v_tlb_hit = (~(r.last_va[63: 30] ^ wb_tlb_rdata[115: 82]));
            vb_tlb_pa_hit = vb_tlb_pa2;
        end else if (wb_tlb_rdata[9: 8] == 2'h1) begin
            // 8 MB pages:
            v_tlb_hit = (~(r.last_va[63: 21] ^ wb_tlb_rdata[115: 73]));
            vb_tlb_pa_hit = vb_tlb_pa1;
        end else begin
            // 4 KB pages:
            v_tlb_hit = (~(r.last_va[63: 12] ^ wb_tlb_rdata[115: 64]));
            vb_tlb_pa_hit = vb_tlb_pa0;
        end
    end

    t_tlb_wdata[115: 64] = r.last_va[63: 12];
    t_tlb_wdata[63: 12] = vb_resp_ppn;
    t_tlb_wdata[9: 8] = r.tlb_page_size;
    t_tlb_wdata[7: 0] = r.resp_data[7: 0];

    case (r.state)
    Idle: begin
        v.tlb_hit = 1'b0;
        v.resp_load_fault = 1'b0;
        v.resp_store_fault = 1'b0;
        v.ex_page_fault = 1'b0;
        if (i_core_req_valid == 1'b1) begin
            v.last_mmu_ena = (i_mmu_ena && v_va_ena);
            v.last_va = i_core_req_addr;
            v.req_type = i_core_req_type;
            v.req_wdata = i_core_req_wdata;
            v.req_wstrb = i_core_req_wstrb;
            v.req_size = i_core_req_size;
        end
        if (r.req_flush == 1'b1) begin
            v.req_flush = 1'b0;
            v.tlb_wdata = '0;
            v.state = FlushTlb;
        end else if ((i_mmu_ena == 1'b0) || (v_va_ena == 1'b0)) begin// MMU disabled
            // Direct connection to Cache
            v_core_req_ready = i_mem_req_ready;
            v_core_resp_valid = i_mem_resp_valid;
            vb_core_resp_addr = i_mem_resp_addr;
            vb_core_resp_data = i_mem_resp_data;
            v_core_resp_load_fault = i_mem_resp_load_fault;
            v_core_resp_store_fault = i_mem_resp_store_fault;
            v_mem_req_valid = i_core_req_valid;
            vb_mem_req_addr = i_core_req_addr;
            vb_mem_req_type = i_core_req_type;
            vb_mem_req_wdata = i_core_req_wdata;
            vb_mem_req_wstrb = i_core_req_wstrb;
            vb_mem_req_size = i_core_req_size;
            v_mem_resp_ready = i_core_resp_ready;
            if ((i_core_req_valid && i_mem_req_ready) == 1'b1) begin
                v.state = WaitRespNoMmu;
            end
        end else if ((r.last_mmu_ena == 1'b1) && (v_last_valid == 1'b1)) begin// MMU enabled: Check the request to the same page:
            // Direct connection to cache with the fast changing va to last_pa
            v_core_req_ready = i_mem_req_ready;
            v_core_resp_valid = i_mem_resp_valid;
            vb_core_resp_addr = r.last_va;
            vb_core_resp_data = i_mem_resp_data;
            v_core_resp_load_fault = i_mem_resp_load_fault;
            v_core_resp_store_fault = i_mem_resp_store_fault;
            v_mem_req_valid = i_core_req_valid;
            vb_mem_req_addr = vb_last_pa_req;
            vb_mem_req_type = i_core_req_type;
            vb_mem_req_wdata = i_core_req_wdata;
            vb_mem_req_wstrb = i_core_req_wstrb;
            vb_mem_req_size = i_core_req_size;
            v_mem_resp_ready = i_core_resp_ready;
            if ((i_core_req_valid && i_mem_req_ready) == 1'b1) begin
                v.state = WaitRespLast;
            end
        end else begin
            // MMU enabled: check TLB
            v_core_req_ready = 1'b1;
            if (i_core_req_valid == 1'b1) begin
                v.state = CheckTlb;
                v.req_x = v_core_req_x;
                v.req_r = v_core_req_r;
                v.req_w = v_core_req_w;
            end
        end
    end
    WaitRespNoMmu: begin
        v_core_req_ready = i_mem_req_ready;
        v_core_resp_valid = i_mem_resp_valid;
        vb_core_resp_addr = i_mem_resp_addr;
        vb_core_resp_data = i_mem_resp_data;
        v_core_resp_load_fault = i_mem_resp_load_fault;
        v_core_resp_store_fault = i_mem_resp_store_fault;
        v_mem_req_valid = i_core_req_valid;
        vb_mem_req_addr = i_core_req_addr;
        vb_mem_req_type = i_core_req_type;
        vb_mem_req_wdata = i_core_req_wdata;
        vb_mem_req_wstrb = i_core_req_wstrb;
        vb_mem_req_size = i_core_req_size;
        v_mem_resp_ready = i_core_resp_ready;
        if ((i_mem_resp_valid && i_core_resp_ready) == 1'b1) begin
            if (i_mmu_ena == 1'b1) begin
                // Do not accept new request because MMU state changed
                v_core_req_ready = 1'b0;
                v_mem_req_valid = 1'b0;
            end
            if ((v_core_req_ready == 1'b0) || (v_mem_req_valid == 1'b0)) begin
                v.state = Idle;
            end
        end
    end
    WaitRespLast: begin
        v_core_req_ready = i_mem_req_ready;
        v_core_resp_valid = i_mem_resp_valid;
        vb_core_resp_addr = r.last_va;
        vb_core_resp_data = i_mem_resp_data;
        v_core_resp_load_fault = i_mem_resp_load_fault;
        v_core_resp_store_fault = i_mem_resp_store_fault;
        v_mem_req_valid = i_core_req_valid;
        vb_mem_req_addr = vb_last_pa_req;
        vb_mem_req_type = i_core_req_type;
        vb_mem_req_wdata = i_core_req_wdata;
        vb_mem_req_wstrb = i_core_req_wstrb;
        vb_mem_req_size = i_core_req_size;
        v_mem_resp_ready = i_core_resp_ready;
        if ((i_mem_resp_valid && i_core_resp_ready) == 1'b1) begin
            if (v_last_valid == 1'b0) begin
                // Do not accept new request because of new VA request
                v_core_req_ready = 1'b0;
                v_mem_req_valid = 1'b0;
            end
            if ((v_core_req_ready == 1'b0) || (v_mem_req_valid == 1'b0)) begin
                v.state = Idle;
            end else begin
                v.last_va = i_core_req_addr;
                v.req_type = i_core_req_type;
                v.req_wdata = i_core_req_wdata;
                v.req_wstrb = i_core_req_wstrb;
                v.req_size = i_core_req_size;
                v.req_x = v_core_req_x;
                v.req_r = v_core_req_r;
                v.req_w = v_core_req_w;
                if ((v_core_req_x && last_page_fault_x) || (v_core_req_r && last_page_fault_r) || (v_core_req_w && last_page_fault_w)) begin
                    // New request to the same page has not permission
                    v.ex_page_fault = 1'b1;
                    v.state = AcceptCore;
                end
            end
        end
    end
    CheckTlb: begin
        if (v_tlb_hit == 1'b1) begin
            // TLB hit
            v.tlb_hit = 1'b1;
            v.last_pa = wb_tlb_rdata[63: 12];
            v.last_permission = wb_tlb_rdata[7: 0];
            v.req_pa = vb_tlb_pa_hit;
        end else begin
            // TLB miss
            if (i_mmu_sv39 == 1'b1) begin
                v.tlb_level = 4'h2;                         // Start page decoding sv39
                v.tlb_page_size = 2'h2;
                v.req_pa = {vb_pte_start_va, vb_level1_off};
            end else begin
                v.tlb_level = 4'h1;                         // Start page decoding sv48
                v.tlb_page_size = 2'h3;
                v.req_pa = {vb_pte_start_va, vb_level0_off};
            end
        end
        v.state = CacheReq;
    end
    CacheReq: begin
        v_mem_req_valid = 1'b1;
        vb_mem_req_addr = r.req_pa;
        if (r.tlb_hit == 1'b0) begin
            vb_mem_req_type = '0;                           // Load tlb item
        end else begin
            vb_mem_req_type = r.req_type;
        end
        vb_mem_req_wdata = r.req_wdata;
        vb_mem_req_wstrb = r.req_wstrb;
        vb_mem_req_size = r.req_size;
        if (i_mem_req_ready == 1'b1) begin
            v.state = WaitResp;
        end
    end
    WaitResp: begin
        v_mem_resp_ready = 1'b1;
        if (i_mem_resp_valid == 1'b1) begin
            v.resp_addr = i_mem_resp_addr;
            v.resp_data = i_mem_resp_data;
            v.resp_load_fault = i_mem_resp_load_fault;      // Hardware error Load (unmapped access)
            v.resp_store_fault = i_mem_resp_store_fault;    // Hardware error Store/AMO (unmapped access)
            if ((r.tlb_hit || i_mem_resp_load_fault || i_mem_resp_store_fault) == 1'b1) begin
                v.state = AcceptCore;
            end else begin
                v.state = HandleResp;
            end
        end
    end
    HandleResp: begin
        if ((r.resp_data[PTE_V] == 1'b0) || (((~r.resp_data[PTE_R]) && r.resp_data[PTE_W]) == 1'b1)) begin// v=0 or (r=0 && w=1)
            // PTE is invalid
            v.ex_page_fault = 1'b1;
            v.state = AcceptCore;
        end else if ((r.resp_data[PTE_R] || r.resp_data[PTE_W] || r.resp_data[PTE_X]) == 1'b0) begin
            // PTE is a apointer to the next level
            v.state = CacheReq;
            v.tlb_level = {r.tlb_level, {1{1'b0}}};
            v.tlb_page_size = (r.tlb_page_size - 1);
            if (r.tlb_level[0] == 1'b1) begin
                v.req_pa = {vb_resp_ppn, vb_level1_off};
            end else if (r.tlb_level[1] == 1'b1) begin
                v.req_pa = {vb_resp_ppn, vb_level2_off};
            end else if (r.tlb_level[2] == 1'b1) begin
                v.req_pa = {vb_resp_ppn, vb_level3_off};
            end else begin
                // It was the last level
                v.ex_page_fault = 1'b1;
                v.state = AcceptCore;
            end
        end else begin
            // PTE is a leaf
            if (r.resp_data[PTE_A] == 1'b0) begin
                v.state = AcceptCore;
                v.ex_page_fault = 1'b1;
            end else if ((r.req_x == 1'b1) && (r.resp_data[PTE_X] == 1'b0)) begin
                v.state = AcceptCore;
                v.ex_page_fault = 1'b1;
            end else if ((r.req_r == 1'b1) && (r.resp_data[PTE_R] == 1'b0)) begin
                v.state = AcceptCore;
                v.ex_page_fault = 1'b1;
            end else if ((r.req_w == 1'b1) && ((r.resp_data[PTE_W] == 1'b0) || (r.resp_data[PTE_D] == 1'b0))) begin
                v.state = AcceptCore;
                v.ex_page_fault = 1'b1;
            end else begin
                v.state = UpdateTlb;
            end
            v.last_permission = r.resp_data[7: 0];
            v.last_page_size = r.tlb_page_size;
            v.tlb_wdata = t_tlb_wdata;
            // Pages more than 4KB support:
            if (r.tlb_level[0] == 1'b1) begin
                v.req_pa = {vb_resp_ppn[51: 27], r.last_va[38: 0]};
                v.last_pa = {vb_resp_ppn[51: 27], 27'h0000000};
            end else if (r.tlb_level[1] == 1'b1) begin
                v.req_pa = {vb_resp_ppn[51: 18], r.last_va[29: 0]};
                v.last_pa = {vb_resp_ppn[51: 18], 18'h00000};
            end else if (r.tlb_level[2] == 1'b1) begin
                v.req_pa = {vb_resp_ppn[51: 9], r.last_va[20: 0]};
                v.last_pa = {vb_resp_ppn[51: 9], 9'h000};
            end else begin
                v.req_pa = {vb_resp_ppn, r.last_va[11: 0]};
                v.last_pa = vb_resp_ppn;
            end
        end
    end
    UpdateTlb: begin
        // Translation is finished: write va/pa into TLB memory
        v_tlb_wena = 1'b1;
        vb_tlb_adr = r.last_va[t_idx_lsb +: CFG_MMU_TLB_AWIDTH];
        v.state = CacheReq;                                 // Read data by physical address
        v.tlb_hit = 1'b1;
    end
    AcceptCore: begin
        v_core_resp_valid = 1'b1;
        vb_core_resp_addr = r.last_va;
        vb_core_resp_data = r.resp_data;
        v_core_resp_load_fault = r.resp_load_fault;
        v_core_resp_store_fault = r.resp_store_fault;
        if (i_core_resp_ready == 1'b1) begin
            v.state = Idle;
            if (r.ex_page_fault == 1'b1) begin
                v.last_va = '1;
                v.last_pa = '1;
            end
        end
    end
    FlushTlb: begin
        v_tlb_wena = 1'b1;
        vb_tlb_adr = r.tlb_flush_adr;
        v.last_va = '1;
        v.last_pa = '1;
        v.last_mmu_ena = 1'b0;
        if ((|r.tlb_flush_cnt) == 1'b0) begin
            v.state = Idle;
        end else begin
            v.tlb_flush_cnt = (r.tlb_flush_cnt - 1);
            v.tlb_flush_adr = (r.tlb_flush_adr + 1);
        end
    end
    default: begin
    end
    endcase

    if (i_fence == 1'b1) begin
        // Clear whole table ignoring i_fence_addr
        v.req_flush = 1'b1;
        v.tlb_flush_cnt = '1;
        v.tlb_flush_adr = '0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = Mmu_r_reset;
    end

    w_tlb_wena = v_tlb_wena;
    wb_tlb_adr = vb_tlb_adr;
    wb_tlb_wdata = r.tlb_wdata;
    o_core_req_ready = v_core_req_ready;
    o_core_resp_valid = v_core_resp_valid;
    o_core_resp_addr = vb_core_resp_addr;
    o_core_resp_data = vb_core_resp_data;
    o_core_resp_load_fault = v_core_resp_load_fault;
    o_core_resp_store_fault = v_core_resp_store_fault;
    o_core_resp_page_x_fault = (r.ex_page_fault && r.req_x);
    o_core_resp_page_r_fault = (r.ex_page_fault && r.req_r);
    o_core_resp_page_w_fault = (r.ex_page_fault && r.req_w);
    o_mem_req_valid = v_mem_req_valid;
    o_mem_req_addr = vb_mem_req_addr;
    o_mem_req_type = vb_mem_req_type;
    o_mem_req_wdata = vb_mem_req_wdata;
    o_mem_req_wstrb = vb_mem_req_wstrb;
    o_mem_req_size = vb_mem_req_size;
    o_mem_resp_ready = v_mem_resp_ready;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= Mmu_r_reset;
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

endmodule: Mmu
