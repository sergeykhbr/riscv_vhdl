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
    o_fetch_req_ready("o_fetch_req_ready"),
    i_fetch_addr_valid("i_fetch_addr_valid"),
    i_fetch_addr("i_fetch_addr"),
    o_fetch_data_valid("o_fetch_data_valid"),
    o_fetch_data_addr("o_fetch_data_addr"),
    o_fetch_data("o_fetch_data"),
    o_fetch_load_fault("o_fetch_load_fault"),
    o_fetch_executable("o_fetch_executable"),
    o_fetch_page_fault("o_fetch_page_fault"),
    i_fetch_resp_ready("i_fetch_resp_ready"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_addr_valid("o_mem_addr_valid"),
    o_mem_addr("o_mem_addr"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data_addr("i_mem_data_addr"),
    i_mem_data("i_mem_data"),
    i_mem_load_fault("i_mem_load_fault"),
    i_mem_executable("i_mem_executable"),
    o_mem_resp_ready("o_mem_resp_ready"),
    i_mmu_ena("i_mmu_ena"),
    i_mmu_ppn("i_mmu_ppn"),
    i_flush_pipeline("i_flush_pipeline") {

    async_reset_ = async_reset;

    tlb = new MmuTlb("tlb");
    tlb->i_clk(i_clk);
    tlb->i_adr(wb_tlb_adr);
    tlb->i_wena(w_tlb_wena);
    tlb->i_wdata(wb_tlb_wdata);
    tlb->o_rdata(wb_tlb_rdata);


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_fetch_addr_valid;
    sensitive << i_fetch_addr;
    sensitive << i_fetch_resp_ready;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data;
    sensitive << i_mem_load_fault;
    sensitive << i_mem_executable;
    sensitive << i_mmu_ena;
    sensitive << i_mmu_ppn;
    sensitive << i_flush_pipeline;
    sensitive << wb_tlb_adr;
    sensitive << w_tlb_wena;
    sensitive << wb_tlb_wdata;
    sensitive << wb_tlb_rdata;
    sensitive << r.state;
    sensitive << r.req_x;
    sensitive << r.req_r;
    sensitive << r.req_w;
    sensitive << r.req_pa;
    sensitive << r.last_va;
    sensitive << r.last_pa;
    sensitive << r.resp_addr;
    sensitive << r.resp_data;
    sensitive << r.pte_permission;
    sensitive << r.ex_load_fault;
    sensitive << r.ex_mpu_executable;
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
    delete tlb;
}

void Mmu::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, o_fetch_req_ready, o_fetch_req_ready.name());
        sc_trace(o_vcd, i_fetch_addr_valid, i_fetch_addr_valid.name());
        sc_trace(o_vcd, i_fetch_addr, i_fetch_addr.name());
        sc_trace(o_vcd, o_fetch_data_valid, o_fetch_data_valid.name());
        sc_trace(o_vcd, o_fetch_data_addr, o_fetch_data_addr.name());
        sc_trace(o_vcd, o_fetch_data, o_fetch_data.name());
        sc_trace(o_vcd, o_fetch_load_fault, o_fetch_load_fault.name());
        sc_trace(o_vcd, o_fetch_executable, o_fetch_executable.name());
        sc_trace(o_vcd, o_fetch_page_fault, o_fetch_page_fault.name());
        sc_trace(o_vcd, i_fetch_resp_ready, i_fetch_resp_ready.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_mem_addr_valid, o_mem_addr_valid.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data_addr, i_mem_data_addr.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, i_mem_load_fault, i_mem_load_fault.name());
        sc_trace(o_vcd, i_mem_executable, i_mem_executable.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());
        sc_trace(o_vcd, i_mmu_ena, i_mmu_ena.name());
        sc_trace(o_vcd, i_mmu_ppn, i_mmu_ppn.name());
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_x, pn + ".r_req_x");
        sc_trace(o_vcd, r.req_r, pn + ".r_req_r");
        sc_trace(o_vcd, r.req_w, pn + ".r_req_w");
        sc_trace(o_vcd, r.req_pa, pn + ".r_req_pa");
        sc_trace(o_vcd, r.last_va, pn + ".r_last_va");
        sc_trace(o_vcd, r.last_pa, pn + ".r_last_pa");
        sc_trace(o_vcd, r.resp_addr, pn + ".r_resp_addr");
        sc_trace(o_vcd, r.resp_data, pn + ".r_resp_data");
        sc_trace(o_vcd, r.pte_permission, pn + ".r_pte_permission");
        sc_trace(o_vcd, r.ex_load_fault, pn + ".r_ex_load_fault");
        sc_trace(o_vcd, r.ex_mpu_executable, pn + ".r_ex_mpu_executable");
        sc_trace(o_vcd, r.ex_page_fault, pn + ".r_ex_page_fault");
        sc_trace(o_vcd, r.tlb_hit, pn + ".r_tlb_hit");
        sc_trace(o_vcd, r.tlb_level, pn + ".r_tlb_level");
        sc_trace(o_vcd, r.tlb_wdata, pn + ".r_tlb_wdata");
        sc_trace(o_vcd, r.tlb_flush_cnt, pn + ".r_tlb_flush_cnt");
        sc_trace(o_vcd, r.tlb_flush_adr, pn + ".r_tlb_flush_adr");
    }

    tlb->generateVCD(i_vcd, o_vcd);
}

