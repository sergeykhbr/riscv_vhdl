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

/** CPU output: L1 cache inerface */
class axi4_l1_in_type {
 public:
    axi4_l1_in_type() {
        aw_ready = 0;
        w_ready = 0;
        b_valid = 0;
        b_resp = 0;
        b_id = 0;
        b_user = 0;
        ar_ready = 0;
        r_valid = 0;
        r_resp = 0;
        r_data = 0;
        r_last = 0;
        r_id = 0;
        r_user = 0;
        ac_valid = 0;
        ac_addr = 0;
        ac_snoop = 0;
        ac_prot = 0;
        cr_ready = 1;
        cd_ready = 1;
    }

    inline bool operator == (const axi4_l1_in_type &rhs) const {
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

    inline axi4_l1_in_type& operator = (const axi4_l1_in_type &rhs) {
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
                                const axi4_l1_in_type &v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_ready, NAME + "_aw_ready");
        sc_trace(tf, v.w_ready, NAME + "_w_ready");
        sc_trace(tf, v.b_valid, NAME + "_b_valid");
        sc_trace(tf, v.b_resp, NAME + "_b_resp");
        sc_trace(tf, v.b_id, NAME + "_b_id");
        sc_trace(tf, v.b_user, NAME + "_b_user");
        sc_trace(tf, v.ar_ready, NAME + "_ar_ready");
        sc_trace(tf, v.r_valid, NAME + "_r_valid");
        sc_trace(tf, v.r_resp, NAME + "_r_resp");
        sc_trace(tf, v.r_data, NAME + "_r_data");
        sc_trace(tf, v.r_last, NAME + "_r_last");
        sc_trace(tf, v.r_id, NAME + "_r_id");
        sc_trace(tf, v.r_user, NAME + "_r_user");
        sc_trace(tf, v.ac_valid, NAME + "_ac_valid");
        sc_trace(tf, v.ac_addr, NAME + "_ac_addr");
        sc_trace(tf, v.ac_snoop, NAME + "_ac_snoop");
        sc_trace(tf, v.ac_prot, NAME + "_ac_prot");
        sc_trace(tf, v.cr_ready, NAME + "_cr_ready");
        sc_trace(tf, v.cd_ready, NAME + "_cd_ready");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_l1_in_type const &v) {
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


class axi4_l1_out_type {
 public:
    axi4_l1_out_type() {
        aw_valid = 0;
        aw_bits.addr = 0;
        aw_bits.len = 0;
        aw_bits.size = 0;
        aw_bits.burst = 0;
        aw_bits.lock = 0;
        aw_bits.cache = 0;
        aw_bits.prot = 0;
        aw_bits.qos = 0;
        aw_bits.region = 0;
        aw_id = 0;
        aw_user = 0;
        w_valid = 0;
        w_data = 0;
        w_last = 0;
        w_strb = 0;
        w_user = 0;
        b_ready = 0;
        ar_valid = 0;
        ar_bits.addr = 0;
        ar_bits.len = 0;
        ar_bits.size = 0;
        ar_bits.burst = 0;
        ar_bits.lock = 0;
        ar_bits.cache = 0;
        ar_bits.prot = 0;
        ar_bits.qos = 0;
        ar_bits.region = 0;
        ar_id = 0;
        ar_user = 0;
        r_ready = 0;
        ar_domain = 0;
        ar_snoop = 0;
        ar_bar = 0;
        aw_domain = 0;
        aw_snoop = 0;
        aw_bar = 0;
        ac_ready = 1;
        cr_valid = 1;
        cr_resp = 0;
        cd_valid = 0;
        cd_data = 0;
        cd_last = 0;
        rack = 0;
        wack = 0;
    }

    inline bool operator == (const axi4_l1_out_type &rhs) const {
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

    inline axi4_l1_out_type& operator = (const axi4_l1_out_type &rhs) {
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
                                const axi4_l1_out_type &v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_valid, NAME + "_aw_valid");
        sc_trace(tf, v.aw_bits.addr, NAME + "_aw_bits_addr");
        sc_trace(tf, v.aw_bits.len, NAME + "_aw_bits_len");
        sc_trace(tf, v.aw_bits.size, NAME + "_aw_bits_size");
        sc_trace(tf, v.aw_bits.burst, NAME + "_aw_bits_burst");
        sc_trace(tf, v.aw_bits.lock, NAME + "_aw_bits_lock");
        sc_trace(tf, v.aw_bits.cache, NAME + "_aw_bits_cache");
        sc_trace(tf, v.aw_bits.prot, NAME + "_aw_bits_prot");
        sc_trace(tf, v.aw_bits.qos, NAME + "_aw_bits_qos");
        sc_trace(tf, v.aw_bits.region, NAME + "_aw_bits_region");
        sc_trace(tf, v.aw_id, NAME + "_aw_id");
        sc_trace(tf, v.aw_user, NAME + "_aw_user");
        sc_trace(tf, v.w_valid, NAME + "_w_valid");
        sc_trace(tf, v.w_data, NAME + "_w_data");
        sc_trace(tf, v.w_last, NAME + "_w_last");
        sc_trace(tf, v.w_strb, NAME + "_w_strb");
        sc_trace(tf, v.w_user, NAME + "_w_user");
        sc_trace(tf, v.b_ready, NAME + "_b_ready");
        sc_trace(tf, v.ar_valid, NAME + "_ar_valid");
        sc_trace(tf, v.ar_bits.addr, NAME + "_ar_bits_addr");
        sc_trace(tf, v.ar_bits.len, NAME + "_ar_bits_len");
        sc_trace(tf, v.ar_bits.size, NAME + "_ar_bits_size");
        sc_trace(tf, v.ar_bits.burst, NAME + "_ar_bits_burst");
        sc_trace(tf, v.ar_bits.lock, NAME + "_ar_bits_lock");
        sc_trace(tf, v.ar_bits.cache, NAME + "_ar_bits_cache");
        sc_trace(tf, v.ar_bits.prot, NAME + "_ar_bits_prot");
        sc_trace(tf, v.ar_bits.qos, NAME + "_ar_bits_qos");
        sc_trace(tf, v.ar_bits.region, NAME + "_ar_bits_region");
        sc_trace(tf, v.ar_id, NAME + "_ar_id");
        sc_trace(tf, v.ar_user, NAME + "_ar_user");
        sc_trace(tf, v.r_ready, NAME + "_r_ready");
        sc_trace(tf, v.ar_domain, NAME + "_ar_domain");
        sc_trace(tf, v.ar_snoop, NAME + "_ar_snoop");
        sc_trace(tf, v.ar_bar, NAME + "_ar_bar");
        sc_trace(tf, v.aw_domain, NAME + "_aw_domain");
        sc_trace(tf, v.aw_snoop, NAME + "_aw_snoop");
        sc_trace(tf, v.aw_bar, NAME + "_aw_bar");
        sc_trace(tf, v.ac_ready, NAME + "_ac_ready");
        sc_trace(tf, v.cr_valid, NAME + "_cr_valid");
        sc_trace(tf, v.cr_resp, NAME + "_cr_resp");
        sc_trace(tf, v.cd_valid, NAME + "_cd_valid");
        sc_trace(tf, v.cd_data, NAME + "_cd_data");
        sc_trace(tf, v.cd_last, NAME + "_cd_last");
        sc_trace(tf, v.rack, NAME + "_rack");
        sc_trace(tf, v.wack, NAME + "_wack");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_l1_out_type const &v) {
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
    sc_uint<3> aw_snoop;                 // Table C3-8
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


/** L2 cache inerface */
class axi4_l2_in_type {
 public:
    axi4_l2_in_type() {
        aw_ready = 0;
        w_ready = 0;
        b_valid = 0;
        b_resp = 0;
        b_id = 0;
        b_user = 0;
        ar_ready = 0;
        r_valid = 0;
        r_resp = 0;
        r_data = 0;
        r_last = 0;
        r_id = 0;
        r_user = 0;
    }

    inline bool operator == (const axi4_l2_in_type &rhs) const {
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
             && rhs.r_user == r_user);
    }

    inline axi4_l2_in_type& operator = (const axi4_l2_in_type &rhs) {
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
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_l2_in_type &v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_ready, NAME + "_aw_ready");
        sc_trace(tf, v.w_ready, NAME + "_w_ready");
        sc_trace(tf, v.b_valid, NAME + "_b_valid");
        sc_trace(tf, v.b_resp, NAME + "_b_resp");
        sc_trace(tf, v.b_id, NAME + "_b_id");
        sc_trace(tf, v.b_user, NAME + "_b_user");
        sc_trace(tf, v.ar_ready, NAME + "_ar_ready");
        sc_trace(tf, v.r_valid, NAME + "_r_valid");
        sc_trace(tf, v.r_resp, NAME + "_r_resp");
        sc_trace(tf, v.r_data, NAME + "_r_data");
        sc_trace(tf, v.r_last, NAME + "_r_last");
        sc_trace(tf, v.r_id, NAME + "_r_id");
        sc_trace(tf, v.r_user, NAME + "_r_user");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_l2_in_type const &v) {
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
        << v.r_user <<  ")";
        return os;
    }

 public:
    bool aw_ready;
    bool w_ready;
    bool b_valid;
    sc_uint<2> b_resp;
    sc_uint<CFG_CPU_ID_BITS> b_id;       // create ID for L2?
    sc_uint<CFG_CPU_USER_BITS> b_user;
    bool ar_ready;
    bool r_valid;
    sc_uint<2> r_resp;
    sc_biguint<L2CACHE_LINE_BITS> r_data;
    bool r_last;
    sc_uint<CFG_CPU_ID_BITS> r_id;
    sc_uint<CFG_CPU_USER_BITS> r_user;
};

class axi4_l2_out_type {
 public:
    axi4_l2_out_type() {
        aw_valid = 0;
        aw_bits.addr = 0;
        aw_bits.len = 0;
        aw_bits.size = 0;
        aw_bits.burst = 0;
        aw_bits.lock = 0;
        aw_bits.cache = 0;
        aw_bits.prot = 0;
        aw_bits.qos = 0;
        aw_bits.region = 0;
        aw_id = 0;
        aw_user = 0;
        w_valid = 0;
        w_data = 0;
        w_last = 0;
        w_strb = 0;
        w_user = 0;
        b_ready = 0;
        ar_valid = 0;
        ar_bits.addr = 0;
        ar_bits.len = 0;
        ar_bits.size = 0;
        ar_bits.burst = 0;
        ar_bits.lock = 0;
        ar_bits.cache = 0;
        ar_bits.prot = 0;
        ar_bits.qos = 0;
        ar_bits.region = 0;
        ar_id = 0;
        ar_user = 0;
        r_ready = 0;
    }

    inline bool operator == (const axi4_l2_out_type &rhs) const {
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
            && rhs.r_ready == r_ready);
    }

    inline axi4_l2_out_type& operator = (const axi4_l2_out_type &rhs) {
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
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_l2_out_type &v,
                                const std::string &NAME) {
        sc_trace(tf, v.aw_valid, NAME + "_aw_valid");
        sc_trace(tf, v.aw_bits.addr, NAME + "_aw_bits_addr");
        sc_trace(tf, v.aw_bits.len, NAME + "_aw_bits_len");
        sc_trace(tf, v.aw_bits.size, NAME + "_aw_bits_size");
        sc_trace(tf, v.aw_bits.burst, NAME + "_aw_bits_burst");
        sc_trace(tf, v.aw_bits.lock, NAME + "_aw_bits_lock");
        sc_trace(tf, v.aw_bits.cache, NAME + "_aw_bits_cache");
        sc_trace(tf, v.aw_bits.prot, NAME + "_aw_bits_prot");
        sc_trace(tf, v.aw_bits.qos, NAME + "_aw_bits_qos");
        sc_trace(tf, v.aw_bits.region, NAME + "_aw_bits_region");
        sc_trace(tf, v.aw_id, NAME + "_aw_id");
        sc_trace(tf, v.aw_user, NAME + "_aw_user");
        sc_trace(tf, v.w_valid, NAME + "_w_valid");
        sc_trace(tf, v.w_data, NAME + "_w_data");
        sc_trace(tf, v.w_last, NAME + "_w_last");
        sc_trace(tf, v.w_strb, NAME + "_w_strb");
        sc_trace(tf, v.w_user, NAME + "_w_user");
        sc_trace(tf, v.b_ready, NAME + "_b_ready");
        sc_trace(tf, v.ar_valid, NAME + "_ar_valid");
        sc_trace(tf, v.ar_bits.addr, NAME + "_ar_bits_addr");
        sc_trace(tf, v.ar_bits.len, NAME + "_ar_bits_len");
        sc_trace(tf, v.ar_bits.size, NAME + "_ar_bits_size");
        sc_trace(tf, v.ar_bits.burst, NAME + "_ar_bits_burst");
        sc_trace(tf, v.ar_bits.lock, NAME + "_ar_bits_lock");
        sc_trace(tf, v.ar_bits.cache, NAME + "_ar_bits_cache");
        sc_trace(tf, v.ar_bits.prot, NAME + "_ar_bits_prot");
        sc_trace(tf, v.ar_bits.qos, NAME + "_ar_bits_qos");
        sc_trace(tf, v.ar_bits.region, NAME + "_ar_bits_region");
        sc_trace(tf, v.ar_id, NAME + "_ar_id");
        sc_trace(tf, v.ar_user, NAME + "_ar_user");
        sc_trace(tf, v.r_ready, NAME + "_r_ready");
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_l2_out_type const &v) {
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
        << v.r_ready << ")";
        return os;
    }

 public:
    bool aw_valid;
    axi4_meta_type aw_bits;
    sc_uint<CFG_CPU_ID_BITS> aw_id;
    sc_uint<CFG_CPU_USER_BITS> aw_user;
    bool w_valid;
    sc_biguint<L2CACHE_LINE_BITS> w_data;
    bool w_last;
    sc_uint<L2CACHE_BYTES_PER_LINE> w_strb;
    sc_uint<CFG_CPU_USER_BITS> w_user;
    bool b_ready;
    bool ar_valid;
    axi4_meta_type ar_bits;
    sc_uint<CFG_CPU_ID_BITS> ar_id;
    sc_uint<CFG_CPU_USER_BITS> ar_user;
    bool r_ready;
};


/** Workgroup interface */
class axi4_domain_in_vector {
 public:
    axi4_domain_in_vector(){}

