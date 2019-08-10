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

#ifndef __DEBUGGER_RIVERLIB_CACHE_MEM_ILRU_H__
#define __DEBUGGER_RIVERLIB_CACHE_MEM_ILRU_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(ILru) {
    sc_in<bool> i_clk;
    sc_in<bool> i_init;
    sc_in<sc_uint<CFG_IINDEX_WIDTH>> i_adr;
    sc_in<bool> i_we;
    sc_in<sc_uint<2>> i_lru;
    sc_out<sc_uint<2>> o_lru;

    void comb();
    void registers();

    SC_HAS_PROCESS(ILru);

    ILru(sc_module_name name_);

 private:
    static const int LINES_TOTAL = 1 << CFG_IINDEX_WIDTH;

    sc_signal<sc_uint<CFG_IINDEX_WIDTH>> radr;
    sc_uint<8> tbl[LINES_TOTAL];

    sc_signal<sc_uint<8>> wb_tbl_rdata;
    sc_signal<sc_uint<8>> wb_tbl_wdata;
    sc_signal<bool> w_we;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_MEM_ILRU_H__
