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

#ifndef __DEBUGGER_RIVERLIB_CACHE_TAGMEMNWAY_H__
#define __DEBUGGER_RIVERLIB_CACHE_TAGMEMNWAY_H__

#include <systemc.h>
#include "tagmem.h"
#include "lrunway.h"

namespace debugger {

/**
    abus = system bus address width (64 or 32 bits)
    waybits = log2 of number of ways bits (2 for 4 ways cache; 3 for 8 ways)
    ibits = lines memory address width (usually 6..8)
    lnbits = One line bits: log2(bytes_per_line)
    flbits = total flags number saved with address tag
*/
template <int abus, int waybits, int ibits, int lnbits, int flbits>
SC_MODULE(TagMemNWay) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_cs;
    sc_in<bool> i_flush;
    sc_in<sc_uint<abus>> i_addr;
    sc_in<sc_biguint<8*(1<<lnbits)>> i_wdata;
    sc_in<sc_uint<(1<<lnbits)>> i_wstrb;
    sc_in<sc_uint<flbits>> i_wflags;
    sc_out<sc_uint<abus>> o_raddr;
    sc_out<sc_biguint<8*(1<<lnbits)>> o_rdata;
    sc_out<sc_uint<flbits>> o_rflags;
    sc_out<bool> o_hit;

    void comb();
    void registers();

    SC_HAS_PROCESS(TagMemNWay);

    TagMemNWay(sc_module_name name_, bool async_reset)
        : sc_module(name_),
        i_clk("i_clk"),
        i_nrst("i_nrst"),
        i_cs("i_cs"),
        i_flush("i_flush"),
        i_addr("i_addr"),
        i_wdata("i_wdata"),
        i_wstrb("i_wstrb"),
        i_wflags("i_wflags"),
        o_raddr("o_raddr"),
        o_rdata("o_rdata"),
        o_rflags("o_rflags"),
        o_hit("o_hit") {
        async_reset_ = async_reset;

        char tstr1[32] = "way0";
        for (int i = 0; i < NWAYS; i++) {
            tstr1[3] = '0' + static_cast<char>(i);
            wayx[i] = new TagMem<abus, ibits, lnbits, flbits>(
                                tstr1, async_reset, i);
            wayx[i]->i_clk(i_clk);
            wayx[i]->i_nrst(i_nrst);
            wayx[i]->i_addr(way_i[i].addr);
            wayx[i]->i_wstrb(way_i[i].wstrb);
            wayx[i]->i_wdata(way_i[i].wdata);
            wayx[i]->i_wflags(way_i[i].wflags);
            wayx[i]->o_raddr(way_o[i].raddr);
            wayx[i]->o_rdata(way_o[i].rdata);
            wayx[i]->o_rflags(way_o[i].rflags);
            wayx[i]->o_hit(way_o[i].hit);
        }

        lru = new lrunway<ibits, waybits>("lru0");
        lru->i_clk(i_clk);
        lru->i_flush(lrui_flush);
        lru->i_addr(lrui_addr);
        lru->i_we(lrui_we);
        lru->i_lru(lrui_lru);
        lru->o_lru(lruo_lru);

        SC_METHOD(comb);
        sensitive << i_flush;
        sensitive << i_addr;
        sensitive << i_wdata;
        sensitive << i_wstrb;
        sensitive << i_wflags;
        for (int i = 0; i < NWAYS; i++) {
            sensitive << way_o[i].raddr;
            sensitive << way_o[i].rdata;
            sensitive << way_o[i].rflags;
            sensitive << way_o[i].hit;
        }
        sensitive << lruo_lru;
        sensitive << r.req_addr;
        sensitive << r.re;

        SC_METHOD(registers);
        sensitive << i_nrst;
        sensitive << i_clk.pos();
    }

    virtual ~TagMemNWay() {
        for (int i = 0; i < NWAYS; i++) {
            delete wayx[i];
        }
        delete lru;
    }

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int NWAYS = 1 << waybits;
    static const int FL_VALID = 0;

    struct WayInType {
        sc_signal<sc_uint<abus>> addr;
        sc_signal<sc_uint<(1<<lnbits)>> wstrb;
        sc_signal<sc_biguint<8*(1<<lnbits)>> wdata;
        sc_signal<sc_uint<flbits>> wflags;
    };
   struct WayOutType {
        sc_signal<sc_uint<abus>> raddr;
        sc_signal<sc_biguint<8*(1<<lnbits)>> rdata;
        sc_signal<sc_uint<flbits>> rflags;
        sc_signal<bool> hit;
    };

