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
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_ctrl_ready;
    sensitive << i_resp_ctrl_valid;
    sensitive << w.d.jump_valid;
    sensitive << w.e.jump_valid;
    sensitive << r.dbgCnt;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    fetch0 = new InstrFetch("fetch0", vcd);
    fetch0->i_clk(i_clk);
    fetch0->i_nrst(i_nrst);
    fetch0->o_mem_addr_valid(o_req_ctrl_valid);
    fetch0->i_mem_addr_ready(i_req_ctrl_ready);
    fetch0->o_mem_addr(o_req_ctrl_addr);
    fetch0->i_mem_data_valid(i_resp_ctrl_valid);
    fetch0->i_mem_data_addr(i_resp_ctrl_addr);
    fetch0->i_mem_data(i_resp_ctrl_data);
    fetch0->i_jump_valid(w_jump_valid);
    fetch0->i_jump_pc(wb_jump_pc);
    fetch0->o_f_valid(w.f.valid);
    fetch0->o_f_pc(w.f.pc);
    fetch0->o_f_instr(w.f.instr);

    if (vcd) {
        //sc_trace(vcd, fetch0->o_f_pc, "top/fetch0/o_f_pc");
        //sc_trace(vcd, fetch0->o_f_instr, "top/fetch0/o_f_instr");
    }

    //todo:
    w.d.jump_valid = false;
    w.e.jump_valid = false;
};

Processor::~Processor() {
    delete fetch0;
}

void Processor::comb() {
    w_jump_valid = false;
    if (w.e.jump_valid.read()) {
        w_jump_valid = true;
        wb_jump_pc = w.e.jump_pc;
    } else if (w.d.jump_valid.read()) {
        w_jump_valid = true;
        wb_jump_pc = w.d.jump_pc;
    }

    v.dbgCnt = r.dbgCnt.read() + 1;

    if (!i_nrst.read()) {
        v.dbgCnt = 0;
        w_jump_valid = false;
    }
}

void Processor::registers() {
    r = v;
}

}  // namespace debugger

