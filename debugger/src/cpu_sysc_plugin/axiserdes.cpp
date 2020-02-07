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

#include "axiserdes.h"

namespace debugger {

AxiSerDes::AxiSerDes(sc_module_name name, bool async_reset) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("o_nrst"),
    // CPU[0] AXI4 + ACE:
    o_corei_aw_ready("o_corei_aw_ready"),
    o_corei_w_ready("o_corei_w_ready"),
    o_corei_b_valid("o_corei_b_valid"),
    o_corei_b_resp("o_corei_b_resp"),
    o_corei_b_id("o_corei_b_id"),
    o_corei_b_user("o_corei_b_user"),
    o_corei_ar_ready("o_corei_ar_ready"),
    o_corei_r_valid("o_corei_r_valid"),
    o_corei_r_resp("o_corei_r_resp"),
    o_corei_r_data("o_corei_r_data"),
    o_corei_r_last("o_corei_r_last"),
    o_corei_r_id("o_corei_r_id"),
    o_corei_r_user("o_corei_r_user"),
    o_corei_ac_valid("o_corei_ac_valid"),
    o_corei_ac_addr("o_corei_ac_addr"),
    o_corei_ac_snoop("o_corei_ac_snoop"),
    o_corei_ac_prot("o_corei_ac_prot"),
    o_corei_cr_ready("o_corei_cr_ready"),
    o_corei_cd_ready("o_corei_cd_ready"),
    i_coreo_aw_valid("i_coreo_aw_valid"),
    i_coreo_aw_bits_addr("i_coreo_aw_bits_addr"),
    i_coreo_aw_bits_len("i_coreo_aw_bits_len"),
    i_coreo_aw_bits_size("i_coreo_aw_bits_size"),
    i_coreo_aw_bits_burst("i_coreo_aw_bits_burst"),
    i_coreo_aw_bits_lock("i_coreo_aw_bits_lock"),
    i_coreo_aw_bits_cache("i_coreo_aw_bits_cache"),
    i_coreo_aw_bits_prot("i_coreo_aw_bits_prot"),
    i_coreo_aw_bits_qos("i_coreo_aw_bits_qos"),
    i_coreo_aw_bits_region("i_coreo_aw_bits_region"),
    i_coreo_aw_id("i_coreo_aw_id"),
    i_coreo_aw_user("i_coreo_aw_user"),
    i_coreo_w_valid("i_coreo_w_valid"),
    i_coreo_w_data("i_coreo_w_data"),
    i_coreo_w_last("i_coreo_w_last"),
    i_coreo_w_strb("i_coreo_w_strb"),
    i_coreo_w_user("i_coreo_w_user"),
    i_coreo_b_ready("i_coreo_b_ready"),
    i_coreo_ar_valid("i_coreo_ar_valid"),
    i_coreo_ar_bits_addr("i_coreo_ar_bits_addr"),
    i_coreo_ar_bits_len("i_coreo_ar_bits_len"),
    i_coreo_ar_bits_size("i_coreo_ar_bits_size"),
    i_coreo_ar_bits_burst("i_coreo_ar_bits_burst"),
    i_coreo_ar_bits_lock("i_coreo_ar_bits_lock"),
    i_coreo_ar_bits_cache("i_coreo_ar_bits_cache"),
    i_coreo_ar_bits_prot("i_coreo_ar_bits_prot"),
    i_coreo_ar_bits_qos("i_coreo_ar_bits_qos"),
    i_coreo_ar_bits_region("i_coreo_ar_bits_region"),
    i_coreo_ar_id("i_coreo_ar_id"),
    i_coreo_ar_user("i_coreo_ar_user"),
    i_coreo_r_ready("i_coreo_r_ready"),
    i_coreo_ar_domain("i_coreo_ar_domain"),
    i_coreo_ar_snoop("i_coreo_ar_snoop"),
    i_coreo_ar_bar("i_coreo_ar_bar"),
    i_coreo_aw_domain("i_coreo_aw_domain"),
    i_coreo_aw_snoop("i_coreo_aw_snoop"),
    i_coreo_aw_bar("i_coreo_aw_bar"),
    i_coreo_ac_ready("i_coreo_ac_ready"),
    i_coreo_cr_valid("i_coreo_cr_valid"),
    i_coreo_cr_resp("i_coreo_cr_resp"),
    i_coreo_cd_valid("i_coreo_cd_valid"),
    i_coreo_cd_data("i_coreo_cd_data"),
    i_coreo_cd_last("i_coreo_cd_last"),
    i_coreo_rack("i_coreo_rack"),
    i_coreo_wack("i_coreo_wack"),
    // Master interface:
    i_msti_aw_ready("i_msti_aw_ready"),
    i_msti_w_ready("i_msti_w_ready"),
    i_msti_b_valid("i_msti_b_valid"),
    i_msti_b_resp("i_msti_b_resp"),
    i_msti_b_id("i_msti_b_id"),
    i_msti_b_user("i_msti_b_user"),
    i_msti_ar_ready("i_msti_ar_ready"),
    i_msti_r_valid("i_msti_r_valid"),
    i_msti_r_resp("i_msti_r_resp"),
    i_msti_r_data("i_msti_r_data"),
    i_msti_r_last("i_msti_r_last"),
    i_msti_r_id("i_msti_r_id"),
    i_msti_r_user("i_msti_r_user"),
    o_msto_aw_valid("o_msto_aw_valid"),
    o_msto_aw_bits_addr("o_msto_aw_bits_addr"),
    o_msto_aw_bits_len("o_msto_aw_bits_len"),
    o_msto_aw_bits_size("o_msto_aw_bits_size"),
    o_msto_aw_bits_burst("o_msto_aw_bits_burst"),
    o_msto_aw_bits_lock("o_msto_aw_bits_lock"),
    o_msto_aw_bits_cache("o_msto_aw_bits_cache"),
    o_msto_aw_bits_prot("o_msto_aw_bits_prot"),
    o_msto_aw_bits_qos("o_msto_aw_bits_qos"),
    o_msto_aw_bits_region("o_msto_aw_bits_region"),
    o_msto_aw_id("o_msto_aw_id"),
    o_msto_aw_user("o_msto_aw_user"),
    o_msto_w_valid("o_msto_w_valid"),
    o_msto_w_data("o_msto_w_data"),
    o_msto_w_last("o_msto_w_last"),
    o_msto_w_strb("o_msto_w_strb"),
    o_msto_w_user("o_msto_w_user"),
    o_msto_b_ready("o_msto_b_ready"),
    o_msto_ar_valid("o_msto_ar_valid"),
    o_msto_ar_bits_addr("o_msto_ar_bits_addr"),
    o_msto_ar_bits_len("o_msto_ar_bits_len"),
    o_msto_ar_bits_size("o_msto_ar_bits_size"),
    o_msto_ar_bits_burst("o_msto_ar_bits_burst"),
    o_msto_ar_bits_lock("o_msto_ar_bits_lock"),
    o_msto_ar_bits_cache("o_msto_ar_bits_cache"),
    o_msto_ar_bits_prot("o_msto_ar_bits_prot"),
    o_msto_ar_bits_qos("o_msto_ar_bits_qos"),
    o_msto_ar_bits_region("o_msto_ar_bits_region"),
    o_msto_ar_id("o_msto_ar_id"),
    o_msto_ar_user("o_msto_ar_user"),
    o_msto_r_ready("o_msto_r_ready") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_coreo_aw_valid;
    sensitive << i_coreo_aw_bits_addr;
    sensitive << i_coreo_aw_bits_len;
    sensitive << i_coreo_aw_bits_size;
    sensitive << i_coreo_aw_bits_burst;
    sensitive << i_coreo_aw_bits_lock;
    sensitive << i_coreo_aw_bits_cache;
    sensitive << i_coreo_aw_bits_prot;
    sensitive << i_coreo_aw_bits_qos;
    sensitive << i_coreo_aw_bits_region;
    sensitive << i_coreo_aw_id;
    sensitive << i_coreo_aw_user;
    sensitive << i_coreo_w_valid;
    sensitive << i_coreo_w_data;
    sensitive << i_coreo_w_last;
    sensitive << i_coreo_w_strb;
    sensitive << i_coreo_w_user;
    sensitive << i_coreo_b_ready;
    sensitive << i_coreo_ar_valid;
    sensitive << i_coreo_ar_bits_addr;
    sensitive << i_coreo_ar_bits_len;
    sensitive << i_coreo_ar_bits_size;
    sensitive << i_coreo_ar_bits_burst;
    sensitive << i_coreo_ar_bits_lock;
    sensitive << i_coreo_ar_bits_cache;
    sensitive << i_coreo_ar_bits_prot;
    sensitive << i_coreo_ar_bits_qos;
    sensitive << i_coreo_ar_bits_region;
    sensitive << i_coreo_ar_id;
    sensitive << i_coreo_ar_user;
    sensitive << i_coreo_r_ready;
    sensitive << i_coreo_ar_domain;
    sensitive << i_coreo_ar_snoop;
    sensitive << i_coreo_ar_bar;
    sensitive << i_coreo_aw_domain;
    sensitive << i_coreo_aw_snoop;
    sensitive << i_coreo_aw_bar;
    sensitive << i_coreo_ac_ready;
    sensitive << i_coreo_cr_valid;
    sensitive << i_coreo_cr_resp;
    sensitive << i_coreo_cd_valid;
    sensitive << i_coreo_cd_data;
    sensitive << i_coreo_cd_last;
    sensitive << i_coreo_rack;
    sensitive << i_coreo_wack;
    sensitive << i_msti_aw_ready;
    sensitive << i_msti_w_ready;
    sensitive << i_msti_b_valid;
    sensitive << i_msti_b_resp;
    sensitive << i_msti_b_id;
    sensitive << i_msti_b_user;
    sensitive << i_msti_ar_ready;
    sensitive << i_msti_r_valid;
    sensitive << i_msti_r_resp;
    sensitive << i_msti_r_data;
    sensitive << i_msti_r_last;
    sensitive << i_msti_r_id;
    sensitive << i_msti_r_user;
    sensitive << r.req_len;
    sensitive << r.b_wait;
    sensitive << r.state;
    sensitive << r.line;
    sensitive << r.wstrb;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

AxiSerDes::~AxiSerDes() {
}

void AxiSerDes::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_msto_aw_valid, o_msto_aw_valid.name());
        sc_trace(o_vcd, o_msto_aw_bits_addr, o_msto_aw_bits_addr.name());
        sc_trace(o_vcd, o_msto_aw_bits_len, o_msto_aw_bits_len.name());
        sc_trace(o_vcd, o_msto_aw_bits_size, o_msto_aw_bits_size.name());
        sc_trace(o_vcd, o_msto_aw_bits_burst, o_msto_aw_bits_burst.name());
        sc_trace(o_vcd, o_msto_aw_bits_lock, o_msto_aw_bits_lock.name());
        sc_trace(o_vcd, o_msto_aw_bits_cache, o_msto_aw_bits_cache.name());
        sc_trace(o_vcd, o_msto_aw_bits_prot, o_msto_aw_bits_prot.name());
        sc_trace(o_vcd, o_msto_aw_bits_qos, o_msto_aw_bits_qos.name());
        sc_trace(o_vcd, o_msto_aw_bits_region, o_msto_aw_bits_region.name());
        sc_trace(o_vcd, o_msto_aw_id, o_msto_aw_id.name());
        sc_trace(o_vcd, o_msto_aw_user, o_msto_aw_user.name());
        sc_trace(o_vcd, o_msto_w_valid, o_msto_w_valid.name());
        sc_trace(o_vcd, o_msto_w_data, o_msto_w_data.name());
        sc_trace(o_vcd, o_msto_w_last, o_msto_w_last.name());
        sc_trace(o_vcd, o_msto_w_strb, o_msto_w_strb.name());
        sc_trace(o_vcd, o_msto_w_user, o_msto_w_user.name());
        sc_trace(o_vcd, o_msto_b_ready, o_msto_b_ready.name());
        sc_trace(o_vcd, o_msto_ar_valid, o_msto_ar_valid.name());
        sc_trace(o_vcd, o_msto_ar_bits_addr, o_msto_ar_bits_addr.name());
        sc_trace(o_vcd, o_msto_ar_bits_len, o_msto_ar_bits_len.name());
        sc_trace(o_vcd, o_msto_ar_bits_size, o_msto_ar_bits_size.name());
        sc_trace(o_vcd, o_msto_ar_bits_burst, o_msto_ar_bits_burst.name());
        sc_trace(o_vcd, o_msto_ar_bits_lock, o_msto_ar_bits_lock.name());
        sc_trace(o_vcd, o_msto_ar_bits_cache, o_msto_ar_bits_cache.name());
        sc_trace(o_vcd, o_msto_ar_bits_prot, o_msto_ar_bits_prot.name());
        sc_trace(o_vcd, o_msto_ar_bits_qos, o_msto_ar_bits_qos.name());
        sc_trace(o_vcd, o_msto_ar_bits_region, o_msto_ar_bits_region.name());
        sc_trace(o_vcd, o_msto_ar_id, o_msto_ar_id.name());
        sc_trace(o_vcd, o_msto_ar_user, o_msto_ar_user.name());
        sc_trace(o_vcd, o_msto_r_ready, o_msto_r_ready.name());
        sc_trace(o_vcd, i_msti_aw_ready, i_msti_aw_ready.name());
        sc_trace(o_vcd, i_msti_w_ready, i_msti_w_ready.name());
        sc_trace(o_vcd, i_msti_b_valid, i_msti_b_valid.name());
        sc_trace(o_vcd, i_msti_b_resp, i_msti_b_resp.name());
        sc_trace(o_vcd, i_msti_b_id, i_msti_b_id.name());
        sc_trace(o_vcd, i_msti_b_user, i_msti_b_user.name());
        sc_trace(o_vcd, i_msti_ar_ready, i_msti_ar_ready.name());
        sc_trace(o_vcd, i_msti_r_valid, i_msti_r_valid.name());
        sc_trace(o_vcd, i_msti_r_resp, i_msti_r_resp.name());
        sc_trace(o_vcd, i_msti_r_data, i_msti_r_data.name());
        sc_trace(o_vcd, i_msti_r_last, i_msti_r_last.name());
        sc_trace(o_vcd, i_msti_r_id, i_msti_r_id.name());
        sc_trace(o_vcd, i_msti_r_user, i_msti_r_user.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.b_wait, pn + ".r_b_wait");
    }
}

