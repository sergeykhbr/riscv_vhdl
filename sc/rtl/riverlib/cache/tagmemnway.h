// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "tagmem.h"
#include "lrunway.h"
#include "api_core.h"

namespace debugger {

template<int abus = 64,                                     // system bus address width (64 or 32 bits)
         int waybits = 2,                                   // log2 of number of ways bits (2 for 4 ways cache; 3 for 8 ways)
         int ibits = 6,                                     // lines memory address width (usually 6..8)
         int lnbits = 5,                                    // One line bits: log2(bytes_per_line)
         int flbits = 4,                                    // total flags number saved with address tag
         int snoop = 0>                                     // 0 Snoop port disabled; 1 Enabled (L2 caching)
SC_MODULE(TagMemNWay) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_direct_access;                            // lsb bits of address forms way index to access
    sc_in<bool> i_invalidate;
    sc_in<bool> i_re;
    sc_in<bool> i_we;
    sc_in<sc_uint<abus>> i_addr;
    sc_in<sc_biguint<(8 * (1 << lnbits))>> i_wdata;
    sc_in<sc_uint<(1 << lnbits)>> i_wstrb;
    sc_in<sc_uint<flbits>> i_wflags;
    sc_out<sc_uint<abus>> o_raddr;
    sc_out<sc_biguint<(8 * (1 << lnbits))>> o_rdata;
    sc_out<sc_uint<flbits>> o_rflags;
    sc_out<bool> o_hit;
    // L2 snoop port, active when snoop = 1
    sc_in<sc_uint<abus>> i_snoop_addr;
    sc_out<bool> o_snoop_ready;                             // single port memory not used for writing
    sc_out<sc_uint<flbits>> o_snoop_flags;

    void comb();
    void registers();

    SC_HAS_PROCESS(TagMemNWay);

    TagMemNWay(sc_module_name name,
               bool async_reset);
    virtual ~TagMemNWay();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int NWAYS = (1 << waybits);
    static const int FL_VALID = 0;

    struct WayInType {
        sc_signal<sc_uint<abus>> addr;
        sc_signal<sc_uint<(1 << lnbits)>> wstrb;
        sc_signal<sc_biguint<(8 * (1 << lnbits))>> wdata;
        sc_signal<sc_uint<flbits>> wflags;
        sc_signal<sc_uint<abus>> snoop_addr;
    };

    struct WayOutType {
        sc_signal<sc_uint<abus>> raddr;
        sc_signal<sc_biguint<(8 * (1 << lnbits))>> rdata;
        sc_signal<sc_uint<flbits>> rflags;
        sc_signal<bool> hit;
        sc_signal<sc_uint<flbits>> snoop_flags;
    };


    struct TagMemNWay_registers {
        sc_signal<sc_uint<abus>> req_addr;
        sc_signal<bool> direct_access;
        sc_signal<bool> invalidate;
        sc_signal<bool> re;
    } v, r;

    void TagMemNWay_r_reset(TagMemNWay_registers &iv) {
        iv.req_addr = 0ull;
        iv.direct_access = 0;
        iv.invalidate = 0;
        iv.re = 0;
    }

    sc_signal<bool> w_lrui_init;
    sc_signal<sc_uint<ibits>> wb_lrui_raddr;
    sc_signal<sc_uint<ibits>> wb_lrui_waddr;
    sc_signal<bool> w_lrui_up;
    sc_signal<bool> w_lrui_down;
    sc_signal<sc_uint<waybits>> wb_lrui_lru;
    sc_signal<sc_uint<waybits>> wb_lruo_lru;
    WayInType way_i[NWAYS];
    WayOutType way_o[NWAYS];

    TagMem<abus, ibits, lnbits, flbits, snoop> *wayx[NWAYS];
    lrunway<ibits, waybits> *lru0;

};

