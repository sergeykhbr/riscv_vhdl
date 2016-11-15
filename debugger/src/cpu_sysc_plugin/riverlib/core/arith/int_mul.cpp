/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Integer multiplier.
 * @details    Implemented algorithm provides 4 clocks per instruction
 */

#include "int_mul.h"

namespace debugger {

IntMul::IntMul(sc_module_name name_, sc_trace_file *vcd)
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_ena;
    sensitive << i_unsigned;
    sensitive << i_rv32;
    sensitive << i_high;
    sensitive << i_a1;
    sensitive << i_a2;
    sensitive << r.result;
    sensitive << r.ena;
    sensitive << r.busy;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_a1, "/top/proc0/exec0/mul0/i_a1");
        sc_trace(vcd, i_a2, "/top/proc0/exec0/mul0/i_a2");
        sc_trace(vcd, i_ena, "/top/proc0/exec0/mul0/i_ena");
        sc_trace(vcd, o_res, "/top/proc0/exec0/mul0/o_res");
        sc_trace(vcd, o_valid, "/top/proc0/exec0/mul0/o_valid");
        sc_trace(vcd, o_busy, "/top/proc0/exec0/mul0/o_busy");
        sc_trace(vcd, r.ena, "/top/proc0/exec0/mul0/r_ena");
    }
};


void IntMul::comb() {
    sc_uint<2> wb_mux_lvl0;
    Level0Type wb_lvl0;
    Level2Type wb_lvl2;
    Level4Type wb_lvl4;
    sc_uint<64> wb_res;

    v = r;

    v.ena = (r.ena.read() << 1) | (i_ena & !r.busy);

    if (i_ena.read()) {
        v.busy = 1;
        if (i_rv32.read()) {
            v.a1 = i_a1.read()(31, 0);
            if (!i_unsigned.read() && i_a1.read()[31]) {
                v.a1(63, 32) = ~0;
            }
            v.a2 = i_a2.read()(31, 0);
            if (!i_unsigned.read() && i_a2.read()[31]) {
                v.a2(63, 32) = ~0;
            }
        } else {
            v.a1 = i_a1;
            v.a2 = i_a2;
        }
        v.rv32 = i_rv32;
        v.unsign = i_unsigned;
        v.high = i_high;

        // Just for run-rime control (not for VHDL)
        v.reference_mul = compute_reference(i_unsigned.read(),
                                            i_rv32.read(),
                                            i_a1.read(),
                                            i_a2.read());
    }

    if (r.ena.read()[0]) {
        for (int i = 0; i < 32; i++) {
            wb_mux_lvl0 = r.a2(2*i + 1, 2*i);
            if (wb_mux_lvl0 == 0) {
                wb_lvl0.arr[i] = 0;
            } else if (wb_mux_lvl0 == 1) {
                wb_lvl0.arr[i] = sc_biguint<66>(r.a1);
            } else if (wb_mux_lvl0 == 2) {
                wb_lvl0.arr[i] = sc_biguint<66>(r.a1) << 1;
            } else {
                wb_lvl0.arr[i] = sc_biguint<66>(r.a1)
                              + (sc_biguint<66>(r.a1) << 1);
            }
        }

        for (int i = 0; i < 16; i++) {
            v.lvl1.arr[i] = (sc_biguint<69>(wb_lvl0.arr[2*i + 1]) << 2)
                          + sc_biguint<69>(wb_lvl0.arr[2*i]);
        }
    }

    if (r.ena.read()[1]) {
        for (int i = 0; i < 8; i++) {
            wb_lvl2.arr[i] = (sc_biguint<74>(r.lvl1.arr[2*i + 1]) << 4)
                       + sc_biguint<74>(r.lvl1.arr[2*i]);
        }

        for (int i = 0; i < 4; i++) {
            v.lvl3.arr[i] = (sc_biguint<83>(wb_lvl2.arr[2*i + 1]) << 8)
                          + sc_biguint<83>(wb_lvl2.arr[2*i]);
        }
    }

    if (r.ena.read()[2]) {
        v.busy = 0;
        for (int i = 0; i < 2; i++) {
            wb_lvl4.arr[i] = (sc_biguint<100>(r.lvl3.arr[2*i + 1]) << 16)
                           + sc_biguint<100>(r.lvl3.arr[2*i]);
        }

        v.result = (sc_biguint<128>(wb_lvl4.arr[1]) << 32) 
                    + sc_biguint<128>(wb_lvl4.arr[0]);
    }

    wb_res = r.result.read()(63, 0);
    if (r.high.read()) {
        wb_res = r.result.read()(127, 64);  // not tested yet
    }

    if (i_nrst.read() == 0) {
        v.busy = 0;
        v.result = 0;
        v.ena = 0;
        v.a1 = 0;
        v.a2 = 0;
        v.rv32 = 0;
        v.unsign = 0;
        v.high = 0;
        v.reference_mul = 0;
    }

    o_res = wb_res;
    o_valid = r.ena.read()[3];
    o_busy = r.busy;
}

void IntMul::registers() {
    // Debug purpose only"
    if (r.ena.read()[2]) {
        uint64_t t1 = v.result.read()(63,0).to_uint64();
        uint64_t t2 = r.reference_mul.to_uint64();
        if (t1 != t2) {
            printf("IntMul: error\n");
        }
    }
    r = v;
}

uint64_t IntMul::compute_reference(bool unsign, bool rv32, uint64_t a1, uint64_t a2) {
    uint64_t ret;
    if (rv32) {
        if (unsign) {
            ret = (uint32_t)a1 * (uint32_t)a2;
        } else {
            ret = (uint64_t)((int64_t)((int32_t)a1 * (int32_t)a2));
        }
    } else {
        // The results are the same but just for clearence
        if (unsign) {
            ret = a1 * a2;
        } else {
            ret = (int64_t)a1 * (int64_t)a2;
        }
    }
    return ret;
}

}  // namespace debugger