    inline bool operator == (const axi4_domain_in_vector &rhs) const {
        bool t;
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            t = (rhs.arr[i].aw_ready == arr[i].aw_ready
                 && rhs.arr[i].w_ready == arr[i].w_ready
                 && rhs.arr[i].b_valid == arr[i].b_valid
                 && rhs.arr[i].b_resp == arr[i].b_resp
                 && rhs.arr[i].b_id == arr[i].b_id
                 && rhs.arr[i].b_user == arr[i].b_user
                 && rhs.arr[i].ar_ready == arr[i].ar_ready
                 && rhs.arr[i].r_valid == arr[i].r_valid
                 && rhs.arr[i].r_resp == arr[i].r_resp
                 && rhs.arr[i].r_data == arr[i].r_data
                 && rhs.arr[i].r_last == arr[i].r_last
                 && rhs.arr[i].r_id == arr[i].r_id
                 && rhs.arr[i].r_user == arr[i].r_user
                 && rhs.arr[i].ac_valid == arr[i].ac_valid
                 && rhs.arr[i].ac_addr == arr[i].ac_addr
                 && rhs.arr[i].ac_snoop == arr[i].ac_snoop
                 && rhs.arr[i].ac_prot == arr[i].ac_prot
                 && rhs.arr[i].cr_ready == arr[i].cr_ready
                 && rhs.arr[i].cd_ready == arr[i].cd_ready);
            if (!t) {
                return false;
            }
        }
        return true;
    }

