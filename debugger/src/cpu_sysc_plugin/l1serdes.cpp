/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "l1serdes.h"

namespace debugger {

L1SerDes::L1SerDes(sc_module_name name, bool async_reset) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    o_corei("o_corei"),
    i_coreo("i_coreo"),
    i_msti("i_msti"),
    o_msto("o_msto") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_coreo;
    sensitive << i_msti;
    sensitive << r.req_len;
    sensitive << r.b_wait;
    sensitive << r.state;
    sensitive << r.line;
    sensitive << r.wstrb;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

L1SerDes::~L1SerDes() {
}

void L1SerDes::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, i_msti, i_msti.name());

        sc_trace(o_vcd, o_corei, o_corei.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.b_wait, pn + ".r_b_wait");
    }
}

sc_uint<8> L1SerDes::size2len(sc_uint<3> size) {
    sc_uint<8> ret;
    switch (size) {
    case 4: // 16 Bytes
        ret = 1;
        break;
    case 5: // 32 Bytes
        ret = 3;
        break;
    case 6: // 64 Bytes
        ret = 7;
        break;
    case 7: // 128 Bytes
        ret = 15;
        break;
    default:
        ret = 0;
    }
    return ret;
}

void L1SerDes::comb() {
    bool v_req_mem_ready;
    sc_uint<busw> vb_r_data;
    sc_biguint<linew> vb_line_o;
    sc_uint<4> vb_r_resp;
    bool v_r_valid;
    bool v_w_valid;
    bool v_w_last;
    bool v_w_ready;
    sc_uint<8> vb_len;
    axi4_l1_in_type vcorei;
    axi4_master_out_type vmsto;

    v_req_mem_ready = 0;
    v_r_valid = 0;
    v_w_valid = 0;
    v_w_last = 0;
    v_w_ready = 0;
    vb_len = 0;

    vb_r_data = i_msti.read().r_data;
    vb_line_o = r.line.read();
    for (int i = 0; i < SERDES_BURST_LEN; i++) {
        if (r.rmux.read()[i] == 1) {
            vb_line_o((i+1)*busw-1, i*busw) = vb_r_data;
        }
    }

    if (i_coreo.read().b_ready == 1) {
        v.b_wait = 0;
    }

    switch (r.state.read()) {
    case State_Idle:
        v_req_mem_ready = 1;
        break;
    case State_Read:
        if (i_msti.read().r_valid == 1) {
            v.line = vb_line_o;
            v.rmux = r.rmux.read() << 1;
            if (r.req_len.read() == 0) {
                v_r_valid = 1;
                v_req_mem_ready = 1;
            } else {
                v.req_len = r.req_len.read() - 1;
            }
        }
        break;
    case State_Write:
        v_w_valid = 1;
        if (r.req_len.read() == 0) {
            v_w_last = 1;
        }
        if (i_msti.read().w_ready) {
            v.line = (0, r.line.read()(linew-1, busw));
            v.wstrb = (0, r.wstrb.read()(lineb-1, busb));
            if (r.req_len.read() == 0) {
                v_w_ready = 1;
                v.b_wait = 1;
                v_req_mem_ready = 1;
            } else {
                v.req_len = r.req_len.read() - 1;
            }
        }
        break;
    default:;
    }

    if (v_req_mem_ready == 1) {
        if (i_coreo.read().ar_valid && i_msti.read().ar_ready) {
            v.state = State_Read;
            v.rmux = 1;
            vb_len = size2len(i_coreo.read().ar_bits.size);
        } else if (i_coreo.read().aw_valid && i_msti.read().aw_ready) {
            v.line = i_coreo.read().w_data;                     // Undocumented RIVER (Axi-lite feature)
            v.wstrb = i_coreo.read().w_strb;
            v.state = State_Write;
            vb_len = size2len(i_coreo.read().aw_bits.size);
        } else {
            v.state = State_Idle;
        }
        v.req_len = vb_len;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    vb_r_resp(3, 2) = 0;
    vb_r_resp(1, 0) = i_msti.read().r_resp; // OKAY

    vmsto.aw_valid = i_coreo.read().aw_valid;
    vmsto.aw_bits.addr = i_coreo.read().aw_bits.addr;
    vmsto.aw_bits.len = vb_len;              // burst len = len[7:0] + 1
    vmsto.aw_bits.size = 0x3;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    vmsto.aw_bits.burst = 0x1;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    vmsto.aw_bits.lock = i_coreo.read().aw_bits.lock;
    vmsto.aw_bits.cache = i_coreo.read().aw_bits.cache;
    vmsto.aw_bits.prot = i_coreo.read().aw_bits.prot;
    vmsto.aw_bits.qos = i_coreo.read().aw_bits.qos;
    vmsto.aw_bits.region = i_coreo.read().aw_bits.region;
    vmsto.aw_id = (0, i_coreo.read().aw_id);
    vmsto.aw_user = i_coreo.read().aw_user;
    vmsto.w_valid = v_w_valid;
    vmsto.w_last = v_w_last;
    vmsto.w_data = r.line.read()(busw-1, 0).to_uint64();
    vmsto.w_strb = r.wstrb.read()(busb-1, 0);
    vmsto.w_user = i_coreo.read().w_user;
    vmsto.b_ready = i_coreo.read().b_ready;
    vmsto.ar_valid = i_coreo.read().ar_valid;
    vmsto.ar_bits.addr = i_coreo.read().ar_bits.addr;
    vmsto.ar_bits.len = vb_len;              // burst len = len[7:0] + 1
    vmsto.ar_bits.size = 0x3;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    vmsto.ar_bits.burst = 0x1;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    vmsto.ar_bits.lock = i_coreo.read().ar_bits.lock;
    vmsto.ar_bits.cache = i_coreo.read().ar_bits.cache;
    vmsto.ar_bits.prot = i_coreo.read().ar_bits.prot;
    vmsto.ar_bits.qos = i_coreo.read().ar_bits.qos;
    vmsto.ar_bits.region = i_coreo.read().ar_bits.region;
    vmsto.ar_id = (0, i_coreo.read().ar_id);
    vmsto.ar_user = i_coreo.read().ar_user;
    vmsto.r_ready = i_coreo.read().r_ready;
    o_msto = vmsto;     // to trigger event

    vcorei.aw_ready = i_msti.read().aw_ready;
    vcorei.w_ready = v_w_ready;
    vcorei.b_valid = i_msti.read().b_valid && r.b_wait.read();
    vcorei.b_resp = i_msti.read().b_resp;
    vcorei.b_id = (0, i_msti.read().b_id);
    vcorei.b_user = i_msti.read().b_user;
    vcorei.ar_ready = i_msti.read().ar_ready;
    vcorei.r_valid = v_r_valid;
    vcorei.r_resp = vb_r_resp;
    vcorei.r_data = vb_line_o;
    vcorei.r_last = v_r_valid;
    vcorei.r_id = (0, i_msti.read().r_id);
    vcorei.r_user = i_msti.read().r_user;
    vcorei.ac_valid = 0;
    vcorei.ac_addr = 0;
    vcorei.ac_snoop = 0;
    vcorei.ac_prot = 0;
    vcorei.cr_ready = 0;
    vcorei.cd_ready = 0;
    o_corei = vcorei;    // to trigger event
}

void L1SerDes::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
