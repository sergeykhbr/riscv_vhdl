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

#include "bp_btb.h"
#include "api_core.h"

namespace debugger {

BpBTB::BpBTB(sc_module_name name,
             bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_flush_pipeline("i_flush_pipeline"),
    i_e("i_e"),
    i_we("i_we"),
    i_we_pc("i_we_pc"),
    i_we_npc("i_we_npc"),
    i_bp_pc("i_bp_pc"),
    o_bp_npc("o_bp_npc"),
    o_bp_exec("o_bp_exec") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_flush_pipeline;
    sensitive << i_e;
    sensitive << i_we;
    sensitive << i_we_pc;
    sensitive << i_we_npc;
    sensitive << i_bp_pc;
    for (int i = 0; i < CFG_BP_DEPTH; i++) {
        sensitive << dbg_npc[i];
    }
    for (int i = 0; i < CFG_BTB_SIZE; i++) {
        sensitive << r.btb[i].pc;
        sensitive << r.btb[i].npc;
        sensitive << r.btb[i].exec;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void BpBTB::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, i_e, i_e.name());
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_we_pc, i_we_pc.name());
        sc_trace(o_vcd, i_we_npc, i_we_npc.name());
        sc_trace(o_vcd, i_bp_pc, i_bp_pc.name());
        sc_trace(o_vcd, o_bp_npc, o_bp_npc.name());
        sc_trace(o_vcd, o_bp_exec, o_bp_exec.name());
        for (int i = 0; i < CFG_BTB_SIZE; i++) {
            sc_trace(o_vcd, r.btb[i].pc, pn + ".r.btb(" + std::to_string(i) + ").pc");
            sc_trace(o_vcd, r.btb[i].npc, pn + ".r.btb(" + std::to_string(i) + ").npc");
            sc_trace(o_vcd, r.btb[i].exec, pn + ".r.btb(" + std::to_string(i) + ").exec");
        }
    }

}

void BpBTB::comb() {
    sc_biguint<(CFG_BP_DEPTH * RISCV_ARCH)> vb_addr;
    sc_uint<CFG_BP_DEPTH> vb_hit;
    sc_uint<RISCV_ARCH> t_addr;
    sc_uint<CFG_BTB_SIZE> vb_pc_equal;
    sc_uint<CFG_BTB_SIZE> vb_pc_nshift;
    sc_uint<CFG_BP_DEPTH> vb_bp_exec;
    bool v_dont_update;

    for (int i = 0; i < CFG_BTB_SIZE; i++) {
        v.btb[i].pc = r.btb[i].pc.read();
        v.btb[i].npc = r.btb[i].npc.read();
        v.btb[i].exec = r.btb[i].exec.read();
    }
    vb_addr = 0;
    vb_hit = 0;
    t_addr = 0;
    vb_pc_equal = 0;
    vb_pc_nshift = 0;
    vb_bp_exec = 0;
    v_dont_update = 0;

    vb_addr((RISCV_ARCH - 1), 0) = i_bp_pc.read();
    vb_bp_exec[0] = i_e.read();

    for (int i = 1; i < CFG_BP_DEPTH; i++) {
        t_addr = vb_addr(((i - 1) * RISCV_ARCH) + RISCV_ARCH - 1, ((i - 1) * RISCV_ARCH));
        for (int n = (CFG_BTB_SIZE - 1); n >= 0; n--) {
            if (t_addr == r.btb[n].pc.read()) {
                vb_addr((i * RISCV_ARCH) + RISCV_ARCH - 1, (i * RISCV_ARCH)) = r.btb[n].npc.read();
                vb_hit[i] = 1;
                vb_bp_exec[i] = r.btb[n].exec.read();       // Used for: Do not override by pre-decoded jumps
            } else if (vb_hit[i] == 0) {
                vb_addr((i * RISCV_ARCH) + RISCV_ARCH - 1, (i * RISCV_ARCH)) = (t_addr + 4);
            }
        }
    }

    v_dont_update = 0;
    vb_pc_equal = 0;
    for (int i = 0; i < CFG_BTB_SIZE; i++) {
        if (r.btb[i].pc.read() == i_we_pc.read()) {
            vb_pc_equal[i] = 1;
            v_dont_update = (r.btb[i].exec.read() && (!i_e.read()));
        }
    }
    vb_pc_nshift = 0;
    for (int i = 1; i < CFG_BTB_SIZE; i++) {
        vb_pc_nshift[i] = (vb_pc_equal[(i - 1)] || vb_pc_nshift[(i - 1)]);
    }

    if ((i_we.read() && (!v_dont_update)) == 1) {
        v.btb[0].exec = i_e.read();
        v.btb[0].pc = i_we_pc.read();
        v.btb[0].npc = i_we_npc.read();
        for (int i = 1; i < CFG_BTB_SIZE; i++) {
            if (vb_pc_nshift[i] == 0) {
                v.btb[i] = r.btb[(i - 1)];
            } else {
                v.btb[i] = r.btb[i];
            }
        }
    }

    if (((!async_reset_) && (i_nrst.read() == 0)) || i_flush_pipeline.read()) {
        BpBTB_r_reset(v);
    }

    for (int i = 0; i < CFG_BP_DEPTH; i++) {
        dbg_npc[i] = vb_addr((i * RISCV_ARCH) + RISCV_ARCH - 1, (i * RISCV_ARCH)).to_uint64();
    }
    o_bp_npc = vb_addr;
    o_bp_exec = vb_bp_exec;
}

void BpBTB::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        BpBTB_r_reset(r);
    } else {
        for (int i = 0; i < CFG_BTB_SIZE; i++) {
            r.btb[i].pc = v.btb[i].pc.read();
            r.btb[i].npc = v.btb[i].npc.read();
            r.btb[i].exec = v.btb[i].exec.read();
        }
    }
}

}  // namespace debugger

