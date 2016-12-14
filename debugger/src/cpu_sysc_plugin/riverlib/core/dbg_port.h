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
    sc_in<bool> i_dport_valid;                          // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<2>> i_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_in<sc_uint<12>> i_dport_addr;                    // Register idx
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_ready;                         // Response is ready
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    // CPU debugging signals:
    sc_out<sc_uint<12>> o_core_addr;                    // Address of the sub-region register
    sc_out<sc_uint<RISCV_ARCH>> o_core_wdata;           // Write data
    sc_out<bool> o_csr_ena;                             // Region 0: Access to CSR bank is enabled.
    sc_out<bool> o_csr_write;                           // Region 0: CSR write enable
    sc_in<sc_uint<RISCV_ARCH>> i_csr_rdata;             // Region 0: CSR read value
    sc_out<bool> o_ireg_ena;                            // Region 1: Access to integer register bank is enabled
    sc_out<bool> o_ireg_write;                          // Region 1: Integer registers bank write pulse
    sc_out<bool> o_npc_write;                           // Region 1: npc write enable
    sc_in<sc_uint<RISCV_ARCH>> i_ireg_rdata;            // Region 1: Integer register read value
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_pc;                // Region 1: Instruction pointer
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_npc;               // Region 1: Next Instruction pointer
    sc_in<bool> i_e_valid;                              // Stepping control signal
    sc_in<bool> i_m_valid;                              // To compute number of valid executed instruction
    sc_out<sc_uint<64>> o_clock_cnt;                    // Number of clocks excluding halt state
    sc_out<sc_uint<64>> o_executed_cnt;                 // Number of executed instructions
    sc_out<bool> o_halt;                                // Halt signal is equal to hold pipeline

    void comb();
    void registers();

    SC_HAS_PROCESS(DbgPort);

    DbgPort(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    struct RegistersType {
        sc_signal<bool> ready;
        sc_signal<bool> halt;
        sc_signal<bool> stepping_mode;
        sc_signal<sc_uint<RISCV_ARCH>> stepping_mode_cnt;

        sc_signal<sc_uint<RISCV_ARCH>> rdata;
        sc_signal<sc_uint<RISCV_ARCH>> stepping_mode_steps; // Number of steps before halt in stepping mode
        sc_signal<sc_uint<64>> clock_cnt;                   // Timer in clocks.
        sc_signal<sc_uint<64>> executed_cnt;                // Number of valid executed instructions
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DBG_PORT_H__
