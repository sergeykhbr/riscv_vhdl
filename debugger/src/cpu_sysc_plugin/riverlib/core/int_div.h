/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Integer divider.
 */

#ifndef __DEBUGGER_RIVERLIB_INT_DIV_H__
#define __DEBUGGER_RIVERLIB_INT_DIV_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

static const int IDIV_EXEC_DURATION_CYCLES = 64;     // Specify any number to provide desired performance

SC_MODULE(IntDiv) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_ena;
    sc_in<bool> i_unsigned;
    sc_in<bool> i_rv32;
    sc_in<sc_uint<RISCV_ARCH>> i_a1;
    sc_in<sc_uint<RISCV_ARCH>> i_a2;
    sc_out<sc_uint<RISCV_ARCH>> o_result;
    sc_out<sc_uint<RISCV_ARCH>> o_residual;

    void comb();
    void registers();

    SC_HAS_PROCESS(IntDiv);

    IntDiv(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<sc_uint<RISCV_ARCH>> result;
        sc_signal<sc_uint<RISCV_ARCH>> residual;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_INT_DIV_H__
