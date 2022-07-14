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
    i_prv("i_prv"),
    i_satp("i_satp"),
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
    sensitive << i_prv;
    sensitive << i_satp;
    sensitive << i_flush_pipeline;
    sensitive << wb_tlb_adr;
    sensitive << w_tlb_wena;
    sensitive << wb_tlb_wdata;
    sensitive << wb_tlb_rdata;
    sensitive << r.state;
    sensitive << r.req_va;
    sensitive << r.last_va;
    sensitive << r.last_pa;
    sensitive << r.req_valid;
    sensitive << r.resp_ready;
    sensitive << r.req_addr;
    sensitive << r.mem_resp_shadow;
    sensitive << r.pc;
    sensitive << r.instr;
    sensitive << r.instr_load_fault;
    sensitive << r.instr_executable;

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
        sc_trace(o_vcd, i_prv, i_prv.name());
        sc_trace(o_vcd, i_satp, i_satp.name());
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_va, pn + ".r_req_va");
        sc_trace(o_vcd, r.last_va, pn + ".r_last_va");
        sc_trace(o_vcd, r.last_pa, pn + ".r_last_pa");
        sc_trace(o_vcd, r.req_valid, pn + ".r_req_valid");
        sc_trace(o_vcd, r.resp_ready, pn + ".r_resp_ready");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.mem_resp_shadow, pn + ".r_mem_resp_shadow");
        sc_trace(o_vcd, r.pc, pn + ".r_pc");
        sc_trace(o_vcd, r.instr, pn + ".r_instr");
        sc_trace(o_vcd, r.instr_load_fault, pn + ".r_instr_load_fault");
        sc_trace(o_vcd, r.instr_executable, pn + ".r_instr_executable");
    }

    tlb->generateVCD(i_vcd, o_vcd);
}

void Mmu::comb() {
    bool v_sv39;
    bool v_sv48;
    bool v_mmu_ena;
    bool v_fetch_req_ready;
    bool v_fetch_data_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_fetch_data_addr;
    sc_uint<64> vb_fetch_data;
    bool v_fetch_load_fault;
    bool v_fetch_executable;
    bool v_mem_addr_valid;
    sc_uint<CFG_CPU_ADDR_BITS> vb_mem_addr;
    bool v_mem_resp_ready;

    v = r;

    v_sv39 = 0;
    v_sv48 = 0;
    v_fetch_req_ready = 0;
    v_fetch_data_valid = 0;
    vb_fetch_data_addr = 0;
    vb_fetch_data = 0;
    v_fetch_load_fault = 0;
    v_fetch_executable = 0;
    v_mem_addr_valid = 0;
    vb_mem_addr = 0;
    v_mem_resp_ready = 0;

    // S- or U- mode plus Sv paging enabled in SATP
    if (i_satp.read()(63, 60) == 8) {
        v_sv39 = 1;
    } else if (i_satp.read()(63, 60) == 9) {
        v_sv48 = 1;
    }
    v_mmu_ena = ((!i_prv.read()[1]) && (v_sv39 || v_sv48));

    if (v_mmu_ena == 0) {
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
    } else if (i_fetch_addr.read() == r.last_va.read()) {
        // Direct connection to cache with the fast changing va to last_pa
        v_fetch_req_ready = i_mem_req_ready;
        v_fetch_data_valid = i_mem_data_valid;
        vb_fetch_data_addr = i_mem_data_addr;
        vb_fetch_data = i_mem_data;
        v_fetch_load_fault = i_mem_load_fault;
        v_fetch_executable = i_mem_executable;
        v_mem_addr_valid = i_fetch_addr_valid;
        vb_mem_addr = r.last_pa;
        v_mem_resp_ready = i_fetch_resp_ready;
    } else {
        // MMU enabled, check TLB
        switch (r.state.read()) {
        case Idle:
            v_fetch_req_ready = 1;
            if (i_fetch_addr_valid.read() == 1) {
                v.state = CheckTlb;
                v.req_va = i_fetch_addr;
            }
            v.req_valid = 0;
            v.resp_ready = 0;
            v.state = WaitReqAccept;
            v.req_addr = i_fetch_addr;
            v.req_valid = 1;
            break;
        case CheckTlb:
            if (r.req_va.read() == wb_tlb_wdata.read()(127, 64).to_uint64()) {
            }
            break;
        case WaitReqAccept:
            if (i_mem_req_ready) {
                v.mem_resp_shadow = r.req_addr;
                v.resp_ready = 1;
                v.state = WaitResp;
            }
            break;
        case WaitResp:
            if (i_mem_data_valid.read() == 1) {
                v.pc = i_mem_data_addr;
                v.instr = i_mem_data;
                v.instr_load_fault = i_mem_load_fault;
                v.instr_executable = i_mem_executable;

                if (r.req_valid.read() == 1) {
                    if (i_mem_req_ready.read() == 1) {
                        v.mem_resp_shadow = r.req_addr;
                    } else {
                        v.state = WaitReqAccept;
                    }
                } else {
                    v.req_addr = ~0ull;
                    v.state = Idle;
                }
            }
            break;
        default:
            break;
        }
    }

    if (i_flush_pipeline.read() == 1) {
        // Clear pipeline stage
        v.req_valid = 0;
        v.pc = ~0ull;
        v.instr = 0;
        v.instr_load_fault = 0;
        v.instr_executable = 0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        Mmu_r_reset(v);
    }

    wb_tlb_adr = i_fetch_addr.read()(((12 + CFG_MMU_TLB_AWIDTH) - 1), 12);
    o_fetch_req_ready = v_fetch_req_ready;
    o_fetch_data_valid = v_fetch_data_valid;
    o_fetch_data_addr = vb_fetch_data_addr;
    o_fetch_data = vb_fetch_data;
    o_fetch_load_fault = v_fetch_load_fault;
    o_fetch_executable = v_fetch_executable;
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

