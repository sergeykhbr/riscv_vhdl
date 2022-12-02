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
    i_mem_resp_load_fault("i_mem_resp_load_fault"),
    i_mem_resp_store_fault("i_mem_resp_store_fault"),
    o_mem_resp_ready("o_mem_resp_ready"),
    i_mmu_ena("i_mmu_ena"),
    i_mmu_sv39("i_mmu_sv39"),
    i_mmu_sv48("i_mmu_sv48"),
    i_mmu_ppn("i_mmu_ppn"),
    i_mprv("i_mprv"),
    i_mxr("i_mxr"),
    i_sum("i_sum"),
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
    sensitive << i_mem_resp_load_fault;
    sensitive << i_mem_resp_store_fault;
    sensitive << i_mmu_ena;
    sensitive << i_mmu_sv39;
    sensitive << i_mmu_sv48;
    sensitive << i_mmu_ppn;
    sensitive << i_mprv;
    sensitive << i_mxr;
    sensitive << i_sum;
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
    sensitive << r.req_flush;
    sensitive << r.last_mmu_ena;
    sensitive << r.last_va;
    sensitive << r.last_pa;
    sensitive << r.last_permission;
    sensitive << r.last_page_size;
    sensitive << r.resp_addr;
    sensitive << r.resp_data;
    sensitive << r.resp_load_fault;
    sensitive << r.resp_store_fault;
    sensitive << r.ex_page_fault;
    sensitive << r.tlb_hit;
    sensitive << r.tlb_level;
    sensitive << r.tlb_page_size;
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
        sc_trace(o_vcd, i_mem_resp_load_fault, i_mem_resp_load_fault.name());
        sc_trace(o_vcd, i_mem_resp_store_fault, i_mem_resp_store_fault.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());
        sc_trace(o_vcd, i_mmu_ena, i_mmu_ena.name());
        sc_trace(o_vcd, i_mmu_sv39, i_mmu_sv39.name());
        sc_trace(o_vcd, i_mmu_sv48, i_mmu_sv48.name());
        sc_trace(o_vcd, i_mmu_ppn, i_mmu_ppn.name());
        sc_trace(o_vcd, i_mprv, i_mprv.name());
        sc_trace(o_vcd, i_mxr, i_mxr.name());
        sc_trace(o_vcd, i_sum, i_sum.name());
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
        sc_trace(o_vcd, r.req_flush, pn + ".r_req_flush");
        sc_trace(o_vcd, r.last_mmu_ena, pn + ".r_last_mmu_ena");
        sc_trace(o_vcd, r.last_va, pn + ".r_last_va");
        sc_trace(o_vcd, r.last_pa, pn + ".r_last_pa");
        sc_trace(o_vcd, r.last_permission, pn + ".r_last_permission");
        sc_trace(o_vcd, r.last_page_size, pn + ".r_last_page_size");
        sc_trace(o_vcd, r.resp_addr, pn + ".r_resp_addr");
        sc_trace(o_vcd, r.resp_data, pn + ".r_resp_data");
        sc_trace(o_vcd, r.resp_load_fault, pn + ".r_resp_load_fault");
        sc_trace(o_vcd, r.resp_store_fault, pn + ".r_resp_store_fault");
        sc_trace(o_vcd, r.ex_page_fault, pn + ".r_ex_page_fault");
        sc_trace(o_vcd, r.tlb_hit, pn + ".r_tlb_hit");
        sc_trace(o_vcd, r.tlb_level, pn + ".r_tlb_level");
        sc_trace(o_vcd, r.tlb_page_size, pn + ".r_tlb_page_size");
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
    sc_uint<RISCV_ARCH> vb_core_resp_addr;
    sc_uint<64> vb_core_resp_data;
    bool v_core_resp_load_fault;
    bool v_core_resp_store_fault;
    sc_uint<RISCV_ARCH> vb_last_pa_req;
    bool v_mem_req_valid;
    sc_uint<RISCV_ARCH> vb_mem_req_addr;
    sc_uint<MemopType_Total> vb_mem_req_type;
    sc_uint<64> vb_mem_req_wdata;
    sc_uint<8> vb_mem_req_wstrb;
    sc_uint<2> vb_mem_req_size;
    bool v_mem_resp_ready;
    bool v_tlb_wena;
    sc_uint<CFG_MMU_TLB_AWIDTH> vb_tlb_adr;
    sc_uint<(RISCV_ARCH - 12)> vb_pte_start_va;
    sc_uint<(RISCV_ARCH - 12)> vb_resp_ppn;
    bool v_va_ena;
    sc_uint<12> vb_level0_off;
    sc_uint<12> vb_level1_off;
    sc_uint<12> vb_level2_off;
    sc_uint<12> vb_level3_off;
    bool v_last_valid;
    bool v_tlb_hit;
    sc_uint<RISCV_ARCH> vb_tlb_pa0;                         // 4 KB page phys address
    sc_uint<RISCV_ARCH> vb_tlb_pa1;                         // 8 MB page phys address
    sc_uint<RISCV_ARCH> vb_tlb_pa2;                         // 16 GB page phys address
    sc_uint<RISCV_ARCH> vb_tlb_pa3;                         // 32 TB page phys address
    sc_uint<RISCV_ARCH> vb_tlb_pa_hit;
    sc_biguint<CFG_MMU_PTE_DWIDTH> t_tlb_wdata;
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
    vb_tlb_pa0 = 0ull;
    vb_tlb_pa1 = 0ull;
    vb_tlb_pa2 = 0ull;
    vb_tlb_pa3 = 0ull;
    vb_tlb_pa_hit = 0;
    t_tlb_wdata = 0;
    t_idx_lsb = 0;

    v = r;

    t_idx_lsb = (12 + (9 * r.last_page_size.read().to_int()));
    vb_tlb_adr = i_core_req_addr.read()(t_idx_lsb + CFG_MMU_TLB_AWIDTH - 1, t_idx_lsb);

    if (i_core_req_fetch.read() == 1) {
        v_core_req_x = 1;
    } else if ((i_core_req_type.read().or_reduce() == 0)
                || (i_core_req_type.read() == MemopType_Reserve)) {
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
    vb_resp_ppn(43, 0) = r.resp_data.read()(53, 10);
    v_va_ena = i_mmu_ena;
    // M-mode can opearate with physical and virtual addresses when MPRV=1
    if ((i_mprv.read() == 1) && (i_core_req_addr.read()(63, 48).and_reduce() == 0)) {
        // Use physical address
        v_va_ena = 0;
    }
    vb_level0_off = (r.last_va.read()(47, 39) << 3);
    vb_level1_off = (r.last_va.read()(38, 30) << 3);
    vb_level2_off = (r.last_va.read()(29, 21) << 3);
    vb_level3_off = (r.last_va.read()(20, 12) << 3);

    // Pages: 4 KB, 8MB, 16 GB and 32 TB for sv48 only
    // Check the last hit depending page size:
    v_last_valid = 0;
    if ((r.last_page_size.read() == 0)
            && (i_core_req_addr.read()(63, 12) == r.last_va.read()(63, 12))) {
        v_last_valid = 1;
        vb_last_pa_req = (r.last_pa.read(), i_core_req_addr.read()(11, 0));
    } else if ((r.last_page_size.read() == 1)
                && (i_core_req_addr.read()(63, 21) == r.last_va.read()(63, 21))) {
        v_last_valid = 1;
        vb_last_pa_req = (r.last_pa.read()(51, 9), i_core_req_addr.read()(20, 0));
    } else if ((r.last_page_size.read() == 2)
                && (i_core_req_addr.read()(63, 30) == r.last_va.read()(63, 30))) {
        v_last_valid = 1;
        vb_last_pa_req = (r.last_pa.read()(51, 18), i_core_req_addr.read()(29, 0));
    } else if ((r.last_page_size.read() == 3)
                && (i_core_req_addr.read()(63, 39) == r.last_va.read()(63, 39))) {
        v_last_valid = 1;
        vb_last_pa_req = (r.last_pa.read()(51, 27), i_core_req_addr.read()(38, 0));
    }

    // Check table hit depending page size:
    vb_tlb_pa0(63, 12) = wb_tlb_rdata.read()(63, 12).to_uint64();
    vb_tlb_pa0(11, 0) = r.last_va.read()(11, 0);
    vb_tlb_pa1(63, 21) = wb_tlb_rdata.read()(63, 21).to_uint64();
    vb_tlb_pa1(20, 0) = r.last_va.read()(20, 0);
    vb_tlb_pa2(63, 30) = wb_tlb_rdata.read()(63, 30).to_uint64();
    vb_tlb_pa2(29, 0) = r.last_va.read()(29, 0);
    vb_tlb_pa3(63, 39) = wb_tlb_rdata.read()(63, 39).to_uint64();
    vb_tlb_pa3(38, 0) = r.last_va.read()(38, 0);

    if (wb_tlb_rdata.read()[PTE_V] == 1) {
        if (wb_tlb_rdata.read()(9, 8).to_uint64() == 3) {
            // 32 TB pages:
            v_tlb_hit = (!(r.last_va.read()(63, 39) ^ wb_tlb_rdata.read()(115, 91).to_uint64()));
            vb_tlb_pa_hit = vb_tlb_pa3;
        } else if (wb_tlb_rdata.read()(9, 8).to_uint64() == 2) {
            // 16 GB pages:
            v_tlb_hit = (!(r.last_va.read()(63, 30) ^ wb_tlb_rdata.read()(115, 82).to_uint64()));
            vb_tlb_pa_hit = vb_tlb_pa2;
        } else if (wb_tlb_rdata.read()(9, 8).to_uint64() == 1) {
            // 8 MB pages:
            v_tlb_hit = (!(r.last_va.read()(63, 21) ^ wb_tlb_rdata.read()(115, 73).to_uint64()));
            vb_tlb_pa_hit = vb_tlb_pa1;
        } else {
            // 4 KB pages:
            v_tlb_hit = (!(r.last_va.read()(63, 12) ^ wb_tlb_rdata.read()(115, 64).to_uint64()));
            vb_tlb_pa_hit = vb_tlb_pa0;
        }
    }

    t_tlb_wdata(115, 64) = r.last_va.read()(63, 12);
    t_tlb_wdata(63, 12) = vb_resp_ppn;
    t_tlb_wdata(9, 8) = r.tlb_page_size;
    t_tlb_wdata(7, 0) = r.resp_data.read()(7, 0);

    switch (r.state.read()) {
    case Idle:
        v.tlb_hit = 0;
        v.resp_load_fault = 0;
        v.resp_store_fault = 0;
        v.ex_page_fault = 0;
        if (i_core_req_valid.read() == 1) {
            v.last_mmu_ena = (i_mmu_ena && v_va_ena);
            v.last_va = i_core_req_addr;
            v.req_type = i_core_req_type;
            v.req_wdata = i_core_req_wdata;
            v.req_wstrb = i_core_req_wstrb;
            v.req_size = i_core_req_size;
        }
        if (r.req_flush.read() == 1) {
            v.req_flush = 0;
            v.tlb_wdata = 0;
            v.state = FlushTlb;
        } else if ((i_mmu_ena.read() == 0) || (v_va_ena == 0)) {// MMU disabled
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
            if ((i_core_req_valid && i_mem_req_ready) == 1) {
                v.state = WaitRespNoMmu;
            }
        } else if ((r.last_mmu_ena.read() == 1) && (v_last_valid == 1)) {// MMU enabled: Check the request to the same page:
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
        v_core_resp_load_fault = i_mem_resp_load_fault;
        v_core_resp_store_fault = i_mem_resp_store_fault;
        v_mem_req_valid = i_core_req_valid;
        vb_mem_req_addr = vb_last_pa_req;
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
        if (v_tlb_hit == 1) {
            // TLB hit
            v.tlb_hit = 1;
            v.last_pa = wb_tlb_rdata.read()(63, 12).to_uint64();
            v.last_permission = wb_tlb_rdata.read()(7, 0).to_uint64();
            v.req_pa = vb_tlb_pa_hit;
        } else {
            // TLB miss
            if (i_mmu_sv39.read() == 1) {
                v.tlb_level = 0x2;                          // Start page decoding sv39
                v.tlb_page_size = 2;
                v.req_pa = (vb_pte_start_va, vb_level1_off);
            } else {
                v.tlb_level = 0x1;                          // Start page decoding sv48
                v.tlb_page_size = 3;
                v.req_pa = (vb_pte_start_va, vb_level0_off);
            }
        }
        v.state = CacheReq;
        break;
    case CacheReq:
        v_mem_req_valid = 1;
        vb_mem_req_addr = r.req_pa;
        if (r.tlb_hit.read() == 0) {
            vb_mem_req_type = 0;                            // Load tlb item
        } else {
            vb_mem_req_type = r.req_type;
        }
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
            v.tlb_page_size = (r.tlb_page_size.read() - 1);
            if (r.tlb_level.read()[0] == 1) {
                v.req_pa = (vb_resp_ppn, vb_level1_off);
            } else if (r.tlb_level.read()[1] == 1) {
                v.req_pa = (vb_resp_ppn, vb_level2_off);
            } else if (r.tlb_level.read()[2] == 1) {
                v.req_pa = (vb_resp_ppn, vb_level3_off);
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
            v.last_permission = r.resp_data.read()(7, 0);
            v.last_page_size = r.tlb_page_size;
            v.tlb_wdata = t_tlb_wdata;
            // Pages more than 4KB support:
            if (r.tlb_level.read()[0] == 1) {
                v.req_pa = (vb_resp_ppn(51, 27), r.last_va.read()(38, 0));
                v.last_pa = (vb_resp_ppn(51, 27) << 27);
            } else if (r.tlb_level.read()[1] == 1) {
                v.req_pa = (vb_resp_ppn(51, 18), r.last_va.read()(29, 0));
                v.last_pa = (vb_resp_ppn(51, 18) << 18);
            } else if (r.tlb_level.read()[2] == 1) {
                v.req_pa = (vb_resp_ppn(51, 9), r.last_va.read()(20, 0));
                v.last_pa = (vb_resp_ppn(51, 9) << 9);
            } else {
                v.req_pa = (vb_resp_ppn, r.last_va.read()(11, 0));
                v.last_pa = vb_resp_ppn;
            }
        }
        break;
    case UpdateTlb:
        // Translation is finished: write va/pa into TLB memory
        v_tlb_wena = 1;
        vb_tlb_adr = r.last_va.read()(t_idx_lsb + CFG_MMU_TLB_AWIDTH - 1, t_idx_lsb);
        v.state = CacheReq;                                 // Read data by physical address
        v.tlb_hit = 1;
        break;
    case AcceptCore:
        v_core_resp_valid = 1;
        vb_core_resp_addr = r.last_va;
        vb_core_resp_data = r.resp_data;
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
        v.last_mmu_ena = 0;
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
        // Clear whole table ignoring i_fence_addr
        v.req_flush = 1;
        v.tlb_flush_cnt = ~0ull;
        v.tlb_flush_adr = 0;
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