    TagMem<abus, ibits, lnbits, flbits> *wayx[NWAYS];
    lrunway<ibits, waybits> *lru;

    WayInType way_i[NWAYS];
    WayOutType way_o[NWAYS];

    sc_signal<bool> lrui_flush;
    sc_signal<sc_uint<ibits>> lrui_addr;
    sc_signal<bool> lrui_we;
    sc_signal<sc_uint<waybits>> lrui_lru;
    sc_signal<sc_uint<waybits>> lruo_lru;

    sc_uint<abus> mux_raddr;
    sc_biguint<8*(1<<lnbits)> mux_rdata;
    sc_uint<flbits> mux_rflags;

    struct RegistersType {
        sc_signal<sc_uint<abus>> req_addr;
        sc_signal<bool> re;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.req_addr = 0;
        iv.re = 0;
    }

    bool async_reset_;
};


template <int abus, int waybits, int ibits, int lnbits, int flbits>
void TagMemNWay<abus, waybits, ibits, lnbits, flbits>::comb() {
    sc_uint<waybits> vb_wayidx_o;
    sc_uint<ibits> vb_lineadr;
    bool v_lrui_we;
    bool hit;

    hit = 0;
    v_lrui_we = 0;
    v.req_addr = i_addr.read();
    v.re = i_cs.read();

    vb_lineadr = i_addr.read()(ibits+lnbits-1, lnbits);

    if (i_cs.read() == 1) {
        if (i_wstrb.read() != 0) {
            v_lrui_we = 1;
        }
    }

    vb_wayidx_o = lruo_lru.read();
    for (int i = 0; i < NWAYS; i++) {
        if (i_flush.read() == 1) {
            // Use lsb address part for the way selection
            vb_wayidx_o = i_addr.read()(waybits-1, 0);
        } else if (way_o[i].hit.read() == 1 &&
                    way_o[i].rflags.read()[FL_VALID] == 1) {
            hit = 1;
            vb_wayidx_o = i;
            v_lrui_we = r.re.read();
        }
    }

    mux_raddr = way_o[vb_wayidx_o.to_int()].raddr;
    mux_rdata = way_o[vb_wayidx_o.to_int()].rdata;
    mux_rflags = way_o[vb_wayidx_o.to_int()].rflags;

    /**
        Warning: we can write only into previously read line,
                    if the previuosly read line is hit and contains valid flags
                    HIGH we modify it. Otherwise, we write into displacing line.
    */
    for (int i = 0; i < NWAYS; i++) {
        way_i[i].addr = i_addr.read();
        way_i[i].wdata = i_wdata.read();
        if ((i_flush.read() == 1 || i_cs.read() == 1) && vb_wayidx_o == i) {
            way_i[i].wstrb = i_wstrb.read();
        } else {
            way_i[i].wstrb = 0;
        }
        way_i[i].wflags = i_wflags.read();
    }

    lrui_flush = i_flush.read();
    lrui_addr = vb_lineadr;
    lrui_we = v_lrui_we;
    lrui_lru = vb_wayidx_o;

    if (!async_reset_ && i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_raddr = mux_raddr;
    o_rdata = mux_rdata;
    o_rflags = mux_rflags;
    o_hit = hit;
}

template <int abus, int waybits, int ibits, int lnbits, int flbits>
void TagMemNWay<abus, waybits, ibits, lnbits, flbits>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

template <int abus, int waybits, int ibits, int lnbits, int flbits>
void TagMemNWay<abus, waybits, ibits, lnbits, flbits>::
generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd)  {
    if (o_vcd) {
        sc_trace(o_vcd, i_cs, i_cs.name());
        sc_trace(o_vcd, i_flush, i_flush.name());
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wflags, i_wflags.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, o_hit, o_hit.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, way_o[0].rflags, pn + ".rflags0");
        sc_trace(o_vcd, way_o[1].rflags, pn + ".rflags1");
        sc_trace(o_vcd, way_o[2].rflags, pn + ".rflags2");
        sc_trace(o_vcd, way_o[3].rflags, pn + ".rflags3");
        sc_trace(o_vcd, way_o[0].hit, pn + ".hit0");
        sc_trace(o_vcd, way_o[1].hit, pn + ".hit1");
        sc_trace(o_vcd, way_o[2].hit, pn + ".hit2");
        sc_trace(o_vcd, way_o[3].hit, pn + ".hit3");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
    }
    lru->generateVCD(i_vcd, o_vcd);
    for (int i = 0; i < NWAYS; i++) {
        wayx[i]->generateVCD(i_vcd, o_vcd);
    }
}

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TAGMEMNWAY_H__
