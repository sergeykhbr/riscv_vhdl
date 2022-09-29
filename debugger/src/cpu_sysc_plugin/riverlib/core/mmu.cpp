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

#include "mmu.h"
#include "api_core.h"

namespace debugger {

Mmu::Mmu(sc_module_name name,
         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    o_core_req_ready("o_core_req_ready"),
    i_core_req_valid("i_core_req_valid"),
    i_core_req_addr("i_core_req_addr"),
    i_core_req_fetch("i_core_req_fetch"),
    i_core_req_type("i_core_req_type"),
    i_core_req_wdata("i_core_req_wdata"),
    i_core_req_wstrb("i_core_req_wstrb"),
    i_core_req_size("i_core_req_size"),
    o_core_resp_valid("o_core_resp_valid"),
    o_core_resp_addr("o_core_resp_addr"),
    o_core_resp_data("o_core_resp_data"),
    o_core_resp_executable("o_core_resp_executable"),
    o_core_resp_load_fault("o_core_resp_load_fault"),
    o_core_resp_store_fault("o_core_resp_store_fault"),
    o_core_resp_page_x_fault("o_core_resp_page_x_fault"),
    o_core_resp_page_r_fault("o_core_resp_page_r_fault"),
    o_core_resp_page_w_fault("o_core_resp_page_w_fault"),
    i_core_resp_ready("i_core_resp_ready"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_req_valid("o_mem_req_valid"),
    o_mem_req_addr("o_mem_req_addr"),
    o_mem_req_type("o_mem_req_type"),
    o_mem_req_wdata("o_mem_req_wdata"),
    o_mem_req_wstrb("o_mem_req_wstrb"),
    o_mem_req_size("o_mem_req_size"),
    i_mem_resp_valid("i_mem_resp_valid"),
    i_mem_resp_addr("i_mem_resp_addr"),
    i_mem_resp_data("i_mem_resp_data"),
    i_mem_resp_executable("i_mem_resp_executable"),
    i_mem_resp_load_fault("i_mem_resp_load_fault"),
    i_mem_resp_store_fault("i_mem_resp_store_fault"),
    o_mem_resp_ready("o_mem_resp_ready"),
    i_mmu_ena("i_mmu_ena"),
    i_mmu_ppn("i_mmu_ppn"),
    i_fence("i_fence"),
    i_fence_addr("i_fence_addr") {

    async_reset_ = async_reset;
    tlb = 0;

    tlb = new ram_mmu_tech<CFG_MMU_TLB_AWIDTH,
                           CFG_MMU_PTE_DWIDTH>("tlb");
    tlb->i_clk(i_clk);
    tlb->i_addr(wb_tlb_adr);
    tlb->i_wena(w_tlb_wena);
    tlb->i_wdata(wb_tlb_wdata);
    tlb->o_rdata(wb_tlb_rdata);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_core_req_valid;
    sensitive << i_core_req_addr;
    sensitive << i_core_req_fetch;
    sensitive << i_core_req_type;
    sensitive << i_core_req_wdata;
    sensitive << i_core_req_wstrb;
    sensitive << i_core_req_size;
    sensitive << i_core_resp_ready;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_resp_valid;
    sensitive << i_mem_resp_addr;
    sensitive << i_mem_resp_data;
    sensitive << i_mem_resp_executable;
    sensitive << i_mem_resp_load_fault;
    sensitive << i_mem_resp_store_fault;
    sensitive << i_mmu_ena;
    sensitive << i_mmu_ppn;
    sensitive << i_fence;
    sensitive << i_fence_addr;
    sensitive << wb_tlb_adr;
    sensitive << w_tlb_wena;
    sensitive << wb_tlb_wdata;
    sensitive << wb_tlb_rdata;
    sensitive << r.state;
    sensitive << r.req_x;
    sensitive << r.req_r;
    sensitive << r.req_w;
    sensitive << r.req_pa;
    sensitive << r.req_type;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.req_size;
    sensitive << r.last_va;
    sensitive << r.last_pa;
    sensitive << r.last_permission;
    sensitive << r.resp_addr;
    sensitive << r.resp_data;
    sensitive << r.resp_executable;
    sensitive << r.resp_load_fault;
    sensitive << r.resp_store_fault;
    sensitive << r.ex_page_fault;
    sensitive << r.tlb_hit;
    sensitive << r.tlb_level;
    sensitive << r.tlb_wdata;
    sensitive << r.tlb_flush_cnt;
    sensitive << r.tlb_flush_adr;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

Mmu::~Mmu() {
    if (tlb) {
        delete tlb;
    }
}

void Mmu::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, o_core_req_ready, o_core_req_ready.name());
        sc_trace(o_vcd, i_core_req_valid, i_core_req_valid.name());
        sc_trace(o_vcd, i_core_req_addr, i_core_req_addr.name());
        sc_trace(o_vcd, i_core_req_fetch, i_core_req_fetch.name());
        sc_trace(o_vcd, i_core_req_type, i_core_req_type.name());
        sc_trace(o_vcd, i_core_req_wdata, i_core_req_wdata.name());
        sc_trace(o_vcd, i_core_req_wstrb, i_core_req_wstrb.name());
        sc_trace(o_vcd, i_core_req_size, i_core_req_size.name());
        sc_trace(o_vcd, o_core_resp_valid, o_core_resp_valid.name());
        sc_trace(o_vcd, o_core_resp_addr, o_core_resp_addr.name());
        sc_trace(o_vcd, o_core_resp_data, o_core_resp_data.name());
        sc_trace(o_vcd, o_core_resp_executable, o_core_resp_executable.name());
        sc_trace(o_vcd, o_core_resp_load_fault, o_core_resp_load_fault.name());
        sc_trace(o_vcd, o_core_resp_store_fault, o_core_resp_store_fault.name());
        sc_trace(o_vcd, o_core_resp_page_x_fault, o_core_resp_page_x_fault.name());
        sc_trace(o_vcd, o_core_resp_page_r_fault, o_core_resp_page_r_fault.name());
        sc_trace(o_vcd, o_core_resp_page_w_fault, o_core_resp_page_w_fault.name());
        sc_trace(o_vcd, i_core_resp_ready, i_core_resp_ready.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_mem_req_valid, o_mem_req_valid.name());
        sc_trace(o_vcd, o_mem_req_addr, o_mem_req_addr.name());
        sc_trace(o_vcd, o_mem_req_type, o_mem_req_type.name());
        sc_trace(o_vcd, o_mem_req_wdata, o_mem_req_wdata.name());
        sc_trace(o_vcd, o_mem_req_wstrb, o_mem_req_wstrb.name());
        sc_trace(o_vcd, o_mem_req_size, o_mem_req_size.name());
        sc_trace(o_vcd, i_mem_resp_valid, i_mem_resp_valid.name());
        sc_trace(o_vcd, i_mem_resp_addr, i_mem_resp_addr.name());
        sc_trace(o_vcd, i_mem_resp_data, i_mem_resp_data.name());
        sc_trace(o_vcd, i_mem_resp_executable, i_mem_resp_executable.name());
        sc_trace(o_vcd, i_mem_resp_load_fault, i_mem_resp_load_fault.name());
        sc_trace(o_vcd, i_mem_resp_store_fault, i_mem_resp_store_fault.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());
        sc_trace(o_vcd, i_mmu_ena, i_mmu_ena.name());
        sc_trace(o_vcd, i_mmu_ppn, i_mmu_ppn.name());
        sc_trace(o_vcd, i_fence, i_fence.name());
        sc_trace(o_vcd, i_fence_addr, i_fence_addr.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_x, pn + ".r_req_x");
        sc_trace(o_vcd, r.req_r, pn + ".r_req_r");
        sc_trace(o_vcd, r.req_w, pn + ".r_req_w");
        sc_trace(o_vcd, r.req_pa, pn + ".r_req_pa");
        sc_trace(o_vcd, r.req_type, pn + ".r_req_type");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.req_size, pn + ".r_req_size");
        sc_trace(o_vcd, r.last_va, pn + ".r_last_va");
        sc_trace(o_vcd, r.last_pa, pn + ".r_last_pa");
        sc_trace(o_vcd, r.last_permission, pn + ".r_last_permission");
        sc_trace(o_vcd, r.resp_addr, pn + ".r_resp_addr");
        sc_trace(o_vcd, r.resp_data, pn + ".r_resp_data");
        sc_trace(o_vcd, r.resp_executable, pn + ".r_resp_executable");
        sc_trace(o_vcd, r.resp_load_fault, pn + ".r_resp_load_fault");
        sc_trace(o_vcd, r.resp_store_fault, pn + ".r_resp_store_fault");
        sc_trace(o_vcd, r.ex_page_fault, pn + ".r_ex_page_fault");
        sc_trace(o_vcd, r.tlb_hit, pn + ".r_tlb_hit");
        sc_trace(o_vcd, r.tlb_level, pn + ".r_tlb_level");
        sc_trace(o_vcd, r.tlb_wdata, pn + ".r_tlb_wdata");
        sc_trace(o_vcd, r.tlb_flush_cnt, pn + ".r_tlb_flush_cnt");
        sc_trace(o_vcd, r.tlb_flush_adr, pn + ".r_tlb_flush_adr");
    }

}

