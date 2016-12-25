/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Left/Right shifter arithmetic/logic 32/64 bits.
 *
 * @details    Vivado synthesizer (2016.2) doesn't support shift
 *             from dynamic value, so implement this mux.
 */

#ifndef __DEBUGGER_RIVERLIB_RSHIFT_H__
#define __DEBUGGER_RIVERLIB_RSHIFT_H__

#include <systemc.h>
#include "../../river_cfg.h"

namespace debugger {

SC_MODULE(Shifter) {
    sc_in<sc_uint<RISCV_ARCH>> i_a1;      // Operand 1
    sc_in<sc_uint<6>> i_a2;               // Shift bits number
    sc_out<sc_uint<RISCV_ARCH>> o_sll;   // Logical shift left 64-bits operand
    sc_out<sc_uint<RISCV_ARCH>> o_sllw;  // Logical shift left 32-bits operand
    sc_out<sc_uint<RISCV_ARCH>> o_srl;    // Logical shift 64 bits
    sc_out<sc_uint<RISCV_ARCH>> o_sra;    // Arith. shift 64 bits
    sc_out<sc_uint<RISCV_ARCH>> o_srlw;   // Logical shift 32 bits
    sc_out<sc_uint<RISCV_ARCH>> o_sraw;   // Arith. shift 32 bits

    void comb();

    SC_HAS_PROCESS(Shifter);

    Shifter(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_RSHIFT_H__
