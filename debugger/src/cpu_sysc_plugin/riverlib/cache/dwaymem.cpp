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

#include "dwaymem.h"

namespace debugger {

DWayMem::DWayMem(sc_module_name name_, bool async_reset, int wayidx)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_addr("i_addr"),
    i_wstrb("i_wstrb"),
    i_wvalid("i_wvalid"),
    i_wdata("i_wdata"),
    i_load_fault("i_load_fault"),
    i_executable("i_executable"),
    i_readable("i_readable"),
    i_writable("i_writable"),
    o_rtag("o_rtag"),
    o_rdata("o_rdata"),
    o_valid("o_valid"),
    o_load_fault("o_load_fault"),
    o_executable("o_executable"),
    o_readable("o_readable"),
    o_writable("o_writable") {
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
    sensitive << i_load_fault;
    sensitive << i_executable;
    sensitive << i_readable;
    sensitive << i_writable;
    sensitive << wb_rtag;
    for (int i = 0; i < 4*BUS_DATA_BYTES; i++) {
        sensitive << wb_rdata[i];
    }
    sensitive << r.roffset;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

DWayMem::~DWayMem() {
    for (int i = 0; i < BUS_DATA_BYTES; i++) {
        delete datax[i];
    }
    delete tag0;
}

void DWayMem::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_wstrb, i_wstrb.name());
        sc_trace(o_vcd, i_wvalid, i_wvalid.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_rtag, pn + ".wb_rtag");
        sc_trace(o_vcd, wb_wtag, pn + ".wb_wtag");
        sc_trace(o_vcd, r.roffset, pn + ".r_offset");
    }
}

void DWayMem::comb() {
    sc_biguint<4*BUS_DATA_WIDTH> vb_rdata;
    sc_uint<TAG_BITS> vb_rtag;
    sc_uint<TAG_BITS> vb_wtag;

    v = r;

    wb_addr = i_addr.read()(DINDEX_END, DINDEX_START);
    for (int i = 0; i < 4*BUS_DATA_BYTES; i++) {
        w_wdata_we[i] = i_wstrb.read()[i];
        wb_wdata[i] = i_wdata.read()(8*i+7, 8*i).to_uint();

        vb_rdata(8*i+7, 8*i) = wb_rdata[i].read();
    }

    w_wtag_we = i_wstrb.read().or_reduce();
    vb_wtag(DTAG_SIZE-1, 0) = i_addr.read()(DTAG_END, DTAG_START);
    vb_wtag[DTAG_SIZE + 0] = i_wvalid.read();
    vb_wtag[DTAG_SIZE + 1] = i_load_fault.read();
    vb_wtag[DTAG_SIZE + 2] = i_executable.read();
    vb_wtag[DTAG_SIZE + 3] = i_readable.read();
    vb_wtag[DTAG_SIZE + 4] = i_writable.read();

    wb_wtag = vb_wtag;
    vb_rtag = wb_rtag;

    o_rdata = vb_rdata;
    o_rtag = vb_rtag(DTAG_SIZE-1, 0);
    o_valid = vb_rtag[DTAG_SIZE + 0];
    o_load_fault = vb_rtag[DTAG_SIZE + 1];
    o_executable = vb_rtag[DTAG_SIZE + 2];
    o_readable = vb_rtag[DTAG_SIZE + 3];
    o_writable = vb_rtag[DTAG_SIZE + 4];
}

void DWayMem::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