    inline axi4_domain_in_vector& operator = (const axi4_domain_in_vector &rhs) {
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            arr[i].aw_ready = rhs.arr[i].aw_ready;
            arr[i].w_ready = rhs.arr[i].w_ready;
            arr[i].b_valid = rhs.arr[i].b_valid;
            arr[i].b_resp = rhs.arr[i].b_resp;
            arr[i].b_id = rhs.arr[i].b_id;
            arr[i].b_user = rhs.arr[i].b_user;
            arr[i].ar_ready = rhs.arr[i].ar_ready;
            arr[i].r_valid = rhs.arr[i].r_valid;
            arr[i].r_resp = rhs.arr[i].r_resp;
            arr[i].r_data = rhs.arr[i].r_data;
            arr[i].r_last = rhs.arr[i].r_last;
            arr[i].r_id = rhs.arr[i].r_id;
            arr[i].r_user = rhs.arr[i].r_user;
            arr[i].ac_valid = rhs.arr[i].ac_valid;
            arr[i].ac_addr = rhs.arr[i].ac_addr;
            arr[i].ac_snoop = rhs.arr[i].ac_snoop;
            arr[i].ac_prot = rhs.arr[i].ac_prot;
            arr[i].cr_ready = rhs.arr[i].cr_ready;
            arr[i].cd_ready = rhs.arr[i].cd_ready;
        }
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_domain_in_vector &v,
                                const std::string &NAME) {
        char N[4] = "(0)";
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            N[1] = '0' + static_cast<char>(i);
            sc_trace(tf, v.arr[i].aw_ready, NAME + N + "_aw_ready");
            sc_trace(tf, v.arr[i].w_ready, NAME + N + "_w_ready");
            sc_trace(tf, v.arr[i].b_valid, NAME + N + "_b_valid");
            sc_trace(tf, v.arr[i].b_resp, NAME + N + "_b_resp");
            sc_trace(tf, v.arr[i].b_id, NAME + N + "_b_id");
            sc_trace(tf, v.arr[i].b_user, NAME + N + "_b_user");
            sc_trace(tf, v.arr[i].ar_ready, NAME + N + "_ar_ready");
            sc_trace(tf, v.arr[i].r_valid, NAME + N + "_r_valid");
            sc_trace(tf, v.arr[i].r_resp, NAME + N + "_r_resp");
            sc_trace(tf, v.arr[i].r_data, NAME + N + "_r_data");
            sc_trace(tf, v.arr[i].r_last, NAME + N + "_r_last");
            sc_trace(tf, v.arr[i].r_id, NAME + N + "_r_id");
            sc_trace(tf, v.arr[i].r_user, NAME + N + "_r_user");
            sc_trace(tf, v.arr[i].ac_valid, NAME + N + "_ac_valid");
            sc_trace(tf, v.arr[i].ac_addr, NAME + N + "_ac_addr");
            sc_trace(tf, v.arr[i].ac_snoop, NAME + N + "_ac_snoop");
            sc_trace(tf, v.arr[i].ac_prot, NAME + N + "_ac_prot");
            sc_trace(tf, v.arr[i].cr_ready, NAME + N + "_cr_ready");
            sc_trace(tf, v.arr[i].cd_ready, NAME + N + "_cd_ready");
        }
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_domain_in_vector const &v) {
        os << "(";
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            os << "("
            << v.arr[i].aw_ready << ","
            << v.arr[i].w_ready << ","
            << v.arr[i].b_valid << ","
            << v.arr[i].b_resp << ","
            << v.arr[i].b_id << ","
            << v.arr[i].b_user << ","
            << v.arr[i].ar_ready << ","
            << v.arr[i].r_valid << ","
            << v.arr[i].r_resp << ","
            << v.arr[i].r_data << ","
            << v.arr[i].r_last << ","
            << v.arr[i].r_id << ","
            << v.arr[i].r_user << ","
            << v.arr[i].ac_valid << ","
            << v.arr[i].ac_addr << ","
            << v.arr[i].ac_snoop << ","
            << v.arr[i].ac_prot << ","
            << v.arr[i].cr_ready << ","
            << v.arr[i].cd_ready << ")";
        }
        os << ")";
        return os;
    }

 public:
    axi4_l1_in_type arr[CFG_TOTAL_CPU_MAX];
};

