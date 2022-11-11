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

#include "bp.h"
#include "api_core.h"

namespace debugger {

BranchPredictor::BranchPredictor(sc_module_name name,
                                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_flush_pipeline("i_flush_pipeline"),
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
    i_f_fetching_pc("i_f_fetching_pc"),
    i_f_fetched_pc("i_f_fetched_pc"),
    i_d_pc("i_d_pc") {

    async_reset_ = async_reset;
    btb = 0;
    for (int i = 0; i < 2; i++) {
        predec[i] = 0;
    }

    for (int i = 0; i < 2; i++) {
        char tstr[256];
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
    btb->i_e(w_btb_e);
    btb->i_we(w_btb_we);
    btb->i_we_pc(wb_btb_we_pc);
    btb->i_we_npc(wb_btb_we_npc);
    btb->i_bp_pc(wb_start_pc);
    btb->o_bp_npc(wb_npc);
    btb->o_bp_exec(wb_bp_exec);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_flush_pipeline;
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_addr;
    sensitive << i_resp_mem_data;
    sensitive << i_e_jmp;
    sensitive << i_e_pc;
    sensitive << i_e_npc;
    sensitive << i_ra;
    sensitive << i_f_requested_pc;
    sensitive << i_f_fetching_pc;
    sensitive << i_f_fetched_pc;
    sensitive << i_d_pc;
    for (int i = 0; i < 2; i++) {
        sensitive << wb_pd[i].c_valid;
        sensitive << wb_pd[i].addr;
        sensitive << wb_pd[i].data;
        sensitive << wb_pd[i].jmp;
        sensitive << wb_pd[i].pc;
        sensitive << wb_pd[i].npc;
    }
    sensitive << w_btb_e;
    sensitive << w_btb_we;
    sensitive << wb_btb_we_pc;
    sensitive << wb_btb_we_npc;
    sensitive << wb_start_pc;
    sensitive << wb_npc;
    sensitive << wb_bp_exec;
}

BranchPredictor::~BranchPredictor() {
    if (btb) {
        delete btb;
    }
    for (int i = 0; i < 2; i++) {
        if (predec[i]) {
            delete predec[i];
        }
    }
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
        sc_trace(o_vcd, i_f_fetching_pc, i_f_fetching_pc.name());
        sc_trace(o_vcd, i_f_fetched_pc, i_f_fetched_pc.name());
        sc_trace(o_vcd, i_d_pc, i_d_pc.name());
    }

    if (btb) {
        btb->generateVCD(i_vcd, o_vcd);
    }
    for (int i = 0; i < 2; i++) {
        if (predec[i]) {
            predec[i]->generateVCD(i_vcd, o_vcd);
        }
    }
}

void BranchPredictor::comb() {
    sc_uint<RISCV_ARCH> vb_addr[CFG_BP_DEPTH];
    sc_uint<(RISCV_ARCH - 2)> vb_piped[4];
    sc_uint<RISCV_ARCH> vb_fetch_npc;
    bool v_btb_we;
    sc_uint<RISCV_ARCH> vb_btb_we_pc;
    sc_uint<RISCV_ARCH> vb_btb_we_npc;
    sc_uint<4> vb_hit;
    sc_uint<2> vb_ignore_pd;

    for (int i = 0; i < CFG_BP_DEPTH; i++) {
        vb_addr[i] = 0ull;
    }
    for (int i = 0; i < 4; i++) {
        vb_piped[i] = 0ull;
    }
    vb_fetch_npc = 0;
    v_btb_we = 0;
    vb_btb_we_pc = 0;
    vb_btb_we_npc = 0;
    vb_hit = 0;
    vb_ignore_pd = 0;

    // Transform address into 2-dimesional array for convinience
    for (int i = 0; i < CFG_BP_DEPTH; i++) {
        vb_addr[i] = wb_npc.read()((i * RISCV_ARCH) + RISCV_ARCH - 1, (i * RISCV_ARCH));
    }

    vb_piped[0] = i_d_pc.read()((RISCV_ARCH - 1), 2);
    vb_piped[1] = i_f_fetched_pc.read()((RISCV_ARCH - 1), 2);
    vb_piped[2] = i_f_fetching_pc.read()((RISCV_ARCH - 1), 2);
    vb_piped[3] = i_f_requested_pc.read()((RISCV_ARCH - 1), 2);
    // Check availablity of pc in pipeline
    vb_hit = 0;
    for (int n = 0; n < 4; n++) {
        for (int i = n; i < 4; i++) {
            if (vb_addr[n]((RISCV_ARCH - 1), 2) == vb_piped[i]) {
                vb_hit[n] = 1;
            }
        }
    }

    vb_fetch_npc = vb_addr[(CFG_BP_DEPTH - 1)];
    for (int i = 3; i >= 0; i--) {
        if (vb_hit[i] == 0) {
            vb_fetch_npc = vb_addr[i];
        }
    }

    // Pre-decoder input signals (not used for now)
    for (int i = 0; i < 2; i++) {
        wb_pd[i].c_valid = (!i_resp_mem_data.read()((16 * i) + 2 - 1, (16 * i)).and_reduce());
        wb_pd[i].addr = (i_resp_mem_addr.read() + (2 * i));
        wb_pd[i].data = i_resp_mem_data.read()((16 * i) + 32 - 1, (16 * i));
    }
    vb_ignore_pd = 0;
    for (int i = 0; i < 4; i++) {
        if (wb_pd[0].npc.read()((RISCV_ARCH - 1), 2) == vb_piped[i]) {
            vb_ignore_pd[0] = 1;
        }
        if (wb_pd[1].npc.read()((RISCV_ARCH - 1), 2) == vb_piped[i]) {
            vb_ignore_pd[1] = 1;
        }
    }

    v_btb_we = (i_e_jmp || wb_pd[0].jmp || wb_pd[1].jmp);
    if (i_e_jmp.read() == 1) {
        vb_btb_we_pc = i_e_pc;
        vb_btb_we_npc = i_e_npc;
    } else if (wb_pd[0].jmp) {
        vb_btb_we_pc = wb_pd[0].pc;
        vb_btb_we_npc = wb_pd[0].npc;
        if ((vb_hit(2, 0) == 0x7) && (wb_bp_exec.read()[2] == 0) && (vb_ignore_pd[0] == 0)) {
            vb_fetch_npc = wb_pd[0].npc;
        }
    } else if (wb_pd[1].jmp) {
        vb_btb_we_pc = wb_pd[1].pc;
        vb_btb_we_npc = wb_pd[1].npc;
        if ((vb_hit(2, 0) == 0x7) && (wb_bp_exec.read()[2] == 0) && (vb_ignore_pd[1] == 0)) {
            vb_fetch_npc = wb_pd[1].npc;
        }
    } else {
        vb_btb_we_pc = i_e_pc;
        vb_btb_we_npc = i_e_npc;
    }

    wb_start_pc = i_e_npc;
    w_btb_e = i_e_jmp;
    w_btb_we = v_btb_we;
    wb_btb_we_pc = vb_btb_we_pc;
    wb_btb_we_npc = vb_btb_we_npc;

    o_f_valid = 1;
    o_f_pc = (vb_fetch_npc((RISCV_ARCH - 1), 2) << 2);
}

}  // namespace debugger

