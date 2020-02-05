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

#include "l2_top.h"

namespace debugger {

L2Top::L2Top(sc_module_name name, bool async_reset) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("o_nrst"),
    // CPU[0] AXI4 + ACE:
    o_slvi0_aw_ready("o_slvi0_aw_ready"),
    o_slvi0_w_ready("o_slvi0_w_ready"),
    o_slvi0_b_valid("o_slvi0_b_valid"),
    o_slvi0_b_resp("o_slvi0_b_resp"),
    o_slvi0_b_id("o_slvi0_b_id"),
    o_slvi0_b_user("o_slvi0_b_user"),
    o_slvi0_ar_ready("o_slvi0_ar_ready"),
    o_slvi0_r_valid("o_slvi0_r_valid"),
    o_slvi0_r_resp("o_slvi0_r_resp"),
    o_slvi0_r_data("o_slvi0_r_data"),
    o_slvi0_r_last("o_slvi0_r_last"),
    o_slvi0_r_id("o_slvi0_r_id"),
    o_slvi0_r_user("o_slvi0_r_user"),
    o_slvi0_ac_valid("o_slvi0_ac_valid"),
    o_slvi0_ac_addr("o_slvi0_ac_addr"),
    o_slvi0_ac_snoop("o_slvi0_ac_snoop"),
    o_slvi0_ac_prot("o_slvi0_ac_prot"),
    o_slvi0_cr_ready("o_slvi0_cr_ready"),
    o_slvi0_cd_ready("o_slvi0_cd_ready"),
    i_slvo0_aw_valid("i_slvo0_aw_valid"),
    i_slvo0_aw_bits_addr("i_slvo0_aw_bits_addr"),
    i_slvo0_aw_bits_len("i_slvo0_aw_bits_len"),
    i_slvo0_aw_bits_size("i_slvo0_aw_bits_size"),
    i_slvo0_aw_bits_burst("i_slvo0_aw_bits_burst"),
    i_slvo0_aw_bits_lock("i_slvo0_aw_bits_lock"),
    i_slvo0_aw_bits_cache("i_slvo0_aw_bits_cache"),
    i_slvo0_aw_bits_prot("i_slvo0_aw_bits_prot"),
    i_slvo0_aw_bits_qos("i_slvo0_aw_bits_qos"),
    i_slvo0_aw_bits_region("i_slvo0_aw_bits_region"),
    i_slvo0_aw_id("i_slvo0_aw_id"),
    i_slvo0_aw_user("i_slvo0_aw_user"),
    i_slvo0_w_valid("i_slvo0_w_valid"),
    i_slvo0_w_data("i_slvo0_w_data"),
    i_slvo0_w_last("i_slvo0_w_last"),
    i_slvo0_w_strb("i_slvo0_w_strb"),
    i_slvo0_w_user("i_slvo0_w_user"),
    i_slvo0_b_ready("i_slvo0_b_ready"),
    i_slvo0_ar_valid("i_slvo0_ar_valid"),
    i_slvo0_ar_bits_addr("i_slvo0_ar_bits_addr"),
    i_slvo0_ar_bits_len("i_slvo0_ar_bits_len"),
    i_slvo0_ar_bits_size("i_slvo0_ar_bits_size"),
    i_slvo0_ar_bits_burst("i_slvo0_ar_bits_burst"),
    i_slvo0_ar_bits_lock("i_slvo0_ar_bits_lock"),
    i_slvo0_ar_bits_cache("i_slvo0_ar_bits_cache"),
    i_slvo0_ar_bits_prot("i_slvo0_ar_bits_prot"),
    i_slvo0_ar_bits_qos("i_slvo0_ar_bits_qos"),
    i_slvo0_ar_bits_region("i_slvo0_ar_bits_region"),
    i_slvo0_ar_id("i_slvo0_ar_id"),
    i_slvo0_ar_user("i_slvo0_ar_user"),
    i_slvo0_r_ready("i_slvo0_r_ready"),
    i_slvo0_ar_domain("i_slvo0_ar_domain"),
    i_slvo0_ar_snoop("i_slvo0_ar_snoop"),
    i_slvo0_ar_bar("i_slvo0_ar_bar"),
    i_slvo0_aw_domain("i_slvo0_aw_domain"),
    i_slvo0_aw_snoop("i_slvo0_aw_snoop"),
    i_slvo0_aw_bar("i_slvo0_aw_bar"),
    i_slvo0_ac_ready("i_slvo0_ac_ready"),
    i_slvo0_cr_valid("i_slvo0_cr_valid"),
    i_slvo0_cr_resp("i_slvo0_cr_resp"),
    i_slvo0_cd_valid("i_slvo0_cd_valid"),
    i_slvo0_cd_data("i_slvo0_cd_data"),
    i_slvo0_cd_last("i_slvo0_cd_last"),
    i_slvo0_rack("i_slvo0_rack"),
    i_slvo0_wack("i_slvo0_wack"),
    // CPU[1] AXI4 + ACE:
    o_slvi1_aw_ready("o_slvi1_aw_ready"),
    o_slvi1_w_ready("o_slvi1_w_ready"),
    o_slvi1_b_valid("o_slvi1_b_valid"),
    o_slvi1_b_resp("o_slvi1_b_resp"),
    o_slvi1_b_id("o_slvi1_b_id"),
    o_slvi1_b_user("o_slvi1_b_user"),
    o_slvi1_ar_ready("o_slvi1_ar_ready"),
    o_slvi1_r_valid("o_slvi1_r_valid"),
    o_slvi1_r_resp("o_slvi1_r_resp"),
    o_slvi1_r_data("o_slvi1_r_data"),
    o_slvi1_r_last("o_slvi1_r_last"),
    o_slvi1_r_id("o_slvi1_r_id"),
    o_slvi1_r_user("o_slvi1_r_user"),
    o_slvi1_ac_valid("o_slvi1_ac_valid"),
    o_slvi1_ac_addr("o_slvi1_ac_addr"),
    o_slvi1_ac_snoop("o_slvi1_ac_snoop"),
    o_slvi1_ac_prot("o_slvi1_ac_prot"),
    o_slvi1_cr_ready("o_slvi1_cr_ready"),
    o_slvi1_cd_ready("o_slvi1_cd_ready"),
    i_slvo1_aw_valid("i_slvo1_aw_valid"),
    i_slvo1_aw_bits_addr("i_slvo1_aw_bits_addr"),
    i_slvo1_aw_bits_len("i_slvo1_aw_bits_len"),
    i_slvo1_aw_bits_size("i_slvo1_aw_bits_size"),
    i_slvo1_aw_bits_burst("i_slvo1_aw_bits_burst"),
    i_slvo1_aw_bits_lock("i_slvo1_aw_bits_lock"),
    i_slvo1_aw_bits_cache("i_slvo1_aw_bits_cache"),
    i_slvo1_aw_bits_prot("i_slvo1_aw_bits_prot"),
    i_slvo1_aw_bits_qos("i_slvo1_aw_bits_qos"),
    i_slvo1_aw_bits_region("i_slvo1_aw_bits_region"),
    i_slvo1_aw_id("i_slvo1_aw_id"),
    i_slvo1_aw_user("i_slvo1_aw_user"),
    i_slvo1_w_valid("i_slvo1_w_valid"),
    i_slvo1_w_data("i_slvo1_w_data"),
    i_slvo1_w_last("i_slvo1_w_last"),
    i_slvo1_w_strb("i_slvo1_w_strb"),
    i_slvo1_w_user("i_slvo1_w_user"),
    i_slvo1_b_ready("i_slvo1_b_ready"),
    i_slvo1_ar_valid("i_slvo1_ar_valid"),
    i_slvo1_ar_bits_addr("i_slvo1_ar_bits_addr"),
    i_slvo1_ar_bits_len("i_slvo1_ar_bits_len"),
    i_slvo1_ar_bits_size("i_slvo1_ar_bits_size"),
    i_slvo1_ar_bits_burst("i_slvo1_ar_bits_burst"),
    i_slvo1_ar_bits_lock("i_slvo1_ar_bits_lock"),
    i_slvo1_ar_bits_cache("i_slvo1_ar_bits_cache"),
    i_slvo1_ar_bits_prot("i_slvo1_ar_bits_prot"),
    i_slvo1_ar_bits_qos("i_slvo1_ar_bits_qos"),
    i_slvo1_ar_bits_region("i_slvo1_ar_bits_region"),
    i_slvo1_ar_id("i_slvo1_ar_id"),
    i_slvo1_ar_user("i_slvo1_ar_user"),
    i_slvo1_r_ready("i_slvo1_r_ready"),
    i_slvo1_ar_domain("i_slvo1_ar_domain"),
    i_slvo1_ar_snoop("i_slvo1_ar_snoop"),
    i_slvo1_ar_bar("i_slvo1_ar_bar"),
    i_slvo1_aw_domain("i_slvo1_aw_domain"),
    i_slvo1_aw_snoop("i_slvo1_aw_snoop"),
    i_slvo1_aw_bar("i_slvo1_aw_bar"),
    i_slvo1_ac_ready("i_slvo1_ac_ready"),
    i_slvo1_cr_valid("i_slvo1_cr_valid"),
    i_slvo1_cr_resp("i_slvo1_cr_resp"),
    i_slvo1_cd_valid("i_slvo1_cd_valid"),
    i_slvo1_cd_data("i_slvo1_cd_data"),
    i_slvo1_cd_last("i_slvo1_cd_last"),
    i_slvo1_rack("i_slvo1_rack"),
    i_slvo1_wack("i_slvo1_wack"),
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
    sensitive << i_slvo0_aw_valid;
    sensitive << i_slvo0_aw_bits_addr;
    sensitive << i_slvo0_aw_bits_len;
    sensitive << i_slvo0_aw_bits_size;
    sensitive << i_slvo0_aw_bits_burst;
    sensitive << i_slvo0_aw_bits_lock;
    sensitive << i_slvo0_aw_bits_cache;
    sensitive << i_slvo0_aw_bits_prot;
    sensitive << i_slvo0_aw_bits_qos;
    sensitive << i_slvo0_aw_bits_region;
    sensitive << i_slvo0_aw_id;
    sensitive << i_slvo0_aw_user;
    sensitive << i_slvo0_w_valid;
    sensitive << i_slvo0_w_data;
    sensitive << i_slvo0_w_last;
    sensitive << i_slvo0_w_strb;
    sensitive << i_slvo0_w_user;
    sensitive << i_slvo0_b_ready;
    sensitive << i_slvo0_ar_valid;
    sensitive << i_slvo0_ar_bits_addr;
    sensitive << i_slvo0_ar_bits_len;
    sensitive << i_slvo0_ar_bits_size;
    sensitive << i_slvo0_ar_bits_burst;
    sensitive << i_slvo0_ar_bits_lock;
    sensitive << i_slvo0_ar_bits_cache;
    sensitive << i_slvo0_ar_bits_prot;
    sensitive << i_slvo0_ar_bits_qos;
    sensitive << i_slvo0_ar_bits_region;
    sensitive << i_slvo0_ar_id;
    sensitive << i_slvo0_ar_user;
    sensitive << i_slvo0_r_ready;
    sensitive << i_slvo0_ar_domain;
    sensitive << i_slvo0_ar_snoop;
    sensitive << i_slvo0_ar_bar;
    sensitive << i_slvo0_aw_domain;
    sensitive << i_slvo0_aw_snoop;
    sensitive << i_slvo0_aw_bar;
    sensitive << i_slvo0_ac_ready;
    sensitive << i_slvo0_cr_valid;
    sensitive << i_slvo0_cr_resp;
    sensitive << i_slvo0_cd_valid;
    sensitive << i_slvo0_cd_data;
    sensitive << i_slvo0_cd_last;
    sensitive << i_slvo0_rack;
    sensitive << i_slvo0_wack;
    sensitive << i_slvo1_aw_valid;
    sensitive << i_slvo1_aw_bits_addr;
    sensitive << i_slvo1_aw_bits_len;
    sensitive << i_slvo1_aw_bits_size;
    sensitive << i_slvo1_aw_bits_burst;
    sensitive << i_slvo1_aw_bits_lock;
    sensitive << i_slvo1_aw_bits_cache;
    sensitive << i_slvo1_aw_bits_prot;
    sensitive << i_slvo1_aw_bits_qos;
    sensitive << i_slvo1_aw_bits_region;
    sensitive << i_slvo1_aw_id;
    sensitive << i_slvo1_aw_user;
    sensitive << i_slvo1_w_valid;
    sensitive << i_slvo1_w_data;
    sensitive << i_slvo1_w_last;
    sensitive << i_slvo1_w_strb;
    sensitive << i_slvo1_w_user;
    sensitive << i_slvo1_b_ready;
    sensitive << i_slvo1_ar_valid;
    sensitive << i_slvo1_ar_bits_addr;
    sensitive << i_slvo1_ar_bits_len;
    sensitive << i_slvo1_ar_bits_size;
    sensitive << i_slvo1_ar_bits_burst;
    sensitive << i_slvo1_ar_bits_lock;
    sensitive << i_slvo1_ar_bits_cache;
    sensitive << i_slvo1_ar_bits_prot;
    sensitive << i_slvo1_ar_bits_qos;
    sensitive << i_slvo1_ar_bits_region;
    sensitive << i_slvo1_ar_id;
    sensitive << i_slvo1_ar_user;
    sensitive << i_slvo1_r_ready;
    sensitive << i_slvo1_ar_domain;
    sensitive << i_slvo1_ar_snoop;
    sensitive << i_slvo1_ar_bar;
    sensitive << i_slvo1_aw_domain;
    sensitive << i_slvo1_aw_snoop;
    sensitive << i_slvo1_aw_bar;
    sensitive << i_slvo1_ac_ready;
    sensitive << i_slvo1_cr_valid;
    sensitive << i_slvo1_cr_resp;
    sensitive << i_slvo1_cd_valid;
    sensitive << i_slvo1_cd_data;
    sensitive << i_slvo1_cd_last;
    sensitive << i_slvo1_rack;
    sensitive << i_slvo1_wack;
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
    sensitive << r.req_addr;
    sensitive << r.req_len;
    sensitive << r.b_wait;
    sensitive << r.state;
    sensitive << r.line;
    sensitive << r.wstrb;

