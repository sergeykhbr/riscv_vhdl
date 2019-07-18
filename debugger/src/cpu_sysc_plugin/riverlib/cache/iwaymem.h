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

#ifndef __DEBUGGER_RIVERLIB_CACHE_IWAYMEM_H__
#define __DEBUGGER_RIVERLIB_CACHE_IWAYMEM_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"
#include "mem/dpram64i.h"
#include "mem/dpramtagi.h"

namespace debugger {

static const int RAM64_BLOCK_TOTAL =
    (1 << CFG_IOFFSET_WIDTH) / sizeof(uint64_t);

static const int IINDEX_START = CFG_IOFFSET_WIDTH + CFG_IODDEVEN_WIDTH;
static const int IINDEX_END = IINDEX_START + CFG_IINDEX_WIDTH - 1;

static const int ITAG_START = IINDEX_START + CFG_IINDEX_WIDTH;
static const int ITAG_END = BUS_ADDR_WIDTH-1;

SC_MODULE(IWayMem) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_radr;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_wadr;
    sc_in<bool> i_wena;
    sc_in<sc_uint<4>> i_wstrb;
    sc_in<sc_uint<4>> i_wvalid;
    sc_in<sc_uint<64>> i_wdata;
    sc_in<bool> i_load_fault;
    sc_out<sc_uint<CFG_ITAG_WIDTH>> o_rtag;
    sc_out<sc_uint<32>> o_rdata;
    sc_out<bool> o_valid;
    sc_out<bool> o_load_fault;

    void comb();
    void registers();

    SC_HAS_PROCESS(IWayMem);

    IWayMem(sc_module_name name_, bool async_reset, int wayidx);
    virtual ~IWayMem();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    struct RegistersType {
        sc_signal<sc_uint<CFG_IOFFSET_WIDTH-1>> roffset;   // 2-bytes alignment
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.roffset = 0;
    }

    DpRamTagi *tag0;
    DpRam64i *datan[RAM64_BLOCK_TOTAL];

    sc_signal<sc_uint<CFG_IINDEX_WIDTH>> wb_radr;
    sc_signal<sc_uint<CFG_IINDEX_WIDTH>> wb_wadr;

    sc_signal<sc_uint<CFG_ITAG_WIDTH_TOTAL>> wb_tag_rdata;
    sc_signal<bool> w_tag_wena;
    sc_signal<sc_uint<CFG_ITAG_WIDTH_TOTAL>> wb_tag_wdata;

    sc_signal<sc_uint<64>> wb_data_rdata[RAM64_BLOCK_TOTAL];
    sc_signal<bool> w_data_wena[RAM64_BLOCK_TOTAL];

    bool async_reset_;
    int wayidx_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_IWAYMEM_H__