sc_uint<8> AxiSerDes::size2len(sc_uint<3> size) {
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

void AxiSerDes::comb() {
    bool v_req_mem_ready;
    sc_uint<BUS_DATA_WIDTH> vb_r_data;
    sc_biguint<DCACHE_LINE_BITS> vb_line_o;
    sc_uint<4> vb_r_resp;
    bool v_r_valid;
    bool v_w_valid;
    bool v_w_last;
    bool v_w_ready;
    sc_uint<8> vb_len;

    v_req_mem_ready = 0;
    v_r_valid = 0;
    v_w_valid = 0;
    v_w_last = 0;
    v_w_ready = 0;
    vb_len = 0;

    vb_r_data = i_msti_r_data.read();
    vb_line_o = r.line.read();
    for (int i = 0; i < L1CACHE_BURST_LEN; i++) {
        if (r.rmux.read()[i] == 1) {
            vb_line_o((i+1)*BUS_DATA_WIDTH-1, i*BUS_DATA_WIDTH) = vb_r_data;
        }
    }

    if (i_coreo_b_ready.read() == 1) {
        v.b_wait = 0;
    }

    switch (r.state.read()) {
    case State_Idle:
        v_req_mem_ready = 1;
        break;
    case State_Read:
        if (i_msti_r_valid.read() == 1) {
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
        if (i_msti_w_ready.read()) {
            v.line = (0, r.line.read()(L1CACHE_LINE_BITS-1, BUS_DATA_WIDTH));
            v.wstrb = (0, r.wstrb.read()(L1CACHE_BYTES_PER_LINE-1, BUS_DATA_BYTES));
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
        if (i_coreo_ar_valid.read() && i_msti_ar_ready.read()) {
            v.state = State_Read;
            v.rmux = 1;
            vb_len = size2len(i_coreo_ar_bits_size.read());
        } else if (i_coreo_aw_valid.read() && i_msti_aw_ready.read()) {
            v.line = i_coreo_w_data.read();                     // Undocumented RIVER (Axi-lite feature)
            v.wstrb = i_coreo_w_strb.read();
            v.state = State_Write;
            vb_len = size2len(i_coreo_aw_bits_size.read());
        } else {
            v.state = State_Idle;
        }
        v.req_len = vb_len;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    vb_r_resp(3, 2) = 0;
    vb_r_resp(1, 0) = i_msti_r_resp.read(); // OKAY

    o_msto_aw_valid = i_coreo_aw_valid;
    o_msto_aw_bits_addr = i_coreo_aw_bits_addr;
    o_msto_aw_bits_len = vb_len;              // burst len = len[7:0] + 1
    o_msto_aw_bits_size = 0x3;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto_aw_bits_burst = 0x1;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    o_msto_aw_bits_lock = i_coreo_aw_bits_lock;
    o_msto_aw_bits_cache = i_coreo_aw_bits_cache;
    o_msto_aw_bits_prot = i_coreo_aw_bits_prot;
    o_msto_aw_bits_qos = i_coreo_aw_bits_qos;
    o_msto_aw_bits_region = i_coreo_aw_bits_region;
    o_msto_aw_id = i_coreo_aw_id;
    o_msto_aw_user = i_coreo_aw_user;
    o_msto_w_valid = v_w_valid;
    o_msto_w_last = v_w_last;
    o_msto_w_data = r.line.read()(BUS_DATA_WIDTH-1, 0).to_uint64();
    o_msto_w_strb = r.wstrb.read()(BUS_DATA_BYTES-1, 0);
    o_msto_w_user = i_coreo_w_user;
    o_msto_b_ready = i_coreo_b_ready;
    o_msto_ar_valid = i_coreo_ar_valid;
    o_msto_ar_bits_addr = i_coreo_ar_bits_addr;
    o_msto_ar_bits_len = vb_len;              // burst len = len[7:0] + 1
    o_msto_ar_bits_size = 0x3;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto_ar_bits_burst = 0x1;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    o_msto_ar_bits_lock = i_coreo_ar_bits_lock;
    o_msto_ar_bits_cache = i_coreo_ar_bits_cache;
    o_msto_ar_bits_prot = i_coreo_ar_bits_prot;
    o_msto_ar_bits_qos = i_coreo_ar_bits_qos;
    o_msto_ar_bits_region = i_coreo_ar_bits_region;
    o_msto_ar_id = i_coreo_ar_id;
    o_msto_ar_user = i_coreo_ar_user;
    o_msto_r_ready = i_coreo_r_ready;

    o_corei_aw_ready = i_msti_aw_ready;
    o_corei_w_ready = v_w_ready;
    o_corei_b_valid = i_msti_b_valid.read() && r.b_wait.read();
    o_corei_b_resp = i_msti_b_resp.read();
    o_corei_b_id = i_msti_b_id;
    o_corei_b_user = i_msti_b_user;
    o_corei_ar_ready = i_msti_ar_ready;
    o_corei_r_valid = v_r_valid;
    o_corei_r_resp = vb_r_resp;
    o_corei_r_data = vb_line_o;
    o_corei_r_last = v_r_valid;
    o_corei_r_id = i_msti_r_id;
    o_corei_r_user = i_msti_r_user;
    o_corei_ac_valid = 0;
    o_corei_ac_addr = 0;
    o_corei_ac_snoop = 0;
    o_corei_ac_prot = 0;
    o_corei_cr_ready = 0;
    o_corei_cd_ready = 0;
}

void AxiSerDes::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
