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

#ifndef __DEBUGGER_SRC_CPU_SYSC_PLUGIN_TYPES_RIVER_H__
#define __DEBUGGER_SRC_CPU_SYSC_PLUGIN_TYPES_RIVER_H__

#include "river_cfg.h"
#include "../ambalib/types_amba.h"
#include <systemc.h>
#include <string>
#include <iomanip>

namespace debugger {

const int CFG_TOTAL_CPU_MAX = 4;

class axi4_river_in_type {
 public:
    axi4_river_in_type(){}

    inline bool operator == (const axi4_river_in_type &rhs) const {
        return (rhs.aw_ready == aw_ready
             && rhs.w_ready == w_ready
             && rhs.b_valid == b_valid
             && rhs.b_resp == b_resp
             && rhs.b_id == b_id
             && rhs.b_user == b_user
             && rhs.ar_ready == ar_ready
             && rhs.r_valid == r_valid
             && rhs.r_resp == r_resp
             && rhs.r_data == r_data
             && rhs.r_last == r_last
             && rhs.r_id == r_id
             && rhs.r_user == r_user
             && rhs.ac_valid == ac_valid
             && rhs.ac_addr == ac_addr
             && rhs.ac_snoop == ac_snoop
             && rhs.ac_prot == ac_prot
             && rhs.cr_ready == cr_ready
             && rhs.cd_ready == cd_ready);
    }

    inline axi4_river_in_type& operator = (const axi4_river_in_type &rhs) {
        aw_ready = rhs.aw_ready;
        w_ready = rhs.w_ready;
        b_valid = rhs.b_valid;
        b_resp = rhs.b_resp;
        b_id = rhs.b_id;
        b_user = rhs.b_user;
        ar_ready = rhs.ar_ready;
        r_valid = rhs.r_valid;
        r_resp = rhs.r_resp;
        r_data = rhs.r_data;
        r_last = rhs.r_last;
        r_id = rhs.r_id;
        r_user = rhs.r_user;
        ac_valid = rhs.ac_valid;
        ac_addr = rhs.ac_addr;
        ac_snoop = rhs.ac_snoop;
        ac_prot = rhs.ac_prot;
        cr_ready = rhs.cr_ready;
        cd_ready = rhs.cd_ready;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_river_in_type &v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_ready, NAME + ".aw_ready");
        sc_trace(tf, v.w_ready, NAME + ".w_ready");
        sc_trace(tf, v.b_valid, NAME + ".b_valid");
        sc_trace(tf, v.b_resp, NAME + ".b_resp");
        sc_trace(tf, v.b_id, NAME + ".b_id");
        sc_trace(tf, v.b_user, NAME + ".b_user");
        sc_trace(tf, v.ar_ready, NAME + ".ar_ready");
        sc_trace(tf, v.r_valid, NAME + ".r_valid");
        sc_trace(tf, v.r_resp, NAME + ".r_resp");
        sc_trace(tf, v.r_data, NAME + ".r_data");
        sc_trace(tf, v.r_last, NAME + ".r_last");
        sc_trace(tf, v.r_id, NAME + ".r_id");
        sc_trace(tf, v.r_user, NAME + ".r_user");
        sc_trace(tf, v.ac_valid, NAME + ".ac_valid");
        sc_trace(tf, v.ac_addr, NAME + ".ac_addr");
        sc_trace(tf, v.ac_snoop, NAME + ".ac_snoop");
        sc_trace(tf, v.ac_prot, NAME + ".ac_prot");
        sc_trace(tf, v.cr_ready, NAME + ".cr_ready");
        sc_trace(tf, v.cd_ready, NAME + ".cd_ready");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_river_in_type const &v) {
        os << "("
        << v.aw_ready << ","
        << v.w_ready << ","
        << v.b_valid << ","
        << v.b_resp << ","
        << v.b_id << ","
        << v.b_user << ","
        << v.ar_ready << ","
        << v.r_valid << ","
        << v.r_resp << ","
        << v.r_data << ","
        << v.r_last << ","
        << v.r_id << ","
        << v.r_user << ","
        << v.ac_valid << ","
        << v.ac_addr << ","
        << v.ac_snoop << ","
        << v.ac_prot << ","
        << v.cr_ready << ","
        << v.cd_ready << ")";
        return os;
    }

