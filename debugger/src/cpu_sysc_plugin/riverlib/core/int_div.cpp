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
    sensitive << i_a1;
    sensitive << i_a2;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
    }
};


void IntDiv::comb() {
    v = r;
    // todo: algorithm
    uint64_t x1 = i_a1.read();
    uint64_t x2 = i_a2.read();

    if (i_ena.read()) {
        if (i_a2.read() == 0) {
            v.result = 0;
            v.residual = 0;
        } else if (i_rv32.read()) {
            if (i_unsigned.read()) {
                v.result = (uint32_t)i_a1.read() / (uint32_t)i_a2.read();
                v.residual = (uint32_t)i_a1.read() % (uint32_t)i_a2.read();
            } else {
                v.result = (uint64_t)((int64_t)((int32_t)i_a1.read() / (int32_t)i_a2.read()));
                v.residual = (uint64_t)((int64_t)((int32_t)i_a1.read() % (int32_t)i_a2.read()));
            }
        } else {
            if (i_unsigned.read()) {
                v.result = i_a1.read() / i_a2.read();
                v.residual = i_a1.read() % i_a2.read();
            } else {
                v.result = (int64_t)i_a1.read() / (int64_t)i_a2.read();
                v.residual = (int64_t)i_a1.read() % (int64_t)i_a2.read();
            }
        }
    }

    if (i_nrst.read() == 0) {
        v.residual = 0;
        v.result = 0;
    }

    o_result = r.result;
    o_residual = r.residual;
}

void IntDiv::registers() {
    r = v;
}

}  // namespace debugger