void Mmu::comb() {
    bool v_core_req_x;
    bool v_core_req_r;
    bool v_core_req_w;
    bool last_page_fault_x;
    bool last_page_fault_r;
    bool last_page_fault_w;
    bool v_core_req_ready;
    bool v_core_resp_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_core_resp_addr;
    sc_uint<64> vb_core_resp_data;
    bool v_core_resp_executable;
    bool v_core_resp_load_fault;
    bool v_core_resp_store_fault;
    bool v_mem_req_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_mem_req_addr;
    sc_uint<MemopType_Total> vb_mem_req_type;
    sc_uint<64> vb_mem_req_wdata;
    sc_uint<8> vb_mem_req_wstrb;
    sc_uint<2> vb_mem_req_size;
    bool v_mem_resp_ready;
    bool v_tlb_wena;
    sc_uint<CFG_MMU_TLB_AWIDTH> vb_tlb_adr;
    sc_uint<(CFG_CPU_ADDR_BITS - 12)> vb_pte_start_va;
    sc_uint<(CFG_CPU_ADDR_BITS - 12)> vb_pte_base_va;
    sc_uint<12> vb_level0_off;
    sc_uint<12> vb_level1_off;
    sc_uint<12> vb_level2_off;
    sc_uint<12> vb_level3_off;
    bool v_last_valid;
    sc_uint<CFG_CPU_ADDR_BITS> t_req_pa;
    sc_biguint<CFG_MMU_PTE_DWIDTH> t_tlb_wdata;

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
    v_core_resp_executable = 0;
    v_core_resp_load_fault = 0;
    v_core_resp_store_fault = 0;
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
    vb_pte_base_va = 0;
    vb_level0_off = 0;
    vb_level1_off = 0;
    vb_level2_off = 0;
    vb_level3_off = 0;
    v_last_valid = 0;
    t_req_pa = 0;
    t_tlb_wdata = 0;

    v = r;

    vb_tlb_adr = i_core_req_addr.read()(((12 + CFG_MMU_TLB_AWIDTH) - 1), 12);

    if (i_core_req_fetch.read() == 1) {
        v_core_req_x = 1;
    } else if ((i_core_req_type.read().or_reduce() == 0) || (i_core_req_type.read() == MemopType_Reserve)) {
        v_core_req_r = 1;
    } else {
        v_core_req_w = 1;
    }
    if (r.last_permission.read()[PTE_A] == 0) {
        last_page_fault_x = 1;
        last_page_fault_r = 1;
        last_page_fault_w = 1;
    }
    if (r.last_permission.read()[PTE_X] == 0) {
        last_page_fault_x = 1;
    }
    if (r.last_permission.read()[PTE_R] == 0) {
        last_page_fault_r = 1;
    }
    if ((r.last_permission.read()[PTE_W] == 0) || (r.last_permission.read()[PTE_D] == 0)) {
        last_page_fault_w = 1;
    }

    // Start Page Physical Address
    vb_pte_start_va(43, 0) = i_mmu_ppn;
    if (i_mmu_ppn.read()[43] == 1) {
        vb_pte_start_va(51, 44) = ~0ull;
    }
    // Page walking base Physical Address
    vb_pte_base_va(43, 0) = r.resp_data.read()(53, 10);
    if (r.resp_data.read()[53] == 1) {
        vb_pte_base_va(51, 44) = ~0ull;
    }
    vb_level0_off = (r.last_va.read()(47, 39) << 3);
    vb_level1_off = (r.last_va.read()(38, 30) << 3);
    vb_level2_off = (r.last_va.read()(29, 21) << 3);
    vb_level3_off = (r.last_va.read()(20, 12) << 3);

    v_last_valid = 0;
    if (i_core_req_addr.read()(63, 12) == r.last_va.read()(63, 12)) {
        v_last_valid = 1;
    }

    // Temporary variables are neccessary in systemc
    t_req_pa((CFG_CPU_ADDR_BITS - 1), 12) = wb_tlb_rdata.read()((CFG_CPU_ADDR_BITS - 1), 12).to_uint64();
    t_req_pa(11, 0) = r.last_va.read()(11, 0);

    t_tlb_wdata(115, 64) = r.last_va.read()(63, 12);
    t_tlb_wdata(63, 12) = vb_pte_base_va;
    t_tlb_wdata(7, 0) = r.resp_data.read()(7, 0);

    switch (r.state.read()) {
    case Idle:
        v.tlb_hit = 0;
        v.resp_executable = 0;
        v.resp_load_fault = 0;
        v.resp_store_fault = 0;
        v.ex_page_fault = 0;
        if (i_core_req_valid.read() == 1) {
            v.last_va = i_core_req_addr;
            v.req_type = i_core_req_type;
            v.req_wdata = i_core_req_wdata;
            v.req_wstrb = i_core_req_wstrb;
            v.req_size = i_core_req_size;
        }
        if (i_mmu_ena.read() == 0) {                        // MMU disabled
            // Direct connection to Cache
            v_core_req_ready = i_mem_req_ready;
            v_core_resp_valid = i_mem_resp_valid;
            vb_core_resp_addr = i_mem_resp_addr;
            vb_core_resp_data = i_mem_resp_data;
            v_core_resp_executable = i_mem_resp_executable;
            v_core_resp_load_fault = i_mem_resp_load_fault;
            v_core_resp_store_fault = i_mem_resp_store_fault;
            v_mem_req_valid = i_core_req_valid;
            vb_mem_req_addr = i_core_req_addr;
            vb_mem_req_type = i_core_req_type;
            vb_mem_req_wdata = i_core_req_wdata;
            vb_mem_req_wstrb = i_core_req_wstrb;
            vb_mem_req_size = i_core_req_size;
            v_mem_resp_ready = i_core_resp_ready;
            if ((i_core_req_valid && i_mem_req_ready) == 1) {
                v.state = WaitRespNoMmu;
            }
            v.last_va = ~0ull;
        } else if (r.tlb_flush_cnt.read().or_reduce() == 1) {
            v.state = FlushTlb;
            v.tlb_wdata = 0;
        } else if (v_last_valid == 1) {                     // MMU enabled: Check the request to the same page:
            // Direct connection to cache with the fast changing va to last_pa
            v_core_req_ready = i_mem_req_ready;
            v_core_resp_valid = i_mem_resp_valid;
            vb_core_resp_addr = r.last_va;
            vb_core_resp_data = i_mem_resp_data;
            v_core_resp_executable = i_mem_resp_executable;
            v_core_resp_load_fault = i_mem_resp_load_fault;
            v_core_resp_store_fault = i_mem_resp_store_fault;
            v_mem_req_valid = i_core_req_valid;
            vb_mem_req_addr(63, 12) = r.last_pa;
            vb_mem_req_addr(11, 0) = i_core_req_addr.read()(11, 0);
            vb_mem_req_type = i_core_req_type;
            vb_mem_req_wdata = i_core_req_wdata;
            vb_mem_req_wstrb = i_core_req_wstrb;
            vb_mem_req_size = i_core_req_size;
            v_mem_resp_ready = i_core_resp_ready;
            if ((i_core_req_valid && i_mem_req_ready) == 1) {
                v.state = WaitRespLast;
            }
        } else {
            // MMU enabled: check TLB
            v_core_req_ready = 1;
            if (i_core_req_valid.read() == 1) {
                v.state = CheckTlb;
                v.req_x = v_core_req_x;
                v.req_r = v_core_req_r;
                v.req_w = v_core_req_w;
            }
        }
        break;
    case WaitRespNoMmu:
        v_core_req_ready = i_mem_req_ready;
        v_core_resp_valid = i_mem_resp_valid;
        vb_core_resp_addr = i_mem_resp_addr;
        vb_core_resp_data = i_mem_resp_data;
        v_core_resp_executable = i_mem_resp_executable;
        v_core_resp_load_fault = i_mem_resp_load_fault;
        v_core_resp_store_fault = i_mem_resp_store_fault;
        v_mem_req_valid = i_core_req_valid;
        vb_mem_req_addr = i_core_req_addr;
        vb_mem_req_type = i_core_req_type;
        vb_mem_req_wdata = i_core_req_wdata;
        vb_mem_req_wstrb = i_core_req_wstrb;
        vb_mem_req_size = i_core_req_size;
        v_mem_resp_ready = i_core_resp_ready;
        if ((i_mem_resp_valid && i_core_resp_ready) == 1) {
            if (i_mmu_ena.read() == 1) {
                // Do not accept new request because MMU state changed
                v_core_req_ready = 0;
                v_mem_req_valid = 0;
            }
            if ((v_core_req_ready == 0) || (v_mem_req_valid == 0)) {
                v.state = Idle;
            }
        }
        break;
    case WaitRespLast:
        v_core_req_ready = i_mem_req_ready;
        v_core_resp_valid = i_mem_resp_valid;
        vb_core_resp_addr = r.last_va;
        vb_core_resp_data = i_mem_resp_data;
        v_core_resp_executable = i_mem_resp_executable;
        v_core_resp_load_fault = i_mem_resp_load_fault;
        v_core_resp_store_fault = i_mem_resp_store_fault;
        v_mem_req_valid = i_core_req_valid;
        vb_mem_req_addr(63, 12) = r.last_pa;
        vb_mem_req_addr(11, 0) = i_core_req_addr.read()(11, 0);
        vb_mem_req_type = i_core_req_type;
        vb_mem_req_wdata = i_core_req_wdata;
        vb_mem_req_wstrb = i_core_req_wstrb;
        vb_mem_req_size = i_core_req_size;
        v_mem_resp_ready = i_core_resp_ready;
        if ((i_mem_resp_valid && i_core_resp_ready) == 1) {
            if (v_last_valid == 0) {
                // Do not accept new request because of new VA request
                v_core_req_ready = 0;
                v_mem_req_valid = 0;
            }
            if ((v_core_req_ready == 0) || (v_mem_req_valid == 0)) {
                v.state = Idle;
            } else {
                v.last_va = i_core_req_addr;
                v.req_type = i_core_req_type;
                v.req_wdata = i_core_req_wdata;
                v.req_wstrb = i_core_req_wstrb;
                v.req_size = i_core_req_size;
                v.req_x = v_core_req_x;
                v.req_r = v_core_req_r;
                v.req_w = v_core_req_w;
                if ((v_core_req_x && last_page_fault_x) || (v_core_req_r && last_page_fault_r) || (v_core_req_w && last_page_fault_w)) {
                    // New request to the same page has not permission
                    v.ex_page_fault = 1;
                    v.state = AcceptCore;
                }
            }
        }
        break;
    case CheckTlb:
        if (r.last_va.read()(63, 12) == wb_tlb_rdata.read()(115, 64).to_uint64()) {
            // TLB hit
            v.tlb_hit = 1;
            v.last_pa = wb_tlb_rdata.read()(63, 12).to_uint64();
            v.last_permission = wb_tlb_rdata.read()(7, 0).to_uint64();
            v.req_pa = t_req_pa;
        } else {
            // TLB miss
            v.tlb_level = 0x1;                              // Start page decoding
            v.req_pa = (vb_pte_start_va, vb_level0_off);
        }
        v.state = CacheReq;
        break;
    case CacheReq:
        v_mem_req_valid = 1;
        vb_mem_req_addr = r.req_pa;
        vb_mem_req_type = r.req_type;
        vb_mem_req_wdata = r.req_wdata;
        vb_mem_req_wstrb = r.req_wstrb;
        vb_mem_req_size = r.req_size;
        if (i_mem_req_ready.read() == 1) {
            v.state = WaitResp;
        }
        break;
    case WaitResp:
        v_mem_resp_ready = 1;
        if (i_mem_resp_valid.read() == 1) {
            v.resp_addr = i_mem_resp_addr;
            v.resp_data = i_mem_resp_data;
            v.resp_executable = i_mem_resp_executable;      // MPU executable flag
            v.resp_load_fault = i_mem_resp_load_fault;      // Hardware error Load (unmapped access)
            v.resp_store_fault = i_mem_resp_store_fault;    // Hardware error Store/AMO (unmapped access)
            if ((r.tlb_hit || i_mem_resp_load_fault || i_mem_resp_store_fault) == 1) {
                v.state = AcceptCore;
            } else {
                v.state = HandleResp;
            }
        }
        break;
    case HandleResp:
        if ((r.resp_data.read()[PTE_V] == 0) || (((!r.resp_data.read()[PTE_R]) && r.resp_data.read()[PTE_W]) == 1)) {// v=0 or (r=0 && w=1)
            // PTE is invalid
            v.ex_page_fault = 1;
            v.state = AcceptCore;
        } else if ((r.resp_data.read()[PTE_R] || r.resp_data.read()[PTE_W] || r.resp_data.read()[PTE_X]) == 0) {
            // PTE is a apointer to the next level
            v.state = CacheReq;
            v.tlb_level = (r.tlb_level.read() << 1);
            if (r.tlb_level.read()[0] == 1) {
                v.req_pa = (vb_pte_base_va, vb_level1_off);
            } else if (r.tlb_level.read()[1] == 1) {
                v.req_pa = (vb_pte_base_va, vb_level2_off);
            } else if (r.tlb_level.read()[2] == 1) {
                v.req_pa = (vb_pte_base_va, vb_level3_off);
            } else {
                // It was the last level
                v.ex_page_fault = 1;
                v.state = AcceptCore;
            }
        } else {
            // PTE is a leaf
            if (r.resp_data.read()[PTE_A] == 0) {
                v.state = AcceptCore;
                v.ex_page_fault = 1;
            } else if ((r.req_x.read() == 1) && (r.resp_data.read()[PTE_X] == 0)) {
                v.state = AcceptCore;
                v.ex_page_fault = 1;
            } else if ((r.req_r.read() == 1) && (r.resp_data.read()[PTE_R] == 0)) {
                v.state = AcceptCore;
                v.ex_page_fault = 1;
            } else if ((r.req_w.read() == 1) && ((r.resp_data.read()[PTE_W] == 0) || (r.resp_data.read()[PTE_D] == 0))) {
                v.state = AcceptCore;
                v.ex_page_fault = 1;
            } else {
                v.state = UpdateTlb;
            }
            v.last_pa = vb_pte_base_va;
            v.last_permission = r.resp_data.read()(7, 0);
            v.req_pa = (vb_pte_base_va, r.last_va.read()(11, 0));
            v.tlb_wdata = t_tlb_wdata;
        }
        break;
    case UpdateTlb:
        // Translation is finished: write va/pa into TLB memory
        v_tlb_wena = 1;
        vb_tlb_adr = r.last_va.read()(((12 + CFG_MMU_TLB_AWIDTH) - 1), 12);
        v.state = CacheReq;                                 // Read data by physical address
        v.tlb_hit = 1;
        break;
    case AcceptCore:
        v_core_resp_valid = 1;
        vb_core_resp_addr = r.last_va;
        vb_core_resp_data = r.resp_data;
        v_core_resp_executable = r.resp_executable;
        v_core_resp_load_fault = r.resp_load_fault;
        v_core_resp_store_fault = r.resp_store_fault;
        if (i_core_resp_ready.read() == 1) {
            v.state = Idle;
            if (r.ex_page_fault.read() == 1) {
                v.last_va = ~0ull;
                v.last_pa = ~0ull;
            }
        }
        break;
    case FlushTlb:
        v_tlb_wena = 1;
        vb_tlb_adr = r.tlb_flush_adr;
        v.last_va = ~0ull;
        v.last_pa = ~0ull;
        if (r.tlb_flush_cnt.read().or_reduce() == 0) {
            v.state = Idle;
        } else {
            v.tlb_flush_cnt = (r.tlb_flush_cnt.read() - 1);
            v.tlb_flush_adr = (r.tlb_flush_adr.read() + 1);
        }
        break;
    default:
        break;
    }

    if (i_fence.read() == 1) {
        // Clear pipeline stage
        if (i_fence_addr.read().or_reduce() == 0) {
            v.tlb_flush_cnt = ~0ull;
        } else {
            v.tlb_flush_cnt = 1;
        }
        v.tlb_flush_adr = i_fence_addr;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        Mmu_r_reset(v);
    }

    w_tlb_wena = v_tlb_wena;
    wb_tlb_adr = vb_tlb_adr;
    wb_tlb_wdata = r.tlb_wdata;
    o_core_req_ready = v_core_req_ready;
    o_core_resp_valid = v_core_resp_valid;
    o_core_resp_addr = vb_core_resp_addr;
    o_core_resp_data = vb_core_resp_data;
    o_core_resp_executable = v_core_resp_executable;
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
}

void Mmu::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        Mmu_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

