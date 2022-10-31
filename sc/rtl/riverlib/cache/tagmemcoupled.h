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
#include "tagmemnway.h"
#include "api_core.h"

namespace debugger {

template<int abus = 64,                                     // system bus address width (64 or 32 bits)
         int waybits = 2,                                   // log2 of number of ways bits (2 for 4 ways cache; 3 for 8 ways)
         int ibits = 6,                                     // lines memory address width (usually 6..8)
         int lnbits = 5,                                    // One line bits: log2(bytes_per_line)
         int flbits = 4>                                    // total flags number saved with address tag
SC_MODULE(TagMemCoupled) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_direct_access;
    sc_in<bool> i_invalidate;
    sc_in<bool> i_re;
    sc_in<bool> i_we;
    sc_in<sc_uint<abus>> i_addr;
    sc_in<sc_biguint<(8 * (1 << lnbits))>> i_wdata;
    sc_in<sc_uint<(1 << lnbits)>> i_wstrb;
    sc_in<sc_uint<flbits>> i_wflags;
    sc_out<sc_uint<abus>> o_raddr;
    sc_out<sc_biguint<((8 * (1 << lnbits)) + 32)>> o_rdata;
    sc_out<sc_uint<flbits>> o_rflags;
    sc_out<bool> o_hit;
    sc_out<bool> o_hit_next;

    void comb();
    void registers();

    SC_HAS_PROCESS(TagMemCoupled);

    TagMemCoupled(sc_module_name name,
                  bool async_reset);


 private:
    bool async_reset_;

    static const int LINE_SZ = (1 << lnbits);
    static const int TAG_START = (abus - (ibits + lnbits));
    static const int EVEN = 0;
    static const int ODD = 1;
    static const int MemTotal = 2;

    struct tagmem_in_type {
        sc_signal<bool> direct_access;
        sc_signal<bool> invalidate;
        sc_signal<bool> re;
        sc_signal<bool> we;
        sc_signal<sc_uint<abus>> addr;
        sc_signal<sc_biguint<(8 * (1 << lnbits))>> wdata;
        sc_signal<sc_uint<(1 << lnbits)>> wstrb;
        sc_signal<sc_uint<flbits>> wflags;
        sc_signal<sc_uint<abus>> snoop_addr;
    };

    struct tagmem_out_type {
        sc_signal<sc_uint<abus>> raddr;
        sc_signal<sc_biguint<(8 * (1 << lnbits))>> rdata;
        sc_signal<sc_uint<flbits>> rflags;
        sc_signal<bool> hit;
        sc_signal<bool> snoop_ready;
        sc_signal<sc_uint<flbits>> snoop_flags;
    };


    struct TagMemCoupled_registers {
        sc_signal<sc_uint<abus>> req_addr;
    } v, r;

    void TagMemCoupled_r_reset(TagMemCoupled_registers &iv) {
        iv.req_addr = 0ull;
    }

    tagmem_in_type linei[MemTotal];
    tagmem_out_type lineo[MemTotal];

    TagMemNWay<abus, waybits, (ibits - 1), lnbits, flbits, 0> *memx[MemTotal];

};

