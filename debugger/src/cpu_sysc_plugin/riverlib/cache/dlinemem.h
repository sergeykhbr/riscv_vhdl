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

#ifndef __DEBUGGER_RIVERLIB_CACHE_DLINEMEM_H__
#define __DEBUGGER_RIVERLIB_CACHE_DLINEMEM_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"
#include "dwaymem.h"
#include "ilru.h"

namespace debugger {

SC_MODULE(DLineMem) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_flush;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_addr;
    sc_in<sc_biguint<4*BUS_DATA_WIDTH>> i_wdata;
    sc_in<sc_uint<4*BUS_DATA_BYTES>> i_wstrb;
    sc_in<bool> i_wvalid;
    sc_in<bool> i_wdirty;
    sc_in<bool> i_wload_fault;
    sc_in<bool> i_wexecutable;
    sc_in<bool> i_wreadable;
    sc_in<bool> i_wwritable;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_raddr;
    sc_out<sc_biguint<4*BUS_DATA_WIDTH>> o_rdata;
    sc_out<bool> o_rvalid;
    sc_out<bool> o_rdirty;
    sc_out<bool> o_rload_fault;
    sc_out<bool> o_rexecutable;
    sc_out<bool> o_rreadable;
    sc_out<bool> o_rwritable;
    sc_out<bool> o_hit;

    void comb();
    void registers();

    SC_HAS_PROCESS(DLineMem);

    DLineMem(sc_module_name name_, bool async_reset);
    virtual ~DLineMem();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct WayInType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> addr;
        sc_signal<sc_uint<4*BUS_DATA_BYTES>> wstrb;
        sc_signal<sc_biguint<4*BUS_DATA_WIDTH>> wdata;
        sc_signal<bool> wvalid;
        sc_signal<bool> load_fault;
        sc_signal<bool> executable;
        sc_signal<bool> readable;
        sc_signal<bool> writable;
    };
   struct WayOutType {
        sc_signal<sc_uint<CFG_DTAG_WIDTH>> rtag;
        sc_signal<sc_biguint<4*BUS_DATA_WIDTH>> rdata;
        sc_signal<bool> valid;
        sc_signal<bool> load_fault;
        sc_signal<bool> executable;
        sc_signal<bool> readable;
        sc_signal<bool> writable;
    };

    WayInType way_i[CFG_DCACHE_WAYS];
    WayOutType way_o[CFG_DCACHE_WAYS];

    sc_signal<bool> lrui_init;
    sc_signal<sc_uint<CFG_DINDEX_WIDTH>> lrui_radr;
    sc_signal<sc_uint<CFG_DINDEX_WIDTH>> lrui_wadr;
    sc_signal<bool> lrui_we;
    sc_signal<sc_uint<2>> lrui_lru;
    sc_signal<sc_uint<2>> lruo_lru;

    struct RegistersType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_addr;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.req_addr = 0;
    }


    DWayMem *wayx[CFG_DCACHE_WAYS];
    ILru *lru;

    sc_signal<sc_uint<CFG_IINDEX_WIDTH>> wb_addr;
    sc_signal<sc_uint<8>> wb_rdata[4*BUS_DATA_BYTES];
    sc_signal<sc_uint<8>> wb_wdata[4*BUS_DATA_BYTES];
    sc_signal<bool> w_wena[4*BUS_DATA_BYTES];

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_DLINEMEM_H__
