/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Instruction Decoder stage.
 */

#include "decoder.h"

namespace debugger {

InstrDecoder::InstrDecoder(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_f_valid;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    if (vcd) {
        //sc_trace(vcd, r.f.pc, "r.f.pc");
    }
};


void InstrDecoder::comb() {
    v = r;
    v.pc = i_f_pc;
    v.valid = i_f_valid;


    o_f_ready = true;
    o_valid = r.valid;
    o_pc = r.pc;
}

void InstrDecoder::registers() {
    if (!i_nrst.read()) {
        r.valid = false;
        r.pc = 0;
    } else {
        r = v;
    }
}

}  // namespace debugger