void Mmu::comb() {
    bool v_fetch_req_ready;
    bool v_fetch_data_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_fetch_data_addr;
    sc_uint<64> vb_fetch_data;
    bool v_fetch_load_fault;
    bool v_fetch_executable;
    bool v_mem_addr_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_mem_addr;
    bool v_mem_resp_ready;
    bool v_mpu_fault;
    bool v_tlb_wena;
    sc_uint<CFG_MMU_TLB_AWIDTH> vb_tlb_adr;
    sc_uint<(CFG_CPU_ADDR_BITS - 12)> vb_pte_start_va;
    sc_uint<(CFG_CPU_ADDR_BITS - 12)> vb_pte_base_va;
    sc_uint<12> vb_level0_off;
    sc_uint<12> vb_level1_off;
    sc_uint<12> vb_level2_off;
    sc_uint<12> vb_level3_off;
    sc_uint<CFG_CPU_ADDR_BITS> t_req_pa;
    sc_biguint<CFG_MMU_PTE_DWIDTH> t_tlb_wdata;

    v = r;

    v_fetch_req_ready = 0;
    v_fetch_data_valid = 0;
    vb_fetch_data_addr = 0;
    vb_fetch_data = 0;
    v_fetch_load_fault = 0;
    v_fetch_executable = 0;
    v_mem_addr_valid = 0;
    vb_mem_addr = 0;
    v_mem_resp_ready = 0;
    vb_pte_start_va = 0;
    vb_pte_base_va = 0;
    v_tlb_wena = 0;
    vb_tlb_adr = i_fetch_addr.read()(((12 + CFG_MMU_TLB_AWIDTH) - 1), 12);
    v_mpu_fault = (i_mem_load_fault || (!i_mem_executable));
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

    // Temporary variables are neccessary in systemc
    t_req_pa((CFG_CPU_ADDR_BITS - 1), 12) = wb_tlb_rdata.read()((CFG_CPU_ADDR_BITS - 1), 12).to_uint64();
    t_req_pa(11, 0) = r.last_va.read()(11, 0);

    t_tlb_wdata(115, 64) = r.last_va.read()(63, 12);
    t_tlb_wdata(63, 12) = vb_pte_base_va;
    t_tlb_wdata(7, 0) = r.resp_data.read()(7, 0);

    switch (r.state.read()) {
    case Idle:
        v.tlb_hit = 0;
        v.ex_page_fault = 0;
        if (i_fetch_addr_valid.read() == 1) {
            v.last_va = i_fetch_addr;
        }
        if (i_mmu_ena.read() == 0) {
            // Direct connection to Cache
            v_fetch_req_ready = i_mem_req_ready;
            v_fetch_data_valid = i_mem_data_valid;
            vb_fetch_data_addr = i_mem_data_addr;
            vb_fetch_data = i_mem_data;
            v_fetch_load_fault = i_mem_load_fault;
            v_fetch_executable = i_mem_executable;
            v_mem_addr_valid = i_fetch_addr_valid;
            vb_mem_addr = i_fetch_addr;
            v_mem_resp_ready = i_fetch_resp_ready;
        } else if (r.tlb_flush_cnt.read().or_reduce() == 1) {
            v.state = FlushTlb;
            v.tlb_wdata = 0;
        } else {
            // MMU enabled: check TLB
            v_fetch_req_ready = 1;
            if (i_fetch_addr_valid.read() == 1) {
                v.state = CheckTlb;
                v.req_x = 1;
            }
        }
        break;
    case CheckTlb:
        if (r.last_va.read()(63, 12) == wb_tlb_rdata.read()(115, 64).to_uint64()) {
            // TLB hit
            v.tlb_hit = 1;
            v.last_pa = wb_tlb_rdata.read()(63, 12).to_uint64();
            v.req_pa = t_req_pa;
            v.pte_permission = wb_tlb_rdata.read()(7, 0).to_uint64();
        } else {
            // TLB miss
            v.tlb_level = 0x1;                             // Start page decoding
            v.req_pa = (vb_pte_start_va, vb_level0_off);
        }
        v.state = CacheReq;
        break;
    case CacheReq:
        v_mem_addr_valid = 1;
        vb_mem_addr = r.req_pa;
        if (i_mem_req_ready.read() == 1) {
            v.state = WaitResp;
        }
        break;
    case WaitResp:
        v_mem_resp_ready = 1;
        if (i_mem_data_valid.read() == 1) {
            v.resp_addr = i_mem_data_addr;
            v.resp_data = i_mem_data;
            v.ex_load_fault = i_mem_load_fault;            // Hardware error (unmapped access)
            v.ex_mpu_executable = i_mem_executable;        // MPU executable flag
            if ((r.tlb_hit || v_mpu_fault) == 1) {
                v.state = AcceptFetch;
            } else {
                v.state = HandleResp;
            }
        }
        break;
    case HandleResp:
        if ((r.resp_data.read()[PTE_V] == 0) || (((!r.resp_data.read()[PTE_R]) && r.resp_data.read()[PTE_W]) == 1)) {
            // PTE is invalid
            v.ex_page_fault = 1;
            v.state = AcceptFetch;
        } else if ((r.resp_data.read()[PTE_R] || r.resp_data.read()[PTE_X]) == 0) {
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
                v.state = AcceptFetch;
            }
        } else {
            // PTE is a leaf
            if ((r.req_x.read() == 1) && (r.resp_data.read()[PTE_X] == 0)) {
                v.state = AcceptFetch;
                v.ex_page_fault = 1;
            } else if ((r.resp_data.read()[PTE_A] == 0) || (r.req_w && (!r.resp_data.read()[PTE_D]))) {
                // Implement option 1: raise a page-fault instead of (2) memory update with the new A,D-bits
                v.state = AcceptFetch;
                v.ex_page_fault = 1;
            } else {
                v.state = UpdateTlb;
            }
            v.last_pa = vb_pte_base_va;
            v.req_pa = (vb_pte_base_va, r.last_va.read()(11, 0));
            v.tlb_wdata = t_tlb_wdata;
        }
        break;
    case UpdateTlb:
        // Translation is finished: write va/pa into TLB memory
        v_tlb_wena = 1;
        vb_tlb_adr = r.last_va.read()(((12 + CFG_MMU_TLB_AWIDTH) - 1), 12);
        v.state = CacheReq;                                // Read data by physical address
        v.tlb_hit = 1;
        break;
    case AcceptFetch:
        v_fetch_data_valid = 1;
        vb_fetch_data_addr = r.last_va;
        vb_fetch_data = r.resp_data;
        v_fetch_load_fault = r.ex_load_fault;
        v_fetch_executable = r.ex_mpu_executable;
        if (i_fetch_resp_ready.read() == 1) {
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

    if (i_flush_pipeline.read() == 1) {
        // Clear pipeline stage
        v.tlb_flush_cnt = ~0ull;
        v.tlb_flush_adr = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        Mmu_r_reset(v);
    }

    w_tlb_wena = v_tlb_wena;
    wb_tlb_adr = vb_tlb_adr;
    wb_tlb_wdata = r.tlb_wdata;
    o_fetch_req_ready = v_fetch_req_ready;
    o_fetch_data_valid = v_fetch_data_valid;
    o_fetch_data_addr = vb_fetch_data_addr;
    o_fetch_data = vb_fetch_data;
    o_fetch_load_fault = v_fetch_load_fault;
    o_fetch_executable = v_fetch_executable;
    o_fetch_page_fault = r.ex_page_fault;
    o_mem_addr_valid = v_mem_addr_valid;
    o_mem_addr = vb_mem_addr;
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

