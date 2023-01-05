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

template<int dbits = 8,                                     // Data width bits
         int log2_depth = 4>                                // Fifo depth
SC_MODULE(sfifo) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<log2_depth>> i_thresh;                    // Threshold to generate less/greater signals
    sc_in<bool> i_we;
    sc_in<sc_uint<dbits>> i_wdata;
    sc_in<bool> i_re;
    sc_out<sc_uint<dbits>> o_rdata;
    sc_out<bool> o_full;
    sc_out<bool> o_empty;
    sc_out<bool> o_less;
    sc_out<bool> o_greater;

    void comb();
    void registers();

    SC_HAS_PROCESS(sfifo);

    sfifo(sc_module_name name,
          bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int DEPTH = (1 << log2_depth);

    struct sfifo_registers {
        sc_signal<sc_uint<dbits>> databuf[DEPTH];
        sc_signal<sc_uint<log2_depth>> wr_cnt;
        sc_signal<sc_uint<log2_depth>> rd_cnt;
        sc_signal<sc_uint<log2_depth>> total_cnt;
    } v, r;


};

template<int dbits, int log2_depth>
sfifo<dbits, log2_depth>::sfifo(sc_module_name name,
                                bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_thresh("i_thresh"),
    i_we("i_we"),
    i_wdata("i_wdata"),
    i_re("i_re"),
    o_rdata("o_rdata"),
    o_full("o_full"),
    o_empty("o_empty"),
    o_less("o_less"),
    o_greater("o_greater") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_thresh;
    sensitive << i_we;
    sensitive << i_wdata;
    sensitive << i_re;
    for (int i = 0; i < DEPTH; i++) {
        sensitive << r.databuf[i];
    }
    sensitive << r.wr_cnt;
    sensitive << r.rd_cnt;
    sensitive << r.total_cnt;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int dbits, int log2_depth>
void sfifo<dbits, log2_depth>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_thresh, i_thresh.name());
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_re, i_re.name());
        sc_trace(o_vcd, o_rdata, o_rdata.name());
        sc_trace(o_vcd, o_full, o_full.name());
        sc_trace(o_vcd, o_empty, o_empty.name());
        sc_trace(o_vcd, o_less, o_less.name());
        sc_trace(o_vcd, o_greater, o_greater.name());
        for (int i = 0; i < DEPTH; i++) {
            char tstr[1024];
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_databuf%d", pn.c_str(), i);
            sc_trace(o_vcd, r.databuf[i], tstr);
        }
        sc_trace(o_vcd, r.wr_cnt, pn + ".r_wr_cnt");
        sc_trace(o_vcd, r.rd_cnt, pn + ".r_rd_cnt");
        sc_trace(o_vcd, r.total_cnt, pn + ".r_total_cnt");
    }

}

template<int dbits, int log2_depth>
void sfifo<dbits, log2_depth>::comb() {
    bool v_less;
    bool v_greater;
    bool v_full;
    bool v_empty;

    v_less = 0;
    v_greater = 0;
    v_full = 0;
    v_empty = 0;

    for (int i = 0; i < DEPTH; i++) {
        v.databuf[i] = r.databuf[i];
    }
    v.wr_cnt = r.wr_cnt;
    v.rd_cnt = r.rd_cnt;
    v.total_cnt = r.total_cnt;


    // Check FIFOs counters with thresholds:
    if (r.total_cnt.read() < i_thresh.read()) {
        v_less = 1;
    }

    if (r.total_cnt.read() > i_thresh.read()) {
        v_greater = 1;
    }

    if (r.total_cnt.read().or_reduce() == 0) {
        v_empty = 1;
    }

    if (r.total_cnt.read() == (DEPTH - 1)) {
        v_full = 1;
    }

    if ((i_we.read() == 1) && ((v_full == 0) || (i_re.read() == 1))) {
        v.wr_cnt = (r.wr_cnt.read() + 1);
        v.databuf[r.wr_cnt.read().to_int()] = i_wdata;
        if (i_re.read() == 0) {
            v.total_cnt = (r.total_cnt.read() + 1);
        }
    }

    if ((i_re.read() == 1) && (v_empty == 0)) {
        v.rd_cnt = (r.rd_cnt.read() + 1);
        if (i_we.read() == 0) {
            v.total_cnt = (r.total_cnt.read() - 1);
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < DEPTH; i++) {
            v.databuf[i] = 0;
        }
        v.wr_cnt = 0;
        v.rd_cnt = 0;
        v.total_cnt = 0;
    }

    o_rdata = r.databuf[r.rd_cnt.read().to_int()];
    o_full = v_full;
    o_empty = v_empty;
    o_less = v_less;
    o_greater = v_greater;
}

template<int dbits, int log2_depth>
void sfifo<dbits, log2_depth>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < DEPTH; i++) {
            r.databuf[i] = 0;
        }
        r.wr_cnt = 0;
        r.rd_cnt = 0;
        r.total_cnt = 0;
    } else {
        for (int i = 0; i < DEPTH; i++) {
            r.databuf[i] = v.databuf[i];
        }
        r.wr_cnt = v.wr_cnt;
        r.rd_cnt = v.rd_cnt;
        r.total_cnt = v.total_cnt;
    }
}

}  // namespace debugger

