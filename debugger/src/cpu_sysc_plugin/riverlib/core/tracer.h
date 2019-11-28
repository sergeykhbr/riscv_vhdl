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
 *
 * @brief      Stack trace buffer on hardware level.
 */

#ifndef __DEBUGGER_RIVERLIB_TRACER_H__
#define __DEBUGGER_RIVERLIB_TRACER_H__

#include <systemc.h>
#include <string>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(Tracer) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<64>> i_dbg_executed_cnt;
    sc_in<bool> i_e_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_e_pc;
    sc_in<sc_uint<32>> i_e_instr;
    sc_in<bool> i_e_memop_store;
    sc_in<bool> i_e_memop_load;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_e_memop_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_e_res_data;
    sc_in<sc_uint<6>> i_e_res_addr;
    sc_in<bool> i_m_wena;
    sc_in<sc_uint<6>> i_m_waddr;
    sc_in<sc_uint<RISCV_ARCH>> i_m_wdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(Tracer);

    Tracer(sc_module_name name_, bool async_reset, const char *trace_file);

 private:
    void task_disassembler(uint32_t instr);

    sc_signal<bool> r_load_reg;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> r_load_addr;

    FILE *fl_;
    char disasm[1024];

    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_TRACER_H__
