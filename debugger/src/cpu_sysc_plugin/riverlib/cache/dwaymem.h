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

#ifndef __DEBUGGER_RIVERLIB_CACHE_DWAYMEM_H__
#define __DEBUGGER_RIVERLIB_CACHE_DWAYMEM_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"
#include "mem/ram.h"

namespace debugger {

SC_MODULE(DWayMem) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_addr;
    sc_in<sc_uint<4*BUS_DATA_BYTES>> i_wstrb;
    sc_in<bool> i_wvalid;
    sc_in<sc_biguint<4*BUS_DATA_WIDTH>> i_wdata;
    sc_in<bool> i_load_fault;
    sc_in<bool> i_executable;
    sc_in<bool> i_readable;
    sc_in<bool> i_writable;
    sc_out<sc_uint<CFG_DTAG_WIDTH>> o_rtag;
    sc_out<sc_biguint<4*BUS_DATA_WIDTH>> o_rdata;
    sc_out<bool> o_valid;
    sc_out<bool> o_load_fault;
    sc_out<bool> o_executable;
    sc_out<bool> o_readable;
    sc_out<bool> o_writable;

    void comb();
    void registers();

    SC_HAS_PROCESS(DWayMem);

    DWayMem(sc_module_name name_, bool async_reset, int wayidx);
    virtual ~DWayMem();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    struct RegistersType {
        sc_signal<sc_uint<CFG_DOFFSET_WIDTH-1>> roffset;   // 2-bytes alignment
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.roffset = 0;
    }

    static const int TAG_BITS = DTAG_SIZE + 6;

    ram<CFG_DINDEX_WIDTH, 8> *datax[4*BUS_DATA_BYTES];
    ram<CFG_DINDEX_WIDTH, TAG_BITS> *tag0;
    
    sc_signal<sc_uint<CFG_DINDEX_WIDTH>> wb_addr;
    sc_signal<sc_uint<8>> wb_rdata[4*BUS_DATA_BYTES];
    sc_signal<sc_uint<8>> wb_wdata[4*BUS_DATA_BYTES];
    sc_signal<bool> w_wdata_we[4*BUS_DATA_BYTES];

    sc_signal<sc_uint<TAG_BITS>> wb_rtag;
    sc_signal<sc_uint<TAG_BITS>> wb_wtag;
    sc_signal<bool> w_wtag_we;

    bool async_reset_;
    int wayidx_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_DWAYMEM_H__
