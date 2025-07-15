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
#include "cdc_dp_mem.h"
#include "cdc_afifo_gray.h"
#include "api_core.h"

namespace debugger {

template<int abits = 3,                                     // fifo log2(depth)
         int dbits = 65>                                    // payload width
SC_MODULE(cdc_afifo) {
 public:
    sc_in<bool> i_nrst;                                     // reset active LOW
    sc_in<bool> i_wclk;                                     // clock write
    sc_in<bool> i_wr;                                       // write enable strob
    sc_in<sc_biguint<dbits>> i_wdata;                       // write data
    sc_out<bool> o_wready;                                  // ready to accept (fifo is not full) in wclk domain
    sc_in<bool> i_rclk;                                     // read clock
    sc_in<bool> i_rd;                                       // read enable strob
    sc_out<sc_biguint<dbits>> o_rdata;                      // fifo payload read
    sc_out<bool> o_rvalid;                                  // new valid data (fifo is not empty) in rclk domain

    void comb();
    void proc_wff();
    void proc_rff();

    cdc_afifo(sc_module_name name);
    virtual ~cdc_afifo();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<bool> w_wr_ena;
    sc_signal<sc_uint<abits>> wb_wgray_addr;
    sc_signal<sc_uint<(abits + 1)>> wgray;
    sc_uint<(abits + 1)> q1_wgray;
    sc_signal<sc_uint<(abits + 1)>> q2_wgray;
    sc_signal<bool> w_wgray_full;
    sc_signal<bool> w_wgray_empty_unused;
    sc_signal<bool> w_rd_ena;
    sc_signal<sc_uint<abits>> wb_rgray_addr;
    sc_signal<sc_uint<(abits + 1)>> rgray;
    sc_uint<(abits + 1)> q1_rgray;
    sc_signal<sc_uint<(abits + 1)>> q2_rgray;
    sc_signal<bool> w_rgray_full_unused;
    sc_signal<bool> w_rgray_empty;

    cdc_dp_mem<abits, dbits> *mem0;
    cdc_afifo_gray<abits> *wgray0;
    cdc_afifo_gray<abits> *rgray0;

};

template<int abits, int dbits>
cdc_afifo<abits, dbits>::cdc_afifo(sc_module_name name)
    : sc_module(name),
    i_nrst("i_nrst"),
    i_wclk("i_wclk"),
    i_wr("i_wr"),
    i_wdata("i_wdata"),
    o_wready("o_wready"),
    i_rclk("i_rclk"),
    i_rd("i_rd"),
    o_rdata("o_rdata"),
    o_rvalid("o_rvalid") {

    mem0 = 0;
    wgray0 = 0;
    rgray0 = 0;

    wgray0 = new cdc_afifo_gray<abits>("wgray0");
    wgray0->i_nrst(i_nrst);
    wgray0->i_clk(i_wclk);
    wgray0->i_ena(w_wr_ena);
    wgray0->i_q2_gray(q2_rgray);
    wgray0->o_addr(wb_wgray_addr);
    wgray0->o_gray(wgray);
    wgray0->o_empty(w_wgray_empty_unused);
    wgray0->o_full(w_wgray_full);

    rgray0 = new cdc_afifo_gray<abits>("rgray0");
    rgray0->i_nrst(i_nrst);
    rgray0->i_clk(i_rclk);
    rgray0->i_ena(w_rd_ena);
    rgray0->i_q2_gray(q2_wgray);
    rgray0->o_addr(wb_rgray_addr);
    rgray0->o_gray(rgray);
    rgray0->o_empty(w_rgray_empty);
    rgray0->o_full(w_rgray_full_unused);

    mem0 = new cdc_dp_mem<abits,
                          dbits>("mem0");
    mem0->i_wclk(i_wclk);
    mem0->i_wena(w_wr_ena);
    mem0->i_addr(wb_wgray_addr);
    mem0->i_wdata(i_wdata);
    mem0->i_rclk(i_rclk);
    mem0->i_raddr(wb_rgray_addr);
    mem0->o_rdata(o_rdata);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_wclk;
    sensitive << i_wr;
    sensitive << i_wdata;
    sensitive << i_rclk;
    sensitive << i_rd;
    sensitive << w_wr_ena;
    sensitive << wb_wgray_addr;
    sensitive << wgray;
    sensitive << q2_wgray;
    sensitive << w_wgray_full;
    sensitive << w_wgray_empty_unused;
    sensitive << w_rd_ena;
    sensitive << wb_rgray_addr;
    sensitive << rgray;
    sensitive << q2_rgray;
    sensitive << w_rgray_full_unused;
    sensitive << w_rgray_empty;

    SC_METHOD(proc_wff);
    sensitive << i_nrst;
    sensitive << i_wclk.pos();

    SC_METHOD(proc_rff);
    sensitive << i_nrst;
    sensitive << i_rclk.pos();
}

template<int abits, int dbits>
cdc_afifo<abits, dbits>::~cdc_afifo() {
    if (mem0) {
        delete mem0;
    }
    if (wgray0) {
        delete wgray0;
    }
    if (rgray0) {
        delete rgray0;
    }
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_wclk, i_wclk.name());
        sc_trace(o_vcd, i_wr, i_wr.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_wready, o_wready.name());
        sc_trace(o_vcd, i_rclk, i_rclk.name());
        sc_trace(o_vcd, i_rd, i_rd.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_rvalid, o_rvalid.name());
    }

    if (wgray0) {
        wgray0->generateVCD(i_vcd, o_vcd);
    }
    if (rgray0) {
        rgray0->generateVCD(i_vcd, o_vcd);
    }
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::comb() {
    w_wr_ena = (i_wr.read() & (!w_wgray_full.read()));
    w_rd_ena = (i_rd.read() & (!w_rgray_empty.read()));
    o_wready = (!w_wgray_full.read());
    o_rvalid = (!w_rgray_empty.read());
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::proc_wff() {
    if (i_nrst.read() == 0) {
        q1_wgray = 0;
        q2_wgray = 0;
    } else {
        q1_wgray = wgray.read();
        q2_wgray = q1_wgray;
    }
}

template<int abits, int dbits>
void cdc_afifo<abits, dbits>::proc_rff() {
    if (i_nrst.read() == 0) {
        q1_rgray = 0;
        q2_rgray = 0;
    } else {
        q1_rgray = rgray.read();
        q2_rgray = q1_rgray;
    }
}

}  // namespace debugger

