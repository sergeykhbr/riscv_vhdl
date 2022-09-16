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
#include "../../techmap/mem/ram_cache_bwe_tech.h"
#include "../../techmap/mem/ram_tech.h"
#include "api_core.h"

namespace debugger {

template<int abus = 64,                                     // system bus address width (64 or 32 bits)
         int ibits = 6,                                     // lines memory address width (usually 6..8)
         int lnbits = 5,                                    // One line bits: log2(bytes_per_line)
         int flbits = 4,                                    // total flags number saved with address tag
         int snoop = 0>                                     // 0 Snoop port disabled; 1 Enabled (L2 caching)
SC_MODULE(TagMem) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<abus>> i_addr;
    sc_in<sc_uint<(1 << lnbits)>> i_wstrb;
    sc_in<sc_biguint<(8 * (1 << lnbits))>> i_wdata;
    sc_in<sc_uint<flbits>> i_wflags;
    sc_out<sc_uint<abus>> o_raddr;
    sc_out<sc_biguint<(8 * (1 << lnbits))>> o_rdata;
    sc_out<sc_uint<flbits>> o_rflags;
    sc_out<bool> o_hit;
    // L2 snoop port, active when snoop = 1
    sc_in<sc_uint<abus>> i_snoop_addr;
    sc_out<sc_uint<flbits>> o_snoop_flags;

    void comb();
    void registers();

    SC_HAS_PROCESS(TagMem);

    TagMem(sc_module_name name,
           bool async_reset);
    virtual ~TagMem();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int TAG_BITS = ((abus - ibits) - lnbits);
    static const int TAG_WITH_FLAGS = (TAG_BITS + flbits);

    struct TagMem_registers {
        sc_signal<sc_uint<TAG_BITS>> tagaddr;
        sc_signal<sc_uint<ibits>> index;
        sc_signal<sc_uint<TAG_BITS>> snoop_tagaddr;
    } v, r;

    void TagMem_r_reset(TagMem_registers &iv) {
        iv.tagaddr = 0ull;
        iv.index = 0;
        iv.snoop_tagaddr = 0ull;
    }

    sc_signal<sc_uint<ibits>> wb_index;
    sc_signal<sc_uint<TAG_WITH_FLAGS>> wb_tago_rdata;
    sc_signal<sc_uint<TAG_WITH_FLAGS>> wb_tagi_wdata;
    sc_signal<bool> w_tagi_we;
    sc_signal<sc_uint<ibits>> wb_snoop_index;
    sc_signal<sc_uint<TAG_BITS>> wb_snoop_tagaddr;
    sc_signal<sc_uint<TAG_WITH_FLAGS>> wb_tago_snoop_rdata;

    ram_cache_bwe_tech<ibits, (8 * (1 << lnbits))> *data0;
    ram_tech<ibits, TAG_WITH_FLAGS> *tag0;
    ram_tech<ibits, TAG_WITH_FLAGS> *tagsnoop0;

};

template<int abus, int ibits, int lnbits, int flbits, int snoop>
TagMem<abus, ibits, lnbits, flbits, snoop>::TagMem(sc_module_name name,
                                                   bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_addr("i_addr"),
    i_wstrb("i_wstrb"),
    i_wdata("i_wdata"),
    i_wflags("i_wflags"),
    o_raddr("o_raddr"),
    o_rdata("o_rdata"),
    o_rflags("o_rflags"),
    o_hit("o_hit"),
    i_snoop_addr("i_snoop_addr"),
    o_snoop_flags("o_snoop_flags") {

    async_reset_ = async_reset;
    data0 = 0;
    tag0 = 0;
    tagsnoop0 = 0;

    // bwe = byte write enable

    data0 = new ram_cache_bwe_tech<ibits,
                                   (8 * (1 << lnbits))>("data0");
    data0->i_clk(i_clk);
    data0->i_addr(wb_index);
    data0->i_wena(i_wstrb);
    data0->i_wdata(i_wdata);
    data0->o_rdata(o_rdata);


    tag0 = new ram_tech<ibits,
                        TAG_WITH_FLAGS>("tag0");
    tag0->i_clk(i_clk);
    tag0->i_addr(wb_index);
    tag0->i_wena(w_tagi_we);
    tag0->i_wdata(wb_tagi_wdata);
    tag0->o_rdata(wb_tago_rdata);


    // generate
    if (snoop) {
        tagsnoop0 = new ram_tech<ibits,
                                 TAG_WITH_FLAGS>("tagsnoop0");
        tagsnoop0->i_clk(i_clk);
        tagsnoop0->i_addr(wb_snoop_index);
        tagsnoop0->i_wena(w_tagi_we);
        tagsnoop0->i_wdata(wb_tagi_wdata);
        tagsnoop0->o_rdata(wb_tago_snoop_rdata);
    } else {
        wb_tago_snoop_rdata = 0;
    }

    // endgenerate


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_addr;
    sensitive << i_wstrb;
    sensitive << i_wdata;
    sensitive << i_wflags;
    sensitive << i_snoop_addr;
    sensitive << wb_index;
    sensitive << wb_tago_rdata;
    sensitive << wb_tagi_wdata;
    sensitive << w_tagi_we;
    sensitive << wb_snoop_index;
    sensitive << wb_snoop_tagaddr;
    sensitive << wb_tago_snoop_rdata;
    sensitive << r.tagaddr;
    sensitive << r.index;
    sensitive << r.snoop_tagaddr;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int abus, int ibits, int lnbits, int flbits, int snoop>
TagMem<abus, ibits, lnbits, flbits, snoop>::~TagMem() {
    if (data0) {
        delete data0;
    }
    if (tag0) {
        delete tag0;
    }
    if (tagsnoop0) {
        delete tagsnoop0;
    }
}

template<int abus, int ibits, int lnbits, int flbits, int snoop>
void TagMem<abus, ibits, lnbits, flbits, snoop>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wflags, i_wflags.name());
        sc_trace(o_vcd, o_raddr, o_raddr.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_rflags, o_rflags.name());
        sc_trace(o_vcd, o_hit, o_hit.name());
        sc_trace(o_vcd, i_snoop_addr, i_snoop_addr.name());
        sc_trace(o_vcd, o_snoop_flags, o_snoop_flags.name());
        sc_trace(o_vcd, r.tagaddr, pn + ".r_tagaddr");
        sc_trace(o_vcd, r.index, pn + ".r_index");
        sc_trace(o_vcd, r.snoop_tagaddr, pn + ".r_snoop_tagaddr");
    }

}