template<int abus, int waybits, int ibits, int lnbits, int flbits>
TagMemCoupled<abus, waybits, ibits, lnbits, flbits>::TagMemCoupled(sc_module_name name,
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
    o_hit_next("o_hit_next") {

    async_reset_ = async_reset;
    for (int i = 0; i < MemTotal; i++) {
        memx[i] = 0;
    }

    for (int i = 0; i < MemTotal; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "memx%d", i);
        memx[i] = new TagMemNWay<abus,
                                 waybits,
                                 (ibits - 1),
                                 lnbits,
                                 flbits,
                                 0>(tstr, async_reset);
        memx[i]->i_clk(i_clk);
        memx[i]->i_nrst(i_nrst);
        memx[i]->i_direct_access(linei[i].direct_access);
        memx[i]->i_invalidate(linei[i].invalidate);
        memx[i]->i_re(linei[i].re);
        memx[i]->i_we(linei[i].we);
        memx[i]->i_addr(linei[i].addr);
        memx[i]->i_wdata(linei[i].wdata);
        memx[i]->i_wstrb(linei[i].wstrb);
        memx[i]->i_wflags(linei[i].wflags);
        memx[i]->o_raddr(lineo[i].raddr);
        memx[i]->o_rdata(lineo[i].rdata);
        memx[i]->o_rflags(lineo[i].rflags);
        memx[i]->o_hit(lineo[i].hit);
        memx[i]->i_snoop_addr(linei[i].snoop_addr);
        memx[i]->o_snoop_ready(lineo[i].snoop_ready);
        memx[i]->o_snoop_flags(lineo[i].snoop_flags);

    }


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
    for (int i = 0; i < MemTotal; i++) {
        sensitive << linei[i].direct_access;
        sensitive << linei[i].invalidate;
        sensitive << linei[i].re;
        sensitive << linei[i].we;
        sensitive << linei[i].addr;
        sensitive << linei[i].wdata;
        sensitive << linei[i].wstrb;
        sensitive << linei[i].wflags;
        sensitive << linei[i].snoop_addr;
    }
    for (int i = 0; i < MemTotal; i++) {
        sensitive << lineo[i].raddr;
        sensitive << lineo[i].rdata;
        sensitive << lineo[i].rflags;
        sensitive << lineo[i].hit;
        sensitive << lineo[i].snoop_ready;
        sensitive << lineo[i].snoop_flags;
    }
    sensitive << r.req_addr;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int abus, int waybits, int ibits, int lnbits, int flbits>
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
    sc_uint<abus> vb_raddr_tag;
    sc_uint<abus> vb_o_raddr;
    sc_biguint<((8 * (1 << lnbits)) + 32)> vb_o_rdata;
    bool v_o_hit;
    bool v_o_hit_next;
    sc_uint<flbits> vb_o_rflags;

    v_addr_sel = 0;
    v_addr_sel_r = 0;
    v_use_overlay = 0;
    v_use_overlay_r = 0;
    vb_index = 0;
    vb_index_next = 0;
    vb_addr_next = 0;
    vb_addr_tag_direct = 0;
    vb_addr_tag_next = 0;
    vb_raddr_tag = 0;
    vb_o_raddr = 0;
    vb_o_rdata = 0;
    v_o_hit = 0;
    v_o_hit_next = 0;
    vb_o_rflags = 0;

    v = r;


    v.req_addr = i_addr;
    v_addr_sel = i_addr.read()[lnbits];
    v_addr_sel_r = r.req_addr.read()[lnbits];

    vb_addr_next = (i_addr.read() + LINE_SZ);

    vb_index = i_addr.read()(((ibits + lnbits) - 1), lnbits);
    vb_index_next = vb_addr_next(((ibits + lnbits) - 1), lnbits);

    if (i_addr.read()((lnbits - 1), 2).and_reduce() == 1) {
        v_use_overlay = 1;
    }
    if (r.req_addr.read()((lnbits - 1), 2).and_reduce() == 1) {
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
    vb_addr_tag_direct = i_addr;
    vb_addr_tag_direct(((ibits + lnbits) - 1), lnbits) = (vb_index >> 1);

    vb_addr_tag_next = vb_addr_next;
    vb_addr_tag_next(((ibits + lnbits) - 1), lnbits) = (vb_index_next >> 1);

    if (v_addr_sel == 0) {
        linei[EVEN].addr = vb_addr_tag_direct;
        linei[EVEN].wstrb = i_wstrb;
        linei[ODD].addr = vb_addr_tag_next;
        linei[ODD].wstrb = 0;
    } else {
        linei[EVEN].addr = vb_addr_tag_next;
        linei[EVEN].wstrb = 0;
        linei[ODD].addr = vb_addr_tag_direct;
        linei[ODD].wstrb = i_wstrb;
    }

    linei[EVEN].direct_access = (i_direct_access && ((!v_addr_sel) || v_use_overlay));
    linei[ODD].direct_access = (i_direct_access && (v_addr_sel || v_use_overlay));

    linei[EVEN].invalidate = (i_invalidate && ((!v_addr_sel) || v_use_overlay));
    linei[ODD].invalidate = (i_invalidate && (v_addr_sel || v_use_overlay));

    linei[EVEN].re = (i_re && ((!v_addr_sel) || v_use_overlay));
    linei[ODD].re = (i_re && (v_addr_sel || v_use_overlay));

    linei[EVEN].we = (i_we && ((!v_addr_sel) || v_use_overlay));
    linei[ODD].we = (i_we && (v_addr_sel || v_use_overlay));

    linei[EVEN].wdata = i_wdata;
    linei[ODD].wdata = i_wdata;

    linei[EVEN].wflags = i_wflags;
    linei[ODD].wflags = i_wflags;

    // Form output:
    if (v_addr_sel_r == 0) {
        vb_o_rdata = (lineo[ODD].rdata.read()(31, 0), lineo[EVEN].rdata);
        vb_raddr_tag = lineo[EVEN].raddr;
        vb_o_rflags = lineo[EVEN].rflags;

        v_o_hit = lineo[EVEN].hit;
        if (v_use_overlay_r == 0) {
            v_o_hit_next = lineo[EVEN].hit;
        } else {
            v_o_hit_next = lineo[ODD].hit;
        }
    } else {
        vb_o_rdata = (lineo[EVEN].rdata.read()(31, 0), lineo[ODD].rdata);
        vb_raddr_tag = lineo[ODD].raddr;
        vb_o_rflags = lineo[ODD].rflags;

        v_o_hit = lineo[ODD].hit;
        if (v_use_overlay_r == 0) {
            v_o_hit_next = lineo[ODD].hit;
        } else {
            v_o_hit_next = lineo[EVEN].hit;
        }
    }

    vb_o_raddr = vb_raddr_tag;
    vb_o_raddr[lnbits] = v_addr_sel_r;
    vb_o_raddr(((ibits + lnbits) - 1), (lnbits + 1)) = vb_raddr_tag(((ibits + lnbits) - 2), lnbits);

    o_raddr = vb_o_raddr;
    o_rdata = vb_o_rdata;
    o_rflags = vb_o_rflags;
    o_hit = v_o_hit;
    o_hit_next = v_o_hit_next;
}

template<int abus, int waybits, int ibits, int lnbits, int flbits>
void TagMemCoupled<abus, waybits, ibits, lnbits, flbits>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        TagMemCoupled_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