    SC_METHOD(registers);
    sensitive << i_clk.posedge_event();
}

L2Top::~L2Top() {
}

void L2Top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
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
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.b_wait, pn + ".r_b_wait");
    }
}

void L2Top::comb() {
    bool v_req_mem_ready;
    sc_uint<BUS_ADDR_WIDTH> vb_req_addr;
    sc_uint<BUS_DATA_WIDTH> vb_r_data;
    sc_biguint<DCACHE_LINE_BITS> vb_line_cached_o;
    sc_biguint<DCACHE_LINE_BITS> vb_line_uncached_o;
    sc_biguint<DCACHE_LINE_BITS> vb_line_o;
    sc_uint<4> vb_r_resp;
    bool v_r_valid;
    bool v_w_valid;
    bool v_w_ready;
    sc_uint<8> vb_len;

    v_req_mem_ready = 0;
    v_r_valid = 0;
    v_w_valid = 0;
    v_w_ready = 0;
    vb_len = 0;

    vb_r_data = i_msti_r_data.read();
    vb_line_cached_o = (vb_r_data, r.line.read()(DCACHE_LINE_BITS-1, BUS_DATA_WIDTH));
    vb_line_uncached_o = (0, vb_r_data);
    vb_line_o = 0;

    if (i_slvo0_b_ready.read() == 1) {
        v.b_wait = 0;
    }

    switch (r.state.read()) {
    case State_Idle:
        v_req_mem_ready = 1;
        break;
    case State_ReadUncached:
        if (i_msti_r_valid.read() == 1) {
            v_r_valid = 1;
            vb_line_o = vb_line_uncached_o;
            v_req_mem_ready = 1;
        }
        break;
    case State_ReadCached:
        if (i_msti_r_valid.read() == 1) {
            vb_line_o = vb_line_cached_o;
            v.line = vb_line_cached_o;
            if (r.req_len.read() == 0) {
                v_r_valid = 1;
                v_req_mem_ready = 1;
            } else {
                v.req_len = r.req_len.read() - 1;
                vb_req_addr = r.req_addr.read() + 8;
                v.req_addr = vb_req_addr;
            }
        }
        break;
    case State_WriteUncached:
        v_w_valid = 1;
        if (i_msti_w_ready.read()) {
            v_w_ready = 1;
            v.b_wait = 1;
            v_req_mem_ready = 1;
        }
        break;
    case State_WriteCached:
        v_w_valid = 1;
        if (i_msti_w_ready.read()) {
            v.line = (0, r.line.read()(L1CACHE_LINE_BITS, BUS_DATA_WIDTH));
            v.wstrb = (0, r.wstrb.read()(L1CACHE_BYTES_PER_LINE, BUS_DATA_BYTES));
            if (r.req_len.read() == 0) {
                v_w_ready = 1;
                v.b_wait = 1;
                v_req_mem_ready = 1;
            } else {
                v.req_len = r.req_len.read() - 1;
                vb_req_addr = r.req_addr.read() + 8;
                v.req_addr = vb_req_addr;
            }
        }
        break;
    default:;
    }

    if (v_req_mem_ready == 1) {
        if (i_slvo0_ar_valid.read() && i_msti_ar_ready.read()) {
            v.req_addr = i_slvo0_ar_bits_addr.read();
            if (i_slvo0_ar_bits_cache.read()[0] == 1) {         // cached:
                v.state = State_ReadCached;
                vb_len = L1CACHE_BURST_LEN - 1;
            } else {                                            // uncached:
                v.state = State_ReadUncached;
                if (i_slvo0_ar_bits_prot.read()[2] == 1) {      // instruction
                    vb_len = 1;
                } else {                                        // data
                    vb_len = 0;
                }

            }
        } else if (i_slvo0_aw_valid.read() && i_msti_aw_ready.read()) {
            v.req_addr = i_slvo0_aw_bits_addr.read();
            v.line = i_slvo0_w_data.read();                     // Undocumented feature of RIVER
            if (i_slvo0_aw_bits_cache.read()[0] == 1) {         // cached (data only):
                v.state = State_WriteCached;
                v.wstrb = ~0ul;
                vb_len = L1CACHE_BURST_LEN - 1;
            } else {                                            // uncached (data only):
                v.state = State_WriteUncached;
                v.wstrb = i_slvo0_w_strb.read();
                vb_len = 0;
            }
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

    o_msto_aw_valid = i_slvo0_aw_valid;
    o_msto_aw_bits_addr = i_slvo0_aw_bits_addr;
    o_msto_aw_bits_len = vb_len;              // burst len = len[7:0] + 1
    o_msto_aw_bits_size = 0x3;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto_aw_bits_burst = 0x1;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    o_msto_aw_bits_lock = i_slvo0_aw_bits_lock;
    o_msto_aw_bits_cache = i_slvo0_aw_bits_cache;
    o_msto_aw_bits_prot = i_slvo0_aw_bits_prot;
    o_msto_aw_bits_qos = i_slvo0_aw_bits_qos;
    o_msto_aw_bits_region = i_slvo0_aw_bits_qos;
    o_msto_aw_id = i_slvo0_aw_id;
    o_msto_aw_user = i_slvo0_aw_user;
    o_msto_w_valid = v_w_valid;
    o_msto_w_last = v_w_valid;
    o_msto_w_data = r.line.read()(BUS_DATA_WIDTH-1, 0).to_uint64();
    o_msto_w_strb = r.wstrb.read()(BUS_DATA_BYTES-1, 0);
    o_msto_w_user = i_slvo0_w_user;
    o_msto_b_ready = i_slvo0_b_ready;
    o_msto_ar_valid = i_slvo0_ar_valid;
    o_msto_ar_bits_addr = i_slvo0_ar_bits_addr;
    o_msto_ar_bits_len = vb_len;              // burst len = len[7:0] + 1
    o_msto_ar_bits_size = 0x3;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto_ar_bits_burst = 0x1;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    o_msto_ar_bits_lock = i_slvo0_ar_bits_lock;
    o_msto_ar_bits_cache = i_slvo0_ar_bits_cache;
    o_msto_ar_bits_prot = i_slvo0_ar_bits_prot;
    o_msto_ar_bits_qos = i_slvo0_ar_bits_qos;
    o_msto_ar_bits_region = i_slvo0_ar_bits_region;
    o_msto_ar_id = i_slvo0_ar_id;
    o_msto_ar_user = i_slvo0_ar_user;
    o_msto_r_ready = i_slvo0_r_ready;

    o_slvi0_aw_ready = i_msti_aw_ready;
    o_slvi0_w_ready = v_w_ready;
    o_slvi0_b_valid = i_msti_b_valid.read() && r.b_wait.read();
    o_slvi0_b_resp = i_msti_b_resp.read();
    o_slvi0_b_id = i_msti_b_id;
    o_slvi0_b_user = i_msti_b_user;
    o_slvi0_ar_ready = i_msti_ar_ready;
    o_slvi0_r_valid = v_r_valid;
    o_slvi0_r_resp = vb_r_resp;
    o_slvi0_r_data = vb_line_o;
    o_slvi0_r_last = v_r_valid;
    o_slvi0_r_id = i_msti_r_id;
    o_slvi0_r_user = i_msti_r_user;
    o_slvi0_ac_valid = 0;
    o_slvi0_ac_addr = 0;
    o_slvi0_ac_snoop = 0;
    o_slvi0_ac_prot = 0;
    o_slvi0_cr_ready = 0;
    o_slvi0_cd_ready = 0;
}

void L2Top::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
