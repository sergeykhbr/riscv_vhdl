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

template<int abits = 6,
         int dbits = 128>
SC_MODULE(Queue) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_re;
    sc_in<bool> i_we;
    sc_in<sc_biguint<dbits>> i_wdata;
    sc_out<sc_biguint<dbits>> o_rdata;
    sc_out<bool> o_full;
    sc_out<bool> o_nempty;

    void comb();
    void registers();
    void rxegisters();

    Queue(sc_module_name name,
          bool async_reset);


 private:
    bool async_reset_;

    static const int DEPTH = (1 << abits);

    struct Queue_registers {
        sc_signal<sc_uint<(abits + 1)>> wcnt;
    };

    void Queue_r_reset(Queue_registers& iv) {
        iv.wcnt = 0;
    }

    struct Queue_rxegisters {
        sc_signal<sc_biguint<dbits>> mem[DEPTH];
    };

    Queue_registers v;
    Queue_registers r;
    Queue_rxegisters vx;
    Queue_rxegisters rx;

};

template<int abits, int dbits>
Queue<abits, dbits>::Queue(sc_module_name name,
                           bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_re("i_re"),
    i_we("i_we"),
    i_wdata("i_wdata"),
    o_rdata("o_rdata"),
    o_full("o_full"),
    o_nempty("o_nempty") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_re;
    sensitive << i_we;
    sensitive << i_wdata;
    sensitive << r.wcnt;
    for (int i = 0; i < DEPTH; i++) {
        sensitive << rx.mem[i];
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    SC_METHOD(rxegisters);
    sensitive << i_clk.pos();
}

template<int abits, int dbits>
void Queue<abits, dbits>::comb() {
    bool nempty;
    sc_biguint<dbits> vb_data_o;
    bool full;
    bool show_full;

    for (int i = 0; i < DEPTH; i++) {
        vx.mem[i] = rx.mem[i].read();
    }
    v = r;
    nempty = 0;
    vb_data_o = 0;
    full = 0;
    show_full = 0;

    if (r.wcnt.read() == DEPTH) {
        full = 1;
    }
    if (r.wcnt.read() >= (DEPTH - 1)) {
        show_full = 1;
    }

    if ((i_re.read() == 1) && (i_we.read() == 1)) {
        for (int i = 1; i < DEPTH; i++) {
            vx.mem[(i - 1)] = rx.mem[i].read();
        }
        if (r.wcnt.read().or_reduce() == 1) {
            vx.mem[(r.wcnt.read().to_int() - 1)] = i_wdata.read();
        } else {
            // do nothing, it will directly pass to output
        }
    } else if ((i_re.read() == 0) && (i_we.read() == 1)) {
        if (full == 0) {
            v.wcnt = (r.wcnt.read() + 1);
            vx.mem[r.wcnt.read().to_int()] = i_wdata.read();
        }
    } else if ((i_re.read() == 1) && (i_we.read() == 0)) {
        if (r.wcnt.read().or_reduce() == 1) {
            v.wcnt = (r.wcnt.read() - 1);
        }
        for (int i = 1; i < DEPTH; i++) {
            vx.mem[(i - 1)] = rx.mem[i].read();
        }
    }

    if (r.wcnt.read().or_reduce() == 0) {
        vb_data_o = i_wdata.read();
    } else {
        vb_data_o = rx.mem[0].read();
    }

    if ((i_we.read() == 1) || (r.wcnt.read().or_reduce() == 1)) {
        nempty = 1;
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        Queue_r_reset(v);
    }

    o_nempty = nempty;
    o_full = show_full;
    o_rdata = vb_data_o;
}

template<int abits, int dbits>
void Queue<abits, dbits>::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        Queue_r_reset(r);
    } else {
        r = v;
    }
}

template<int abits, int dbits>
void Queue<abits, dbits>::rxegisters() {
    for (int i = 0; i < DEPTH; i++) {
        rx.mem[i] = vx.mem[i].read();
    }
}

}  // namespace debugger

