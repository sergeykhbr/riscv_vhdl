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

#ifndef __DEBUGGER_RIVERLIB_CACHE_TAGMEM_H__
#define __DEBUGGER_RIVERLIB_CACHE_TAGMEM_H__

#include <systemc.h>
#include "mem/ram.h"

namespace debugger {
/**
    abus = system bus address width (64 or 32 bits)
    ibits = lines memory address width (usually 6..8)
    lnbits = One line bits: log2(bytes_per_line)
    flbits = total flags number saved with address tag
*/
template <int abus, int ibits, int lnbits, int flbits>
SC_MODULE(TagMem) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<sc_uint<abus>> i_addr;
    sc_in<sc_uint<(1<<lnbits)>> i_wstrb;
    sc_in<sc_biguint<8*(1<<lnbits)>> i_wdata;
    sc_in<sc_uint<flbits>> i_wflags;
    sc_out<sc_uint<abus>> o_raddr;
    sc_out<sc_biguint<8*(1<<lnbits)>> o_rdata;
    sc_out<sc_uint<flbits>> o_rflags;
    sc_out<bool> o_hit;

    void comb();
    void registers();
 
    SC_HAS_PROCESS(TagMem);

    TagMem(sc_module_name name_, bool async_reset, int wayidx)
        : sc_module(name_),
        i_clk("i_clk"),
        i_nrst("i_nrst"),
        i_addr("i_addr"),
        i_wstrb("i_wstrb"),
        i_wdata("i_wdata"),
        i_wflags("i_wflags"),
        o_raddr("o_raddr"),
        o_rdata("o_rdata"),
        o_rflags("o_rflags"),
        o_hit("o_hit") {
        async_reset_ = async_reset;
        wayidx_ = wayidx;

        char tstr1[] = "data0";
        for (int i = 0; i < (1<<lnbits); i++) {
            tstr1[4] = '0' + i;
            datax[i] = new ram<ibits, 8>(tstr1);

            datax[i]->i_clk(i_clk);
            datax[i]->i_adr(wb_index);
            datax[i]->i_wena(datai_we[i]);
            datax[i]->i_wdata(datai_wdata[i]);
            datax[i]->o_rdata(datao_rdata[i]);
        }

        tag0 = new ram<ibits, TAG_WITH_FLAGS>("tag0");
        tag0->i_clk(i_clk);
        tag0->i_adr(wb_index);
        tag0->i_wena(tagi_we);
        tag0->i_wdata(tagi_wdata);
        tag0->o_rdata(tago_rdata);
    
        SC_METHOD(comb);
        sensitive << i_addr;
        sensitive << i_wstrb;
        sensitive << i_wdata;
        sensitive << i_wflags;
        sensitive << tago_rdata;
        for (int i = 0; i < (1<<lnbits); i++) {
            sensitive << datao_rdata[i];
        }
        sensitive << rb_tagi;

        SC_METHOD(registers);
        sensitive << i_nrst;
        sensitive << i_clk.pos();
    }

    virtual ~TagMem() {
        for (int i = 0; i < (1<<lnbits); i++) {
            delete datax[i];
        }
        delete tag0;
    }

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    static const int TAG_BITS = abus - ibits - lnbits;
    static const int TAG_WITH_FLAGS = TAG_BITS + flbits;

    ram<ibits, 8> *datax[(1<<lnbits)];
    ram<ibits, TAG_WITH_FLAGS> *tag0;
    
    sc_signal<sc_uint<ibits>> wb_index;
    sc_signal<sc_uint<8>> datao_rdata[(1<<lnbits)];
    sc_signal<sc_uint<8>> datai_wdata[(1<<lnbits)];
    sc_signal<bool> datai_we[(1<<lnbits)];

    sc_signal<sc_uint<TAG_WITH_FLAGS>> tago_rdata;
    sc_signal<sc_uint<TAG_WITH_FLAGS>> tagi_wdata;
    sc_signal<bool> tagi_we;

    sc_signal<sc_uint<TAG_BITS>> rb_tagi;
    sc_signal<sc_uint<ibits>> rb_index;
    sc_uint<TAG_BITS> t_tago_tag;

    bool async_reset_;
    int wayidx_;
};

template <int abus, int ibits, int lnbits, int flbits>
void TagMem<abus, ibits, lnbits, flbits>::generateVCD (
    sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wflags, i_wflags.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_rflags, o_rflags.name());

        std::string pn(name());
        sc_trace(o_vcd, tago_rdata, pn + ".tago_rdata");
        sc_trace(o_vcd, tagi_wdata, pn + ".tagi_wdata");
        sc_trace(o_vcd, rb_tagi, pn + ".rb_tagi");
        sc_trace(o_vcd, rb_index, pn + ".rb_index");
        sc_trace(o_vcd, t_tago_tag, pn + ".t_tago_tag");
    }
}

template <int abus, int ibits, int lnbits, int flbits>
void TagMem<abus, ibits, lnbits, flbits>::comb() {
    sc_uint<ibits> vb_index;
    sc_uint<abus> vb_raddr;
    sc_biguint<8*(1<<lnbits)> vb_rdata;
    sc_uint<TAG_WITH_FLAGS> vb_tagi_wdata;
    bool v_hit;

    v_hit = 0;
    for (int i = 0; i < (1<<lnbits); i++) {
        datai_we[i] = i_wstrb.read()[i];
        datai_wdata[i] = i_wdata.read()(8*i+7, 8*i).to_uint();

        vb_rdata(8*i+7, 8*i) = datao_rdata[i].read();
    }

    t_tago_tag = tago_rdata.read()(TAG_BITS-1, 0);
    if (rb_tagi.read() == t_tago_tag) {
        v_hit = 1;
    }

    vb_raddr = 0;
    vb_raddr(abus-1, ibits + lnbits) = tago_rdata.read()(TAG_BITS-1, 0);
    vb_raddr(ibits + lnbits - 1, lnbits) = rb_index.read();

    vb_index = i_addr.read()(ibits + lnbits - 1, lnbits);
    tagi_we = i_wstrb.read().or_reduce();
    vb_tagi_wdata(TAG_BITS-1, 0) = i_addr.read()(abus - 1, ibits + lnbits);
    vb_tagi_wdata(TAG_WITH_FLAGS-1, TAG_BITS) = i_wflags.read();

    if (!async_reset_ && i_nrst.read() == 0) {
        vb_tagi_wdata = 0;
        vb_index = 0;
    }

    wb_index = vb_index;
    tagi_wdata = vb_tagi_wdata;

    o_raddr = vb_raddr;
    o_rdata = vb_rdata;
    o_rflags = tago_rdata.read()(TAG_WITH_FLAGS-1, TAG_BITS);
    o_hit = v_hit;
}

template <int abus, int ibits, int lnbits, int flbits>
void TagMem<abus, ibits, lnbits, flbits>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        rb_tagi = 0;
        rb_index = 0;
    } else {
        rb_tagi = tagi_wdata.read()(TAG_BITS-1, 0);
        rb_index = wb_index;
    }
}

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TAGMEM_H__