template<int abus, int ibits, int lnbits, int flbits, int snoop>
void TagMem<abus, ibits, lnbits, flbits, snoop>::comb() {
    sc_uint<ibits> vb_index;
    sc_uint<abus> vb_raddr;
    sc_uint<TAG_WITH_FLAGS> vb_tagi_wdata;
    bool v_hit;
    sc_uint<ibits> vb_snoop_index;
    sc_uint<TAG_BITS> vb_snoop_tagaddr;
    sc_uint<flbits> vb_snoop_flags;

    vb_index = 0;
    vb_raddr = 0;
    vb_tagi_wdata = 0;
    v_hit = 0;
    vb_snoop_index = 0;
    vb_snoop_tagaddr = 0;
    vb_snoop_flags = 0;

    v = r;


    if (r.tagaddr.read() == wb_tago_rdata.read()((TAG_BITS - 1), 0)) {
        v_hit = wb_tago_rdata.read()[TAG_BITS];             // valid bit
    }

    vb_raddr((abus - 1), (ibits + lnbits)) = wb_tago_rdata.read()((TAG_BITS - 1), 0);
    vb_raddr(((ibits + lnbits) - 1), lnbits) = r.index;

    vb_index = i_addr.read()(((ibits + lnbits) - 1), lnbits);
    vb_tagi_wdata((TAG_BITS - 1), 0) = i_addr.read()((abus - 1), (ibits + lnbits));
    vb_tagi_wdata((TAG_WITH_FLAGS - 1), TAG_BITS) = i_wflags;

    if (snoop == 1) {
        vb_snoop_flags = wb_tago_snoop_rdata.read()((TAG_WITH_FLAGS - 1), TAG_BITS);
        vb_snoop_index = i_snoop_addr.read()(((ibits + lnbits) - 1), lnbits);
        vb_snoop_tagaddr = i_snoop_addr.read()((abus - 1), (ibits + lnbits));
        if (i_wstrb.read().or_reduce() == 1) {
            vb_snoop_index = vb_index;
        }
        if (r.snoop_tagaddr.read() != wb_tago_snoop_rdata.read()((TAG_BITS - 1), 0)) {
            vb_snoop_flags = 0;
        }
    }

    v.tagaddr = vb_tagi_wdata((TAG_BITS - 1), 0);
    v.index = vb_index;
    v.snoop_tagaddr = vb_snoop_tagaddr;

    if (!async_reset_ && i_nrst.read() == 0) {
        TagMem_r_reset(v);
    }

    wb_index = vb_index;
    w_tagi_we = i_wstrb.read().or_reduce();
    wb_tagi_wdata = vb_tagi_wdata;

    o_raddr = vb_raddr;
    o_rflags = wb_tago_rdata.read()((TAG_WITH_FLAGS - 1), TAG_BITS);
    o_hit = v_hit;

    wb_snoop_index = vb_snoop_index;
    wb_snoop_tagaddr = vb_snoop_tagaddr;
    o_snoop_flags = vb_snoop_flags;
}

template<int abus, int ibits, int lnbits, int flbits, int snoop>
void TagMem<abus, ibits, lnbits, flbits, snoop>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        TagMem_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

