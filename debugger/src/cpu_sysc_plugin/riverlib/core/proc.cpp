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
    fetch0->o_valid(w.f.valid);
    fetch0->o_pc(w.f.pc);
    fetch0->o_instr(w.f.instr);

    dec0 = new InstrDecoder("dec0", vcd);
    dec0->i_clk(i_clk);
    dec0->i_nrst(i_nrst);
    dec0->i_f_valid(w.f.valid);
    dec0->i_f_pc(w.f.pc);
    dec0->i_f_instr(w.f.instr);
    dec0->o_valid(w.d.instr_valid);
    dec0->o_pc(w.d.pc);
    dec0->o_instr(w.d.instr);
    dec0->o_sign_ext(w.d.sign_ext);
    dec0->o_isa_type(w.d.isa_type);
    dec0->o_instr_vec(w.d.instr_vec);
    dec0->o_user_level(w.d.user_level);
    dec0->o_priv_level(w.d.priv_level);
    dec0->o_exception(w.d.exception);

    exec0 = new InstrExecute("exec0", vcd);
    exec0->i_clk(i_clk);
    exec0->i_nrst(i_nrst);
    exec0->i_d_valid(w.d.instr_valid);
    exec0->i_d_pc(w.d.pc);
    exec0->i_d_instr(w.d.instr);
    exec0->i_sign_ext(w.d.sign_ext);
    exec0->i_isa_type(w.d.isa_type);
    exec0->i_ivec(w.d.instr_vec);
    exec0->i_user_level(w.d.user_level);
    exec0->i_priv_level(w.d.priv_level);
    exec0->i_exception(w.d.exception);
    exec0->o_radr1(w.e.radr1);
    exec0->i_rdata1(w.e.rdata1);
    exec0->o_radr2(w.e.radr2);
    exec0->i_rdata2(w.e.rdata2);
    exec0->o_res_addr(w.e.res_addr);
    exec0->o_res_data(w.e.res_data);
    exec0->i_m_ready(w.m.ready);
    exec0->o_memop_load(w.e.memop_load);
    exec0->o_memop_store(w.e.memop_store);
    exec0->o_memop_size(w.e.memop_size);
    exec0->o_valid(w.e.valid);
    exec0->o_pc(w.e.pc);
    exec0->o_npc(w.e.npc);
    exec0->o_instr(w.e.instr);


    iregs0 = new RegIntBank("iregs0", vcd);
    iregs0->i_clk(i_clk);
    iregs0->i_nrst(i_nrst);
    iregs0->i_radr1(w.e.radr1);
    iregs0->o_rdata1(w.e.rdata1);
    iregs0->i_radr2(w.e.radr2);
    iregs0->o_rdata2(w.e.rdata2);
    iregs0->i_wadr(w.w.waddr);
    iregs0->i_wena(w.w.wena);
    iregs0->i_wdata(w.w.wdata);
    iregs0->o_ra(wb_ra);   // Return address
    iregs0->o_ra_updated(w_ra_updated);


    if (vcd) {
        //sc_trace(vcd, fetch0->o_f_pc, "top/fetch0/o_f_pc");
        //sc_trace(vcd, fetch0->o_f_instr, "top/fetch0/o_f_instr");
    }

    //todo:
    w.d.jump_valid = false;
    w.w.wena = 0;
};

Processor::~Processor() {
    delete fetch0;
}

void Processor::comb() {

    v.dbgCnt = r.dbgCnt.read() + 1;

    if (!i_nrst.read()) {
        v.dbgCnt = 0;
    }
}

void Processor::registers() {
    r = v;
}

}  // namespace debugger

