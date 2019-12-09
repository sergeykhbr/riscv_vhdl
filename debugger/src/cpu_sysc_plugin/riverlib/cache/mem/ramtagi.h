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

#ifndef __DEBUGGER_RIVERLIB_CACHE_MEM_RAMTAGI_H__
#define __DEBUGGER_RIVERLIB_CACHE_MEM_RAMTAGI_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../../river_cfg.h"

namespace debugger {

SC_MODULE(RamTagi) {
    sc_in<bool> i_clk;
    sc_in<sc_uint<CFG_IINDEX_WIDTH>> i_adr;
    sc_out<sc_biguint<ILINE_MEM_WIDTH>> o_rdata;
    sc_in<bool> i_wena;
    sc_in<sc_biguint<ILINE_MEM_WIDTH>> i_wdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(RamTagi);

    RamTagi(sc_module_name name_);

 private:
    struct RegistersType {
        sc_signal<bool> update;  // To generate SystemC delta event only.
        sc_signal<sc_uint<CFG_IINDEX_WIDTH>> adr;
        sc_biguint<ILINE_MEM_WIDTH> mem[1 << CFG_IINDEX_WIDTH];
    } v, r;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_MEM_RAMTAGI_H__
