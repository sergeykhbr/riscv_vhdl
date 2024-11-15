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
#include "api_core.h"

namespace debugger {

template<int abits = 3,                                     // fifo log2(depth)
         int dbits = 32>                                    // payload width
SC_MODULE(cdc_afifo) {
 public:
    sc_in<bool> i_wclk;                                     // clock write
    sc_in<bool> i_wrstn;                                    // write reset active LOW
    sc_in<bool> i_wr;                                       // write enable strob
    sc_in<sc_uint<dbits>> i_wdata;                          // write data
    sc_out<bool> o_wfull;                                   // fifo is full in wclk domain
    sc_in<bool> i_rclk;                                     // read clock
    sc_in<bool> i_rrstn;                                    // read reset active LOW
    sc_in<bool> i_rd;                                       // read enable strob
    sc_out<sc_uint<dbits>> o_rdata;                         // fifo payload read
    sc_out<bool> o_rempty;                                  // fifo is empty it rclk domain

    void comb();
    void registers();
    void r2egisters();
    void rx2egisters();

    SC_HAS_PROCESS(cdc_afifo);

    cdc_afifo(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int DEPTH = (1 << abits);

    struct cdc_afifo_registers {
        sc_signal<sc_uint<(abits + 1)>> wgray;
        sc_signal<sc_uint<(abits + 1)>> wbin;
        sc_signal<sc_uint<(abits + 1)>> wq2_rgray;
        sc_signal<sc_uint<(abits + 1)>> wq1_rgray;
        sc_signal<bool> wfull;
    } v, r;

    void cdc_afifo_r_reset(cdc_afifo_registers &iv) {
        iv.wgray = 0;
        iv.wbin = 0;
        iv.wq2_rgray = 0;
        iv.wq1_rgray = 0;
        iv.wfull = 0;
    }

    struct cdc_afifo_r2egisters {
        sc_signal<sc_uint<(abits + 1)>> rgray;
        sc_signal<sc_uint<(abits + 1)>> rbin;
        sc_signal<sc_uint<(abits + 1)>> rq2_wgray;
        sc_signal<sc_uint<(abits + 1)>> rq1_wgray;
        sc_signal<bool> rempty;
    } v2, r2;

    void cdc_afifo_r2_reset(cdc_afifo_r2egisters &iv) {
        iv.rgray = 0;
        iv.rbin = 0;
        iv.rq2_wgray = 0;
        iv.rq1_wgray = 0;
        iv.rempty = 1;
    }

    struct cdc_afifo_rx2egisters {
        sc_signal<sc_uint<dbits>> mem[DEPTH];
    } vx2, rx2;

};

template<int abits, int dbits>
cdc_afifo<abits, dbits>::cdc_afifo(sc_module_name name)
    : sc_module(name),
    i_wclk("i_wclk"),
    i_wrstn("i_wrstn"),
    i_wr("i_wr"),
    i_wdata("i_wdata"),
    o_wfull("o_wfull"),
    i_rclk("i_rclk"),
    i_rrstn("i_rrstn"),
    i_rd("i_rd"),
    o_rdata("o_rdata"),
    o_rempty("o_rempty") {


    SC_METHOD(comb);
    sensitive << i_wclk;
    sensitive << i_wrstn;
    sensitive << i_wr;
    sensitive << i_wdata;
    sensitive << i_rclk;
    sensitive << i_rrstn;
    sensitive << i_rd;
    sensitive << r.wgray;
    sensitive << r.wbin;
    sensitive << r.wq2_rgray;
    sensitive << r.wq1_rgray;
    sensitive << r.wfull;
    sensitive << r2.rgray;
    sensitive << r2.rbin;
    sensitive << r2.rq2_wgray;
    sensitive << r2.rq1_wgray;
    sensitive << r2.rempty;
    for (int i = 0; i < DEPTH; i++) {
        sensitive << rx2.mem[i];
    }

    SC_METHOD(registers);
    sensitive << i_wrstn;
    sensitive << i_wclk.pos();

    SC_METHOD(r2egisters);
    sensitive << i_rrstn;
    sensitive << i_rclk.pos();

    SC_METHOD(rx2egisters);
    sensitive << i_wclk.pos();
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_wclk, i_wclk.name());
        sc_trace(o_vcd, i_wrstn, i_wrstn.name());
        sc_trace(o_vcd, i_wr, i_wr.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_wfull, o_wfull.name());
        sc_trace(o_vcd, i_rclk, i_rclk.name());
        sc_trace(o_vcd, i_rrstn, i_rrstn.name());
        sc_trace(o_vcd, i_rd, i_rd.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_rempty, o_rempty.name());
        sc_trace(o_vcd, r.wgray, pn + ".r_wgray");
        sc_trace(o_vcd, r.wbin, pn + ".r_wbin");
        sc_trace(o_vcd, r.wq2_rgray, pn + ".r_wq2_rgray");
        sc_trace(o_vcd, r.wq1_rgray, pn + ".r_wq1_rgray");
        sc_trace(o_vcd, r.wfull, pn + ".r_wfull");
        sc_trace(o_vcd, r2.rgray, pn + ".r2_rgray");
        sc_trace(o_vcd, r2.rbin, pn + ".r2_rbin");
        sc_trace(o_vcd, r2.rq2_wgray, pn + ".r2_rq2_wgray");
        sc_trace(o_vcd, r2.rq1_wgray, pn + ".r2_rq1_wgray");
        sc_trace(o_vcd, r2.rempty, pn + ".r2_rempty");
        for (int i = 0; i < DEPTH; i++) {
            char tstr[1024];
            RISCV_sprintf(tstr, sizeof(tstr), "%s.rx2_mem%d", pn.c_str(), i);
            sc_trace(o_vcd, rx2.mem[i], tstr);
        }
    }

}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::comb() {
    sc_uint<abits> vb_waddr;
    sc_uint<abits> vb_raddr;
    bool v_wfull_next;
    bool v_rempty_next;
    sc_uint<(abits + 1)> vb_wgraynext;
    sc_uint<(abits + 1)> vb_wbinnext;
    sc_uint<(abits + 1)> vb_rgraynext;
    sc_uint<(abits + 1)> vb_rbinnext;

    vb_waddr = 0;
    vb_raddr = 0;
    v_wfull_next = 0;
    v_rempty_next = 0;
    vb_wgraynext = 0;
    vb_wbinnext = 0;
    vb_rgraynext = 0;
    vb_rbinnext = 0;

    v = r;
    v2 = r2;
    for (int i = 0; i < DEPTH; i++) {
        vx2.mem[i] = rx2.mem[i];
    }

    // Cross the Gray pointer to write clock domain:
    v.wq1_rgray = r2.rgray;
    v.wq2_rgray = r.wq1_rgray;

    // Next write address and Gray write pointer
    vb_wbinnext = (r.wbin.read() + (0, (i_wr.read() && (!r.wfull.read()))));
    vb_wgraynext = ((vb_wbinnext >> 1) ^ vb_wbinnext);
    vb_waddr = r.wbin.read()((abits - 1), 0);
    v.wgray = vb_wgraynext;
    v.wbin = vb_wbinnext;

    if (vb_wgraynext == ((~r.wq2_rgray.read()(abits, (abits - 1))), r.wq2_rgray.read()((abits - 2), 0))) {
        v_wfull_next = 1;
    }
    v.wfull = v_wfull_next;

    if ((i_wr.read() && (!r.wfull.read())) == 1) {
        vx2.mem[vb_waddr.to_int()] = i_wdata;
    }

    // Write Gray pointer into read clock domain
    v2.rq1_wgray = r.wgray;
    v2.rq2_wgray = r2.rq1_wgray;
    vb_rbinnext = (r2.rbin.read() + (0, (i_rd.read() && (!r2.rempty.read()))));
    vb_rgraynext = ((vb_rbinnext >> 1) ^ vb_rbinnext);
    v2.rgray = vb_rgraynext;
    v2.rbin = vb_rbinnext;
    vb_raddr = r2.rbin.read()((abits - 1), 0);

    if (vb_rgraynext == r2.rq2_wgray.read()) {
        v_rempty_next = 1;
    }
    v2.rempty = v_rempty_next;

    o_wfull = r.wfull;
    o_rempty = r2.rempty;
    o_rdata = rx2.mem[vb_raddr.to_int()];
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::registers() {
    if (i_wrstn.read() == 0) {
        cdc_afifo_r_reset(r);
    } else {
        r = v;
    }
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::r2egisters() {
    if (i_rrstn.read() == 0) {
        cdc_afifo_r2_reset(r2);
    } else {
        r2 = v2;
    }
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::rx2egisters() {
    for (int i = 0; i < DEPTH; i++) {
        rx2.mem[i] = vx2.mem[i];
    }
}

}  // namespace debugger

