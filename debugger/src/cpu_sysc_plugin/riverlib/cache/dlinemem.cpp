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

#include "dlinemem.h"

namespace debugger {

DLineMem::DLineMem(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_flush("i_flush"),
    i_addr("i_addr"),
    i_wdata("i_wdata"),
    i_wstrb("i_wstrb"),
    i_wvalid("i_wvalid"),
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
    o_rwritable("o_rwritable"),
    o_hit("o_hit") {
    async_reset_ = async_reset;

    char tstr1[32] = "way0";
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        tstr1[3] = '0' + static_cast<char>(i);
        wayx[i] = new DWayMem(tstr1, async_reset, i);
        wayx[i]->i_clk(i_clk);
        wayx[i]->i_nrst(i_nrst);
        wayx[i]->i_addr(way_i[i].addr);
        wayx[i]->i_wstrb(way_i[i].wstrb);
        wayx[i]->i_wvalid(way_i[i].wvalid);
        wayx[i]->i_wdata(way_i[i].wdata);
        wayx[i]->i_load_fault(way_i[i].load_fault);
        wayx[i]->i_executable(way_i[i].executable);
        wayx[i]->i_readable(way_i[i].readable);
        wayx[i]->i_writable(way_i[i].writable);
        wayx[i]->o_rtag(way_o[i].rtag);
        wayx[i]->o_rdata(way_o[i].rdata);
        wayx[i]->o_valid(way_o[i].valid);
        wayx[i]->o_load_fault(way_o[i].load_fault);
        wayx[i]->o_executable(way_o[i].executable);
        wayx[i]->o_readable(way_o[i].readable);
        wayx[i]->o_writable(way_o[i].writable);
    }

    lru = new ILru("lru0");
    lru->i_clk(i_clk);
    lru->i_init(lrui_init);
    lru->i_radr(lrui_radr);
    lru->i_wadr(lrui_wadr);
    lru->i_we(lrui_we);
    lru->i_lru(lrui_lru);
    lru->o_lru(lruo_lru);

    SC_METHOD(comb);
    sensitive << i_flush;
    sensitive << i_addr;
    sensitive << i_wdata;
    sensitive << i_wstrb;
    sensitive << i_wvalid;
    sensitive << i_wdirty;
    sensitive << i_wload_fault;
    sensitive << i_wexecutable;
    sensitive << i_wreadable;
    sensitive << i_wwritable;
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        sensitive << way_o[i].rtag;
        sensitive << way_o[i].rdata;
        sensitive << way_o[i].valid;
        sensitive << way_o[i].load_fault;
        sensitive << way_o[i].executable;
        sensitive << way_o[i].readable;
        sensitive << way_o[i].writable;
    }
    sensitive << lruo_lru;
    sensitive << r.req_addr;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

DLineMem::~DLineMem() {
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        delete wayx[i];
    }
    delete lru;
}

void DLineMem::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wvalid, i_wvalid.name());
        sc_trace(o_vcd, i_wdirty, i_wdirty.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_rvalid, o_rvalid.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_hit, o_hit.name());

        //std::string pn(name());
        //sc_trace(o_vcd, wb_tag_rdata, pn + ".wb_tag_rdata");
        //sc_trace(o_vcd, wb_tag_wdata, pn + ".wb_tag_wdata");
        //sc_trace(o_vcd, r.roffset, pn + ".r_offset");
    }
}

void DLineMem::comb() {
    sc_uint<2> vb_wayidx;
    sc_uint<CFG_DINDEX_WIDTH> vb_lineadr;
    sc_uint<DTAG_SIZE> vb_tag_addr;
    bool v_lrui_we;
    bool v_hit;

    v_hit = 0;
    v_lrui_we = 0;
    if (i_wstrb.read() != 0) {
        v_lrui_we = 1;
    }
    vb_lineadr = i_addr.read()(DINDEX_END, DINDEX_START);
    vb_tag_addr = i_addr.read()(DTAG_END, DTAG_START);
    v.req_addr = i_addr.read();

    vb_wayidx = lruo_lru.read();
    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        if (way_o[i].rtag.read() == vb_tag_addr) {
            vb_wayidx = i;
            v_lrui_we = 1;  // Read hit update lru anyway
            v_hit = 1;
        }
    }

    for (int i = 0; i < CFG_DCACHE_WAYS; i++) {
        way_i[i].addr = i_addr.read();
        way_i[i].wdata = i_wdata.read();
        if (vb_wayidx == i) {
            way_i[i].wstrb = i_wstrb.read();
        } else {
            way_i[i].wstrb = 0;
        }
        way_i[i].wvalid = i_wvalid.read();
        way_i[i].load_fault = i_wload_fault.read();
        way_i[i].executable = i_wexecutable.read();
        way_i[i].readable = i_wreadable.read();
        way_i[i].writable = i_wwritable.read();
    }

    lrui_init = i_flush.read();
    lrui_radr = vb_lineadr;
    lrui_wadr = vb_lineadr;
    lrui_we = v_lrui_we;
    lrui_lru = vb_wayidx;

    o_raddr = r.req_addr.read();
    o_rdata = way_o[vb_wayidx.to_int()].rdata;
    o_rvalid = way_o[vb_wayidx.to_int()].valid;
    o_rdirty = 0;
    o_rload_fault = way_o[vb_wayidx.to_int()].load_fault;
    o_rexecutable = way_o[vb_wayidx.to_int()].executable;
    o_rreadable = way_o[vb_wayidx.to_int()].readable;
    o_rwritable = way_o[vb_wayidx.to_int()].writable;
    o_hit = v_hit;
}

void DLineMem::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}


}  // namespace debugger

