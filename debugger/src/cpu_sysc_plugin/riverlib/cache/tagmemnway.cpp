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

#include "tagmemnway.h"

namespace debugger {

TagMemNWay::TagMemNWay(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_cs("i_cs"),
    i_flush("i_flush"),
    i_addr("i_addr"),
    i_wdata("i_wdata"),
    i_wstrb("i_wstrb"),
    i_wdirty("i_wdirty"),
    i_wload_fault("i_wload_fault"),
    i_wexecutable("i_wexecutable"),
    i_wreadable("i_wreadable"),
    i_wwritable("i_wwritable"),
    o_raddr("o_raddr"),
    o_rdata("o_rdata"),
    o_rvalid("o_rvalid"),
    o_rdirty("o_rdirty"),
    o_rload_fault("o_rload_fault"),
    o_rexecutable("o_rexecutable"),
    o_rreadable("o_rreadable"),
    o_rwritable("o_rwritable") {
    async_reset_ = async_reset;

    char tstr1[32] = "way0";
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        tstr1[3] = '0' + static_cast<char>(i);
        wayx[i] = new TagWayMem<CFG_DINDEX_WIDTH, 
                                4*BUS_DATA_BYTES,
                                DTAG_SIZE,
                                DTAG_FL_TOTAL>(tstr1, async_reset, i);
        wayx[i]->i_clk(i_clk);
        wayx[i]->i_nrst(i_nrst);
        wayx[i]->i_addr(way_i[i].addr);
        wayx[i]->i_wstrb(way_i[i].wstrb);
        wayx[i]->i_wdata(way_i[i].wdata);
        wayx[i]->i_wflags(way_i[i].wflags);
        wayx[i]->i_valid(way_i[i].valid);
        wayx[i]->o_rdata(way_o[i].rdata);
        wayx[i]->o_rflags(way_o[i].rflags);
        wayx[i]->o_valid(way_o[i].valid);
    }

    lru = new lru4way<CFG_DINDEX_WIDTH>("lru0");
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
    sensitive << i_wdirty;
    sensitive << i_wload_fault;
    sensitive << i_wexecutable;
    sensitive << i_wreadable;
    sensitive << i_wwritable;
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        sensitive << way_o[i].rdata;
        sensitive << way_o[i].rflags;
        sensitive << way_o[i].valid;
    }
    sensitive << lruo_lru;
    sensitive << r.req_addr;
    sensitive << r.re;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

TagMemNWay::~TagMemNWay() {
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        delete wayx[i];
    }
    delete lru;
}

void TagMemNWay::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_cs, i_cs.name());
        sc_trace(o_vcd, i_flush, i_flush.name());
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wdirty, i_wdirty.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, o_rvalid, o_rvalid.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, way_o[0].valid, pn + ".valid0");
        sc_trace(o_vcd, way_o[1].valid, pn + ".valid1");
        sc_trace(o_vcd, way_o[2].valid, pn + ".valid2");
        sc_trace(o_vcd, way_o[3].valid, pn + ".valid3");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
    }
    lru->generateVCD(i_vcd, o_vcd);
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        wayx[i]->generateVCD(i_vcd, o_vcd);
    }
}

void TagMemNWay::comb() {
    sc_uint<2> vb_wayidx;
    sc_uint<CFG_DINDEX_WIDTH> vb_lineadr;
    sc_uint<DTAG_FL_TOTAL> vb_wflags;
    bool v_lrui_we;
    bool hit;

    hit = 0;
    v_lrui_we = 0;
    v.req_addr = i_addr.read();
    v.re = i_cs.read();

    vb_lineadr = i_addr.read()(DINDEX_END, DINDEX_START);

    if (i_cs.read() == 1) {
        if (i_wstrb.read() != 0) {
            v_lrui_we = 1;
        }
    }

    vb_wayidx = lruo_lru.read();
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        if (way_o[i].valid.read() == 1) {
            vb_wayidx = i;
            v_lrui_we = r.re.read();
            hit = 1;
        }
    }

    mux_rdata = way_o[vb_wayidx.to_int()].rdata;
    mux_rflags = way_o[vb_wayidx.to_int()].rflags;

    vb_wflags[DTAG_FL_DIRTY] = i_wdirty.read();
    vb_wflags[DTAG_FL_LOAD_FAULT] = i_wload_fault.read();
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        way_i[i].valid = !i_flush.read();
        way_i[i].addr = i_addr.read();
        way_i[i].wdata = i_wdata.read();
        if (i_cs.read() == 1 && vb_wayidx == i) {
            way_i[i].wstrb = i_wstrb.read();
        } else {
            way_i[i].wstrb = 0;
        }
        way_i[i].wflags = vb_wflags;
    }

    lrui_flush = i_flush.read();
    lrui_addr = vb_lineadr;
    lrui_we = v_lrui_we;
    lrui_lru = vb_wayidx;

    if (!async_reset_ && i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_raddr = r.req_addr.read();
    o_rdata = mux_rdata;
    o_rvalid = hit;
    o_rdirty = mux_rflags[DTAG_FL_DIRTY];
    o_rload_fault = mux_rflags[DTAG_FL_LOAD_FAULT];
    o_rexecutable = 0;
    o_rreadable = 0;
    o_rwritable = 0;
}

void TagMemNWay::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

