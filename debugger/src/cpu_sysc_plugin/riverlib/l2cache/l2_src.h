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

#ifndef __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2SOURCE_H__
#define __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2SOURCE_H__

#include "api_core.h"
#include "../types_river.h"
#include "../../ambalib/types_amba.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(L2Source) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset active LOW
    sc_in<axi4_l1_out_type> i_coreo0;
    sc_in<axi4_l1_out_type> i_coreo1;
    sc_in<axi4_l1_out_type> i_coreo2;
    sc_in<axi4_l1_out_type> i_coreo3;
    sc_in<axi4_master_out_type> i_acpo;
    sc_in<bool> i_eos;                                  // End of sequence
    // cache interface
    sc_in<bool> i_req_ready;
    sc_out<bool> o_req_valid;
    sc_out<sc_uint<5>> o_req_src;    // 0=acp; 1=core0; 2=core1; 3=core2; 4=core3
    sc_out<bool> o_req_write;
    sc_out<bool> o_req_cached;
    sc_out<sc_uint<5>> o_msg_src;    // 0=acp; 1=core0; 2=core1; 3=core2; 4=core3
    sc_out<sc_uint<3>> o_msg_type;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_addr;
    sc_out<sc_uint<3>> o_req_size;
    sc_out<sc_uint<3>> o_req_prot;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_wdata;
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_wstrb;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2Source);

    L2Source(sc_module_name name, bool async_reset);
    virtual ~L2Source();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    enum EState {
        State_Idle,
        State_Read,
        State_Write,
        State_Busy
    };

    struct req_addr_type {
        bool aw_valid;
        bool aw_cached;
        sc_uint<CFG_CPU_ADDR_BITS> aw_addr;
        sc_uint<3> aw_size;
        sc_uint<3> aw_prot;
        bool ar_valid;
        bool ar_cached;
        sc_uint<CFG_CPU_ADDR_BITS> ar_addr;
        sc_uint<3> ar_size;
        sc_uint<3> ar_prot;
    };

    struct req_data_type {
        sc_biguint<L1CACHE_LINE_BITS> req_wdata;
        sc_uint<L1CACHE_BYTES_PER_LINE> req_wstrb;
    };

    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<sc_uint<5>> req_src;
        sc_signal<bool> req_cached;
        sc_signal<sc_uint<3>> chansel;
    } r, v;

    void R_RESET(RegistersType &iv) {
        iv.state = State_Idle;
        iv.req_addr = 0;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.req_src = 0;
        iv.req_cached = 0;
        iv.chansel = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2SOURCE_H__
