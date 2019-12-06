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

#include "iwaymem.h"

namespace debugger {

IWayMem::IWayMem(sc_module_name name_, bool async_reset, int wayidx)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_radr("i_radr"),
    i_wadr("i_wadr"),
    i_wena("i_wena"),
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

    tag1 = new RamTagi("itag0");

    tag1->i_clk(i_clk);
    tag1->i_adr(wb_radr);
    tag1->o_rdata(wb_tag_rdata);
    tag1->i_wena(w_tag_wena);
    tag1->i_wdata(wb_tag_wdata);

    SC_METHOD(comb);
    sensitive << i_radr;
    sensitive << i_wadr;
    sensitive << i_wena;
    sensitive << i_wstrb;
    sensitive << i_wdata;
    sensitive << i_load_fault;
    sensitive << i_executable;
    sensitive << i_readable;
    sensitive << i_writable;
    sensitive << wb_tag_rdata;
    sensitive << r.roffset;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

IWayMem::~IWayMem() {
    delete tag1;
}

void IWayMem::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_radr, i_radr.name());
        sc_trace(o_vcd, i_wadr, i_wadr.name());
        sc_trace(o_vcd, i_wena, i_wena.name());
        sc_trace(o_vcd, i_wvalid, i_wvalid.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_tag_rdata, pn + ".wb_tag_rdata");
        sc_trace(o_vcd, wb_tag_wdata, pn + ".wb_tag_wdata");
        sc_trace(o_vcd, r.roffset, pn + ".r_offset");
    }
}

void IWayMem::comb() {
    sc_biguint<LINE_MEM_WIDTH> vb_wline;
    sc_biguint<LINE_MEM_WIDTH> vb_rline;
    sc_uint<32> vb_rdata;

    v = r;

    wb_radr = i_radr.read()(IINDEX_END, IINDEX_START);
    wb_wadr = i_wadr.read()(IINDEX_END, IINDEX_START);

    static const int LINE_VALID_BIT = 4*BUS_DATA_WIDTH + ITAG_SIZE;
    static const int LINE_LOAD_FAULT_BIT = LINE_VALID_BIT + 1;
    static const int LINE_EXEC_BIT = LINE_VALID_BIT + 2;
    static const int LINE_RD_BIT = LINE_VALID_BIT + 3;
    static const int LINE_WR_BIT = LINE_VALID_BIT + 4;

    w_tag_wena = i_wena.read();
    vb_wline(4*BUS_DATA_WIDTH-1, 0) = i_wdata;           // [255:0]
    vb_wline(4*BUS_DATA_WIDTH+ITAG_SIZE-1, 4*BUS_DATA_WIDTH) = 
            i_wadr.read()(ITAG_END, ITAG_START);         // [307:256]
    vb_wline[LINE_VALID_BIT] = i_wvalid.read();          // [308]
    vb_wline[LINE_LOAD_FAULT_BIT] = i_load_fault.read(); // [309]
    vb_wline[LINE_EXEC_BIT] = i_executable.read();       // [310]
    vb_wline[LINE_RD_BIT] = i_readable.read();           // [311]
    vb_wline[LINE_WR_BIT] = i_writable.read();           // [312]

    v.roffset = i_radr.read()(CFG_IOFFSET_WIDTH-1, 1);
    vb_rline = wb_tag_rdata.read();
    vb_rdata = (0, vb_rline(255, 240)); // r.offset == 15
    for (unsigned i = 0; i < 15; i++) {
        if (i == r.roffset.read()) {
            vb_rdata = vb_rline(16*i+31, 16*i);
        }
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    wb_tag_wdata = vb_wline;
    o_rdata = vb_rdata;
    o_rtag = vb_rline(4*BUS_DATA_WIDTH + CFG_ITAG_WIDTH-1,
                      4*BUS_DATA_WIDTH).to_uint();
    o_valid = vb_rline[LINE_VALID_BIT];
    o_load_fault = vb_rline[LINE_LOAD_FAULT_BIT];
    o_executable = vb_rline[LINE_EXEC_BIT];
    o_readable = vb_rline[LINE_RD_BIT];
    o_writable = vb_rline[LINE_WR_BIT];
}

void IWayMem::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

