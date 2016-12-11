/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug port.
 * @details    Must be connected to DSU.
 */

#ifndef __DEBUGGER_RIVERLIB_DBG_PORT_H__
#define __DEBUGGER_RIVERLIB_DBG_PORT_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(DbgPort) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset. Active LOW.
    // "RIVER" Debug interface
    sc_in<bool> i_dsu_valid;                            // Debug access from DSU command is valid
    sc_in<bool> i_dsu_write;                            // Write Debug value
    sc_in<sc_uint<16>> i_dsu_addr;                      // Debug register address
    sc_in<sc_uint<RISCV_ARCH>> i_dsu_wdata;             // Write value
    sc_out<sc_uint<RISCV_ARCH>> o_dsu_rdata;            // Read value
    // CPU debugging signals:
    sc_out<sc_uint<RISCV_ARCH>> o_core_wdata;           // Write data into Core
    sc_out<bool> o_halt;                                // Halt signal is equal to hold pipeline
    sc_out<bool> o_ireg_ena;                            // Access to integer register bank is enabled
    sc_out<bool> o_ireg_write;                          // Write integer register enabled
    sc_in<sc_uint<RISCV_ARCH>> i_ireg_rdata;            // Integer register read value
    sc_out<bool> o_csr_ena;                             // Access to CSR bank is enabled
    sc_out<bool> o_csr_write;                           // Write CSR enabled
    sc_in<sc_uint<RISCV_ARCH>> i_csr_rdata;             // CSR read value

    void comb();
    void registers();

    SC_HAS_PROCESS(DbgPort);

    DbgPort(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<sc_uint<2>> bank_idx;
        sc_signal<bool> halt;

        sc_signal<sc_uint<RISCV_ARCH>> timer;                // Timer.
        sc_signal<sc_uint<64>> step_cnt;                     // Number of valid executed instructions
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DBG_PORT_H__
