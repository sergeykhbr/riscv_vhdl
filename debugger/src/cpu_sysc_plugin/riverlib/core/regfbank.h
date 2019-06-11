/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_RIVERLIB_REGFBANK_H__
#define __DEBUGGER_RIVERLIB_REGFBANK_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(RegFloatBank) {
    sc_in<bool> i_clk;                      // Clock
    sc_in<bool> i_nrst;                     // Reset. Active LOW
    sc_in<sc_uint<5>> i_radr1;              // Port 1 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata1;   // Port 1 read value

    sc_in<sc_uint<5>> i_radr2;              // Port 2 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata2;   // Port 2 read value

    sc_in<sc_uint<5>> i_waddr;              // Writing value
    sc_in<bool> i_wena;                     // Writing is enabled
    sc_in<sc_uint<RISCV_ARCH>> i_wdata;     // Writing value

    sc_in<sc_uint<5>> i_dport_addr;             // Debug port address
    sc_in<bool> i_dport_ena;                    // Debug port is enabled
    sc_in<bool> i_dport_write;                  // Debug port write is enabled
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;   // Debug port write value
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;  // Debug port read value

    void comb();
    void registers();

    SC_HAS_PROCESS(RegFloatBank);

    RegFloatBank(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct RegistersType {
        sc_signal<bool> update;             // To generate SystemC delta event only.
        sc_uint<RISCV_ARCH> mem[RegFpu_Total]; // Multi-ports memory
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.mem[0] = 0;
        for (int i = 1; i < RegFpu_Total; i++) {
            iv.mem[i] = 0xfeedface;
        }
        iv.update = 0;
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_REGFBANK_H__