template<int abus, int waybits, int ibits, int lnbits, int flbits, int snoop>
TagMemNWay<abus, waybits, ibits, lnbits, flbits, snoop>::TagMemNWay(sc_module_name name,
                                                                    bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_direct_access("i_direct_access"),
    i_invalidate("i_invalidate"),
    i_re("i_re"),
    i_we("i_we"),
    i_addr("i_addr"),
    i_wdata("i_wdata"),
    i_wstrb("i_wstrb"),
    i_wflags("i_wflags"),
    o_raddr("o_raddr"),
    o_rdata("o_rdata"),
    o_rflags("o_rflags"),
    o_hit("o_hit"),
    i_snoop_addr("i_snoop_addr"),
    o_snoop_ready("o_snoop_ready"),
    o_snoop_flags("o_snoop_flags") {

    async_reset_ = async_reset;
    for (int i = 0; i < NWAYS; i++) {
        wayx[i] = 0;
    }
    lru0 = 0;

    for (int i = 0; i < NWAYS; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "wayx%d", i);
        wayx[i] = new TagMem<abus,
                             ibits,
                             lnbits,
                             flbits,
                             snoop>(tstr, async_reset);
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
        wayx[i]->i_snoop_addr(way_i[i].snoop_addr);
        wayx[i]->o_snoop_flags(way_o[i].snoop_flags);

    }

    lru0 = new lrunway<ibits,
                       waybits>("lru0");
    lru0->i_clk(i_clk);
    lru0->i_init(w_lrui_init);
    lru0->i_raddr(wb_lrui_raddr);
    lru0->i_waddr(wb_lrui_waddr);
    lru0->i_up(w_lrui_up);
    lru0->i_down(w_lrui_down);
    lru0->i_lru(wb_lrui_lru);
    lru0->o_lru(wb_lruo_lru);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_direct_access;
    sensitive << i_invalidate;
    sensitive << i_re;
    sensitive << i_we;
    sensitive << i_addr;
    sensitive << i_wdata;
    sensitive << i_wstrb;
    sensitive << i_wflags;
    sensitive << i_snoop_addr;
    sensitive << w_lrui_init;
    sensitive << wb_lrui_raddr;
    sensitive << wb_lrui_waddr;
    sensitive << w_lrui_up;
    sensitive << w_lrui_down;
    sensitive << wb_lrui_lru;
    sensitive << wb_lruo_lru;
    for (int i = 0; i < NWAYS; i++) {
        sensitive << way_i[i].addr;
        sensitive << way_i[i].wstrb;
        sensitive << way_i[i].wdata;
        sensitive << way_i[i].wflags;
        sensitive << way_i[i].snoop_addr;
    }
    for (int i = 0; i < NWAYS; i++) {
        sensitive << way_o[i].raddr;
        sensitive << way_o[i].rdata;
        sensitive << way_o[i].rflags;
        sensitive << way_o[i].hit;
        sensitive << way_o[i].snoop_flags;
    }
    sensitive << r.req_addr;
    sensitive << r.direct_access;
    sensitive << r.invalidate;
    sensitive << r.re;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int abus, int waybits, int ibits, int lnbits, int flbits, int snoop>
TagMemNWay<abus, waybits, ibits, lnbits, flbits, snoop>::~TagMemNWay() {
    for (int i = 0; i < NWAYS; i++) {
        if (wayx[i]) {
            delete wayx[i];
        }
    }
    if (lru0) {
        delete lru0;
    }
}

template<int abus, int waybits, int ibits, int lnbits, int flbits, int snoop>
void TagMemNWay<abus, waybits, ibits, lnbits, flbits, snoop>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_direct_access, i_direct_access.name());
        sc_trace(o_vcd, i_invalidate, i_invalidate.name());
        sc_trace(o_vcd, i_re, i_re.name());
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, i_wflags, i_wflags.name());
        sc_trace(o_vcd, o_raddr, o_raddr.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_rflags, o_rflags.name());
        sc_trace(o_vcd, o_hit, o_hit.name());
        sc_trace(o_vcd, i_snoop_addr, i_snoop_addr.name());
        sc_trace(o_vcd, o_snoop_ready, o_snoop_ready.name());
        sc_trace(o_vcd, o_snoop_flags, o_snoop_flags.name());
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.direct_access, pn + ".r_direct_access");
        sc_trace(o_vcd, r.invalidate, pn + ".r_invalidate");
        sc_trace(o_vcd, r.re, pn + ".r_re");
    }

    for (int i = 0; i < NWAYS; i++) {
        if (wayx[i]) {
            wayx[i]->generateVCD(i_vcd, o_vcd);
        }
    }
}

