/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Integer multiplier.
 * @details    Implemented algorithm provides 4 clocks per instruction
 */

#ifndef __DEBUGGER_RIVERLIB_INT_MUL_H__
#define __DEBUGGER_RIVERLIB_INT_MUL_H__

#include <systemc.h>
#include "../../river_cfg.h"

namespace debugger {

SC_MODULE(IntMul) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_ena;                      // Enable bit
    sc_in<bool> i_unsigned;                 // Unsigned operands
    sc_in<bool> i_high;                     // High multiplied bits [127:64]
    sc_in<bool> i_rv32;                     // 32-bits operands enabled
    sc_in<sc_uint<RISCV_ARCH>> i_a1;        // Operand 1
    sc_in<sc_uint<RISCV_ARCH>> i_a2;        // Operand 2
    sc_out<sc_uint<RISCV_ARCH>> o_res;      // Result
    sc_out<bool> o_valid;                   // Result is valid
    sc_out<bool> o_busy;                    // Multiclock instruction under processing

    void comb();
    void registers();

    SC_HAS_PROCESS(IntMul);

    IntMul(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    uint64_t compute_reference(bool unsign, bool rv32, uint64_t a1, uint64_t a2);

    struct Level0Type {
        sc_biguint<66> arr[32];
    };

    struct Level1Type {
        sc_biguint<69> arr[16];
    };

    struct Level2Type {
        sc_biguint<74> arr[8];
    };

    struct Level3Type {
        sc_biguint<83> arr[4];
    };

    struct Level4Type {
        sc_biguint<100> arr[2];
    };

    struct RegistersType {
        sc_signal<bool> busy;
        sc_signal<sc_uint<4>> ena;
        sc_uint<RISCV_ARCH> a1;
        sc_uint<RISCV_ARCH> a2;
        sc_signal<bool> unsign;
        sc_signal<bool> high;
        sc_signal<bool> rv32;
        Level1Type lvl1;
        Level3Type lvl3;
        sc_signal<sc_biguint<128>> result;

        sc_uint<RISCV_ARCH> a1_dbg;
        sc_uint<RISCV_ARCH> a2_dbg;
        sc_uint<RISCV_ARCH> reference_mul;          // Used for run-time comparision
    } v, r;

};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_INT_MUL_H__
