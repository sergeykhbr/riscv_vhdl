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

#include "api_core.h"
#include "fadd_d.h"

namespace debugger {

DoubleAdd::DoubleAdd(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_add;
    sensitive << i_sub;
    sensitive << i_eq;
    sensitive << i_lt;
    sensitive << i_le;
    sensitive << i_max;
    sensitive << i_min;
    sensitive << i_a;
    sensitive << i_b;
    sensitive << r.busy;
    sensitive << r.ena;
    sensitive << r.a;
    sensitive << r.b;
    sensitive << r.result;
    sensitive << r.add;
    sensitive << r.sub;
    sensitive << r.eq;
    sensitive << r.lt;
    sensitive << r.le;
    sensitive << r.max;
    sensitive << r.min;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void DoubleAdd::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_ena, "/top/proc0/exec0/fadd_d0/i_ena");
        sc_trace(o_vcd, i_add, "/top/proc0/exec0/fadd_d0/i_add");
        sc_trace(o_vcd, i_sub, "/top/proc0/exec0/fadd_d0/i_sub");
        sc_trace(o_vcd, i_eq, "/top/proc0/exec0/fadd_d0/i_eq");
        sc_trace(o_vcd, i_lt, "/top/proc0/exec0/fadd_d0/i_lt");
        sc_trace(o_vcd, i_le, "/top/proc0/exec0/fadd_d0/i_le");
        sc_trace(o_vcd, i_max, "/top/proc0/exec0/fadd_d0/i_max");
        sc_trace(o_vcd, i_min, "/top/proc0/exec0/fadd_d0/i_min");
        sc_trace(o_vcd, i_a, "/top/proc0/exec0/fadd_d0/i_a");
        sc_trace(o_vcd, i_b, "/top/proc0/exec0/fadd_d0/i_b");
        sc_trace(o_vcd, o_res, "/top/proc0/exec0/fadd_d0/o_res");
        sc_trace(o_vcd, o_valid, "/top/proc0/exec0/fadd_d0/o_valid");
        sc_trace(o_vcd, o_busy, "/top/proc0/exec0/fadd_d0/o_busy");
        sc_trace(o_vcd, r.ena, "/top/proc0/exec0/fadd_d0/r_ena");
        sc_trace(o_vcd, r.result, "/top/proc0/exec0/fadd_d0/r_result");
    }
}

void DoubleAdd::comb() {
    v = r;

    v.ena = (r.ena.read() << 1) | (i_ena.read() & !r.busy);

    if (i_ena.read()) {
        v.busy = 1;
        v.add = i_add;
        v.sub = i_sub;
        v.eq = i_eq;
        v.lt = i_lt;
        v.le = i_le;
        v.max = i_max;
        v.min = i_min;
        v.a = i_a;
        v.b = i_b;

        // Just for run-rime control (not for VHDL)
        v.a_dbg = i_a;
        v.b_dbg = i_b;
        v.reference_res = compute_reference(i_add.read(),
                                            i_sub.read(),
                                            i_eq.read(),
                                            i_lt.read(),
                                            i_le.read(),
                                            i_max.read(),
                                            i_min.read(),
                                            i_a.read(),
                                            i_b.read());

        v.result = v.reference_res;  // temporary
    }

    if (r.ena.read()[2] == 1) {
        v.busy = 0;
        v.add = 0;
        v.sub = 0;
        v.eq = 0;
        v.lt = 0;
        v.le = 0;
        v.max = 0;
        v.min = 0;
    }

    if (i_nrst.read() == 0) {
        R_RESET(v);
    }

    o_res = r.result;
    o_valid = r.ena.read()[3];
    o_busy = r.busy;
}

void DoubleAdd::registers() {
    r = v;
}

uint64_t DoubleAdd::compute_reference(bool addEna, bool subEna,
                                      bool eqEna, bool ltEna, bool leEna,
                                      bool maxEna, bool minEna,
                                      uint64_t a, uint64_t b) {
    Reg64Type ra, rb, ret;
    ra.val = a;
    rb.val = b;
    ret.val = 0;
    if (addEna) {
        ret.f64 = ra.f64 + rb.f64;
    } else if (subEna) {
        ret.f64 = ra.f64 - rb.f64;
    } else if (eqEna) {
        ret.val = ra.f64 == rb.f64 ? 1 : 0;
    } else if (ltEna) {
        ret.val = ra.f64 < rb.f64 ? 1 : 0;
    } else if (leEna) {
        ret.val = ra.f64 <= rb.f64 ? 1 : 0;
    } else if (maxEna) {
        ret.f64 = ra.f64 > rb.f64 ? ra.f64 : rb.f64;
    } else if (minEna) {
        ret.f64 = ra.f64 < rb.f64 ? ra.f64 : rb.f64;
    }
    return ret.val;
}

}  // namespace debugger

