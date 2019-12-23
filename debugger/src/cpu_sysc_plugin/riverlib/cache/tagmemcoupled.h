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

#ifndef __DEBUGGER_RIVERLIB_CACHE_TAGMEMCOUPLED_H__
#define __DEBUGGER_RIVERLIB_CACHE_TAGMEMCOUPLED_H__

#include <systemc.h>
#include "tagmemnway.h"

namespace debugger {

/**
    abus = system bus address width (64 or 32 bits)
    waybits = log2 of number of ways bits (2 for 4 ways cache; 3 for 8 ways)
    ibits = lines memory address width (usually 6..8)
    lnbits = One line bits: log2(bytes_per_line)
    flbits = total flags number saved with address tag
*/
template <int abus, int waybits, int ibits, int lnbits, int flbits>
SC_MODULE(TagMemCoupled) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_cs;
    sc_in<bool> i_flush;
    sc_in<sc_uint<abus>> i_addr;
    sc_in<sc_biguint<8*(1<<lnbits)>> i_wdata;
    sc_in<sc_uint<(1<<lnbits)>> i_wstrb;
    sc_in<sc_uint<flbits>> i_wflags;
    sc_out<sc_uint<abus>> o_raddr;
    sc_out<sc_biguint<8*(1<<lnbits)+16>> o_rdata;
    sc_out<sc_uint<flbits>> o_rflags;
    sc_out<bool> o_hit;
    sc_out<bool> o_miss_next;

    void comb();
    void registers();

    SC_HAS_PROCESS(TagMemCoupled);

    TagMemCoupled(sc_module_name name_, bool async_reset)
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
        o_hit("o_hit"),
        o_miss_next("o_miss_next") {
        async_reset_ = async_reset;

        char tagname[32] = "mem0";
        for (int i = 0; i < WAY_SubNum; i++) {
            tagname[3] = '0' + static_cast<char>(i);
            mem[i] = new TagMemNWay<abus,
                                    waybits,
                                    ibits-1,
                                    lnbits,
                                    flbits>(tagname, async_reset);

            mem[i]->i_clk(i_clk);
            mem[i]->i_nrst(i_nrst);
            mem[i]->i_cs(linei[i].cs);
            mem[i]->i_flush(linei[i].flush);
            mem[i]->i_addr(linei[i].addr);
            mem[i]->i_wdata(linei[i].wdata);
            mem[i]->i_wstrb(linei[i].wstrb);
            mem[i]->i_wflags(linei[i].wflags);
            mem[i]->o_raddr(lineo[i].raddr);
            mem[i]->o_rdata(lineo[i].rdata);
            mem[i]->o_rflags(lineo[i].rflags);
            mem[i]->o_hit(lineo[i].hit);
        }

        SC_METHOD(comb);
        sensitive << i_cs;
        sensitive << i_flush;
        sensitive << i_addr;
        sensitive << i_wdata;
        sensitive << i_wstrb;
        sensitive << i_wflags;
        for (int i = 0; i < WAY_SubNum; i++) {
            sensitive << lineo[i].raddr;
            sensitive << lineo[i].rdata;
            sensitive << lineo[i].rflags;
            sensitive << lineo[i].hit;
        }
        sensitive << r.req_addr;
        sensitive << r.re;

        SC_METHOD(registers);
        sensitive << i_nrst;
        sensitive << i_clk.pos();
    }

    virtual ~TagMemCoupled() {
        for (int i = 0; i < WAY_SubNum; i++) {
            delete mem[i];
        }
    }

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int TAG_START = abus - (ibits + lnbits);

    enum EWays {
        WAY_EVEN,
        WAY_ODD,
        WAY_SubNum
    };

    struct tagmem_in_type {
        sc_signal<bool> cs;
        sc_signal<sc_uint<abus>> addr;
        sc_signal<sc_biguint<8*(1<<lnbits)>> wdata;
        sc_signal<sc_uint<(1<<lnbits)>> wstrb;
        sc_signal<sc_uint<flbits>> wflags;
        sc_signal<bool> flush;
    };

    struct tagmem_out_type {
        sc_signal<sc_uint<abus>> raddr;
        sc_signal<sc_biguint<8*(1<<lnbits)>> rdata;
        sc_signal<sc_uint<flbits>> rflags;
        sc_signal<bool> hit;
    };

    tagmem_in_type linei[WAY_SubNum];
    tagmem_out_type lineo[WAY_SubNum];

