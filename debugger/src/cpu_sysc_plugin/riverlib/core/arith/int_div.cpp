/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Integer divider.
 */

#include "int_div.h"

namespace debugger {

IntDiv::IntDiv(sc_module_name name_, sc_trace_file *vcd)
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_unsigned;
    sensitive << i_rv32;
    sensitive << i_residual;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << r.result;
    sensitive << r.ena;
    sensitive << r.busy;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_ena, "/top/proc0/exec0/div0/i_ena");
        sc_trace(vcd, o_res, "/top/proc0/exec0/div0/o_res");
        sc_trace(vcd, o_valid, "/top/proc0/exec0/div0/o_valid");
    }
    uint64_t t = developing(15, 3);
    t = developing(10, ~0ull);
};


void IntDiv::comb() {
    v = r;
    // todo: algorithm
    uint64_t x1 = i_a1.read();
    uint64_t x2 = i_a2.read();

    v.ena = (r.ena.read() << 1) | (i_ena & !r.busy);

    if (i_ena.read()) {
        v.busy = 1;
    } else if (r.ena.read()[62]) {
        v.busy = 0;
    }

    if (i_ena.read()) {
        v.result = compute_reference(i_unsigned.read(), i_rv32.read(),
                                     i_residual.read(),
                                     i_a1.read(), i_a2.read());

    }

    if (i_nrst.read() == 0) {
        v.result = 0;
        v.ena = 0;
        v.busy = 0;
    }

    o_res = r.result;
    o_valid = r.ena.read()[63];
    o_busy = r.busy;
}

void IntDiv::registers() {
    r = v;
}

uint64_t IntDiv::compute_reference(bool unsign, bool rv32, bool resid,
                                   uint64_t a1, uint64_t a2) {
    uint64_t ret;
    if (a2 == 0) {
        ret = 0;
    } else if (rv32) {
        if (unsign) {
            if (resid) {
                ret = (uint32_t)a1 % (uint32_t)a2;
            } else {
                ret = (uint32_t)a1 / (uint32_t)a2;
            }
        } else {
            if (resid) {
                ret = (uint64_t)((int64_t)((int32_t)a1 % (int32_t)a2));
            } else {
                ret = (uint64_t)((int64_t)((int32_t)a1 / (int32_t)a2));
            }
        }
    } else {
        if (unsign) {
            if (resid) {
                ret = a1 % a2;
            } else {
                ret = a1 / a2;
            }
        } else {
            if (resid) {
                ret = (int64_t)a1 % (int64_t)a2;
            } else {
                ret = (int64_t)a1 / (int64_t)a2;
            }
        }
    }
    return ret;
}

// http://www.ece.lsu.edu/ee3755/2012f/l07.v.html
uint64_t IntDiv::developing(uint64_t a1, uint64_t a2) {
    sc_biguint<128> qr;
    sc_biguint<65> diff;
    sc_biguint<65> divident = a1; 
    sc_biguint<65> divider = a2; 
    qr = a1;
    for (int i = 0; i < 64; i++) {
        diff = qr(127, 63) - divider;
        if (diff[64]) {
            qr = qr << 1;
        } else {
            qr = (diff(63, 0), qr(62, 0), 1);
        }
    }
    uint64_t xxx = v.result.read();
    uint64_t rem = qr(127, 64).to_uint64();
    uint64_t quot = qr(63, 0).to_uint64();
    bool st = true;

    return quot;
}

}  // namespace debugger