 public:
    bool aw_ready;
    bool w_ready;
    bool b_valid;
    sc_uint<2> b_resp;
    sc_uint<CFG_CPU_ID_BITS> b_id;
    sc_uint<CFG_CPU_USER_BITS> b_user;
    bool ar_ready;
    bool r_valid;
    sc_uint<4> r_resp;
    sc_biguint<L1CACHE_LINE_BITS> r_data;
    bool r_last;
    sc_uint<CFG_CPU_ID_BITS> r_id;
    sc_uint<CFG_CPU_USER_BITS> r_user;
    bool ac_valid;
    sc_uint<CFG_CPU_ADDR_BITS> ac_addr;
    sc_uint<4> ac_snoop;                  // Table C3-19
    sc_uint<3> ac_prot;
    bool cr_ready;
    bool cd_ready;
};


class axi4_river_out_type {
 public:
    axi4_river_out_type(){}

    inline bool operator == (const axi4_river_out_type &rhs) const {
        return (rhs.aw_valid == aw_valid
            && rhs.aw_bits.addr == aw_bits.addr
            && rhs.aw_bits.len == aw_bits.len
            && rhs.aw_bits.size == aw_bits.size
            && rhs.aw_bits.burst == aw_bits.burst
            && rhs.aw_bits.lock == aw_bits.lock
            && rhs.aw_bits.cache == aw_bits.cache
            && rhs.aw_bits.prot == aw_bits.prot
            && rhs.aw_bits.qos == aw_bits.qos
            && rhs.aw_bits.region == aw_bits.region
            && rhs.aw_id == aw_id
            && rhs.aw_user == aw_user
            && rhs.w_valid == w_valid
            && rhs.w_data == w_data
            && rhs.w_last == w_last
            && rhs.w_strb == w_strb
            && rhs.w_user == w_user
            && rhs.b_ready == b_ready
            && rhs.ar_valid == ar_valid
            && rhs.ar_bits.addr == ar_bits.addr
            && rhs.ar_bits.len == ar_bits.len
            && rhs.ar_bits.size == ar_bits.size
            && rhs.ar_bits.burst == ar_bits.burst
            && rhs.ar_bits.lock == ar_bits.lock
            && rhs.ar_bits.cache == ar_bits.cache
            && rhs.ar_bits.prot == ar_bits.prot
            && rhs.ar_bits.qos == ar_bits.qos
            && rhs.ar_bits.region == ar_bits.region
            && rhs.ar_id == ar_id
            && rhs.ar_user == ar_user
            && rhs.r_ready == r_ready
            && rhs.ar_domain == ar_domain
            && rhs.ar_snoop == ar_snoop
            && rhs.ar_bar == ar_bar
            && rhs.aw_domain == aw_domain
            && rhs.aw_snoop == aw_snoop
            && rhs.aw_bar == aw_bar
            && rhs.ac_ready == ac_ready
            && rhs.cr_valid == cr_valid
            && rhs.cr_resp == cr_resp
            && rhs.cd_valid == cd_valid
            && rhs.cd_data == cd_data
            && rhs.cd_last == cd_last
            && rhs.rack == rack
            && rhs.wack == wack);
    }

