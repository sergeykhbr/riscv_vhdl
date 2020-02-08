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

#ifndef __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_TOP_H__
#define __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_TOP_H__

#include "api_core.h"
#include "../ambalib/types_amba.h"
#include "../riverlib/river_cfg.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(L2Top) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset active LOW
    // CPU[0] AXI4 + ACE:
    sc_out<bool> o_slvi0_aw_ready;
    sc_out<bool> o_slvi0_w_ready;
    sc_out<bool> o_slvi0_b_valid;
    sc_out<sc_uint<2>> o_slvi0_b_resp;
    sc_out<sc_uint<CFG_CPU_ID_BITS>> o_slvi0_b_id;
    sc_out<bool> o_slvi0_b_user;
    sc_out<bool> o_slvi0_ar_ready;
    sc_out<bool> o_slvi0_r_valid;
    sc_out<sc_uint<4>> o_slvi0_r_resp;                    // 0000=OKAY;0001=EXOKAY;0010=SLVERR;0011=DECER
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_slvi0_r_data;
    sc_out<bool> o_slvi0_r_last;
    sc_out<sc_uint<CFG_CPU_ID_BITS>> o_slvi0_r_id;
    sc_out<bool> o_slvi0_r_user;
    sc_out<bool> o_slvi0_ac_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_slvi0_ac_addr;
    sc_out<sc_uint<4>> o_slvi0_ac_snoop;                  // Table C3-19
    sc_out<sc_uint<3>> o_slvi0_ac_prot;
    sc_out<bool> o_slvi0_cr_ready;
    sc_out<bool> o_slvi0_cd_ready;
    sc_in<bool> i_slvo0_aw_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_slvo0_aw_bits_addr;
    sc_in<sc_uint<8>> i_slvo0_aw_bits_len;              // burst len = len[7:0] + 1
    sc_in<sc_uint<3>> i_slvo0_aw_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_in<sc_uint<2>> i_slvo0_aw_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_in<bool> i_slvo0_aw_bits_lock;
    sc_in<sc_uint<4>> i_slvo0_aw_bits_cache;
    sc_in<sc_uint<3>> i_slvo0_aw_bits_prot;
    sc_in<sc_uint<4>> i_slvo0_aw_bits_qos;
    sc_in<sc_uint<4>> i_slvo0_aw_bits_region;
    sc_in<sc_uint<CFG_CPU_ID_BITS>> i_slvo0_aw_id;
    sc_in<bool> i_slvo0_aw_user;
    sc_in<bool> i_slvo0_w_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_slvo0_w_data;
    sc_in<bool> i_slvo0_w_last;
    sc_in<sc_uint<L1CACHE_BYTES_PER_LINE>> i_slvo0_w_strb;
    sc_in<bool> i_slvo0_w_user;
    sc_in<bool> i_slvo0_b_ready;
    sc_in<bool> i_slvo0_ar_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_slvo0_ar_bits_addr;
    sc_in<sc_uint<8>> i_slvo0_ar_bits_len;              // burst len = len[7:0] + 1
    sc_in<sc_uint<3>> i_slvo0_ar_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_in<sc_uint<2>> i_slvo0_ar_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_in<bool> i_slvo0_ar_bits_lock;
    sc_in<sc_uint<4>> i_slvo0_ar_bits_cache;
    sc_in<sc_uint<3>> i_slvo0_ar_bits_prot;
    sc_in<sc_uint<4>> i_slvo0_ar_bits_qos;
    sc_in<sc_uint<4>> i_slvo0_ar_bits_region;
    sc_in<sc_uint<CFG_CPU_ID_BITS>> i_slvo0_ar_id;
    sc_in<bool> i_slvo0_ar_user;
    sc_in<bool> i_slvo0_r_ready;
    sc_in<sc_uint<2>> i_slvo0_ar_domain;                // 00=Non-shareable (single master in domain)
    sc_in<sc_uint<4>> i_slvo0_ar_snoop;                 // Table C3-7:
    sc_in<sc_uint<2>> i_slvo0_ar_bar;                   // read barrier transaction
    sc_in<sc_uint<2>> i_slvo0_aw_domain;
    sc_in<sc_uint<4>> i_slvo0_aw_snoop;                 // Table C3-8
    sc_in<sc_uint<2>> i_slvo0_aw_bar;                   // write barrier transaction
    sc_in<bool> i_slvo0_ac_ready;
    sc_in<bool> i_slvo0_cr_valid;
    sc_in<sc_uint<5>> i_slvo0_cr_resp;
    sc_in<bool> i_slvo0_cd_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_slvo0_cd_data;
    sc_in<bool> i_slvo0_cd_last;
    sc_in<bool> i_slvo0_rack;
    sc_in<bool> i_slvo0_wack;
    // CPU[1] AXI4 + ACE:
    sc_out<bool> o_slvi1_aw_ready;
    sc_out<bool> o_slvi1_w_ready;
    sc_out<bool> o_slvi1_b_valid;
    sc_out<sc_uint<2>> o_slvi1_b_resp;
    sc_out<sc_uint<CFG_CPU_ID_BITS>> o_slvi1_b_id;
    sc_out<bool> o_slvi1_b_user;
    sc_out<bool> o_slvi1_ar_ready;
    sc_out<bool> o_slvi1_r_valid;
    sc_out<sc_uint<4>> o_slvi1_r_resp;                    // 0000=OKAY;0001=EXOKAY;0010=SLVERR;0011=DECER
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_slvi1_r_data;
    sc_out<bool> o_slvi1_r_last;
    sc_out<sc_uint<CFG_CPU_ID_BITS>> o_slvi1_r_id;
    sc_out<bool> o_slvi1_r_user;
    sc_out<bool> o_slvi1_ac_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_slvi1_ac_addr;
    sc_out<sc_uint<4>> o_slvi1_ac_snoop;                  // Table C3-19
    sc_out<sc_uint<3>> o_slvi1_ac_prot;
    sc_out<bool> o_slvi1_cr_ready;
    sc_out<bool> o_slvi1_cd_ready;
    sc_in<bool> i_slvo1_aw_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_slvo1_aw_bits_addr;
    sc_in<sc_uint<8>> i_slvo1_aw_bits_len;              // burst len = len[7:0] + 1
    sc_in<sc_uint<3>> i_slvo1_aw_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_in<sc_uint<2>> i_slvo1_aw_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_in<bool> i_slvo1_aw_bits_lock;
    sc_in<sc_uint<4>> i_slvo1_aw_bits_cache;
    sc_in<sc_uint<3>> i_slvo1_aw_bits_prot;
    sc_in<sc_uint<4>> i_slvo1_aw_bits_qos;
    sc_in<sc_uint<4>> i_slvo1_aw_bits_region;
    sc_in<sc_uint<CFG_CPU_ID_BITS>> i_slvo1_aw_id;
    sc_in<bool> i_slvo1_aw_user;
    sc_in<bool> i_slvo1_w_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_slvo1_w_data;
    sc_in<bool> i_slvo1_w_last;
    sc_in<sc_uint<L1CACHE_BYTES_PER_LINE>> i_slvo1_w_strb;
    sc_in<bool> i_slvo1_w_user;
    sc_in<bool> i_slvo1_b_ready;
    sc_in<bool> i_slvo1_ar_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_slvo1_ar_bits_addr;
    sc_in<sc_uint<8>> i_slvo1_ar_bits_len;              // burst len = len[7:0] + 1
    sc_in<sc_uint<3>> i_slvo1_ar_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_in<sc_uint<2>> i_slvo1_ar_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_in<bool> i_slvo1_ar_bits_lock;
    sc_in<sc_uint<4>> i_slvo1_ar_bits_cache;
    sc_in<sc_uint<3>> i_slvo1_ar_bits_prot;
    sc_in<sc_uint<4>> i_slvo1_ar_bits_qos;
    sc_in<sc_uint<4>> i_slvo1_ar_bits_region;
    sc_in<sc_uint<CFG_CPU_ID_BITS>> i_slvo1_ar_id;
    sc_in<bool> i_slvo1_ar_user;
    sc_in<bool> i_slvo1_r_ready;
    sc_in<sc_uint<2>> i_slvo1_ar_domain;                // 00=Non-shareable (single master in domain)
    sc_in<sc_uint<4>> i_slvo1_ar_snoop;                 // Table C3-7:
    sc_in<sc_uint<2>> i_slvo1_ar_bar;                   // read barrier transaction
    sc_in<sc_uint<2>> i_slvo1_aw_domain;
    sc_in<sc_uint<4>> i_slvo1_aw_snoop;                 // Table C3-8
    sc_in<sc_uint<2>> i_slvo1_aw_bar;                   // write barrier transaction
    sc_in<bool> i_slvo1_ac_ready;
    sc_in<bool> i_slvo1_cr_valid;
    sc_in<sc_uint<5>> i_slvo1_cr_resp;
    sc_in<bool> i_slvo1_cd_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_slvo1_cd_data;
    sc_in<bool> i_slvo1_cd_last;
    sc_in<bool> i_slvo1_rack;
    sc_in<bool> i_slvo1_wack;
    // Master interface:
    sc_in<bool> i_msti_aw_ready;
    sc_in<bool> i_msti_w_ready;
    sc_in<bool> i_msti_b_valid;
    sc_in<sc_uint<2>> i_msti_b_resp;
    sc_in<sc_uint<CFG_BUS_ID_BITS>> i_msti_b_id;
    sc_in<bool> i_msti_b_user;
    sc_in<bool> i_msti_ar_ready;
    sc_in<bool> i_msti_r_valid;
    sc_in<sc_uint<2>> i_msti_r_resp;                    // 00=OKAY;01=EXOKAY;10=SLVERR;11=DECER
    sc_in<sc_biguint<BUS_DATA_WIDTH>> i_msti_r_data;
    sc_in<bool> i_msti_r_last;
    sc_in<sc_uint<CFG_BUS_ID_BITS>> i_msti_r_id;
    sc_in<bool> i_msti_r_user;
    sc_out<bool> o_msto_aw_valid;
    sc_out<sc_uint<CFG_BUS_ADDR_WIDTH>> o_msto_aw_bits_addr;
    sc_out<sc_uint<8>> o_msto_aw_bits_len;              // burst len = len[7:0] + 1
    sc_out<sc_uint<3>> o_msto_aw_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_out<sc_uint<2>> o_msto_aw_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_out<bool> o_msto_aw_bits_lock;
    sc_out<sc_uint<4>> o_msto_aw_bits_cache;
    sc_out<sc_uint<3>> o_msto_aw_bits_prot;
    sc_out<sc_uint<4>> o_msto_aw_bits_qos;
    sc_out<sc_uint<4>> o_msto_aw_bits_region;
    sc_out<sc_uint<CFG_BUS_ID_BITS>> o_msto_aw_id;
    sc_out<bool> o_msto_aw_user;
    sc_out<bool> o_msto_w_valid;
    sc_out<sc_biguint<BUS_DATA_WIDTH>> o_msto_w_data;
    sc_out<bool> o_msto_w_last;
    sc_out<sc_uint<BUS_DATA_BYTES>> o_msto_w_strb;
    sc_out<bool> o_msto_w_user;
    sc_out<bool> o_msto_b_ready;
    sc_out<bool> o_msto_ar_valid;
    sc_out<sc_uint<CFG_BUS_ADDR_WIDTH>> o_msto_ar_bits_addr;
    sc_out<sc_uint<8>> o_msto_ar_bits_len;              // burst len = len[7:0] + 1
    sc_out<sc_uint<3>> o_msto_ar_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_out<sc_uint<2>> o_msto_ar_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_out<bool> o_msto_ar_bits_lock;
    sc_out<sc_uint<4>> o_msto_ar_bits_cache;
    sc_out<sc_uint<3>> o_msto_ar_bits_prot;
    sc_out<sc_uint<4>> o_msto_ar_bits_qos;
    sc_out<sc_uint<4>> o_msto_ar_bits_region;
    sc_out<sc_uint<CFG_BUS_ID_BITS>> o_msto_ar_id;
    sc_out<bool> o_msto_ar_user;
    sc_out<bool> o_msto_r_ready;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2Top);

    L2Top(sc_module_name name, bool async_reset);
    virtual ~L2Top();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    enum EState {
        State_Idle,
        State_ReadUncached,
        State_ReadCached,
        State_WriteUncached,
        State_WriteCached,
    };

    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CFG_BUS_ADDR_WIDTH>> req_addr;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<bool> b_wait;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> line;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> wstrb;
    } r, v;

    void R_RESET(RegistersType &iv) {
        iv.state = State_Idle;
        iv.req_addr = 0;
        iv.req_len = 0;
        iv.b_wait = 0;
        iv.line = 0;
        iv.wstrb = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_TOP_H__
