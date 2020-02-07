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

#include "br_predic.h"

namespace debugger {

BranchPredictor::BranchPredictor(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_req_mem_fire("i_req_mem_fire"),
    i_resp_mem_valid("i_resp_mem_valid"),
    i_resp_mem_addr("i_resp_mem_addr"),
    i_resp_mem_data("i_resp_mem_data"),
    i_e_npc("i_e_npc"),
    i_ra("i_ra"),
    o_npc_predict("o_npc_predict") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_mem_fire;
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_addr;
    sensitive << i_resp_mem_data;
    sensitive << i_e_npc;
    sensitive << i_ra;
    sensitive << r.h[0].resp_pc;
    sensitive << r.h[0].resp_npc;
    sensitive << r.h[1].resp_pc;
    sensitive << r.h[1].resp_npc;
    sensitive << r.h[2].resp_pc;
    sensitive << r.h[2].resp_npc;
    sensitive << r.wait_resp;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void BranchPredictor::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_mem_fire, i_req_mem_fire.name());
        sc_trace(o_vcd, i_resp_mem_valid, i_resp_mem_valid.name());
        sc_trace(o_vcd, i_resp_mem_addr, i_resp_mem_addr.name());
        sc_trace(o_vcd, i_resp_mem_data, i_resp_mem_data.name());
        sc_trace(o_vcd, i_e_npc, i_e_npc.name());
        sc_trace(o_vcd, i_ra, i_ra.name());
        sc_trace(o_vcd, o_npc_predict, o_npc_predict.name());

        std::string pn(name());
        sc_trace(o_vcd, vb_npc, pn + ".vb_npc");
        sc_trace(o_vcd, r.h[0].resp_pc, pn + ".r_h0_resp_pc");
        sc_trace(o_vcd, r.h[0].resp_npc, pn + ".r_h0_resp_npc");
        sc_trace(o_vcd, r.h[1].resp_pc, pn + ".r_h1_resp_pc");
        sc_trace(o_vcd, r.h[1].resp_npc, pn + ".r_h1_resp_npc");
        sc_trace(o_vcd, r.h[2].resp_pc, pn + ".r_h2_resp_pc");
        sc_trace(o_vcd, r.h[2].resp_npc, pn + ".r_h2_resp_npc");
        sc_trace(o_vcd, r.wait_resp, pn + ".r_wait_resp");
        sc_trace(o_vcd, v_c_j, pn + ".v_c_j");
        sc_trace(o_vcd, v_jal, pn + ".v_jal");
        sc_trace(o_vcd, v_branch, pn + ".v_branch");
        sc_trace(o_vcd, v_c_ret, pn + ".v_c_ret");
    }
}

