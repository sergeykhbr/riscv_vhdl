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
    sensitive << i_cache_hold;
    sensitive << w.e.hazard_hold;
    sensitive << w.f.imem_req_valid;
    sensitive << w.f.imem_req_addr;
    sensitive << w.f.valid;
    sensitive << r.clk_cnt;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

#ifdef GENERATE_DEBUG_FILE
    SC_METHOD(negedge_dbg_print);
    sensitive << i_clk.neg();
#endif

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
    dec0->i_any_hold(w_any_hold);
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
    exec0->i_wb_done(w.m.valid);
    exec0->i_sign_ext(w.d.sign_ext);
    exec0->i_isa_type(w.d.isa_type);
    exec0->i_ivec(w.d.instr_vec);
    exec0->i_user_level(w.d.user_level);
    exec0->i_priv_level(w.d.priv_level);
    exec0->i_ie(csr.ie);
    exec0->i_idt(csr.mvec);
    exec0->i_mode(csr.mode);
    exec0->i_unsup_exception(w.d.exception);
    exec0->i_ext_irq(i_ext_irq);
    exec0->o_radr1(w.e.radr1);
    exec0->i_rdata1(w.e.rdata1);
    exec0->o_radr2(w.e.radr2);
    exec0->i_rdata2(w.e.rdata2);
    exec0->o_res_addr(w.e.res_addr);
    exec0->o_res_data(w.e.res_data);
    exec0->o_hazard_hold(w.e.hazard_hold);
    exec0->o_csr_addr(csr.addr);
    exec0->o_csr_wena(csr.wena);
    exec0->i_csr_rdata(csr.rdata);
    exec0->o_csr_wdata(csr.wdata);
    exec0->o_memop_sign_ext(w.e.memop_sign_ext);
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
    mem0->i_memop_sign_ext(w.e.memop_sign_ext);
    mem0->i_memop_load(w.e.memop_load);
    mem0->i_memop_store(w.e.memop_store);
    mem0->i_memop_size(w.e.memop_size);
    mem0->i_memop_addr(w.e.memop_addr);
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

    csr0 = new CsrRegs("csr0", vcd);
    csr0->i_clk(i_clk);
    csr0->i_nrst(i_nrst);
    csr0->i_addr(csr.addr);
    csr0->i_wena(csr.wena);
    csr0->i_wdata(csr.wdata);
    csr0->o_rdata(csr.rdata);
    csr0->o_ie(csr.ie);
    csr0->o_mode(csr.mode);
    csr0->o_mvec(csr.mvec);

    if (vcd) {
        sc_trace(vcd, r.clk_cnt, "top/r_clk_cnt");
#ifdef GENERATE_DEBUG_FILE
        sc_trace(vcd, line_cnt, "top/line_cnt");
#endif
    }

#ifdef GENERATE_DEBUG_FILE
    reg_dbg = new ofstream("river_sysc_regs.log");
    mem_dbg = new ofstream("river_sysc_mem.log");
    line_cnt = 0;
    mem_dbg_write_flag = false;
#endif
};

Processor::~Processor() {
    delete fetch0;
    delete dec0;
    delete exec0;
    delete mem0;
    delete predic0;
    delete iregs0;
    delete csr0;
#ifdef GENERATE_DEBUG_FILE
    reg_dbg->close();
    mem_dbg->close();
    delete reg_dbg;
    delete mem_dbg;
#endif
}

void Processor::comb() {
    v = r;

    v.clk_cnt = r.clk_cnt.read() + 1;

    if (!i_nrst.read()) {
        v.clk_cnt = 0;
    }

    o_req_ctrl_valid = w.f.imem_req_valid;
    o_req_ctrl_addr = w.f.imem_req_addr;

    w_any_hold = i_cache_hold.read() || w.e.hazard_hold.read();
}

void Processor::registers() {
    r = v;
}

#ifdef GENERATE_DEBUG_FILE
void Processor::negedge_dbg_print() {
    int sz;
    if (w.m.valid.read()) {
        line_cnt++;
        sz = sprintf(tstr, "%8I64d [%08x] %08x: ",
            line_cnt,
            w.m.pc.read().to_int(),
            w.m.instr.read().to_int());
        uint64_t prev_val = iregs0->r.mem[w.w.waddr.read().to_int()].to_int64();
        uint64_t cur_val = w.w.wdata.read().to_int64();
        if (w.w.waddr.read() == 0 || prev_val == cur_val) {
            // not writing
            sz += sprintf(&tstr[sz], "%s", "-\n");
        } else {
            sz += sprintf(&tstr[sz], "%3s <= %016I64x\n",
                        IREGS_NAMES[w.w.waddr.read().to_int()],
                        cur_val);
        }

        (*reg_dbg) << tstr;
        reg_dbg->flush();
    }
    // Memory access debug:
    if (i_resp_data_valid.read()) {
        sz = sprintf(tstr, "[%08x] ", i_resp_data_addr.read().to_uint());
        if (mem_dbg_write_flag) {
            sz += sprintf(&tstr[sz], "<= %016I64x\n", 
                dbg_mem_write_value & dbg_mem_value_mask);
        } else {
            sz += sprintf(&tstr[sz], "=> %016I64x\n", 
                i_resp_data_data.read().to_uint64() & dbg_mem_value_mask);
        }
        (*mem_dbg) << tstr;
        mem_dbg->flush();
    }
    if (w.e.memop_store.read() || w.e.memop_load.read()) {
        mem_dbg_write_flag = w.e.memop_store;
        if (mem_dbg_write_flag) {
            dbg_mem_write_value = w.e.res_data.read();
        }
        switch (w.e.memop_size.read()) {
        case 0: dbg_mem_value_mask = 0xFFull; break;
        case 1: dbg_mem_value_mask = 0xFFFFull; break;
        case 2: dbg_mem_value_mask = 0xFFFFFFFFull; break;
        default: dbg_mem_value_mask = ~0ull;
        }
    }
}
#endif


}  // namespace debugger

