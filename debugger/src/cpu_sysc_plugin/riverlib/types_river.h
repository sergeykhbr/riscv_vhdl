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
#include <systemc.h>
#include <string>
#include <iomanip>

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
    sc_uint<debugger::CFG_RIVER_ID_BITS> b_id;
    sc_uint<1> b_user;
    bool ar_ready;
    bool r_valid;
    sc_uint<4> r_resp;
    sc_biguint<debugger::L1CACHE_LINE_BITS> r_data;
    bool r_last;
    sc_uint<debugger::CFG_RIVER_ID_BITS> r_id;
    sc_uint<1> r_user;
    bool ac_valid;
    sc_uint<debugger::BUS_ADDR_WIDTH> ac_addr;
    sc_uint<4> ac_snoop;                  // Table C3-19
    sc_uint<3> ac_prot;
    bool cr_ready;
    bool cd_ready;
};

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_TYPES_RIVER_H__
