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

#ifndef __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2DESTINATION_H__
#define __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2DESTINATION_H__

#include "api_core.h"
#include "../types_river.h"
#include "../../ambalib/types_amba.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(L2Destination) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<3>> i_msg_type;
    sc_in<sc_uint<5>> i_msg_src;    // 0=acp; 1=core0; 2=core1; 3=core2; 4=core3
    sc_in<sc_biguint<L2_MSG_PAYLOAD_BITS>> i_msg_payload;
    sc_in<axi4_l1_out_type> i_l1o0;
    sc_out<axi4_l1_in_type> o_l1i0;
    sc_in<axi4_l1_out_type> i_l1o1;
    sc_out<axi4_l1_in_type> o_l1i1;
    sc_in<axi4_l1_out_type> i_l1o2;
    sc_out<axi4_l1_in_type> o_l1i2;
    sc_in<axi4_l1_out_type> i_l1o3;
    sc_out<axi4_l1_in_type> o_l1i3;
    sc_in<axi4_master_out_type> i_acpo;
    sc_out<axi4_master_in_type> o_acpi;
    sc_out<bool> o_eos;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2Destination);

    L2Destination(sc_module_name name, bool async_reset);
    virtual ~L2Destination();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    enum EState {
        Idle,
        ReadMem,
        WriteMem,
        WriteNoAck,
        WriteAck,
    };


    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<5>> src;
    } r, v;

    void R_RESET(RegistersType &iv) {
        iv.state = Idle;
        iv.src = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2DESTINATION_H__