template<int abus, int waybits, int ibits, int lnbits, int flbits, int snoop>
void TagMemNWay<abus, waybits, ibits, lnbits, flbits, snoop>::comb() {
    sc_uint<abus> vb_raddr;
    sc_biguint<(8 * (1 << lnbits))> vb_rdata;
    sc_uint<flbits> vb_rflags;
    bool v_hit;
    sc_uint<waybits> vb_hit_idx;
    bool v_way_we;
    sc_uint<(1 << lnbits)> vb_wstrb;
    sc_uint<flbits> vb_wflags;
    bool v_snoop_ready;
    sc_uint<flbits> vb_snoop_flags;

    vb_raddr = 0;
    vb_rdata = 0;
    vb_rflags = 0;
    v_hit = 0;
    vb_hit_idx = 0;
    v_way_we = 0;
    vb_wstrb = 0;
    vb_wflags = 0;
    v_snoop_ready = 0;
    vb_snoop_flags = 0;

    v = r;


    v.direct_access = i_direct_access;
    v.invalidate = i_invalidate;
    v.re = i_re;
    v.req_addr = i_addr;

    vb_hit_idx = wb_lruo_lru;
    if (r.direct_access.read() == 1) {
        vb_hit_idx = r.req_addr.read()((waybits - 1), 0);
    } else {
        for (int i = 0; i < NWAYS; i++) {
            if (way_o[i].hit == 1) {
                vb_hit_idx = i;
            }
        }
    }
    vb_raddr = way_o[vb_hit_idx.to_int()].raddr;
    vb_rdata = way_o[vb_hit_idx.to_int()].rdata;
    vb_rflags = way_o[vb_hit_idx.to_int()].rflags;
    v_hit = way_o[vb_hit_idx.to_int()].hit;

    if (r.invalidate.read() == 1) {
        vb_wflags = 0;
        vb_wstrb = ~0ull;
    } else {
        vb_wflags = i_wflags;
        vb_wstrb = i_wstrb;
    }

    //     Warning: we can write only into previously read line,
    //                 if the previuosly read line is hit and contains valid flags
    //                 HIGH we modify it. Otherwise, we write into displacing line.

    for (int i = 0; i < NWAYS; i++) {
        way_i[i].addr = i_addr;
        way_i[i].wdata = i_wdata;
        way_i[i].wstrb = 0;
        way_i[i].wflags = vb_wflags;
        way_i[i].snoop_addr = i_snoop_addr;
    }

    v_way_we = (i_we || (r.invalidate && v_hit));
    if (v_way_we == 1) {
        way_i[vb_hit_idx.to_int()].wstrb = vb_wstrb;
    }

    v_snoop_ready = 1;
    if (snoop == 1) {
        for (int i = 0; i < NWAYS; i++) {
            // tagmem already cleared snoop flags if there's no snoop hit
            if (way_o[i].snoop_flags.read()[FL_VALID] == 1) {
                vb_snoop_flags = way_o[i].snoop_flags;
            }
        }
        // Writing into snoop tag memory, output value won't be valid on next clock
        if (v_way_we == 1) {
            v_snoop_ready = 0;
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        TagMemNWay_r_reset(v);
    }

    w_lrui_init = r.direct_access;
    wb_lrui_raddr = i_addr.read()(((ibits + lnbits) - 1), lnbits);
    wb_lrui_waddr = r.req_addr.read()(((ibits + lnbits) - 1), lnbits);
    w_lrui_up = (i_we || (v_hit && r.re));
    w_lrui_down = (v_hit && r.invalidate);
    wb_lrui_lru = vb_hit_idx;

    o_raddr = vb_raddr;
    o_rdata = vb_rdata;
    o_rflags = vb_rflags;
    o_hit = v_hit;
    o_snoop_ready = v_snoop_ready;
    o_snoop_flags = vb_snoop_flags;
}

template<int abus, int waybits, int ibits, int lnbits, int flbits, int snoop>
void TagMemNWay<abus, waybits, ibits, lnbits, flbits, snoop>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        TagMemNWay_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

