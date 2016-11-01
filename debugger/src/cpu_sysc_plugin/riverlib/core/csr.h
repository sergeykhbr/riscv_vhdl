/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CSR registers module.
 */

#ifndef __DEBUGGER_RIVERLIB_CSR_H__
#define __DEBUGGER_RIVERLIB_CSR_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(CsrRegs) {
    sc_in<bool> i_clk;                      // Clock signal
    sc_in<bool> i_nrst;                     // Reset (active low)
    sc_in<sc_uint<12>> i_addr;              // CSR address
    sc_in<bool> i_wena;                     // Write enable
    sc_in<sc_uint<RISCV_ARCH>> i_wdata;     // CSR writing value
    sc_out<sc_uint<RISCV_ARCH>> o_rdata;    // CSR read value

    sc_out<bool> o_ie;                      // Interrupt enable bit
    sc_out<sc_uint<2>> o_mode;              // CPU mode
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_mvec; // Interrupt descriptors table

    void comb();
    void registers();

    SC_HAS_PROCESS(CsrRegs);

    CsrRegs(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<sc_uint<RISCV_ARCH>> mvec;
        sc_signal<sc_uint<2>> mode;
        sc_signal<bool> ie;
    } v, r;
};


}  // namespace debugger

#endif  // #ifndef __DEBUGGER_RIVERLIB_CSR_H__
