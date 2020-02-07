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

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

}

void L2Top::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
