/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU pipeline implementation.
 */

#include "proc.h"
#include "api_core.h"

namespace debugger {

Processor::Processor(sc_module_name name_, uint32_t hartid)
    : sc_module(name_) {

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_resp_ctrl_valid;
    sensitive << w.f.pipeline_hold;
    sensitive << w.e.valid;
    sensitive << w.e.pipeline_hold;
    sensitive << w.m.pipeline_hold;
    sensitive << w.f.imem_req_valid;
    sensitive << w.f.imem_req_addr;
    sensitive << w.f.valid;
    sensitive << dbg.clock_cnt;
    sensitive << dbg.executed_cnt;
    sensitive << dbg.core_addr;
    sensitive << dbg.halt;
    sensitive << dbg.core_wdata;

    SC_METHOD(negedge_dbg_print);
    sensitive << i_clk.neg();

    fetch0 = new InstrFetch("fetch0");
    fetch0->i_clk(i_clk);
    fetch0->i_nrst(i_nrst);
    fetch0->i_pipeline_hold(w_fetch_pipeline_hold);
    fetch0->i_mem_req_ready(i_req_ctrl_ready);
    fetch0->o_mem_addr_valid(w.f.imem_req_valid);
    fetch0->o_mem_addr(w.f.imem_req_addr);
    fetch0->i_mem_data_valid(i_resp_ctrl_valid);
    fetch0->i_mem_data_addr(i_resp_ctrl_addr);
    fetch0->i_mem_data(i_resp_ctrl_data);
    fetch0->i_mem_load_fault(i_resp_ctrl_load_fault);
    fetch0->o_mem_resp_ready(o_resp_ctrl_ready);
    fetch0->i_e_npc(w.e.npc);
    fetch0->i_predict_npc(bp.npc);
    fetch0->i_predict(bp.predict);
    fetch0->o_predict_miss(w.f.predict_miss);
    fetch0->o_mem_req_fire(w.f.req_fire);
    fetch0->o_ex_load_fault(w.f.load_fault);
    fetch0->o_valid(w.f.valid);
    fetch0->o_pc(w.f.pc);
    fetch0->o_instr(w.f.instr);
    fetch0->o_hold(w.f.pipeline_hold);
    fetch0->i_br_fetch_valid(dbg.br_fetch_valid);
    fetch0->i_br_address_fetch(dbg.br_address_fetch);
    fetch0->i_br_instr_fetch(dbg.br_instr_fetch);
    fetch0->o_instr_buf(w.f.instr_buf);

    dec0 = new InstrDecoder("dec0");
    dec0->i_clk(i_clk);
    dec0->i_nrst(i_nrst);
    dec0->i_any_hold(w_any_pipeline_hold);
    dec0->i_f_valid(w.f.valid);
    dec0->i_f_pc(w.f.pc);
    dec0->i_f_instr(w.f.instr);
    dec0->o_valid(w.d.instr_valid);
    dec0->o_pc(w.d.pc);
    dec0->o_instr(w.d.instr);
    dec0->o_memop_store(w.d.memop_store);
    dec0->o_memop_load(w.d.memop_load);
    dec0->o_memop_sign_ext(w.d.memop_sign_ext);
    dec0->o_memop_size(w.d.memop_size);
    dec0->o_unsigned_op(w.d.unsigned_op);
    dec0->o_rv32(w.d.rv32);
    dec0->o_compressed(w.d.compressed);
    dec0->o_isa_type(w.d.isa_type);
    dec0->o_instr_vec(w.d.instr_vec);
    dec0->o_exception(w.d.exception);

    exec0 = new InstrExecute("exec0");
    exec0->i_clk(i_clk);
    exec0->i_nrst(i_nrst);
    exec0->i_pipeline_hold(w_exec_pipeline_hold);
    exec0->i_d_valid(w.d.instr_valid);
    exec0->i_d_pc(w.d.pc);
    exec0->i_d_instr(w.d.instr);
    exec0->i_wb_done(w.m.valid);
    exec0->i_memop_store(w.d.memop_store);
    exec0->i_memop_load(w.d.memop_load);
    exec0->i_memop_sign_ext(w.d.memop_sign_ext);
    exec0->i_memop_size(w.d.memop_size);
    exec0->i_unsigned_op(w.d.unsigned_op);
    exec0->i_rv32(w.d.rv32);
    exec0->i_compressed(w.d.compressed);
    exec0->i_isa_type(w.d.isa_type);
    exec0->i_ivec(w.d.instr_vec);
    exec0->i_unsup_exception(w.d.exception);
    exec0->i_dport_npc_write(dbg.npc_write);
    exec0->i_dport_npc(wb_exec_dport_npc);
    exec0->o_radr1(w.e.radr1);
    exec0->i_rdata1(ireg.rdata1);
    exec0->o_radr2(w.e.radr2);
    exec0->i_rdata2(ireg.rdata2);
    exec0->o_res_addr(w.e.res_addr);
    exec0->o_res_data(w.e.res_data);
    exec0->o_pipeline_hold(w.e.pipeline_hold);
    exec0->o_csr_addr(w.e.csr_addr);
    exec0->o_csr_wena(w.e.csr_wena);
    exec0->i_csr_rdata(csr.rdata);
    exec0->o_csr_wdata(w.e.csr_wdata);
    exec0->i_trap_valid(csr.trap_valid);
    exec0->i_trap_pc(csr.trap_pc);
    exec0->o_ex_npc(w.e.ex_npc);
    exec0->o_ex_illegal_instr(w.e.ex_illegal_instr);
    exec0->o_ex_unalign_store(w.e.ex_unalign_store);
    exec0->o_ex_unalign_load(w.e.ex_unalign_load);
    exec0->o_ex_breakpoint(w.e.ex_breakpoint);
    exec0->o_ex_ecall(w.e.ex_ecall);
    exec0->o_memop_sign_ext(w.e.memop_sign_ext);
    exec0->o_memop_load(w.e.memop_load);
    exec0->o_memop_store(w.e.memop_store);
    exec0->o_memop_size(w.e.memop_size);
    exec0->o_memop_addr(w.e.memop_addr);
    exec0->o_pre_valid(w.e.pre_valid);
    exec0->o_valid(w.e.valid);
    exec0->o_pc(w.e.pc);
    exec0->o_npc(w.e.npc);
    exec0->o_instr(w.e.instr);
    exec0->o_call(w.e.call);
    exec0->o_ret(w.e.ret);
    exec0->o_mret(w.e.mret);
    exec0->o_uret(w.e.uret);

    mem0 = new MemAccess("mem0");
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
    mem0->i_mem_req_ready(i_req_data_ready);
    mem0->o_mem_valid(o_req_data_valid);
    mem0->o_mem_write(o_req_data_write);
    mem0->o_mem_sz(o_req_data_size);
    mem0->o_mem_addr(o_req_data_addr);
    mem0->o_mem_data(o_req_data_data);
    mem0->i_mem_data_valid(i_resp_data_valid);
    mem0->i_mem_data_addr(i_resp_data_addr);
    mem0->i_mem_data(i_resp_data_data);
    mem0->o_mem_resp_ready(o_resp_data_ready);
    mem0->o_hold(w.m.pipeline_hold);
    mem0->o_valid(w.m.valid);
    mem0->o_pc(w.m.pc);
    mem0->o_instr(w.m.instr);

    predic0 = new BranchPredictor("predic0");
    predic0->i_clk(i_clk);
    predic0->i_nrst(i_nrst);
    predic0->i_req_mem_fire(w.f.req_fire);
    predic0->i_resp_mem_valid(i_resp_ctrl_valid);
    predic0->i_resp_mem_addr(i_resp_ctrl_addr);
    predic0->i_resp_mem_data(i_resp_ctrl_data);
    predic0->i_f_predic_miss(w.f.predict_miss);
    predic0->i_e_npc(w.e.npc);
    predic0->i_ra(ireg.ra);
    predic0->o_npc_predict(bp.npc);
    predic0->o_predict(bp.predict);

    iregs0 = new RegIntBank("iregs0");
    iregs0->i_clk(i_clk);
    iregs0->i_nrst(i_nrst);
    iregs0->i_radr1(w.e.radr1);
    iregs0->o_rdata1(ireg.rdata1);
    iregs0->i_radr2(w.e.radr2);
    iregs0->o_rdata2(ireg.rdata2);
    iregs0->i_waddr(w.w.waddr);
    iregs0->i_wena(w.w.wena);
    iregs0->i_wdata(w.w.wdata);
    iregs0->i_dport_addr(wb_ireg_dport_addr);
    iregs0->i_dport_ena(dbg.ireg_ena);
    iregs0->i_dport_write(dbg.ireg_write);
    iregs0->i_dport_wdata(dbg.core_wdata);
    iregs0->o_dport_rdata(ireg.dport_rdata);

    iregs0->o_ra(ireg.ra);   // Return address

    csr0 = new CsrRegs("csr0", hartid);
    csr0->i_clk(i_clk);
    csr0->i_nrst(i_nrst);
    csr0->i_mret(w.e.mret);
    csr0->i_uret(w.e.uret);
    csr0->i_addr(w.e.csr_addr);
    csr0->i_wena(w.e.csr_wena);
    csr0->i_wdata(w.e.csr_wdata);
    csr0->o_rdata(csr.rdata);
    csr0->i_e_pre_valid(w.e.pre_valid);
    csr0->i_ex_pc(w.e.npc);
    csr0->i_ex_npc(w.e.ex_npc);
    csr0->i_ex_data_addr(i_resp_data_addr);
    csr0->i_ex_data_load_fault(i_resp_data_load_fault);
    csr0->i_ex_data_store_fault(i_resp_data_store_fault);
    csr0->i_ex_ctrl_load_fault(w.f.load_fault);
    csr0->i_ex_illegal_instr(w.e.ex_illegal_instr);
    csr0->i_ex_unalign_store(w.e.ex_unalign_store);
    csr0->i_ex_unalign_load(w.e.ex_unalign_load);
    csr0->i_ex_breakpoint(w.e.ex_breakpoint);
    csr0->i_ex_ecall(w.e.ex_ecall);
    csr0->i_irq_external(i_ext_irq);
    csr0->o_trap_valid(csr.trap_valid);
    csr0->o_trap_pc(csr.trap_pc);
    csr0->i_break_mode(dbg.break_mode);
    csr0->o_break_event(csr.break_event);
    csr0->i_dport_ena(dbg.csr_ena);
    csr0->i_dport_write(dbg.csr_write);
    csr0->i_dport_addr(dbg.core_addr);
    csr0->i_dport_wdata(dbg.core_wdata);
    csr0->o_dport_rdata(csr.dport_rdata);

    dbg0 = new DbgPort("dbg0");
    dbg0->i_clk(i_clk);
    dbg0->i_nrst(i_nrst);
    dbg0->i_dport_valid(i_dport_valid);
    dbg0->i_dport_write(i_dport_write);
    dbg0->i_dport_region(i_dport_region);
    dbg0->i_dport_addr(i_dport_addr);
    dbg0->i_dport_wdata(i_dport_wdata);
    dbg0->o_dport_ready(o_dport_ready);
    dbg0->o_dport_rdata(o_dport_rdata);
    dbg0->o_core_addr(dbg.core_addr);
    dbg0->o_core_wdata(dbg.core_wdata);
    dbg0->o_csr_ena(dbg.csr_ena);
    dbg0->o_csr_write(dbg.csr_write);
    dbg0->i_csr_rdata(csr.dport_rdata);
    dbg0->o_ireg_ena(dbg.ireg_ena);
    dbg0->o_ireg_write(dbg.ireg_write);
    dbg0->o_npc_write(dbg.npc_write);
    dbg0->i_ireg_rdata(ireg.dport_rdata);
    dbg0->i_pc(w.e.pc);
    dbg0->i_npc(w.e.npc);
    dbg0->i_e_call(w.e.call);
    dbg0->i_e_ret(w.e.ret);
    dbg0->i_e_valid(w.e.valid);
    dbg0->i_m_valid(w.m.valid);
    dbg0->o_clock_cnt(dbg.clock_cnt);
    dbg0->o_executed_cnt(dbg.executed_cnt);
    dbg0->o_halt(dbg.halt);
    dbg0->i_ebreak(csr.break_event);
    dbg0->o_break_mode(dbg.break_mode);
    dbg0->o_br_fetch_valid(dbg.br_fetch_valid);
    dbg0->o_br_address_fetch(dbg.br_address_fetch);
    dbg0->o_br_instr_fetch(dbg.br_instr_fetch);
    dbg0->i_istate(i_istate);
    dbg0->i_dstate(i_dstate);
    dbg0->i_cstate(i_cstate);
    dbg0->i_instr_buf(w.f.instr_buf);

    reg_dbg = 0;
    mem_dbg = 0;
};

Processor::~Processor() {
    delete fetch0;
    delete dec0;
    delete exec0;
    delete mem0;
    delete predic0;
    delete iregs0;
    delete csr0;
    delete dbg0;
    if (reg_dbg) {
        reg_dbg->close();
        delete reg_dbg;
    }
    if (mem_dbg) {
        mem_dbg->close();
        delete mem_dbg;
    }
}

void Processor::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, dbg.clock_cnt, "top/dbg_clock_cnt");
        sc_trace(o_vcd, dbg.executed_cnt, "top/dbg_executed_cnt");
    }
    predic0->generateVCD(i_vcd, o_vcd);
    csr0->generateVCD(i_vcd, o_vcd);
    dbg0->generateVCD(i_vcd, o_vcd);
    dec0->generateVCD(i_vcd, o_vcd);
    exec0->generateVCD(i_vcd, o_vcd);
    fetch0->generateVCD(i_vcd, o_vcd);
    mem0->generateVCD(i_vcd, o_vcd);
    iregs0->generateVCD(i_vcd, o_vcd);
}

