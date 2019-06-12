/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Multi-port CPU Integer Registers memory.
 */

#ifndef __DEBUGGER_RIVERLIB_REGIBANK_H__
#define __DEBUGGER_RIVERLIB_REGIBANK_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(RegIntBank) {
    sc_in<bool> i_clk;                      // Clock
    sc_in<bool> i_nrst;                     // Reset. Active LOW
    sc_in<sc_uint<6>> i_radr1;              // Port 1 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata1;   // Port 1 read value

    sc_in<sc_uint<6>> i_radr2;              // Port 2 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata2;   // Port 2 read value

    sc_in<sc_uint<6>> i_waddr;              // Writing value
    sc_in<bool> i_wena;                     // Writing is enabled
    sc_in<sc_uint<RISCV_ARCH>> i_wdata;     // Writing value

    sc_in<sc_uint<5>> i_dport_addr;             // Debug port address
    sc_in<bool> i_dport_ena;                    // Debug port is enabled
    sc_in<bool> i_dport_write;                  // Debug port write is enabled
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;   // Debug port write value
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;  // Debug port read value

    sc_out<sc_uint<RISCV_ARCH>> o_ra;       // Return address for branch predictor

    void comb();
    void registers();

    SC_HAS_PROCESS(RegIntBank);

    RegIntBank(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    friend struct Processor; // for debug purposes(remove it)s

    struct RegistersType {
        sc_signal<bool> update;             // To generate SystemC delta event only.
        sc_uint<RISCV_ARCH> mem[Reg_Total]; // Multi-ports memory
    } v, r;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_REGIBANK_H__