class axi4_domain_out_type {
 public:
    axi4_domain_out_type(){}

    inline bool operator == (const axi4_domain_out_type &rhs) const {
        bool t;
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            t = (rhs.arr[i].aw_valid == arr[i].aw_valid
                && rhs.arr[i].aw_bits.addr == arr[i].aw_bits.addr
                && rhs.arr[i].aw_bits.len == arr[i].aw_bits.len
                && rhs.arr[i].aw_bits.size == arr[i].aw_bits.size
                && rhs.arr[i].aw_bits.burst == arr[i].aw_bits.burst
                && rhs.arr[i].aw_bits.lock == arr[i].aw_bits.lock
                && rhs.arr[i].aw_bits.cache == arr[i].aw_bits.cache
                && rhs.arr[i].aw_bits.prot == arr[i].aw_bits.prot
                && rhs.arr[i].aw_bits.qos == arr[i].aw_bits.qos
                && rhs.arr[i].aw_bits.region == arr[i].aw_bits.region
                && rhs.arr[i].aw_id == arr[i].aw_id
                && rhs.arr[i].aw_user == arr[i].aw_user
                && rhs.arr[i].w_valid == arr[i].w_valid
                && rhs.arr[i].w_data == arr[i].w_data
                && rhs.arr[i].w_last == arr[i].w_last
                && rhs.arr[i].w_strb == arr[i].w_strb
                && rhs.arr[i].w_user == arr[i].w_user
                && rhs.arr[i].b_ready == arr[i].b_ready
                && rhs.arr[i].ar_valid == arr[i].ar_valid
                && rhs.arr[i].ar_bits.addr == arr[i].ar_bits.addr
                && rhs.arr[i].ar_bits.len == arr[i].ar_bits.len
                && rhs.arr[i].ar_bits.size == arr[i].ar_bits.size
                && rhs.arr[i].ar_bits.burst == arr[i].ar_bits.burst
                && rhs.arr[i].ar_bits.lock == arr[i].ar_bits.lock
                && rhs.arr[i].ar_bits.cache == arr[i].ar_bits.cache
                && rhs.arr[i].ar_bits.prot == arr[i].ar_bits.prot
                && rhs.arr[i].ar_bits.qos == arr[i].ar_bits.qos
                && rhs.arr[i].ar_bits.region == arr[i].ar_bits.region
                && rhs.arr[i].ar_id == arr[i].ar_id
                && rhs.arr[i].ar_user == arr[i].ar_user
                && rhs.arr[i].r_ready == arr[i].r_ready
                && rhs.arr[i].ar_domain == arr[i].ar_domain
                && rhs.arr[i].ar_snoop == arr[i].ar_snoop
                && rhs.arr[i].ar_bar == arr[i].ar_bar
                && rhs.arr[i].aw_domain == arr[i].aw_domain
                && rhs.arr[i].aw_snoop == arr[i].aw_snoop
                && rhs.arr[i].aw_bar == arr[i].aw_bar
                && rhs.arr[i].ac_ready == arr[i].ac_ready
                && rhs.arr[i].cr_valid == arr[i].cr_valid
                && rhs.arr[i].cr_resp == arr[i].cr_resp
                && rhs.arr[i].cd_valid == arr[i].cd_valid
                && rhs.arr[i].cd_data == arr[i].cd_data
                && rhs.arr[i].cd_last == arr[i].cd_last
                && rhs.arr[i].rack == arr[i].rack
                && rhs.arr[i].wack == arr[i].wack);
            if (!t) {
                return false;
            }
        }
        return true;
    }

    inline axi4_domain_out_type& operator = (const axi4_domain_out_type &rhs) {
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            arr[i].aw_valid = rhs.arr[i].aw_valid;
            arr[i].aw_bits.addr = rhs.arr[i].aw_bits.addr;
            arr[i].aw_bits.len = rhs.arr[i].aw_bits.len;
            arr[i].aw_bits.size = rhs.arr[i].aw_bits.size;
            arr[i].aw_bits.burst = rhs.arr[i].aw_bits.burst;
            arr[i].aw_bits.lock = rhs.arr[i].aw_bits.lock;
            arr[i].aw_bits.cache = rhs.arr[i].aw_bits.cache;
            arr[i].aw_bits.prot = rhs.arr[i].aw_bits.prot;
            arr[i].aw_bits.qos = rhs.arr[i].aw_bits.qos;
            arr[i].aw_bits.region = rhs.arr[i].aw_bits.region;
            arr[i].aw_id = rhs.arr[i].aw_id;
            arr[i].aw_user = rhs.arr[i].aw_user;
            arr[i].w_valid = rhs.arr[i].w_valid;
            arr[i].w_data = rhs.arr[i].w_data;
            arr[i].w_last = rhs.arr[i].w_last;
            arr[i].w_strb = rhs.arr[i].w_strb;
            arr[i].w_user = rhs.arr[i].w_user;
            arr[i].b_ready = rhs.arr[i].b_ready;
            arr[i].ar_valid = rhs.arr[i].ar_valid;
            arr[i].ar_bits.addr = rhs.arr[i].ar_bits.addr;
            arr[i].ar_bits.len = rhs.arr[i].ar_bits.len;
            arr[i].ar_bits.size = rhs.arr[i].ar_bits.size;
            arr[i].ar_bits.burst = rhs.arr[i].ar_bits.burst;
            arr[i].ar_bits.lock = rhs.arr[i].ar_bits.lock;
            arr[i].ar_bits.cache = rhs.arr[i].ar_bits.cache;
            arr[i].ar_bits.prot = rhs.arr[i].ar_bits.prot;
            arr[i].ar_bits.qos = rhs.arr[i].ar_bits.qos;
            arr[i].ar_bits.region = rhs.arr[i].ar_bits.region;
            arr[i].ar_id = rhs.arr[i].ar_id;
            arr[i].ar_user = rhs.arr[i].ar_user;
            arr[i].r_ready = rhs.arr[i].r_ready;
            arr[i].ar_domain = rhs.arr[i].ar_domain;
            arr[i].ar_snoop = rhs.arr[i].ar_snoop;
            arr[i].ar_bar = rhs.arr[i].ar_bar;
            arr[i].aw_domain = rhs.arr[i].aw_domain;
            arr[i].aw_snoop = rhs.arr[i].aw_snoop;
            arr[i].aw_bar = rhs.arr[i].aw_bar;
            arr[i].ac_ready = rhs.arr[i].ac_ready;
            arr[i].cr_valid = rhs.arr[i].cr_valid;
            arr[i].cr_resp = rhs.arr[i].cr_resp;
            arr[i].cd_valid = rhs.arr[i].cd_valid;
            arr[i].cd_data = rhs.arr[i].cd_data;
            arr[i].cd_last = rhs.arr[i].cd_last;
            arr[i].rack = rhs.arr[i].rack;
            arr[i].wack = rhs.arr[i].wack;
        }
        return *this;
    }

    inline friend void sc_trace(sc_trace_file *tf,
                                const axi4_domain_out_type &v,
                                const std::string &NAME) {
        char N[4] = "(0)";
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            N[1] = '0' + static_cast<char>(i);
            sc_trace(tf, v.arr[i].aw_valid, NAME + N + "_aw_valid");
            sc_trace(tf, v.arr[i].aw_bits.addr, NAME + N + "_aw_bits_addr");
            sc_trace(tf, v.arr[i].aw_bits.len, NAME + N + "_aw_bits_len");
            sc_trace(tf, v.arr[i].aw_bits.size, NAME + N + "_aw_bits_size");
            sc_trace(tf, v.arr[i].aw_bits.burst, NAME + N + "_aw_bits_burst");
            sc_trace(tf, v.arr[i].aw_bits.lock, NAME + N + "_aw_bits_lock");
            sc_trace(tf, v.arr[i].aw_bits.cache, NAME + N + "_aw_bits_cache");
            sc_trace(tf, v.arr[i].aw_bits.prot, NAME + N + "_aw_bits_prot");
            sc_trace(tf, v.arr[i].aw_bits.qos, NAME + N + "_aw_bits_qos");
            sc_trace(tf, v.arr[i].aw_bits.region, NAME + N + "_aw_bits_region");
            sc_trace(tf, v.arr[i].aw_id, NAME + N + "_aw_id");
            sc_trace(tf, v.arr[i].aw_user, NAME + N + "_aw_user");
            sc_trace(tf, v.arr[i].w_valid, NAME + N + "_w_valid");
            sc_trace(tf, v.arr[i].w_data, NAME + N + "_w_data");
            sc_trace(tf, v.arr[i].w_last, NAME + N + "_w_last");
            sc_trace(tf, v.arr[i].w_strb, NAME + N + "_w_strb");
            sc_trace(tf, v.arr[i].w_user, NAME + N + "_w_user");
            sc_trace(tf, v.arr[i].b_ready, NAME + N + "_b_ready");
            sc_trace(tf, v.arr[i].ar_valid, NAME + N + "_ar_valid");
            sc_trace(tf, v.arr[i].ar_bits.addr, NAME + N + "_ar_bits_addr");
            sc_trace(tf, v.arr[i].ar_bits.len, NAME + N + "_ar_bits_len");
            sc_trace(tf, v.arr[i].ar_bits.size, NAME + N + "_ar_bits_size");
            sc_trace(tf, v.arr[i].ar_bits.burst, NAME + N + "_ar_bits_burst");
            sc_trace(tf, v.arr[i].ar_bits.lock, NAME + N + "_ar_bits_lock");
            sc_trace(tf, v.arr[i].ar_bits.cache, NAME + N + "_ar_bits_cache");
            sc_trace(tf, v.arr[i].ar_bits.prot, NAME + N + "_ar_bits_prot");
            sc_trace(tf, v.arr[i].ar_bits.qos, NAME + N + "_ar_bits_qos");
            sc_trace(tf, v.arr[i].ar_bits.region, NAME + N + "_ar_bits_region");
            sc_trace(tf, v.arr[i].ar_id, NAME + N + "_ar_id");
            sc_trace(tf, v.arr[i].ar_user, NAME + N + "_ar_user");
            sc_trace(tf, v.arr[i].r_ready, NAME + N + "_r_ready");
            sc_trace(tf, v.arr[i].ar_domain, NAME + N + "_ar_domain");
            sc_trace(tf, v.arr[i].ar_snoop, NAME + N + "_ar_snoop");
            sc_trace(tf, v.arr[i].ar_bar, NAME + N + "_ar_bar");
            sc_trace(tf, v.arr[i].aw_domain, NAME + N + "_aw_domain");
            sc_trace(tf, v.arr[i].aw_snoop, NAME + N + "_aw_snoop");
            sc_trace(tf, v.arr[i].aw_bar, NAME + N + "_aw_bar");
            sc_trace(tf, v.arr[i].ac_ready, NAME + N + "_ac_ready");
            sc_trace(tf, v.arr[i].cr_valid, NAME + N + "_cr_valid");
            sc_trace(tf, v.arr[i].cr_resp, NAME + N + "_cr_resp");
            sc_trace(tf, v.arr[i].cd_valid, NAME + N + "_cd_valid");
            sc_trace(tf, v.arr[i].cd_data, NAME + N + "_cd_data");
            sc_trace(tf, v.arr[i].cd_last, NAME + N + "_cd_last");
            sc_trace(tf, v.arr[i].rack, NAME + N + "_rack");
            sc_trace(tf, v.arr[i].wack, NAME + N + "_wack");
        }
    }

    inline friend ostream &operator << (ostream &os,
                                        axi4_domain_out_type const &v) {
        os << "(";
        for (int i = 0; i < CFG_TOTAL_CPU_MAX; i++) {
            os << "("
            << v.arr[i].aw_valid << ","
            << v.arr[i].aw_bits.addr << ","
            << v.arr[i].aw_bits.len << ","
            << v.arr[i].aw_bits.size << ","
            << v.arr[i].aw_bits.burst << ","
            << v.arr[i].aw_bits.lock << ","
            << v.arr[i].aw_bits.cache << ","
            << v.arr[i].aw_bits.prot << ","
            << v.arr[i].aw_bits.qos << ","
            << v.arr[i].aw_bits.region << ","
            << v.arr[i].aw_id << ","
            << v.arr[i].aw_user << ","
            << v.arr[i].w_valid << ","
            << v.arr[i].w_data << ","
            << v.arr[i].w_last << ","
            << v.arr[i].w_strb << ","
            << v.arr[i].w_user << ","
            << v.arr[i].b_ready << ","
            << v.arr[i].ar_valid << ","
            << v.arr[i].ar_bits.addr << ","
            << v.arr[i].ar_bits.len << ","
            << v.arr[i].ar_bits.size << ","
            << v.arr[i].ar_bits.burst << ","
            << v.arr[i].ar_bits.lock << ","
            << v.arr[i].ar_bits.cache << ","
            << v.arr[i].ar_bits.prot << ","
            << v.arr[i].ar_bits.qos << ","
            << v.arr[i].ar_bits.region << ","
            << v.arr[i].ar_id << ","
            << v.arr[i].ar_user << ","
            << v.arr[i].r_ready << ","
            << v.arr[i].ar_domain << ","
            << v.arr[i].ar_snoop << ","
            << v.arr[i].ar_bar << ","
            << v.arr[i].aw_domain << ","
            << v.arr[i].aw_snoop << ","
            << v.arr[i].aw_bar << ","
            << v.arr[i].ac_ready << ","
            << v.arr[i].cr_valid << ","
            << v.arr[i].cr_resp << ","
            << v.arr[i].cd_valid << ","
            << v.arr[i].cd_data << ","
            << v.arr[i].cd_last << ","
            << v.arr[i].rack << ","
            << v.arr[i].wack << ")";
        }
        os << ")";
        return os;
    }

 public:
    axi4_l1_out_type arr[CFG_TOTAL_CPU_MAX];
};

static const axi4_l1_in_type axi4_l1_in_none;
static const axi4_l1_out_type axi4_l1_out_none;

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_TYPES_RIVER_H__
