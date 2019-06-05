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
        sc_trace(o_vcd, i_e_npc, "/top/proc0/bp0/i_e_npc");
        sc_trace(o_vcd, i_ra, "/top/proc0/bp0/i_ra");

        sc_trace(o_vcd, o_npc_predict, "/top/proc0/bp0/o_npc_predict");
        sc_trace(o_vcd, vb_npc2, "/top/proc0/bp0/vb_npc2");
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
    }
}

void BranchPredictor::comb() {
    sc_uint<32> vb_tmp;
    sc_uint<BUS_ADDR_WIDTH> vb_jal_off;
    bool v_predict;
    bool v_sequence;
    bool v_c0;

    v = r;

    vb_tmp = i_resp_mem_data.read();

    if (vb_tmp[31]) {
        vb_jal_off(BUS_ADDR_WIDTH-1, 20) = ~0;
    } else {
        vb_jal_off(BUS_ADDR_WIDTH-1, 20) = 0;
    }
    vb_jal_off(19, 12) = vb_tmp(19, 12);
    vb_jal_off[11] = vb_tmp[20];
    vb_jal_off(10, 1) = vb_tmp(30, 21);
    vb_jal_off[0] = 0;

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
                vb_npc2 = r.h[0].resp_npc.read();
            } else {
                vb_npc2 = r.h[1].resp_npc.read();
            }
        } else if (r.h[2].resp_npc.read() == r.h[0].resp_pc.read()) {
            vb_npc2 = r.h[0].resp_npc.read();
        } else {
            vb_npc2 = r.h[2].resp_npc.read();
        }
    } else if (i_e_npc.read() == r.h[1].resp_pc.read()) {
        if (r.h[1].resp_npc.read() == r.h[0].resp_pc.read()) {
            vb_npc2 = r.h[0].resp_npc.read();
        } else {
            vb_npc2 = r.h[1].resp_npc.read();
        }
    } else if (i_e_npc.read() == r.h[0].resp_pc.read()) {
        vb_npc2 = r.h[0].resp_npc.read();
    } else {
        vb_npc2 = i_e_npc.read();
        v_sequence = 0;
    }

    if (i_resp_mem_valid.read() == 1) {
        v.c0 = !(i_resp_mem_data.read()[1] & i_resp_mem_data.read()[0]);
        v.c1 = !(i_resp_mem_data.read()[17] & i_resp_mem_data.read()[16]);
    }

    if (i_req_mem_fire.read() == 1) {
        v.sequence = v_sequence;
    }

    if (i_req_mem_fire.read() == 1 && r.wait_resp.read() == 0) {
        v.wait_resp = 1;
        v.minus2 = 0;
        v.minus4 = 0;
        v.h[0].req_addr = vb_npc2;
        v.h[0].ignore = 0;
        v.h[0].resp_pc = vb_npc2;
        v.h[0].resp_npc = vb_npc2 + 4;   // default instruction size
        for (int i = 0; i < 2; i++) {
            v.h[i+1].resp_pc = r.h[i].resp_pc;
            v.h[i+1].resp_npc = r.h[i].resp_npc;
            v.h[i+1].req_addr = r.h[i].req_addr;
            v.h[i+1].ignore = r.h[i].ignore;
        }
        v.h[1].resp_npc = vb_npc2;
    } else if (i_req_mem_fire.read() == 1 && i_resp_mem_valid.read() == 1) {
        v.h[0].req_addr = vb_npc2;
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
                    v.h[0].resp_pc = vb_npc2 - 2;
                    v.h[0].resp_npc = vb_npc2 + 2;
                    v.h[1].resp_pc = r.h[0].resp_pc;
                    v.h[1].resp_npc = r.h[0].resp_pc.read() + 2;
                } else {
                    // Previous computed npc was predicted correctly switch
                    // to normal increment (+4)
                    v.minus2 = 0;
                    v.h[0].resp_pc = vb_npc2;
                    v.h[0].resp_npc = vb_npc2 + 4;
                }
            } else {
                // 4-bytes instruction received
                v.minus2 = 0;
                v.h[0].resp_pc = vb_npc2;
                v.h[0].resp_npc = vb_npc2 + 4;
            }
        } else {
            // start a new sequence
            v.minus2 = 0;
            v.h[0].resp_pc = vb_npc2;
            v.h[0].resp_npc = vb_npc2 + 4;
            v.h[1].resp_npc = vb_npc2;
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
        for (int i = 0; i < 3; i++) {
            v.h[i].resp_pc = ~0ul;
            v.h[i].resp_npc = ~0ul;
            v.h[i].req_addr = 0;
            v.h[i].ignore = 0;
        }
        v.wait_resp = 0;
        v.sequence = 0;
        v.minus2 = 0;
        v.minus4 = 0;
        v.c0 = 0;
        v.c1 = 0;
    }

    o_npc_predict = vb_npc2;
    o_predict = v_predict;
    o_minus2 = r.minus2.read();
    o_minus4 = r.minus4.read();
}

void BranchPredictor::registers() {
    r = v;
}

}  // namespace debugger

