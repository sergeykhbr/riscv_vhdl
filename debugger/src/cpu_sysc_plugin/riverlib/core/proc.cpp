/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU pipeline implementation.
 */

#include "proc.h"

namespace debugger {

Processor::Processor(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(proc0);
    sensitive << i_resp_ctrl_ready;
    sensitive << r.f.pc;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, r.f.pc, "r.f.pc");
    }
};


void Processor::proc0() {
    v = r;

    //
    v.f.pc_valid.write(false);
    if (dbgCnt_.read()  == 7) {
        v.f.pc_valid.write(true);
        v.f.pc = (r.f.pc.read() + 4) % 0x2000;
    }

    v.i.instr_valid.write(false);
    if (i_resp_ctrl_ready.read()) {
        v.i.instr_valid.write(true);
        v.i.instr = i_resp_ctrl_data;
        v.i.pc = r.f.pc;
    }

    rin = v;

    o_req_ctrl_valid = r.f.pc_valid ;
    o_req_ctrl_addr = r.f.pc;

}

void Processor::registers() {
    if (!i_nrst.read()) {
        dbgCnt_.write(0);

        r.f.pc.write(RESET_VECTOR);
        r.f.pc_valid.write(false);
    } else {
        r = rin;
        dbgCnt_.write(dbgCnt_.read() + 1);
    }
}

}  // namespace debugger

