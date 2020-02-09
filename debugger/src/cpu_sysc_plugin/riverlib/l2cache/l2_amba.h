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

#ifndef __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_AMBA_H__
#define __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_AMBA_H__

#include "api_core.h"
#include "../types_river.h"
#include "../../ambalib/types_amba.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(L2Amba) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset: active LOW
    sc_out<bool> o_req_ready;
    sc_in<bool> i_req_valid;
    sc_in<bool> i_req_write;
    sc_in<bool> i_req_cached;
    sc_in<sc_uint<3>> i_req_size;
    sc_in<sc_uint<3>> i_req_prot;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_addr;
    sc_in<sc_uint<L2CACHE_BYTES_PER_LINE>> i_req_strob;
    sc_in<sc_biguint<L2CACHE_LINE_BITS>> i_req_data;
    sc_out<sc_biguint<L2CACHE_LINE_BITS>> o_resp_data;
    sc_out<bool> o_resp_valid;
    sc_out<bool> o_resp_ack;
    sc_out<bool> o_resp_load_fault;
    sc_out<bool> o_resp_store_fault;
    sc_in<axi4_l2_in_type> i_msti;
    sc_out<axi4_l2_out_type> o_msto;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2Amba);

    L2Amba(sc_module_name name_,
             bool async_reset);
    virtual ~L2Amba();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    enum state_type {
        idle,
        reading,
        writing,
        wack
    };

    struct RegistersType {
        sc_signal<sc_uint<2>> state;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = idle;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_AMBA_H__
