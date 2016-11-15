/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Integer divider.
 */

#ifndef __DEBUGGER_RIVERLIB_INT_DIV_H__
#define __DEBUGGER_RIVERLIB_INT_DIV_H__

#include <systemc.h>
#include "../../river_cfg.h"

namespace debugger {

SC_MODULE(IntDiv) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                 // Reset: active LOW
    sc_in<bool> i_ena;                  // Enable pulse
    sc_in<bool> i_unsigned;             // Unsigned operands
    sc_in<bool> i_rv32;                 // 32-bits instruction flag
    sc_in<bool> i_residual;             // Compute: 0 =division; 1=residual
    sc_in<sc_uint<RISCV_ARCH>> i_a1;    // Operand 1
    sc_in<sc_uint<RISCV_ARCH>> i_a2;    // Operand 2
    sc_out<sc_uint<RISCV_ARCH>> o_res;  // Result
    sc_out<bool> o_valid;               // Result is valid
    sc_out<bool> o_busy;                // Multiclock instruction processing

    void comb();
    void registers();

    SC_HAS_PROCESS(IntDiv);

    IntDiv(sc_module_name name_, sc_trace_file *vcd=0);

private:
    uint64_t compute_reference(bool unsign, bool rv32, bool resid,
                               uint64_t a1, uint64_t a2);

    uint64_t developing(uint64_t a1, uint64_t a2);

    struct RegistersType {
        sc_signal<bool> busy;
        sc_signal<sc_uint<64>> ena;
        sc_signal<sc_uint<RISCV_ARCH>> result;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_INT_DIV_H__