void BranchPredictor::comb() {
    sc_uint<32> vb_tmp;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_npc_predicted;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_pc;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_jal_off;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_jal_addr;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_branch_off;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_branch_addr;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_c_j_off;
    sc_uint<CFG_RIVER_ADDR_BITS> vb_c_j_addr;

    v = r;

    vb_pc = r.h[0].resp_pc.read();
    vb_tmp = i_resp_mem_data.read();

    // Unconditional jump "J"
    if (vb_tmp[31]) {
        vb_jal_off(CFG_RIVER_ADDR_BITS-1, 20) = ~0;
    } else {
        vb_jal_off(CFG_RIVER_ADDR_BITS-1, 20) = 0;
    }
    vb_jal_off(19, 12) = vb_tmp(19, 12);
    vb_jal_off[11] = vb_tmp[20];
    vb_jal_off(10, 1) = vb_tmp(30, 21);
    vb_jal_off[0] = 0;
    vb_jal_addr = vb_pc + vb_jal_off;

    v_jal = 0;
    if (vb_tmp.range(6, 0) == 0x6F) {
        if (vb_jal_addr != r.h[1].resp_pc && vb_jal_addr != r.h[2].resp_pc) {
            v_jal = 1;
        }
    }

    // Conditional branches "BEQ", "BNE", "BLT", "BGE", BLTU", "BGEU"
    // Only negative offset leads to predicted jumps
    if (vb_tmp[31]) {
        vb_branch_off(CFG_RIVER_ADDR_BITS-1, 12) = ~0;
    } else {
        vb_branch_off(CFG_RIVER_ADDR_BITS-1, 12) = 0;
    }
    vb_branch_off[11] = vb_tmp[7];
    vb_branch_off(10, 5) = vb_tmp(30, 25);
    vb_branch_off(4, 1) = vb_tmp(11, 8);
    vb_branch_off[0] = 0;
    vb_branch_addr = vb_pc + vb_branch_off;

    v_branch = 0;
    if (vb_tmp.range(6, 0) == 0x63 && vb_tmp[31] == 1) {
        if (vb_branch_addr != r.h[1].resp_pc 
            && vb_branch_addr != r.h[2].resp_pc) {
            v_branch = 1;
        }
    }

    // Check Compressed "C_J" unconditional jump
    if (vb_tmp[12]) {
        vb_c_j_off(CFG_RIVER_ADDR_BITS-1, 11) = ~0;
    } else {
        vb_c_j_off(CFG_RIVER_ADDR_BITS-1, 11) = 0;
    }
    vb_c_j_off[10] = vb_tmp[8];
    vb_c_j_off(9, 8) = vb_tmp(10, 9);
    vb_c_j_off[7] = vb_tmp[6];
    vb_c_j_off[6] = vb_tmp[7];
    vb_c_j_off[5] = vb_tmp[2];
    vb_c_j_off[4] = vb_tmp[11];
    vb_c_j_off(3, 1) = vb_tmp(5, 3);
    vb_c_j_off[0] = 0;
    vb_c_j_addr = vb_pc + vb_c_j_off;

    v_c_j = 0;
    if (vb_tmp.range(15, 13) == 0x5 && vb_tmp.range(1, 0) == 0x1) {
        if (vb_c_j_addr != r.h[1].resp_pc && vb_c_j_addr != r.h[2].resp_pc) {
            v_c_j = 1;
        }
    }

    // Compressed RET pseudo-instruction
    v_c_ret = 0;
    if (vb_tmp.range(15, 0) == 0x8082) {
        v_c_ret = 1;
    }

    if (v_jal == 1) {
        vb_npc_predicted = vb_jal_addr;
    } else if (v_branch == 1) {
        vb_npc_predicted = vb_branch_addr;
    } else if (v_c_j == 1) {
        vb_npc_predicted = vb_c_j_addr;
    } else if (v_c_ret == 1) {
        vb_npc_predicted = i_ra.read();
    } else if (vb_tmp(1, 0) == 0x3) {
        vb_npc_predicted = r.h[0].resp_pc.read() + 4;
    } else {
        vb_npc_predicted = r.h[0].resp_pc.read() + 2;
    }

    if (i_e_npc.read() == r.h[2].resp_pc.read()) {
        if (r.h[2].resp_npc.read() == r.h[1].resp_pc.read()) {
            if (r.h[1].resp_npc.read() == r.h[0].resp_pc.read()) {
                vb_npc = vb_npc_predicted;
            } else {
                vb_npc = r.h[1].resp_npc.read();
            }
        } else if (r.h[2].resp_npc.read() == r.h[0].resp_pc.read()) {
            vb_npc = vb_npc_predicted;
        } else {
            vb_npc = r.h[2].resp_npc.read();
        }
    } else if (i_e_npc.read() == r.h[1].resp_pc.read()) {
        if (r.h[1].resp_npc.read() == r.h[0].resp_pc.read()) {
            vb_npc = vb_npc_predicted;
        } else {
            vb_npc = r.h[1].resp_npc.read();
        }
    } else if (i_e_npc.read() == r.h[0].resp_pc.read()) {
        vb_npc = vb_npc_predicted;
    } else {
        vb_npc = i_e_npc.read();
    }

    if (i_req_mem_fire.read() == 1 && r.wait_resp.read() == 0) {
        v.wait_resp = 1;
        v.h[0].resp_pc = vb_npc;
        v.h[0].resp_npc = ~0ul;
        for (int i = 0; i < 2; i++) {
            v.h[i+1].resp_pc = r.h[i].resp_pc;
            v.h[i+1].resp_npc = r.h[i].resp_npc;
        }
    } else if (i_req_mem_fire.read() == 1 && i_resp_mem_valid.read() == 1) {
        v.wait_resp = 1;
        v.h[0].resp_pc = vb_npc;
        v.h[0].resp_npc = ~0ul;
        for (int i = 0; i < 2; i++) {
            v.h[i+1].resp_pc = r.h[i].resp_pc;
            v.h[i+1].resp_npc = r.h[i].resp_npc;
        }
        v.h[1].resp_npc = vb_npc;
    } else if (i_resp_mem_valid.read() == 1 && r.wait_resp.read() == 1) {
        v.wait_resp = 0;
        v.h[0].resp_npc = vb_npc;
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_npc_predict = vb_npc;
}

void BranchPredictor::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

