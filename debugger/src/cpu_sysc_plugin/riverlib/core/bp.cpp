/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "api_core.h"
#include "bp.h"

namespace debugger {

BranchPredictor::BranchPredictor(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_resp_mem_valid("i_resp_mem_valid"),
    i_resp_mem_addr("i_resp_mem_addr"),
    i_resp_mem_data("i_resp_mem_data"),
    i_e_jmp("i_e_jmp"),
    i_e_pc("i_e_pc"),
    i_e_npc("i_e_npc"),
    i_ra("i_ra"),
    o_f_valid("o_f_valid"),
    o_f_pc("o_f_pc"),
    i_f_requested_pc("i_f_requested_pc"),
    i_f_fetched_pc("i_f_fetched_pc"),
    i_d_decoded_pc("i_d_decoded_pc") {

    char tstr[256];
    for (int i = 0; i < 2; i++) {
        RISCV_sprintf(tstr, sizeof(tstr), "predec%d", i);
        predec[i] = new BpPreDecoder(tstr);
        predec[i]->i_c_valid(wb_pd[i].c_valid);
        predec[i]->i_addr(wb_pd[i].addr);
        predec[i]->i_data(wb_pd[i].data);
        predec[i]->i_ra(i_ra);
        predec[i]->o_jmp(wb_pd[i].jmp);
        predec[i]->o_pc(wb_pd[i].pc);
        predec[i]->o_npc(wb_pd[i].npc);
    }

    btb = new BpBTB("btb", async_reset);
    btb->i_clk(i_clk);
    btb->i_nrst(i_nrst);
    btb->i_flush_pipeline(i_flush_pipeline);
    btb->i_we(w_btb_we);
    btb->i_we_pc(wb_btb_we_pc);
    btb->i_we_npc(wb_btb_we_npc);
    btb->i_bp_pc(wb_start_pc);
    btb->o_bp_npc(wb_npc);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_addr;
    sensitive << i_resp_mem_data;
    sensitive << i_e_jmp;
    sensitive << i_e_pc;
    sensitive << i_e_npc;
    sensitive << i_ra;
    sensitive << i_f_requested_pc;
    sensitive << i_f_fetched_pc;
    sensitive << i_d_decoded_pc;
    sensitive << wb_npc;
    for (int i = 0; i < 2; i++) {
        sensitive << wb_pd[i].jmp;
        sensitive << wb_pd[i].pc;
        sensitive << wb_pd[i].npc;
    }
};

BranchPredictor::~BranchPredictor() {
    for (int i = 0; i < 2; i++) {
        delete predec[i];
    }
    delete btb;
}

void BranchPredictor::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, i_resp_mem_valid, i_resp_mem_valid.name());
        sc_trace(o_vcd, i_resp_mem_addr, i_resp_mem_addr.name());
        sc_trace(o_vcd, i_resp_mem_data, i_resp_mem_data.name());
        sc_trace(o_vcd, i_e_jmp, i_e_jmp.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_npc, i_e_npc.name());
        sc_trace(o_vcd, i_ra, i_ra.name());
        sc_trace(o_vcd, o_f_valid, o_f_valid.name());
        sc_trace(o_vcd, o_f_pc, o_f_pc.name());
        sc_trace(o_vcd, i_f_requested_pc, i_f_requested_pc.name());
        sc_trace(o_vcd, i_f_fetched_pc, i_f_fetched_pc.name());
        sc_trace(o_vcd, i_d_decoded_pc, i_d_decoded_pc.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_start_pc, pn + ".wb_start_pc");
        sc_trace(o_vcd, wb_npc, pn + ".wb_npc");
    }
}

void BranchPredictor::comb() {
    sc_uint<CFG_CPU_ADDR_BITS> t_d_addr[CFG_DEC_DEPTH];
    sc_uint<CFG_CPU_ADDR_BITS> vb_addr[CFG_BP_DEPTH];
    sc_uint<CFG_BP_DEPTH> vb_skip;
    sc_uint<CFG_CPU_ADDR_BITS> vb_fetch_npc;


    for (int i = 0; i < CFG_DEC_DEPTH; i++) {
        t_d_addr[i] = i_d_decoded_pc.read()((i+1)*CFG_CPU_ADDR_BITS-1, i*CFG_CPU_ADDR_BITS);
    }

    // Transform address into 2-dimesional array for convinience
    vb_skip = 0;
    for (int i = 0; i < CFG_BP_DEPTH; i++) {
        vb_addr[i] = wb_npc.read()((i+1)*CFG_CPU_ADDR_BITS-1, i*CFG_CPU_ADDR_BITS);
    }

    // Check availablity of pc in pipeline
    for (int i = 0; i < CFG_BP_DEPTH; i++) {
        for (int n = 0; n < CFG_DEC_DEPTH; n++) {
            if (vb_addr[i] == t_d_addr[n]) {
                vb_skip[i] = 1;
            }
        }
        if (vb_addr[i] == i_f_requested_pc.read()
            || vb_addr[i] == i_f_fetched_pc.read()) {
            vb_skip[i] = 1;
        }
    }

    // Select instruction to fetch
    vb_fetch_npc = ~0ull;
    for (int i = CFG_BP_DEPTH-1; i >= 0; i--) {
        if (vb_skip[i] == 0) {
            vb_fetch_npc = vb_addr[i];
        }
    }

    // Pre-decoder input signals (not used for now)
    for (int i = 0; i < 2; i++) {
        wb_pd[i].c_valid = !i_resp_mem_data.read()(16*i+1, 16*i).and_reduce();
        wb_pd[i].addr = i_resp_mem_addr.read() + 2*i;
        wb_pd[i].data = i_resp_mem_data.read()(16*i + 31, 16*i);
    }


    wb_start_pc = (i_e_npc.read() >> 2) << 2;
    w_btb_we = i_e_jmp;
    wb_btb_we_pc = (i_e_pc.read() >> 2) << 2;
    wb_btb_we_npc = (i_e_npc.read() >> 2) << 2;

    o_f_valid = !vb_skip.and_reduce();
    o_f_pc = vb_fetch_npc;
}

}  // namespace debugger