void Processor::comb() {
    w_fetch_pipeline_hold = w.e.pipeline_hold | w.m.pipeline_hold | dbg.halt;
    w_any_pipeline_hold = w.f.pipeline_hold | w.e.pipeline_hold 
                        | w.m.pipeline_hold | dbg.halt;
    w_exec_pipeline_hold = w.f.pipeline_hold | w.m.pipeline_hold | dbg.halt;

    wb_ireg_dport_addr = dbg.core_addr.read()(4, 0);
    wb_exec_dport_npc = dbg.core_wdata.read()(BUS_ADDR_WIDTH-1, 0);

    o_req_ctrl_valid = w.f.imem_req_valid;
    o_req_ctrl_addr = w.f.imem_req_addr;
    if (generate_ref_) {
        o_time = dbg.executed_cnt;
    } else {
        o_time = dbg.clock_cnt;
    }
}

void Processor::generateRef(bool v) {
    generate_ref_ = v;
    if (generate_ref_) {
        reg_dbg = new ofstream("river_sysc_regs.log");
        mem_dbg = new ofstream("river_sysc_mem.log");
        mem_dbg_write_flag = false;
    }
}

void Processor::negedge_dbg_print() {
    if (!generate_ref_) {
        return;
    }
    int sz;
    if (w.m.valid.read()) {
        uint64_t line_cnt = dbg.executed_cnt.read() + 1;
        sz = RISCV_sprintf(tstr, sizeof(tstr), "%8" RV_PRI64 "d [%08x]: ",
            line_cnt,
            w.m.pc.read().to_int());
        uint64_t prev_val = iregs0->r.mem[w.w.waddr.read().to_int()].to_int64();
        uint64_t cur_val = w.w.wdata.read().to_int64();
        if (w.w.waddr.read() == 0 || prev_val == cur_val) {
            // not writing
            sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz, "%s", "-\n");
        } else {
            sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz, 
                   "%3s <= %016" RV_PRI64 "x\n", 
                   IREGS_NAMES[w.w.waddr.read().to_int()], cur_val);
        }

        (*reg_dbg) << tstr;
        reg_dbg->flush();
    }
    // Memory access debug:
    if (i_resp_data_valid.read()) {
        sz = RISCV_sprintf(tstr, sizeof(tstr), "%08x: [%08x] ",
                        w.m.pc.read().to_uint(),
                        i_resp_data_addr.read().to_uint());
        if (mem_dbg_write_flag) {
            sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz, 
                "<= %016" RV_PRI64 "x\n", 
                dbg_mem_write_value & dbg_mem_value_mask);
        } else {
            sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz, 
                "=> %016" RV_PRI64 "x\n", 
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


}  // namespace debugger

