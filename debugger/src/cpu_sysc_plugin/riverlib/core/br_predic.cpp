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

BranchPredictor::BranchPredictor(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_mem_fire;
    sensitive << i_resp_mem_valid;
    sensitive << i_resp_mem_addr;
    sensitive << i_resp_mem_data;
    sensitive << i_f_pc;
    sensitive << i_f_instr;
    sensitive << i_e_npc;
    sensitive << i_ra;
    sensitive << r.h[0].resp_pc;
    sensitive << r.h[0].resp_npc;
    sensitive << r.h[0].req_addr;
    sensitive << r.h[0].ignore;
    sensitive << r.h[1].resp_pc;
    sensitive << r.h[1].resp_npc;
    sensitive << r.h[1].req_addr;
    sensitive << r.h[1].ignore;
    sensitive << r.h[2].resp_pc;
    sensitive << r.h[2].resp_npc;
    sensitive << r.h[2].req_addr;
    sensitive << r.h[2].ignore;
    sensitive << r.wait_resp;
    sensitive << r.sequence;
    sensitive << r.minus2;
    sensitive << r.minus4;
    sensitive << r.c0;
    sensitive << r.c1;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void BranchPredictor::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_req_mem_fire, "/top/proc0/bp0/i_req_mem_fire");
        sc_trace(o_vcd, i_resp_mem_valid, "/top/proc0/bp0/i_resp_mem_valid");
        sc_trace(o_vcd, i_resp_mem_addr, "/top/proc0/bp0/i_resp_mem_addr");
        sc_trace(o_vcd, i_resp_mem_data, "/top/proc0/bp0/i_resp_mem_data");
        sc_trace(o_vcd, i_f_pc, "/top/proc0/bp0/i_f_pc");
        sc_trace(o_vcd, i_f_instr, "/top/proc0/bp0/i_f_instr");
        sc_trace(o_vcd, i_e_npc, "/top/proc0/bp0/i_e_npc");
        sc_trace(o_vcd, i_ra, "/top/proc0/bp0/i_ra");

        sc_trace(o_vcd, o_npc_predict, "/top/proc0/bp0/o_npc_predict");
        sc_trace(o_vcd, vb_npc, "/top/proc0/bp0/vb_npc");
        sc_trace(o_vcd, r.h[0].resp_pc, "/top/proc0/bp0/r_h(0)_resp_pc");
        sc_trace(o_vcd, r.h[0].resp_npc, "/top/proc0/bp0/r_h(0)_resp_npc");
        sc_trace(o_vcd, r.h[0].req_addr, "/top/proc0/bp0/r_h(0)_req_addr");
        sc_trace(o_vcd, r.h[1].resp_pc, "/top/proc0/bp0/r_h(1)_resp_pc");
        sc_trace(o_vcd, r.h[1].resp_npc, "/top/proc0/bp0/r_h(1)_resp_npc");
        sc_trace(o_vcd, r.h[1].req_addr, "/top/proc0/bp0/r_h(1)_req_addr");
        sc_trace(o_vcd, r.wait_resp, "/top/proc0/bp0/r_wait_resp");
        sc_trace(o_vcd, r.sequence, "/top/proc0/bp0/r_sequence");
        sc_trace(o_vcd, r.minus2, "/top/proc0/bp0/r_minus2");
        sc_trace(o_vcd, r.minus4, "/top/proc0/bp0/r_minus4");
        sc_trace(o_vcd, r.c0, "/top/proc0/bp0/r_c0");
        sc_trace(o_vcd, r.c1, "/top/proc0/bp0/r_c1");
        sc_trace(o_vcd, v_c_j, "/top/proc0/bp0/v_c_j");
        sc_trace(o_vcd, v_jal, "/top/proc0/bp0/v_jal");
        sc_trace(o_vcd, v_branch, "/top/proc0/bp0/v_branch");
        sc_trace(o_vcd, v_c_ret, "/top/proc0/bp0/v_c_ret");
    }
}