    inline axi4_river_out_type& operator = (const axi4_river_out_type &rhs) {
        aw_valid = rhs.aw_valid;
        aw_bits.addr = rhs.aw_bits.addr;
        aw_bits.len = rhs.aw_bits.len;
        aw_bits.size = rhs.aw_bits.size;
        aw_bits.burst = rhs.aw_bits.burst;
        aw_bits.lock = rhs.aw_bits.lock;
        aw_bits.cache = rhs.aw_bits.cache;
        aw_bits.prot = rhs.aw_bits.prot;
        aw_bits.qos = rhs.aw_bits.qos;
        aw_bits.region = rhs.aw_bits.region;
        aw_id = rhs.aw_id;
        aw_user = rhs.aw_user;
        w_valid = rhs.w_valid;
        w_data = rhs.w_data;
        w_last = rhs.w_last;
        w_strb = rhs.w_strb;
        w_user = rhs.w_user;
        b_ready = rhs.b_ready;
        ar_valid = rhs.ar_valid;
        ar_bits.addr = rhs.ar_bits.addr;
        ar_bits.len = rhs.ar_bits.len;
        ar_bits.size = rhs.ar_bits.size;
        ar_bits.burst = rhs.ar_bits.burst;
        ar_bits.lock = rhs.ar_bits.lock;
        ar_bits.cache = rhs.ar_bits.cache;
        ar_bits.prot = rhs.ar_bits.prot;
        ar_bits.qos = rhs.ar_bits.qos;
        ar_bits.region = rhs.ar_bits.region;
        ar_id = rhs.ar_id;
        ar_user = rhs.ar_user;
        r_ready = rhs.r_ready;
        ar_domain = rhs.ar_domain;
        ar_snoop = rhs.ar_snoop;
        ar_bar = rhs.ar_bar;
        aw_domain = rhs.aw_domain;
        aw_snoop = rhs.aw_snoop;
        aw_bar = rhs.aw_bar;
        ac_ready = rhs.ac_ready;
        cr_valid = rhs.cr_valid;
        cr_resp = rhs.cr_resp;
        cd_valid = rhs.cd_valid;
        cd_data = rhs.cd_data;
        cd_last = rhs.cd_last;
        rack = rhs.rack;
        wack = rhs.wack;
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_river_out_type &v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_valid, NAME + ".aw_valid");
        sc_trace(tf, v.aw_bits.addr, NAME + ".aw_bits.addr");
        sc_trace(tf, v.aw_bits.len, NAME + ".aw_bits.len");
        sc_trace(tf, v.aw_bits.size, NAME + ".aw_bits.size");
        sc_trace(tf, v.aw_bits.burst, NAME + ".aw_bits.burst");
        sc_trace(tf, v.aw_bits.lock, NAME + ".aw_bits.lock");
        sc_trace(tf, v.aw_bits.cache, NAME + ".aw_bits.cache");
        sc_trace(tf, v.aw_bits.prot, NAME + ".aw_bits.prot");
        sc_trace(tf, v.aw_bits.qos, NAME + ".aw_bits.qos");
        sc_trace(tf, v.aw_bits.region, NAME + ".aw_bits.region");
        sc_trace(tf, v.aw_id, NAME + ".aw_id");
        sc_trace(tf, v.aw_user, NAME + ".aw_user");
        sc_trace(tf, v.w_valid, NAME + ".w_valid");
        sc_trace(tf, v.w_data, NAME + ".w_data");
        sc_trace(tf, v.w_last, NAME + ".w_last");
        sc_trace(tf, v.w_strb, NAME + ".w_strb");
        sc_trace(tf, v.w_user, NAME + ".w_user");
        sc_trace(tf, v.b_ready, NAME + ".b_ready");
        sc_trace(tf, v.ar_valid, NAME + ".ar_valid");
        sc_trace(tf, v.ar_bits.addr, NAME + ".ar_bits.addr");
        sc_trace(tf, v.ar_bits.len, NAME + ".ar_bits.len");
        sc_trace(tf, v.ar_bits.size, NAME + ".ar_bits.size");
        sc_trace(tf, v.ar_bits.burst, NAME + ".ar_bits.burst");
        sc_trace(tf, v.ar_bits.lock, NAME + ".ar_bits.lock");
        sc_trace(tf, v.ar_bits.cache, NAME + ".ar_bits.cache");
        sc_trace(tf, v.ar_bits.prot, NAME + ".ar_bits.prot");
        sc_trace(tf, v.ar_bits.qos, NAME + ".ar_bits.qos");
        sc_trace(tf, v.ar_bits.region, NAME + ".ar_bits.region");
        sc_trace(tf, v.ar_id, NAME + ".ar_id");
        sc_trace(tf, v.ar_user, NAME + ".ar_user");
        sc_trace(tf, v.r_ready, NAME + ".r_ready");
        sc_trace(tf, v.ar_domain, NAME + ".ar_domain");
        sc_trace(tf, v.ar_snoop, NAME + ".ar_snoop");
        sc_trace(tf, v.ar_bar, NAME + ".ar_bar");
        sc_trace(tf, v.aw_domain, NAME + ".aw_domain");
        sc_trace(tf, v.aw_snoop, NAME + ".aw_snoop");
        sc_trace(tf, v.aw_bar, NAME + ".aw_bar");
        sc_trace(tf, v.ac_ready, NAME + ".ac_ready");
        sc_trace(tf, v.cr_valid, NAME + ".cr_valid");
        sc_trace(tf, v.cr_resp, NAME + ".cr_resp");
        sc_trace(tf, v.cd_valid, NAME + ".cd_valid");
        sc_trace(tf, v.cd_data, NAME + ".cd_data");
        sc_trace(tf, v.cd_last, NAME + ".cd_last");
        sc_trace(tf, v.rack, NAME + ".rack");
        sc_trace(tf, v.wack, NAME + ".wack");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_river_out_type const &v) {
        os << "("
        << v.aw_valid << ","
        << v.aw_bits.addr << ","
        << v.aw_bits.len << ","
        << v.aw_bits.size << ","
        << v.aw_bits.burst << ","
        << v.aw_bits.lock << ","
        << v.aw_bits.cache << ","
        << v.aw_bits.prot << ","
        << v.aw_bits.qos << ","
        << v.aw_bits.region << ","
        << v.aw_id << ","
        << v.aw_user << ","
        << v.w_valid << ","
        << v.w_data << ","
        << v.w_last << ","
        << v.w_strb << ","
        << v.w_user << ","
        << v.b_ready << ","
        << v.ar_valid << ","
        << v.ar_bits.addr << ","
        << v.ar_bits.len << ","
        << v.ar_bits.size << ","
        << v.ar_bits.burst << ","
        << v.ar_bits.lock << ","
        << v.ar_bits.cache << ","
        << v.ar_bits.prot << ","
        << v.ar_bits.qos << ","
        << v.ar_bits.region << ","
        << v.ar_id << ","
        << v.ar_user << ","
        << v.r_ready << ","
        << v.ar_domain << ","
        << v.ar_snoop << ","
        << v.ar_bar << ","
        << v.aw_domain << ","
        << v.aw_snoop << ","
        << v.aw_bar << ","
        << v.ac_ready << ","
        << v.cr_valid << ","
        << v.cr_resp << ","
        << v.cd_valid << ","
        << v.cd_data << ","
        << v.cd_last << ","
        << v.rack << ","
        << v.wack << ")";
        return os;
    }

 public:
    bool aw_valid;
    axi4_meta_type aw_bits;
    sc_uint<CFG_CPU_ID_BITS> aw_id;
    sc_uint<CFG_CPU_USER_BITS> aw_user;
    bool w_valid;
    sc_biguint<L1CACHE_LINE_BITS> w_data;
    bool w_last;
    sc_uint<L1CACHE_BYTES_PER_LINE> w_strb;
    sc_uint<CFG_CPU_USER_BITS> w_user;
    bool b_ready;
    bool ar_valid;
    axi4_meta_type ar_bits;
    sc_uint<CFG_CPU_ID_BITS> ar_id;
    sc_uint<CFG_CPU_USER_BITS> ar_user;
    bool r_ready;
    sc_uint<2> ar_domain;                // 00=Non-shareable (single master in domain)
    sc_uint<4> ar_snoop;                 // Table C3-7:
    sc_uint<2> ar_bar;                   // read barrier transaction
    sc_uint<2> aw_domain;
    sc_uint<4> aw_snoop;                 // Table C3-8
    sc_uint<2> aw_bar;                   // write barrier transaction
    bool ac_ready;
    bool cr_valid;
    sc_uint<5> cr_resp;
    bool cd_valid;
    sc_biguint<L1CACHE_LINE_BITS> cd_data;
    bool cd_last;
    bool rack;
    bool wack;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_TYPES_RIVER_H__
