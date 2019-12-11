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

#ifndef __DEBUGGER_RIVERLIB_CACHE_TAGWAYMEM_H__
#define __DEBUGGER_RIVERLIB_CACHE_TAGWAYMEM_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"
#include "mem/ram.h"

namespace debugger {

template <int tagbits, int dbits, int flbits>
SC_MODULE(TagWayMem) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_addr;
    sc_in<sc_uint<4*BUS_DATA_BYTES>> i_wstrb;
    sc_in<sc_biguint<4*BUS_DATA_WIDTH>> i_wdata;
    sc_in<sc_uint<DTAG_FL_TOTAL>> i_wflags;
    sc_out<sc_uint<CFG_DTAG_WIDTH>> o_rtag;
    sc_out<sc_biguint<4*BUS_DATA_WIDTH>> o_rdata;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_rflags;

    void comb();
 
    SC_HAS_PROCESS(TagWayMem);

    TagWayMem(sc_module_name name_, bool async_reset, int wayidx)
        : sc_module(name_),
        i_clk("i_clk"),
        i_nrst("i_nrst"),
        i_addr("i_addr"),
        i_wstrb("i_wstrb"),
        i_wdata("i_wdata"),
        i_wflags("i_wflags"),
        o_rtag("o_rtag"),
        o_rdata("o_rdata"),
        o_rflags("o_rflags") {
        async_reset_ = async_reset;
        wayidx_ = wayidx;

        char tstr1[] = "data0";
        for (int i = 0; i < 4*BUS_DATA_BYTES; i++) {
            tstr1[4] = '0' + i;
            datax[i] = new ram<CFG_DINDEX_WIDTH, 8>(tstr1);

            datax[i]->i_clk(i_clk);
            datax[i]->i_adr(wb_addr);
            datax[i]->i_wena(w_wdata_we[i]);
            datax[i]->i_wdata(wb_wdata[i]);
            datax[i]->o_rdata(wb_rdata[i]);
        }

        tag0 = new ram<CFG_DINDEX_WIDTH, TAG_BITS>("tag0");
        tag0->i_clk(i_clk);
        tag0->i_adr(wb_addr);
        tag0->i_wena(w_wtag_we);
        tag0->i_wdata(wb_wtag);
        tag0->o_rdata(wb_rtag);
    
        SC_METHOD(comb);
        sensitive << i_addr;
        sensitive << i_wstrb;
        sensitive << i_wdata;
        sensitive << i_wflags;
        sensitive << wb_rtag;
        for (int i = 0; i < 4*BUS_DATA_BYTES; i++) {
            sensitive << wb_rdata[i];
        }
    }

    virtual ~TagWayMem() {
        for (int i = 0; i < BUS_DATA_BYTES; i++) {
            delete datax[i];
        }
        delete tag0;
    }

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    static const int TAG_BITS = DTAG_SIZE + DTAG_FL_TOTAL;

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

template <int tagbits, int dbits, int flbits>
void TagWayMem<tagbits, dbits, flbits>::generateVCD(sc_trace_file *i_vcd,
                                                    sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wflags, i_wflags.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_rflags, o_rflags.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_rtag, pn + ".wb_rtag");
        sc_trace(o_vcd, wb_wtag, pn + ".wb_wtag");
    }
}

template <int tagbits, int dbits, int flbits>
void TagWayMem<tagbits, dbits, flbits>::comb() {
    sc_biguint<4*BUS_DATA_WIDTH> vb_rdata;
    sc_uint<TAG_BITS> vb_rtag;
    sc_uint<TAG_BITS> vb_wtag;

    wb_addr = i_addr.read()(DINDEX_END, DINDEX_START);
    for (int i = 0; i < 4*BUS_DATA_BYTES; i++) {
        w_wdata_we[i] = i_wstrb.read()[i];
        wb_wdata[i] = i_wdata.read()(8*i+7, 8*i).to_uint();

        vb_rdata(8*i+7, 8*i) = wb_rdata[i].read();
    }

    w_wtag_we = i_wstrb.read().or_reduce();
    vb_wtag(DTAG_SIZE-1, 0) = i_addr.read()(DTAG_END, DTAG_START);
    vb_wtag(TAG_BITS-1, DTAG_SIZE) = i_wflags.read();

    wb_wtag = vb_wtag;
    vb_rtag = wb_rtag;

    o_rdata = vb_rdata;
    o_rtag = vb_rtag(DTAG_SIZE-1, 0);
    o_rflags = vb_rtag(TAG_BITS-1, DTAG_SIZE);
}

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TAGWAYMEM_H__