void BranchPredictor::comb() {
    sc_uint<32> vb_tmp;
    sc_uint<BUS_ADDR_WIDTH> vb_jal_off;
    sc_uint<BUS_ADDR_WIDTH> vb_jal_addr;
    sc_uint<BUS_ADDR_WIDTH> vb_branch_off;
    sc_uint<BUS_ADDR_WIDTH> vb_branch_addr;
    sc_uint<BUS_ADDR_WIDTH> vb_c_j_off;
    sc_uint<BUS_ADDR_WIDTH> vb_c_j_addr;
    bool v_sequence;
    bool v_c0;

    v = r;

    vb_tmp = i_f_instr.read();

    // Unconditional jump "J"
    if (vb_tmp[31]) {
        vb_jal_off(BUS_ADDR_WIDTH-1, 20) = ~0;
    } else {
        vb_jal_off(BUS_ADDR_WIDTH-1, 20) = 0;
    }
    vb_jal_off(19, 12) = vb_tmp(19, 12);
    vb_jal_off[11] = vb_tmp[20];
    vb_jal_off(10, 1) = vb_tmp(30, 21);
    vb_jal_off[0] = 0;
    vb_jal_addr = i_f_pc.read() + vb_jal_off;

    v_jal = 0;
    if (vb_tmp.range(6, 0) == 0x6F) {
        if (vb_jal_addr != r.h[1].resp_pc && vb_jal_addr != r.h[2].resp_pc) {
            v_jal = 1;
        }
    }

    // Conditional branches "BEQ", "BNE", "BLT", "BGE", BLTU", "BGEU"
    // Only negative offset leads to predicted jumps
    if (vb_tmp[31]) {
        vb_branch_off(BUS_ADDR_WIDTH-1, 12) = ~0;
    } else {
        vb_branch_off(BUS_ADDR_WIDTH-1, 12) = 0;
    }
    vb_branch_off[11] = vb_tmp[7];
    vb_branch_off(10, 5) = vb_tmp(30, 25);
    vb_branch_off(4, 1) = vb_tmp(11, 8);
    vb_branch_off[0] = 0;
    vb_branch_addr = i_f_pc.read() + vb_branch_off;

    v_branch = 0;
    if (vb_tmp.range(6, 0) == 0x63 && vb_tmp[31] == 1) {
        if (vb_branch_addr != r.h[1].resp_pc 
            && vb_branch_addr != r.h[2].resp_pc) {
            v_branch = 1;
        }
    }

    // Check Compressed "C_J" unconditional jump
    if (vb_tmp[12]) {
        vb_c_j_off(BUS_ADDR_WIDTH-1, 11) = ~0;
    } else {
        vb_c_j_off(BUS_ADDR_WIDTH-1, 11) = 0;
    }
    vb_c_j_off[10] = vb_tmp[8];
    vb_c_j_off(9, 8) = vb_tmp(10, 9);
    vb_c_j_off[7] = vb_tmp[6];
    vb_c_j_off[6] = vb_tmp[7];
    vb_c_j_off[5] = vb_tmp[2];
    vb_c_j_off[4] = vb_tmp[11];
    vb_c_j_off(3, 1) = vb_tmp(5, 3);
    vb_c_j_off[0] = 0;
    vb_c_j_addr = i_f_pc.read() + vb_c_j_off;

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

    if (r.minus4.read() == 1) {
        v_c0 = r.c0.read();
    } else if (r.minus2.read() == 1) {
        v_c0 = r.c1.read();
    } else {
        v_c0 = !(i_resp_mem_data.read()[1] & i_resp_mem_data.read()[0]);
    }

    v_sequence = 1;
    if (i_e_npc.read() == r.h[2].resp_pc.read()) {
        if (r.h[2].resp_npc.read() == r.h[1].resp_pc.read()) {
            if (r.h[1].resp_npc.read() == r.h[0].resp_pc.read()) {
                vb_npc = r.h[0].resp_npc.read();
            } else {
                vb_npc = r.h[1].resp_npc.read();
            }
        } else if (r.h[2].resp_npc.read() == r.h[0].resp_pc.read()) {
            vb_npc = r.h[0].resp_npc.read();
        } else {
            vb_npc = r.h[2].resp_npc.read();
        }
    } else if (i_e_npc.read() == r.h[1].resp_pc.read()) {
        if (r.h[1].resp_npc.read() == r.h[0].resp_pc.read()) {
            vb_npc = r.h[0].resp_npc.read();
        } else {
            vb_npc = r.h[1].resp_npc.read();
        }
    } else if (i_e_npc.read() == r.h[0].resp_pc.read()) {
        vb_npc = r.h[0].resp_npc.read();
    } else {
        vb_npc = i_e_npc.read();
        v_sequence = 0;
    }

    // 1 fault clock on jump, ignore instruction after jumping
    if (v_sequence == 1 && r.sequence == 1) {
        if (v_jal == 1) {
            v_sequence = 0;
            vb_npc = vb_jal_addr;
        } else if (v_branch == 1) {
            v_sequence = 0;
            vb_npc = vb_branch_addr;
        } else if (v_c_j == 1) {
            v_sequence = 0;
            vb_npc = vb_c_j_addr;
        } else if (v_c_ret == 1) {
            v_sequence = 0;
            vb_npc = i_ra.read();
        }
    }

    if (i_req_mem_fire.read() == 1) {
        /** To avoid double branching when two Jump instructions
            placed sequentually we ignore the second jump instruction */
        v.sequence = v_sequence;
    }

    if (i_resp_mem_valid.read() == 1) {
        v.c0 = !(i_resp_mem_data.read()[1] & i_resp_mem_data.read()[0]);
        v.c1 = !(i_resp_mem_data.read()[17] & i_resp_mem_data.read()[16]);
    }


    if (i_req_mem_fire.read() == 1 && r.wait_resp.read() == 0) {
        v.wait_resp = 1;
        v.minus2 = 0;
        v.minus4 = 0;
        v.h[0].req_addr = vb_npc;
        v.h[0].ignore = 0;
        v.h[0].resp_pc = vb_npc;
        v.h[0].resp_npc = vb_npc + 4;   // default instruction size
        for (int i = 0; i < 2; i++) {
            v.h[i+1].resp_pc = r.h[i].resp_pc;
            v.h[i+1].resp_npc = r.h[i].resp_npc;
            v.h[i+1].req_addr = r.h[i].req_addr;
            v.h[i+1].ignore = r.h[i].ignore;
        }
        v.h[1].resp_npc = vb_npc;
    } else if (i_req_mem_fire.read() == 1 && i_resp_mem_valid.read() == 1) {
        v.h[0].req_addr = vb_npc;
        v.h[0].ignore = 0;
        v.minus4 = 0;
        v.h[1].resp_pc = r.h[0].resp_pc;
        v.h[1].resp_npc = r.h[0].resp_npc;
        v.h[1].req_addr = r.h[0].req_addr;
        v.h[1].ignore = r.h[0].ignore;
        if (v_sequence == 1) {
            if (v_c0 == 1) {
                if (r.minus2.read() == 1) {
                    // Two C-instructions, 
                    //   ignore memory response and re-use full fetched previous value
                    if (i_resp_mem_data.read().range(1, 0) != 0x3) {
                        v.h[0].resp_npc = i_resp_mem_addr.read() + 2;
                    } else {
                        v.h[0].resp_npc = i_resp_mem_addr.read() + 4;
                    }
                    v.h[0].ignore = 1;
                    v.h[0].resp_pc = i_resp_mem_addr;
                    v.h[1].resp_npc = r.h[0].resp_pc.read() + 2;
                    v.minus2 = 0;
                    v.minus4 = 1;
                } else if (r.minus4.read() == 0) {
                    // 1-st of two C-instructions
                    v.minus2 = 1;
                    v.h[0].resp_pc = vb_npc - 2;
                    v.h[0].resp_npc = vb_npc + 2;
                    v.h[1].resp_pc = r.h[0].resp_pc;
                    v.h[1].resp_npc = r.h[0].resp_pc.read() + 2;
                } else {
                    // Previous computed npc was predicted correctly switch
                    // to normal increment (+4)
                    v.minus2 = 0;
                    v.h[0].resp_pc = vb_npc;
                    v.h[0].resp_npc = vb_npc + 4;
                }
            } else {
                // 4-bytes instruction received
                v.minus2 = 0;
                v.h[0].resp_pc = vb_npc;
                v.h[0].resp_npc = vb_npc + 4;
            }
        } else {
            // start a new sequence
            v.minus2 = 0;
            v.h[0].resp_pc = vb_npc;
            v.h[0].resp_npc = vb_npc + 4;
            v.h[1].resp_npc = vb_npc;
        }
        v.h[2].resp_pc = r.h[1].resp_pc;
        v.h[2].resp_npc = r.h[1].resp_npc;
        v.h[2].req_addr = r.h[1].req_addr;
        v.h[2].ignore = r.h[1].ignore;
    } else if (i_resp_mem_valid.read() == 1 && r.wait_resp.read() == 1) {
        v.wait_resp = 0;
        if (r.sequence == 1) {
            if (r.minus4.read() == 1) {
                if (r.c0 == 1) {
                    v.h[0].resp_npc = r.h[0].resp_pc.read() + 2;
                }
            } else if (r.minus2.read() == 1) {
                if (r.c1 == 1) {
                    v.h[0].resp_npc = r.h[0].resp_pc.read() + 2;
                }
            } else {
                if ((i_resp_mem_data.read()[1] & i_resp_mem_data.read()[0]) == 0) {
                    v.h[0].resp_npc = r.h[0].resp_pc.read() + 2;
                }
            }
        } else {
            if ((i_resp_mem_data.read()[1] & i_resp_mem_data.read()[0]) == 0) {
                v.h[0].resp_npc = r.h[0].resp_pc.read() + 2;
            }
        }
    }


    if (!i_nrst.read()) {
        R_RESET(v);
    }

    o_npc_predict = vb_npc;
    o_minus2 = r.minus2.read();
    o_minus4 = r.minus4.read();
}

void BranchPredictor::registers() {
    r = v;
}

}  // namespace debugger

