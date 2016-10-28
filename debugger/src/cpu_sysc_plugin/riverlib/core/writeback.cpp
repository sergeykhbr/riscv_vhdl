/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Write Back stage.
 */

#include "writeback.h"

namespace debugger {

InstrWriteBack::InstrWriteBack(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_m_valid;
    sensitive << i_m_pc;
    sensitive << i_m_instr;
    sensitive << r.valid;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
    }
};


void InstrWriteBack::comb() {
    v = r;


    if (!i_nrst.read()) {
        v.valid = false;
        v.pc = 0;
        v.instr = 0;
    }


    o_valid = r.valid;
    o_pc = r.pc;
    o_instr = r.instr;
}

void InstrWriteBack::registers() {
    r = v;
}

}  // namespace debugger

