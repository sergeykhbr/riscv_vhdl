/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Write Back stage.
 */

#ifndef __DEBUGGER_RIVERLIB_WRITEBACK_H__
#define __DEBUGGER_RIVERLIB_WRITEBACK_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(InstrWriteBack) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_hold;
    sc_in<bool> i_m_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_m_pc;
    sc_in<sc_uint<32>> i_m_instr;

    sc_out<bool> o_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_pc;
    sc_out<sc_uint<32>> o_instr;


    void comb();
    void registers();

    SC_HAS_PROCESS(InstrWriteBack);

    InstrWriteBack(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_WRITEBACK_H__
