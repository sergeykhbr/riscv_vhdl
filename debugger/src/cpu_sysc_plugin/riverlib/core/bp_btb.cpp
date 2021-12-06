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
#include "bp_btb.h"

namespace debugger {

BpBTB::BpBTB(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_flush_pipeline("i_flush_pipeline"),
    i_we("i_we"),
    i_we_pc("i_we_pc"),
    i_we_npc("i_we_npc"),
    i_bp_pc("i_bp_pc"),
    o_bp_npc("o_bp_npc") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_flush_pipeline;
    sensitive << i_we;
    sensitive << i_we_pc;
    sensitive << i_we_npc;
    sensitive << i_bp_pc;
    for (int i = 0; i < CFG_BTB_SIZE; i++) {
        sensitive << r_btb[i].pc;
        sensitive << r_btb[i].npc;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void BpBTB::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_we_pc, i_we_pc.name());
        sc_trace(o_vcd, i_we_npc, i_we_npc.name());
        sc_trace(o_vcd, i_bp_pc, i_bp_pc.name());
        sc_trace(o_vcd, o_bp_npc, o_bp_npc.name());

        std::string pn(name());
        sc_trace(o_vcd, r_btb[0].pc, pn + ".btb0_pc");
        sc_trace(o_vcd, r_btb[0].npc, pn + ".btb0_npc");
        sc_trace(o_vcd, r_btb[1].pc, pn + ".btb1_pc");
        sc_trace(o_vcd, r_btb[1].npc, pn + ".btb1_npc");
        sc_trace(o_vcd, r_btb[2].pc, pn + ".btb2_pc");
        sc_trace(o_vcd, r_btb[2].npc, pn + ".btb2_npc");
        sc_trace(o_vcd, r_btb[3].pc, pn + ".btb3_pc");
        sc_trace(o_vcd, r_btb[3].npc, pn + ".btb3_npc");
        sc_trace(o_vcd, dbg_hit, pn + ".dbg_hit");
    }
}

void BpBTB::comb() {
    sc_biguint<CFG_BP_DEPTH*CFG_CPU_ADDR_BITS> vb_addr;
    sc_uint<CFG_CPU_ADDR_BITS> t_addr;
    dbg_hit = 0;

    for (int i = 0; i < CFG_BTB_SIZE; i++) {
        v_btb[i] = r_btb[i];
    }

    vb_addr(CFG_CPU_ADDR_BITS-1,0) = i_bp_pc.read();
    for (int i = 1; i < CFG_BP_DEPTH; i++) {
        t_addr = vb_addr(i*CFG_CPU_ADDR_BITS-1, (i-1)*CFG_CPU_ADDR_BITS);
        for (int n = CFG_BTB_SIZE-1; n >= 0; n--) {
            if (t_addr == r_btb[n].pc) {
                vb_addr((i+1)*CFG_CPU_ADDR_BITS-1, i*CFG_CPU_ADDR_BITS) = r_btb[n].npc;
                dbg_hit[n]= 1;
            } else {
                vb_addr((i+1)*CFG_CPU_ADDR_BITS-1, i*CFG_CPU_ADDR_BITS) = t_addr + 4;
            }
        }
    }

    if (i_we 
        && (r_btb[0].pc != i_we_pc || r_btb[0].npc != i_we_npc)
        && (i_we_pc != i_we_npc)) {
        v_btb[0].pc = i_we_pc;
        v_btb[0].npc = i_we_npc;
        for (int i = 1; i < CFG_BTB_SIZE - 1; i++) {
            v_btb[i] = r_btb[i - 1];
        }
    }

    if ((!async_reset_ && !i_nrst.read()) || i_flush_pipeline) {
        for (int i = 0; i < CFG_BTB_SIZE; i++) {
            R_RESET(v_btb[i]);
        }
    }

    o_bp_npc = vb_addr;
}

void BpBTB::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < CFG_BTB_SIZE; i++) {
            R_RESET(r_btb[i]);
        }
    } else {
        for (int i = 0; i < CFG_BTB_SIZE; i++) {
            r_btb[i] = v_btb[i];
        }
    }
}

}  // namespace debugger

