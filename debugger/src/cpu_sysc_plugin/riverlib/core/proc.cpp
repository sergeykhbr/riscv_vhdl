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
    sensitive << i_resp_ctrl_valid;
    sensitive << w.f.imem_req_valid;
    sensitive << w.f.imem_req_addr;
    sensitive << w.f.valid;
    //sensitive << r.predict_npc;
    sensitive << r.dbgCnt;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    fetch0 = new InstrFetch("fetch0", vcd);
    fetch0->i_clk(i_clk);
    fetch0->i_nrst(i_nrst);
    fetch0->i_cache_hold(i_cache_hold);
    fetch0->i_pipeline_hold(w.e.hazard_hold);
    fetch0->o_mem_addr_valid(w.f.imem_req_valid);
    fetch0->o_mem_addr(w.f.imem_req_addr);
    fetch0->i_mem_data_valid(i_resp_ctrl_valid);
    fetch0->i_mem_data_addr(i_resp_ctrl_addr);
    fetch0->i_mem_data(i_resp_ctrl_data);
    fetch0->i_e_npc_valid(w.e.valid);
    fetch0->i_e_npc(w.e.npc);
    fetch0->i_predict_npc(wb_npc_predict);
    fetch0->o_predict_miss(w.f.predict_miss);
    fetch0->o_valid(w.f.valid);
    fetch0->o_pc(w.f.pc);
    fetch0->o_instr(w.f.instr);

    dec0 = new InstrDecoder("dec0", vcd);
    dec0->i_clk(i_clk);
    dec0->i_nrst(i_nrst);
    dec0->i_pipeline_hold(i_cache_hold);// || w.e.hazard_hold.read());
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
    exec0->i_cache_hold(i_cache_hold);
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
    exec0->o_hazard_hold(w.e.hazard_hold);
    exec0->o_memop_load(w.e.memop_load);
    exec0->o_memop_store(w.e.memop_store);
    exec0->o_memop_size(w.e.memop_size);
    exec0->o_memop_addr(w.e.memop_addr);
    exec0->o_valid(w.e.valid);
    exec0->o_pc(w.e.pc);
    exec0->o_npc(w.e.npc);
    exec0->o_instr(w.e.instr);

    mem0 = new MemAccess("mem0", vcd);
    mem0->i_clk(i_clk);
    mem0->i_nrst(i_nrst);
    mem0->i_e_valid(w.e.valid);
    mem0->i_e_pc(w.e.pc);
    mem0->i_e_instr(w.e.instr);
    mem0->i_res_addr(w.e.res_addr);
    mem0->i_res_data(w.e.res_data);
    mem0->i_memop_load(w.e.memop_load);
    mem0->i_memop_store(w.e.memop_store);
    mem0->i_memop_size(w.e.memop_size);
    mem0->o_waddr(w.w.waddr);
    mem0->o_wena(w.w.wena);
    mem0->o_wdata(w.w.wdata);
    mem0->o_mem_valid(o_req_data_valid);
    mem0->o_mem_write(o_req_data_write);
    mem0->o_mem_sz(o_req_data_size);
    mem0->o_mem_addr(o_req_data_addr);
    mem0->o_mem_data(o_req_data_data);
    mem0->i_mem_data_valid(i_resp_data_valid);
    mem0->i_mem_data_addr(i_resp_data_addr);
    mem0->i_mem_data(i_resp_data_data);
    mem0->o_valid(w.m.valid);
    mem0->o_pc(w.m.pc);
    mem0->o_instr(w.m.instr);

    predic0 = new BranchPredictor("predic0", vcd);
    predic0->i_clk(i_clk);
    predic0->i_nrst(i_nrst);
    predic0->i_hold(i_cache_hold);
    predic0->i_f_mem_request(w.f.imem_req_valid);
    predic0->i_f_predic_miss(w.f.predict_miss);
    predic0->i_f_instr_valid(w.f.valid);
    predic0->i_f_instr(w.f.instr);
    predic0->i_e_npc(w.e.npc);
    predic0->i_ra(wb_ra);
    predic0->o_npc_predict(wb_npc_predict);


    iregs0 = new RegIntBank("iregs0", vcd);
    iregs0->i_clk(i_clk);
    iregs0->i_nrst(i_nrst);
    iregs0->i_radr1(w.e.radr1);
    iregs0->o_rdata1(w.e.rdata1);
    iregs0->i_radr2(w.e.radr2);
    iregs0->o_rdata2(w.e.rdata2);
    iregs0->i_waddr(w.w.waddr);
    iregs0->i_wena(w.w.wena);
    iregs0->i_wdata(w.w.wdata);
    iregs0->o_ra(wb_ra);   // Return address


    if (vcd) {
        //sc_trace(vcd, fetch0->o_f_pc, "top/fetch0/o_f_pc");
        //sc_trace(vcd, fetch0->o_f_instr, "top/fetch0/o_f_instr");
    }

    //todo:
    w.w.wena = 0;
};

Processor::~Processor() {
    delete fetch0;
}

void Processor::comb() {

    if (w.f.imem_req_valid.read()) {
        //v.predict_npc = r.predict_npc.read() + 4;
    }

    v.dbgCnt = r.dbgCnt.read() + 1;

    if (!i_nrst.read()) {
        v.dbgCnt = 0;
        //v.predict_npc = RESET_VECTOR;
    }

    o_req_ctrl_valid = w.f.imem_req_valid;
    o_req_ctrl_addr = w.f.imem_req_addr;
}

void Processor::registers() {
    r = v;
}

}  // namespace debugger