    TagMemNWay<abus,
            waybits,
            ibits-1,
            lnbits,
            flbits> *mem[WAY_SubNum];

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
void TagMemCoupled<abus, waybits, ibits, lnbits, flbits>::comb() {
    bool v_addr_sel;
    bool v_addr_sel_r;
    bool v_use_overlay;
    bool v_use_overlay_r;
    sc_uint<ibits> vb_index;
    sc_uint<ibits> vb_index_next;
    sc_uint<abus> vb_addr_next;
    sc_uint<abus> vb_addr_tag_direct;
    sc_uint<abus> vb_addr_tag_next;
    const int ones = (1 << (lnbits-1)) - 1;

    v.req_addr = i_addr.read();
    v_addr_sel = i_addr.read()[lnbits];
    v_addr_sel_r = r.req_addr.read()[lnbits];

    vb_addr_next = i_addr.read() + (1 << lnbits);

    vb_index = i_addr.read()(ibits+lnbits-1, lnbits);
    vb_index_next = vb_addr_next(ibits+lnbits-1, lnbits);

    v_use_overlay = 0;
    if (i_addr.read()(lnbits-1, 1) == ones) {
        v_use_overlay = 1;
    }
    v_use_overlay_r = 0;
    if (r.req_addr.read()(lnbits-1, 1) == ones) {
        v_use_overlay_r = 1;
    }


    // Change the bit order in the requested address:
    //    [tag][line_idx][odd/evenbit][line_bytes] on
    //    [tag][1'b0]    [line_idx]   [line_bytes]
    //
    // Example (abus=32; ibits=7; lnbits=5;):
    //   [4:0]   byte in line           [4:0]
    //   [11:5]  line index             {[1'b0],[11:6]}
    //   [31:12] tag                    [31:12]
    vb_addr_tag_direct = i_addr.read();
    vb_addr_tag_direct(ibits + lnbits - 1, lnbits) = vb_index >> 1;

    vb_addr_tag_next = vb_addr_next;
    vb_addr_tag_next(ibits + lnbits - 1, lnbits) = vb_index_next >> 1;

    if (v_addr_sel == 0) {
        linei[WAY_EVEN].addr = vb_addr_tag_direct;
        linei[WAY_ODD].addr = vb_addr_tag_next;
    } else {
        linei[WAY_EVEN].addr =vb_addr_tag_next;
        linei[WAY_ODD].addr =  vb_addr_tag_direct;
    }

    linei[WAY_EVEN].flush = i_flush.read() && !v_addr_sel;
    linei[WAY_ODD].flush = i_flush.read() && v_addr_sel;

    linei[WAY_EVEN].cs = i_cs.read() && (!v_addr_sel || v_use_overlay);
    linei[WAY_ODD].cs = i_cs.read() && (v_addr_sel || v_use_overlay);

    linei[WAY_EVEN].wdata = i_wdata.read();
    linei[WAY_ODD].wdata = i_wdata.read();

    linei[WAY_EVEN].wstrb = i_wstrb.read();
    linei[WAY_ODD].wstrb = i_wstrb.read();

    linei[WAY_EVEN].wflags = i_wflags.read();
    linei[WAY_ODD].wflags = i_wflags.read();

    // Form output:
    sc_uint<abus> vb_raddr_tag;
    sc_uint<abus> vb_o_raddr;
    sc_biguint<8*(1<<lnbits)+16> vb_o_rdata;
    bool v_o_hit;
    bool v_o_miss_next;
    sc_uint<flbits> vb_o_rflags;
    if (v_addr_sel_r == 0) {
        vb_o_rdata = (lineo[WAY_ODD].rdata.read()(15, 0), lineo[WAY_EVEN].rdata);
        vb_raddr_tag = lineo[WAY_EVEN].raddr;
        vb_o_rflags = lineo[WAY_EVEN].rflags;

        if (v_use_overlay_r == 0) {
            v_o_hit = lineo[WAY_EVEN].hit;
            v_o_miss_next = 0;
        } else {
            v_o_hit = lineo[WAY_EVEN].hit && lineo[WAY_ODD].hit;
            v_o_miss_next = lineo[WAY_EVEN].hit && !lineo[WAY_ODD].hit;
        }
    } else {
        vb_o_rdata = (lineo[WAY_EVEN].rdata.read()(15, 0), lineo[WAY_ODD].rdata);
        vb_raddr_tag = lineo[WAY_ODD].raddr;
        vb_o_rflags = lineo[WAY_ODD].rflags;

        if (v_use_overlay_r == 0) {
            v_o_hit = lineo[WAY_ODD].hit;
            v_o_miss_next = 0;
        } else {
            v_o_hit = lineo[WAY_ODD].hit && lineo[WAY_EVEN].hit;
            v_o_miss_next = lineo[WAY_ODD].hit && !lineo[WAY_EVEN].hit;
        }
    }

    vb_o_raddr = vb_raddr_tag;
    vb_o_raddr[lnbits] = v_addr_sel_r;
    vb_o_raddr(ibits + lnbits - 1, lnbits + 1) =
                    vb_raddr_tag(ibits + lnbits - 2, lnbits);

    o_raddr = vb_o_raddr;
    o_rdata = vb_o_rdata;
    o_rflags = vb_o_rflags;
    o_hit = v_o_hit;
    o_miss_next = v_o_miss_next;
}

template <int abus, int waybits, int ibits, int lnbits, int flbits>
void TagMemCoupled<abus, waybits, ibits, lnbits, flbits>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

template <int abus, int waybits, int ibits, int lnbits, int flbits>
void TagMemCoupled<abus, waybits, ibits, lnbits, flbits>::
generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd)  {
    if (o_vcd) {
        sc_trace(o_vcd, i_cs, i_cs.name());
        sc_trace(o_vcd, i_flush, i_flush.name());
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wflags, i_wflags.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, o_raddr, o_raddr.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_hit, o_hit.name());
        sc_trace(o_vcd, o_miss_next, o_miss_next.name());

        std::string pn(name());
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
    }
    for (int i = 0; i < WAY_SubNum; i++) {
        mem[i]->generateVCD(i_vcd, o_vcd);
    }
}

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TAGMEMCOUPLED_H__
