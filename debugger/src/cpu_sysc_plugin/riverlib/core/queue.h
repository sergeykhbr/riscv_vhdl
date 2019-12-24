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

#ifndef __DEBUGGER_RIVERLIB_QUEUE_H__
#define __DEBUGGER_RIVERLIB_QUEUE_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

template<int szbits, int dbits>
SC_MODULE(Queue) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_re;
    sc_in<bool> i_we;
    sc_in<sc_biguint<dbits>> i_wdata;
    sc_out<sc_biguint<dbits>> o_rdata;
    sc_out<bool> o_full;
    sc_out<bool> o_nempty;

    void qproc();
    void registers();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

    SC_HAS_PROCESS(Queue);

    Queue(sc_module_name name_, bool async_reset) : sc_module(name_),
        i_clk("i_clk"),
        i_nrst("i_nrst"),
        i_re("i_re"),
        i_we("i_we"),
        i_wdata("i_wdata"),
        o_rdata("o_rdata"),
        o_full("o_full"),
        o_nempty("o_nempty") {
        async_reset_ = async_reset;

        SC_METHOD(qproc);
        sensitive << i_nrst;
        sensitive << i_we;
        sensitive << i_re;
        sensitive << i_wdata;
        sensitive << r.wcnt;
        for(int i = 0; i < QUEUE_DEPTH; i++) {
            sensitive << r.mem[i];
        }

        SC_METHOD(registers);
        sensitive << i_nrst;
        sensitive << i_clk.pos();
    }

 private:
    static const int QUEUE_DEPTH = 1 << szbits;

    struct QueueRegisterType {
        sc_signal<sc_uint<szbits+1>> wcnt;
        sc_signal<sc_biguint<dbits>> mem[QUEUE_DEPTH];
    } v, r;

    bool async_reset_;
};

template<int szbits, int dbits>
void Queue<szbits, dbits>::qproc() {
    bool nempty;
    sc_biguint<dbits> vb_data_o;
    bool full;
    bool show_full;

    v = r;

    full = 0;
    show_full = 0;
    if (r.wcnt.read() == QUEUE_DEPTH) {
        full = 1;
    }
    if (r.wcnt.read() >= (QUEUE_DEPTH - 1)) {
        show_full = 1;
    }

    if (i_re.read() == 1 && i_we.read() == 1) {
        for (int i = 1; i < QUEUE_DEPTH; i++) {
            v.mem[i-1] = r.mem[i];
        }
        if (r.wcnt.read().to_int() != 0) {
            v.mem[r.wcnt.read().to_int() - 1] = i_wdata.read();
        } else {
            // do nothing, it will directly pass to output
        }
    } else if (i_re.read() == 0 && i_we.read() == 1) {
        if (full == 0) {
            v.wcnt = r.wcnt.read() + 1;
            v.mem[r.wcnt.read().to_int()] = i_wdata.read();
        }
    } else if (i_re.read() == 1 && i_we.read() == 0) {
        if (r.wcnt.read() != 0) {
            v.wcnt = r.wcnt.read() - 1;
        }
        for (int i = 1; i < QUEUE_DEPTH; i++) {
            v.mem[i-1] = r.mem[i];
        }
    }

    if (r.wcnt.read() == 0) {
        vb_data_o = i_wdata.read();
    } else {
        vb_data_o = r.mem[0].read();
    }

    nempty = 0;
    if (i_we.read() == 1 || r.wcnt.read() != 0) {
        nempty = 1;
    }

    if (!async_reset_ && i_nrst == 0) {
        v.wcnt = 0;
        for (int k = 0; k < QUEUE_DEPTH; k++) {
            v.mem[k] =  0;
        }
    }

    o_nempty = nempty;
    o_full = show_full;
    o_rdata = vb_data_o;
}

template<int szbits, int dbits>
void Queue<szbits, dbits>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        r.wcnt = 0;
        for (int k = 0; k < QUEUE_DEPTH; k++) {
            r.mem[k] = 0;
        }
    } else {
        r = v;
    }
}

template<int szbits, int dbits>
void Queue<szbits, dbits>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        std::string pn(name());
        sc_trace(o_vcd, r.wcnt, pn + ".r_wcnt");
        //sc_trace(o_vcd, r.mem[0], pn + ".r_mem0");
        //sc_trace(o_vcd, r.mem[1], pn + ".r_mem1");
        //sc_trace(o_vcd, r.mem[2], pn + ".r_mem2");
        //sc_trace(o_vcd, r.mem[3], pn + ".r_mem3");
    }
}

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_QUEUE_H__
