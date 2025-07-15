// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 

#include "csr.h"
#include "api_core.h"

namespace debugger {

CsrRegs::CsrRegs(sc_module_name name,
                 bool async_reset,
                 uint32_t hartid)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_sp("i_sp"),
    i_req_valid("i_req_valid"),
    o_req_ready("o_req_ready"),
    i_req_type("i_req_type"),
    i_req_addr("i_req_addr"),
    i_req_data("i_req_data"),
    o_resp_valid("o_resp_valid"),
    i_resp_ready("i_resp_ready"),
    o_resp_data("o_resp_data"),
    o_resp_exception("o_resp_exception"),
    i_e_halted("i_e_halted"),
    i_e_pc("i_e_pc"),
    i_e_instr("i_e_instr"),
    i_irq_pending("i_irq_pending"),
    o_irq_pending("o_irq_pending"),
    o_wakeup("o_wakeup"),
    o_stack_overflow("o_stack_overflow"),
    o_stack_underflow("o_stack_underflow"),
    i_f_flush_ready("i_f_flush_ready"),
    i_e_valid("i_e_valid"),
    i_m_memop_ready("i_m_memop_ready"),
    i_m_idle("i_m_idle"),
    i_flushd_end("i_flushd_end"),
    i_mtimer("i_mtimer"),
    o_executed_cnt("o_executed_cnt"),
    o_step("o_step"),
    i_dbg_progbuf_ena("i_dbg_progbuf_ena"),
    o_progbuf_end("o_progbuf_end"),
    o_progbuf_error("o_progbuf_error"),
    o_flushd_valid("o_flushd_valid"),
    o_flushi_valid("o_flushi_valid"),
    o_flushmmu_valid("o_flushmmu_valid"),
    o_flushpipeline_valid("o_flushpipeline_valid"),
    o_flush_addr("o_flush_addr"),
    o_pmp_ena("o_pmp_ena"),
    o_pmp_we("o_pmp_we"),
    o_pmp_region("o_pmp_region"),
    o_pmp_start_addr("o_pmp_start_addr"),
    o_pmp_end_addr("o_pmp_end_addr"),
    o_pmp_flags("o_pmp_flags"),
    o_mmu_ena("o_mmu_ena"),
    o_mmu_sv39("o_mmu_sv39"),
    o_mmu_sv48("o_mmu_sv48"),
    o_mmu_ppn("o_mmu_ppn"),
    o_mprv("o_mprv"),
    o_mxr("o_mxr"),
    o_sum("o_sum") {

    async_reset_ = async_reset;
    hartid_ = hartid;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_sp;
    sensitive << i_req_valid;
    sensitive << i_req_type;
    sensitive << i_req_addr;
    sensitive << i_req_data;
    sensitive << i_resp_ready;
    sensitive << i_e_halted;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_irq_pending;
    sensitive << i_f_flush_ready;
    sensitive << i_e_valid;
    sensitive << i_m_memop_ready;
    sensitive << i_m_idle;
    sensitive << i_flushd_end;
    sensitive << i_mtimer;
    sensitive << i_dbg_progbuf_ena;
    for (int i = 0; i < 4; i++) {
        sensitive << r.xmode[i].xepc;
        sensitive << r.xmode[i].xpp;
        sensitive << r.xmode[i].xpie;
        sensitive << r.xmode[i].xie;
        sensitive << r.xmode[i].xsie;
        sensitive << r.xmode[i].xtie;
        sensitive << r.xmode[i].xeie;
        sensitive << r.xmode[i].xtvec_off;
        sensitive << r.xmode[i].xtvec_mode;
        sensitive << r.xmode[i].xtval;
        sensitive << r.xmode[i].xcause_irq;
        sensitive << r.xmode[i].xcause_code;
        sensitive << r.xmode[i].xscratch;
        sensitive << r.xmode[i].xcounteren;
    }
    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
        sensitive << r.pmp[i].cfg;
        sensitive << r.pmp[i].addr;
        sensitive << r.pmp[i].mask;
    }
    sensitive << r.state;
    sensitive << r.fencestate;
    sensitive << r.irq_pending;
    sensitive << r.cmd_type;
    sensitive << r.cmd_addr;
    sensitive << r.cmd_data;
    sensitive << r.cmd_exception;
    sensitive << r.progbuf_end;
    sensitive << r.progbuf_err;
    sensitive << r.mip_ssip;
    sensitive << r.mip_stip;
    sensitive << r.mip_seip;
    sensitive << r.medeleg;
    sensitive << r.mideleg;
    sensitive << r.mcountinhibit;
    sensitive << r.mstackovr;
    sensitive << r.mstackund;
    sensitive << r.mmu_ena;
    sensitive << r.satp_ppn;
    sensitive << r.satp_sv39;
    sensitive << r.satp_sv48;
    sensitive << r.mode;
    sensitive << r.mprv;
    sensitive << r.mxr;
    sensitive << r.sum;
    sensitive << r.tvm;
    sensitive << r.ex_fpu_invalidop;
    sensitive << r.ex_fpu_divbyzero;
    sensitive << r.ex_fpu_overflow;
    sensitive << r.ex_fpu_underflow;
    sensitive << r.ex_fpu_inexact;
    sensitive << r.trap_addr;
    sensitive << r.mcycle_cnt;
    sensitive << r.minstret_cnt;
    sensitive << r.dscratch0;
    sensitive << r.dscratch1;
    sensitive << r.dpc;
    sensitive << r.halt_cause;
    sensitive << r.dcsr_ebreakm;
    sensitive << r.dcsr_stopcount;
    sensitive << r.dcsr_stoptimer;
    sensitive << r.dcsr_step;
    sensitive << r.dcsr_stepie;
    sensitive << r.stepping_mode_cnt;
    sensitive << r.ins_per_step;
    sensitive << r.pmp_upd_ena;
    sensitive << r.pmp_upd_cnt;
    sensitive << r.pmp_ena;
    sensitive << r.pmp_we;
    sensitive << r.pmp_region;
    sensitive << r.pmp_start_addr;
    sensitive << r.pmp_end_addr;
    sensitive << r.pmp_flags;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void CsrRegs::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_sp, i_sp.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, i_req_type, i_req_type.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, i_req_data, i_req_data.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, i_resp_ready, i_resp_ready.name());
        sc_trace(o_vcd, o_resp_data, o_resp_data.name());
        sc_trace(o_vcd, o_resp_exception, o_resp_exception.name());
        sc_trace(o_vcd, i_e_halted, i_e_halted.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_instr, i_e_instr.name());
        sc_trace(o_vcd, i_irq_pending, i_irq_pending.name());
        sc_trace(o_vcd, o_irq_pending, o_irq_pending.name());
        sc_trace(o_vcd, o_wakeup, o_wakeup.name());
        sc_trace(o_vcd, o_stack_overflow, o_stack_overflow.name());
        sc_trace(o_vcd, o_stack_underflow, o_stack_underflow.name());
        sc_trace(o_vcd, i_f_flush_ready, i_f_flush_ready.name());
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_m_memop_ready, i_m_memop_ready.name());
        sc_trace(o_vcd, i_m_idle, i_m_idle.name());
        sc_trace(o_vcd, i_flushd_end, i_flushd_end.name());
        sc_trace(o_vcd, i_mtimer, i_mtimer.name());
        sc_trace(o_vcd, o_executed_cnt, o_executed_cnt.name());
        sc_trace(o_vcd, o_step, o_step.name());
        sc_trace(o_vcd, i_dbg_progbuf_ena, i_dbg_progbuf_ena.name());
        sc_trace(o_vcd, o_progbuf_end, o_progbuf_end.name());
        sc_trace(o_vcd, o_progbuf_error, o_progbuf_error.name());
        sc_trace(o_vcd, o_flushd_valid, o_flushd_valid.name());
        sc_trace(o_vcd, o_flushi_valid, o_flushi_valid.name());
        sc_trace(o_vcd, o_flushmmu_valid, o_flushmmu_valid.name());
        sc_trace(o_vcd, o_flushpipeline_valid, o_flushpipeline_valid.name());
        sc_trace(o_vcd, o_flush_addr, o_flush_addr.name());
        sc_trace(o_vcd, o_pmp_ena, o_pmp_ena.name());
        sc_trace(o_vcd, o_pmp_we, o_pmp_we.name());
        sc_trace(o_vcd, o_pmp_region, o_pmp_region.name());
        sc_trace(o_vcd, o_pmp_start_addr, o_pmp_start_addr.name());
        sc_trace(o_vcd, o_pmp_end_addr, o_pmp_end_addr.name());
        sc_trace(o_vcd, o_pmp_flags, o_pmp_flags.name());
        sc_trace(o_vcd, o_mmu_ena, o_mmu_ena.name());
        sc_trace(o_vcd, o_mmu_sv39, o_mmu_sv39.name());
        sc_trace(o_vcd, o_mmu_sv48, o_mmu_sv48.name());
        sc_trace(o_vcd, o_mmu_ppn, o_mmu_ppn.name());
        sc_trace(o_vcd, o_mprv, o_mprv.name());
        sc_trace(o_vcd, o_mxr, o_mxr.name());
        sc_trace(o_vcd, o_sum, o_sum.name());
        for (int i = 0; i < 4; i++) {
            sc_trace(o_vcd, r.xmode[i].xepc, pn + ".r.xmode(" + std::to_string(i) + ").xepc");
            sc_trace(o_vcd, r.xmode[i].xpp, pn + ".r.xmode(" + std::to_string(i) + ").xpp");
            sc_trace(o_vcd, r.xmode[i].xpie, pn + ".r.xmode(" + std::to_string(i) + ").xpie");
            sc_trace(o_vcd, r.xmode[i].xie, pn + ".r.xmode(" + std::to_string(i) + ").xie");
            sc_trace(o_vcd, r.xmode[i].xsie, pn + ".r.xmode(" + std::to_string(i) + ").xsie");
            sc_trace(o_vcd, r.xmode[i].xtie, pn + ".r.xmode(" + std::to_string(i) + ").xtie");
            sc_trace(o_vcd, r.xmode[i].xeie, pn + ".r.xmode(" + std::to_string(i) + ").xeie");
            sc_trace(o_vcd, r.xmode[i].xtvec_off, pn + ".r.xmode(" + std::to_string(i) + ").xtvec_off");
            sc_trace(o_vcd, r.xmode[i].xtvec_mode, pn + ".r.xmode(" + std::to_string(i) + ").xtvec_mode");
            sc_trace(o_vcd, r.xmode[i].xtval, pn + ".r.xmode(" + std::to_string(i) + ").xtval");
            sc_trace(o_vcd, r.xmode[i].xcause_irq, pn + ".r.xmode(" + std::to_string(i) + ").xcause_irq");
            sc_trace(o_vcd, r.xmode[i].xcause_code, pn + ".r.xmode(" + std::to_string(i) + ").xcause_code");
            sc_trace(o_vcd, r.xmode[i].xscratch, pn + ".r.xmode(" + std::to_string(i) + ").xscratch");
            sc_trace(o_vcd, r.xmode[i].xcounteren, pn + ".r.xmode(" + std::to_string(i) + ").xcounteren");
        }
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
            sc_trace(o_vcd, r.pmp[i].cfg, pn + ".r.pmp(" + std::to_string(i) + ").cfg");
            sc_trace(o_vcd, r.pmp[i].addr, pn + ".r.pmp(" + std::to_string(i) + ").addr");
            sc_trace(o_vcd, r.pmp[i].mask, pn + ".r.pmp(" + std::to_string(i) + ").mask");
        }
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.fencestate, pn + ".r.fencestate");
        sc_trace(o_vcd, r.irq_pending, pn + ".r.irq_pending");
        sc_trace(o_vcd, r.cmd_type, pn + ".r.cmd_type");
        sc_trace(o_vcd, r.cmd_addr, pn + ".r.cmd_addr");
        sc_trace(o_vcd, r.cmd_data, pn + ".r.cmd_data");
        sc_trace(o_vcd, r.cmd_exception, pn + ".r.cmd_exception");
        sc_trace(o_vcd, r.progbuf_end, pn + ".r.progbuf_end");
        sc_trace(o_vcd, r.progbuf_err, pn + ".r.progbuf_err");
        sc_trace(o_vcd, r.mip_ssip, pn + ".r.mip_ssip");
        sc_trace(o_vcd, r.mip_stip, pn + ".r.mip_stip");
        sc_trace(o_vcd, r.mip_seip, pn + ".r.mip_seip");
        sc_trace(o_vcd, r.medeleg, pn + ".r.medeleg");
        sc_trace(o_vcd, r.mideleg, pn + ".r.mideleg");
        sc_trace(o_vcd, r.mcountinhibit, pn + ".r.mcountinhibit");
        sc_trace(o_vcd, r.mstackovr, pn + ".r.mstackovr");
        sc_trace(o_vcd, r.mstackund, pn + ".r.mstackund");
        sc_trace(o_vcd, r.mmu_ena, pn + ".r.mmu_ena");
        sc_trace(o_vcd, r.satp_ppn, pn + ".r.satp_ppn");
        sc_trace(o_vcd, r.satp_sv39, pn + ".r.satp_sv39");
        sc_trace(o_vcd, r.satp_sv48, pn + ".r.satp_sv48");
        sc_trace(o_vcd, r.mode, pn + ".r.mode");
        sc_trace(o_vcd, r.mprv, pn + ".r.mprv");
        sc_trace(o_vcd, r.mxr, pn + ".r.mxr");
        sc_trace(o_vcd, r.sum, pn + ".r.sum");
        sc_trace(o_vcd, r.tvm, pn + ".r.tvm");
        sc_trace(o_vcd, r.ex_fpu_invalidop, pn + ".r.ex_fpu_invalidop");
        sc_trace(o_vcd, r.ex_fpu_divbyzero, pn + ".r.ex_fpu_divbyzero");
        sc_trace(o_vcd, r.ex_fpu_overflow, pn + ".r.ex_fpu_overflow");
        sc_trace(o_vcd, r.ex_fpu_underflow, pn + ".r.ex_fpu_underflow");
        sc_trace(o_vcd, r.ex_fpu_inexact, pn + ".r.ex_fpu_inexact");
        sc_trace(o_vcd, r.trap_addr, pn + ".r.trap_addr");
        sc_trace(o_vcd, r.mcycle_cnt, pn + ".r.mcycle_cnt");
        sc_trace(o_vcd, r.minstret_cnt, pn + ".r.minstret_cnt");
        sc_trace(o_vcd, r.dscratch0, pn + ".r.dscratch0");
        sc_trace(o_vcd, r.dscratch1, pn + ".r.dscratch1");
        sc_trace(o_vcd, r.dpc, pn + ".r.dpc");
        sc_trace(o_vcd, r.halt_cause, pn + ".r.halt_cause");
        sc_trace(o_vcd, r.dcsr_ebreakm, pn + ".r.dcsr_ebreakm");
        sc_trace(o_vcd, r.dcsr_stopcount, pn + ".r.dcsr_stopcount");
        sc_trace(o_vcd, r.dcsr_stoptimer, pn + ".r.dcsr_stoptimer");
        sc_trace(o_vcd, r.dcsr_step, pn + ".r.dcsr_step");
        sc_trace(o_vcd, r.dcsr_stepie, pn + ".r.dcsr_stepie");
        sc_trace(o_vcd, r.stepping_mode_cnt, pn + ".r.stepping_mode_cnt");
        sc_trace(o_vcd, r.ins_per_step, pn + ".r.ins_per_step");
        sc_trace(o_vcd, r.pmp_upd_ena, pn + ".r.pmp_upd_ena");
        sc_trace(o_vcd, r.pmp_upd_cnt, pn + ".r.pmp_upd_cnt");
        sc_trace(o_vcd, r.pmp_ena, pn + ".r.pmp_ena");
        sc_trace(o_vcd, r.pmp_we, pn + ".r.pmp_we");
        sc_trace(o_vcd, r.pmp_region, pn + ".r.pmp_region");
        sc_trace(o_vcd, r.pmp_start_addr, pn + ".r.pmp_start_addr");
        sc_trace(o_vcd, r.pmp_end_addr, pn + ".r.pmp_end_addr");
        sc_trace(o_vcd, r.pmp_flags, pn + ".r.pmp_flags");
    }

}

void CsrRegs::comb() {
    int iM;
    int iH;
    int iS;
    int iU;
    sc_uint<2> vb_xpp;
    sc_uint<IRQ_TOTAL> vb_pending;
    sc_uint<IRQ_TOTAL> vb_irq_ena;
    sc_uint<64> vb_e_emux;                                  // Exception request from executor to process
    sc_uint<16> vb_e_imux;                                  // Interrupt request from executor to process
    sc_uint<5> wb_trap_cause;
    sc_uint<RISCV_ARCH> vb_xtval;                           // trap value
    bool w_mstackovr;
    bool w_mstackund;
    bool v_csr_rena;
    bool v_csr_wena;
    bool v_csr_trapreturn;
    sc_uint<RISCV_ARCH> vb_rdata;
    bool v_req_halt;
    bool v_req_resume;
    bool v_req_progbuf;
    bool v_req_ready;
    bool v_resp_valid;
    bool v_medeleg_ena;
    bool v_mideleg_ena;
    sc_uint<RISCV_ARCH> vb_xtvec_off_ideleg;                // 4-bytes aligned
    sc_uint<RISCV_ARCH> vb_xtvec_off_edeleg;                // 4-bytes aligned
    bool v_flushd;
    bool v_flushi;
    bool v_flushmmu;
    bool v_flushpipeline;
    sc_uint<CFG_PMP_TBL_SIZE> vb_pmp_upd_ena;
    sc_uint<RISCV_ARCH> vb_pmp_napot_mask;
    bool v_napot_shift;
    int t_pmpdataidx;
    int t_pmpcfgidx;

    for (int i = 0; i < 4; i++) {
        v.xmode[i].xepc = r.xmode[i].xepc.read();
        v.xmode[i].xpp = r.xmode[i].xpp.read();
        v.xmode[i].xpie = r.xmode[i].xpie.read();
        v.xmode[i].xie = r.xmode[i].xie.read();
        v.xmode[i].xsie = r.xmode[i].xsie.read();
        v.xmode[i].xtie = r.xmode[i].xtie.read();
        v.xmode[i].xeie = r.xmode[i].xeie.read();
        v.xmode[i].xtvec_off = r.xmode[i].xtvec_off.read();
        v.xmode[i].xtvec_mode = r.xmode[i].xtvec_mode.read();
        v.xmode[i].xtval = r.xmode[i].xtval.read();
        v.xmode[i].xcause_irq = r.xmode[i].xcause_irq.read();
        v.xmode[i].xcause_code = r.xmode[i].xcause_code.read();
        v.xmode[i].xscratch = r.xmode[i].xscratch.read();
        v.xmode[i].xcounteren = r.xmode[i].xcounteren.read();
    }
    for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
        v.pmp[i].cfg = r.pmp[i].cfg.read();
        v.pmp[i].addr = r.pmp[i].addr.read();
        v.pmp[i].mask = r.pmp[i].mask.read();
    }
    v.state = r.state.read();
    v.fencestate = r.fencestate.read();
    v.irq_pending = r.irq_pending.read();
    v.cmd_type = r.cmd_type.read();
    v.cmd_addr = r.cmd_addr.read();
    v.cmd_data = r.cmd_data.read();
    v.cmd_exception = r.cmd_exception.read();
    v.progbuf_end = r.progbuf_end.read();
    v.progbuf_err = r.progbuf_err.read();
    v.mip_ssip = r.mip_ssip.read();
    v.mip_stip = r.mip_stip.read();
    v.mip_seip = r.mip_seip.read();
    v.medeleg = r.medeleg.read();
    v.mideleg = r.mideleg.read();
    v.mcountinhibit = r.mcountinhibit.read();
    v.mstackovr = r.mstackovr.read();
    v.mstackund = r.mstackund.read();
    v.mmu_ena = r.mmu_ena.read();
    v.satp_ppn = r.satp_ppn.read();
    v.satp_sv39 = r.satp_sv39.read();
    v.satp_sv48 = r.satp_sv48.read();
    v.mode = r.mode.read();
    v.mprv = r.mprv.read();
    v.mxr = r.mxr.read();
    v.sum = r.sum.read();
    v.tvm = r.tvm.read();
    v.ex_fpu_invalidop = r.ex_fpu_invalidop.read();
    v.ex_fpu_divbyzero = r.ex_fpu_divbyzero.read();
    v.ex_fpu_overflow = r.ex_fpu_overflow.read();
    v.ex_fpu_underflow = r.ex_fpu_underflow.read();
    v.ex_fpu_inexact = r.ex_fpu_inexact.read();
    v.trap_addr = r.trap_addr.read();
    v.mcycle_cnt = r.mcycle_cnt.read();
    v.minstret_cnt = r.minstret_cnt.read();
    v.dscratch0 = r.dscratch0.read();
    v.dscratch1 = r.dscratch1.read();
    v.dpc = r.dpc.read();
    v.halt_cause = r.halt_cause.read();
    v.dcsr_ebreakm = r.dcsr_ebreakm.read();
    v.dcsr_stopcount = r.dcsr_stopcount.read();
    v.dcsr_stoptimer = r.dcsr_stoptimer.read();
    v.dcsr_step = r.dcsr_step.read();
    v.dcsr_stepie = r.dcsr_stepie.read();
    v.stepping_mode_cnt = r.stepping_mode_cnt.read();
    v.ins_per_step = r.ins_per_step.read();
    v.pmp_upd_ena = r.pmp_upd_ena.read();
    v.pmp_upd_cnt = r.pmp_upd_cnt.read();
    v.pmp_ena = r.pmp_ena.read();
    v.pmp_we = r.pmp_we.read();
    v.pmp_region = r.pmp_region.read();
    v.pmp_start_addr = r.pmp_start_addr.read();
    v.pmp_end_addr = r.pmp_end_addr.read();
    v.pmp_flags = r.pmp_flags.read();
    iM = PRV_M;
    iH = PRV_H;
    iS = PRV_S;
    iU = PRV_U;
    vb_xpp = 0;
    vb_pending = 0;
    vb_irq_ena = 0;
    vb_e_emux = 0;
    vb_e_imux = 0;
    wb_trap_cause = 0;
    vb_xtval = 0;
    w_mstackovr = 0;
    w_mstackund = 0;
    v_csr_rena = 0;
    v_csr_wena = 0;
    v_csr_trapreturn = 0;
    vb_rdata = 0;
    v_req_halt = 0;
    v_req_resume = 0;
    v_req_progbuf = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    v_medeleg_ena = 0;
    v_mideleg_ena = 0;
    vb_xtvec_off_ideleg = 0;
    vb_xtvec_off_edeleg = 0;
    v_flushd = 0;
    v_flushi = 0;
    v_flushmmu = 0;
    v_flushpipeline = 0;
    vb_pmp_upd_ena = 0;
    vb_pmp_napot_mask = 0;
    v_napot_shift = 0;
    t_pmpdataidx = 0;
    t_pmpcfgidx = 0;

    vb_xpp = r.xmode[r.mode.read().to_int()].xpp.read();
    vb_pmp_upd_ena = r.pmp_upd_ena.read();
    t_pmpdataidx = (r.cmd_addr.read().to_int() - 0x00000000000003B0);
    t_pmpcfgidx = (8 * r.cmd_addr.read()(3, 1).to_int());
    vb_pmp_napot_mask = 0x0000000000000007;

    vb_xtvec_off_edeleg = r.xmode[iM].xtvec_off.read();
    if ((r.mode.read() <= PRV_S) && (r.medeleg.read()[r.cmd_addr.read()(4, 0).to_int()] == 1)) {
        // Exception delegation to S-mode
        v_medeleg_ena = 1;
        vb_xtvec_off_edeleg = r.xmode[iS].xtvec_off.read();
    }

    vb_xtvec_off_ideleg = r.xmode[iM].xtvec_off.read();
    if ((r.mode.read() <= PRV_S) && (r.mideleg.read()[r.cmd_addr.read()(3, 0).to_int()] == 1)) {
        // Interrupt delegation to S-mode
        v_mideleg_ena = 1;
        vb_xtvec_off_ideleg = r.xmode[iS].xtvec_off.read();
    }

    switch (r.state.read()) {
    case State_Idle:
        v.progbuf_end = 0;
        v.progbuf_err = 0;
        v_req_ready = 1;
        if (i_req_valid.read() == 1) {
            v.cmd_type = i_req_type.read();
            v.cmd_addr = i_req_addr.read();
            v.cmd_data = i_req_data.read();
            v.cmd_exception = 0;
            if (i_req_type.read()[CsrReq_ExceptionBit] == 1) {
                v.state = State_Exception;
                if (i_req_addr.read() == EXCEPTION_CallFromXMode) {
                    v.cmd_addr = (i_req_addr.read() + r.mode.read());
                }
            } else if (i_req_type.read()[CsrReq_BreakpointBit] == 1) {
                v.state = State_Breakpoint;
            } else if (i_req_type.read()[CsrReq_HaltBit] == 1) {
                v.state = State_Halt;
            } else if (i_req_type.read()[CsrReq_ResumeBit] == 1) {
                v.state = State_Resume;
            } else if (i_req_type.read()[CsrReq_InterruptBit] == 1) {
                v.state = State_Interrupt;
            } else if (i_req_type.read()[CsrReq_TrapReturnBit] == 1) {
                v.state = State_TrapReturn;
            } else if (i_req_type.read()[CsrReq_WfiBit] == 1) {
                v.state = State_Wfi;
            } else if (i_req_type.read()[CsrReq_FenceBit] == 1) {
                v.state = State_Fence;
                if (i_req_addr.read()[0] == 1) {
                    // FENCE
                    v.fencestate = Fence_DataBarrier;
                } else if (i_req_addr.read()[1] == 1) {
                    // FENCE.I
                    v_flushmmu = 1;
                    v.fencestate = Fence_DataFlush;
                } else if ((i_req_addr.read()[2] == 1)
                            && (!((r.tvm.read() == 1) && (r.mode.read()[1] == 0)))) {
                    // FENCE.VMA: is illegal in S-mode when TVM bit=1
                    v_flushmmu = 1;
                    v.fencestate = Fence_End;
                } else {
                    // Illegal fence
                    v.state = State_Response;
                    v.cmd_exception = 1;
                }
            } else {
                v.state = State_RW;
            }
        }
        break;
    case State_Exception:
        v.state = State_Response;
        vb_e_emux[r.cmd_addr.read()(4, 0).to_int()] = 1;
        vb_xtval = r.cmd_data.read();
        wb_trap_cause = r.cmd_addr.read()(4, 0);
        v.cmd_data = vb_xtvec_off_edeleg;
        if (i_dbg_progbuf_ena.read() == 1) {
            v.progbuf_err = 1;
            v.progbuf_end = 1;
            v.cmd_exception = 1;
        }
        break;
    case State_Breakpoint:                                  // software breakpoint
        v.state = State_Response;
        if (i_dbg_progbuf_ena.read() == 1) {
            // do not modify halt cause in debug mode
            v.progbuf_end = 1;
            v.cmd_data = ~0ull;                             // signal to executor to switch into Debug Mode and halt
        } else if (r.dcsr_ebreakm.read() == 1) {
            v.halt_cause = HALT_CAUSE_EBREAK;
            v.dpc = r.cmd_data.read();
            v.cmd_data = ~0ull;                             // signal to executor to switch into Debug Mode and halt
        } else {
            vb_e_emux[EXCEPTION_Breakpoint] = 1;
            wb_trap_cause = r.cmd_addr.read()(4, 0);
            vb_xtval = i_e_pc.read();
            v.cmd_data = vb_xtvec_off_edeleg;               // Jump to exception handler
        }
        break;
    case State_Halt:
        v.state = State_Response;
        v.halt_cause = r.cmd_addr.read()(2, 0);             // Halt Request or Step done
        v.dpc = i_e_pc.read();
        break;
    case State_Resume:
        v.state = State_Response;
        if (i_dbg_progbuf_ena.read() == 1) {
            v.cmd_data = 0;
        } else {
            v.cmd_data = (0, r.dpc.read());
        }
        break;
    case State_Interrupt:
        v.state = State_Response;
        vb_e_imux[r.cmd_addr.read()(3, 0).to_int()] = 1;
        wb_trap_cause = r.cmd_addr.read()(4, 0);
        v.cmd_data = vb_xtvec_off_ideleg;
        if (r.xmode[r.mode.read().to_int()].xtvec_mode.read() == 1) {
            // vectorized
            v.cmd_data = (vb_xtvec_off_ideleg + (wb_trap_cause << 2));
        }
        break;
    case State_TrapReturn:
        v.state = State_Response;
        v_csr_trapreturn = 1;
        v.cmd_data = (0, r.xmode[r.mode.read().to_int()].xepc.read());
        break;
    case State_RW:
        v.state = State_Response;
        // csr[9:8] encode the loweset priviledge level that can access to CSR
        // csr[11:10] register is read/write (00, 01 or 10) or read-only (11)
        if (r.mode.read() < r.cmd_addr.read()(9, 8)) {
            // Not enough priv to access this register
            v.cmd_exception = 1;
        } else {
            v_csr_rena = r.cmd_type.read()[CsrReq_ReadBit];
            v_csr_wena = r.cmd_type.read()[CsrReq_WriteBit];
            if (r.cmd_addr.read()(11, 4) == 0x3A) {         // pmpcfgx
                v.state = State_WaitPmp;
            }
        }
        // All operation into CSR implemented through the Read-Modify-Write
        // and we cannot generate exception on write access into read-only regs
        // So do not check bits csr[11:10], otherwise always will be the exception.
        break;
    case State_Wfi:
        v.state = State_Response;
        v.cmd_data = 0;                                     // no error, valid for all mdoes
        break;
    case State_Fence:
        if (r.fencestate.read() == Fence_End) {
            v.cmd_data = 0;
            v.state = State_Response;
            v.fencestate = Fence_None;
        }
        break;
    case State_WaitPmp:
        if (r.pmp_upd_ena.read().or_reduce() == 0) {
            v.state = State_Response;
        }
        break;
    case State_Response:
        v_resp_valid = 1;
        if (i_resp_ready.read() == 1) {
            v.state = State_Idle;
        }
        break;
    default:
        break;
    }

    // Caches flushing state machine
    switch (r.fencestate.read()) {
    case Fence_None:
        break;
    case Fence_DataBarrier:
        if (i_m_idle.read() == 1) {
            v.fencestate = Fence_End;
        }
        break;
    case Fence_DataFlush:
        v_flushd = 1;
        if (i_m_memop_ready.read() == 1) {
            v.fencestate = Fence_WaitDataFlushEnd;
        }
        break;
    case Fence_WaitDataFlushEnd:
        if (i_flushd_end.read() == 1) {
            v.fencestate = Fence_FlushInstr;
        }
        break;
    case Fence_FlushInstr:
        v_flushi = 1;
        if (i_f_flush_ready.read() == 1) {
            v.fencestate = Fence_End;
        }
        break;
    case Fence_End:
        v_flushpipeline = 1;
        break;
    default:
        break;
    }

    // CSR registers. Priviledge Arch. V20211203, page 9(19):
    //     CSR[11:10] indicate whether the register is read/write (00, 01, or 10) or read-only (11)
    //     CSR[9:8] encode the lowest privilege level that can access the CSR
    if (r.cmd_addr.read() == 0x000) {                       // ustatus: [URW] User status register
    } else if (r.cmd_addr.read() == 0x004) {                // uie: [URW] User interrupt-enable register
    } else if (r.cmd_addr.read() == 0x005) {                // ustatus: [URW] User trap handler base address
    } else if (r.cmd_addr.read() == 0x040) {                // uscratch: [URW] Scratch register for user trap handlers
    } else if (r.cmd_addr.read() == 0x041) {                // uepc: [URW] User exception program counter
        vb_rdata = r.xmode[iU].xepc.read();
        if (v_csr_wena) {
            v.xmode[iU].xepc = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x042) {                // ucause: [URW] User trap cause
    } else if (r.cmd_addr.read() == 0x043) {                // utval: [URW] User bad address or instruction
    } else if (r.cmd_addr.read() == 0x044) {                // uip: [URW] User interrupt pending
    } else if (r.cmd_addr.read() == 0x001) {                // fflags: [URW] Floating-Point Accrued Exceptions
        vb_rdata[0] = r.ex_fpu_inexact.read();
        vb_rdata[1] = r.ex_fpu_underflow.read();
        vb_rdata[2] = r.ex_fpu_overflow.read();
        vb_rdata[3] = r.ex_fpu_divbyzero.read();
        vb_rdata[4] = r.ex_fpu_invalidop.read();
    } else if (r.cmd_addr.read() == 0x002) {                // fflags: [URW] Floating-Point Dynamic Rounding Mode
        if (CFG_HW_FPU_ENABLE) {
            vb_rdata(2, 0) = 4;                             // Round mode: round to Nearest (RMM)
        }
    } else if (r.cmd_addr.read() == 0x003) {                // fcsr: [URW] Floating-Point Control and Status Register (frm + fflags)
        vb_rdata[0] = r.ex_fpu_inexact.read();
        vb_rdata[1] = r.ex_fpu_underflow.read();
        vb_rdata[2] = r.ex_fpu_overflow.read();
        vb_rdata[3] = r.ex_fpu_divbyzero.read();
        vb_rdata[4] = r.ex_fpu_invalidop.read();
        if (CFG_HW_FPU_ENABLE) {
            vb_rdata(7, 5) = 4;                             // Round mode: round to Nearest (RMM)
        }
    } else if (r.cmd_addr.read() == 0xC00) {                // cycle: [URO] User Cycle counter for RDCYCLE pseudo-instruction
        if ((r.mode.read() == PRV_U)
                && ((r.xmode[iM].xcounteren.read()[0] == 0)
                        || (r.xmode[iS].xcounteren.read()[0] == 0))) {
            // Available only if all more prv. bit CY are set
            v.cmd_exception = 1;
        } else if ((r.mode.read() == PRV_S)
                    && (r.xmode[iM].xcounteren.read()[0] == 0)) {
            // Available only if bit CY is set
            v.cmd_exception = 1;
        } else {
            vb_rdata = r.mcycle_cnt.read();                 // Read-only shadows of mcycle
        }
    } else if (r.cmd_addr.read() == 0xC01) {                // time: [URO] User Timer for RDTIME pseudo-instruction
        if ((r.mode.read() == PRV_U)
                && ((r.xmode[iM].xcounteren.read()[1] == 0)
                        || (r.xmode[iS].xcounteren.read()[1] == 0))) {
            // Available only if all more prv. bit TM are set
            v.cmd_exception = 1;
        } else if ((r.mode.read() == PRV_S)
                    && (r.xmode[iM].xcounteren.read()[1] == 0)) {
            // Available only if bit TM is set
            v.cmd_exception = 1;
        } else {
            vb_rdata = i_mtimer.read();
        }
    } else if (r.cmd_addr.read() == 0xC03) {                // insret: [URO] User Instructions-retired counter for RDINSTRET pseudo-instruction
        if ((r.mode.read() == PRV_U)
                && ((r.xmode[iM].xcounteren.read()[2] == 0)
                        || (r.xmode[iS].xcounteren.read()[2] == 0))) {
            // Available only if all more prv. bit IR are set
            v.cmd_exception = 1;
        } else if ((r.mode.read() == PRV_S)
                    && (r.xmode[iM].xcounteren.read()[2] == 0)) {
            // Available only if bit IR is set
            v.cmd_exception = 1;
        } else {
            vb_rdata = r.minstret_cnt.read();               // Read-only shadow of minstret
        }
    } else if (r.cmd_addr.read() == 0x100) {                // sstatus: [SRW] Supervisor status register
        // [0] WPRI
        vb_rdata[1] = r.xmode[iS].xie.read();
        // [4:2] WPRI
        vb_rdata[5] = r.xmode[iS].xpie.read();
        // [6] UBE: Endianess: 0=little-endian; 1=big-endian. Instruction fetch is always little endian
        // [7] WPRI
        vb_rdata[8] = r.xmode[iS].xpp.read()[0];            // SPP can have onle 0 or 1 values, so 1 bit only
        // [10:9] VS
        // [12:11] WPRI
        if (CFG_HW_FPU_ENABLE) {
            vb_rdata(14, 13) = 0x1;                         // FS field: Initial state
        }
        // [16:15] XS
        // [17] WPRI
        // [18] SUM
        // [19] MXR
        // [31:20] WPRI
        vb_rdata(33, 32) = 0x2;                             // UXL: User is 64-bits
        // [62:34] WPRI
        // [63] SD. Read-only bit that summize FS, VS or XS fields
        if (v_csr_wena == 1) {
            v.xmode[iS].xie = r.cmd_data.read()[1];
            v.xmode[iS].xpie = r.cmd_data.read()[5];
            v.xmode[iS].xpp = (0, r.cmd_data.read()[8]);
        }
    } else if (r.cmd_addr.read() == 0x104) {                // sie: [SRW] Supervisor interrupt-enable register
        vb_rdata[(IRQ_SSIP - 1)] = r.xmode[iS].xsie.read();
        vb_rdata[(IRQ_STIP - 1)] = r.xmode[iS].xtie.read();
        vb_rdata[(IRQ_SEIP - 1)] = r.xmode[iS].xeie.read();
        if (v_csr_wena) {
            // Write only supported interrupts
            v.xmode[iS].xsie = r.cmd_data.read()[IRQ_SSIP];
            v.xmode[iS].xtie = r.cmd_data.read()[IRQ_STIP];
            v.xmode[iS].xeie = r.cmd_data.read()[IRQ_SEIP];
        }
    } else if (r.cmd_addr.read() == 0x105) {                // stvec: [SRW] Supervisor trap handler base address
        vb_rdata = r.xmode[iS].xtvec_off.read();
        vb_rdata(1, 0) = r.xmode[iS].xtvec_mode.read();
        if (v_csr_wena == 1) {
            v.xmode[iS].xtvec_off = (r.cmd_data.read()((RISCV_ARCH - 1), 2) << 2);
            v.xmode[iS].xtvec_mode = r.cmd_data.read()(1, 0);
        }
    } else if (r.cmd_addr.read() == 0x106) {                // scounteren: [SRW] Supervisor counter enable
        vb_rdata = r.xmode[iS].xcounteren.read();
        if (v_csr_wena == 1) {
            v.xmode[iS].xcounteren = r.cmd_data.read()(15, 0);
        }
    } else if (r.cmd_addr.read() == 0x10A) {                // senvcfg: [SRW] Supervisor environment configuration register
    } else if (r.cmd_addr.read() == 0x140) {                // sscratch: [SRW] Supervisor register for supervisor trap handlers
        vb_rdata = r.xmode[iS].xscratch.read();
        if (v_csr_wena) {
            v.xmode[iS].xscratch = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x141) {                // sepc: [SRW] Supervisor exception program counter
        vb_rdata = r.xmode[iS].xepc.read();
        if (v_csr_wena) {
            v.xmode[iS].xepc = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x142) {                // scause: [SRW] Supervisor trap cause
        vb_rdata[63] = r.xmode[iS].xcause_irq.read();
        vb_rdata(4, 0) = r.xmode[iS].xcause_code.read();
    } else if (r.cmd_addr.read() == 0x143) {                // stval: [SRW] Supervisor bad address or instruction
        vb_rdata = r.xmode[iS].xtval.read();
        if (v_csr_wena) {
            v.xmode[iS].xtval = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x144) {                // sip: [SRW] Supervisor interrupt pending
        vb_rdata(15, 0) = (r.irq_pending.read() & 0x0222);  // see fig 4.7. Only s-bits are visible
        if (v_csr_wena) {
            v.mip_ssip = r.cmd_data.read()[IRQ_SSIP];
            v.mip_stip = r.cmd_data.read()[IRQ_STIP];
            v.mip_seip = r.cmd_data.read()[IRQ_SEIP];
        }
    } else if (r.cmd_addr.read() == 0x180) {                // satp: [SRW] Supervisor address translation and protection
        // Writing unssoprted MODE[63:60], entire write has no effect
        //     MODE = 0 Bare. No translation or protection
        //     MODE = 9 Sv48. Page based 48-bit virtual addressing
        if ((r.tvm.read() == 1) && (r.mode.read() == PRV_S)) {
            // SATP is illegal in S-mode when TVM=1
            v.cmd_exception = 1;
        } else {
            vb_rdata(43, 0) = r.satp_ppn.read();
            if (r.satp_sv39.read() == 1) {
                vb_rdata(63, 60) = SATP_MODE_SV39;
            } else if (r.satp_sv48.read() == 1) {
                vb_rdata(63, 60) = SATP_MODE_SV48;
            }
            if (v_csr_wena == 1) {
                v.satp_ppn = r.cmd_data.read()(43, 0);
                v.satp_sv39 = 0;
                v.satp_sv48 = 0;
                if (r.cmd_data.read()(63, 60).to_uint() == SATP_MODE_SV39) {
                    v.satp_sv39 = 1;
                } else if (r.cmd_data.read()(63, 60).to_uint() == SATP_MODE_SV48) {
                    v.satp_sv48 = 1;
                }
            }
        }
    } else if (r.cmd_addr.read() == 0x5A8) {                // scontext: [SRW] Supervisor-mode context register
    } else if (r.cmd_addr.read() == 0xF11) {                // mvendorid: [MRO] Vendor ID
        vb_rdata = CFG_VENDOR_ID;
    } else if (r.cmd_addr.read() == 0xF12) {                // marchid: [MRO] Architecture ID
    } else if (r.cmd_addr.read() == 0xF13) {                // mimplementationid: [MRO] Implementation ID
        vb_rdata = CFG_IMPLEMENTATION_ID;
    } else if (r.cmd_addr.read() == 0xF14) {                // mhartid: [MRO] Hardware thread ID
        vb_rdata(63, 0) = hartid_;
    } else if (r.cmd_addr.read() == 0xF15) {                // mconfigptr: [MRO] Pointer to configuration data structure
    } else if (r.cmd_addr.read() == 0x300) {                // mstatus: [MRW] Machine mode status register
        // [0] WPRI
        vb_rdata[1] = r.xmode[iS].xie.read();
        // [2] WPRI
        vb_rdata[3] = r.xmode[iM].xie.read();
        // [4] WPRI
        vb_rdata[5] = r.xmode[iS].xpie.read();
        // [6] UBE: Endianess: 0=little-endian; 1=big-endian. Instruction fetch is always little endian
        vb_rdata[7] = r.xmode[iM].xpie.read();
        vb_rdata[8] = r.xmode[iS].xpp.read()[0];            // SPP can have onle 0 or 1 values, so 1 bit only
        // [10:9] VS
        vb_rdata(12, 11) = r.xmode[iM].xpp.read();
        if (CFG_HW_FPU_ENABLE) {
            vb_rdata(14, 13) = 0x1;                         // FS field: Initial state
        }
        // [16:15] XS
        vb_rdata[17] = r.mprv.read();
        vb_rdata[18] = r.sum.read();
        vb_rdata[19] = r.mxr.read();
        vb_rdata[20] = r.tvm.read();                        // Trap Virtual Memory
        // [21] TW
        // [22] TSR
        // [31:23] WPRI
        vb_rdata(33, 32) = 0x2;                             // UXL: User is 64-bits
        vb_rdata(35, 34) = 0x2;                             // SXL: Supervisor is 64-bits
        // [36] SBE
        // [37] MBE
        // [62:38] WPRI
        // [63] SD. Read-only bit that summize FS, VS or XS fields
        if (v_csr_wena == 1) {
            v.xmode[iS].xie = r.cmd_data.read()[1];
            v.xmode[iM].xie = r.cmd_data.read()[3];
            v.xmode[iS].xpie = r.cmd_data.read()[5];
            v.xmode[iM].xpie = r.cmd_data.read()[7];
            v.xmode[iS].xpp = (0, r.cmd_data.read()[8]);
            v.xmode[iM].xpp = r.cmd_data.read()(12, 11);
            v.mprv = r.cmd_data.read()[17];
            v.sum = r.cmd_data.read()[18];
            v.mxr = r.cmd_data.read()[19];
            v.tvm = r.cmd_data.read()[20];
        }
    } else if (r.cmd_addr.read() == 0x301) {                // misa: [MRW] ISA and extensions
        // Base[XLEN-1:XLEN-2]
        //      1 = 32
        //      2 = 64
        //      3 = 128

        vb_rdata((RISCV_ARCH - 1), (RISCV_ARCH - 2)) = 2;
        // BitCharacterDescription
        //      0  A Atomic extension
        //      1  B Tentatively reserved for Bit operations extension
        //      2  C Compressed extension
        //      3  D Double-precision Foating-point extension
        //      4  E RV32E base ISA (embedded)
        //      5  F Single-precision Foating-point extension
        //      6  G Additional standard extensions present
        //      7  H Hypervisor mode implemented
        //      8  I RV32I/64I/128I base ISA
        //      9  J Reserved
        //      10 K Reserved
        //      11 L Tentatively reserved for Decimal Floating-Point extension
        //      12 M Integer Multiply/Divide extension
        //      13 N User-level interrupts supported
        //      14 O Reserved
        //      15 P Tentatively reserved for Packed-SIMD extension
        //      16 Q Quad-precision Foating-point extension
        //      17 R Reserved
        //      18 S Supervisor mode implemented
        //      19 T Tentatively reserved for Transactional Memory extension
        //      20 U User mode implemented
        //      21 V Tentatively reserved for Vector extension
        //      22 W Reserved
        //      23 X Non-standard extensions present
        //      24 Y Reserved
        //      25 Z Reserve
        vb_rdata[0] = 1;                                    // A-extension
        vb_rdata[8] = 1;                                    // I-extension
        vb_rdata[12] = 1;                                   // M-extension
        vb_rdata[18] = 1;                                   // S-extension
        vb_rdata[20] = 1;                                   // U-extension
        vb_rdata[2] = 1;                                    // C-extension
        if (CFG_HW_FPU_ENABLE) {
            vb_rdata[3] = 1;                                // D-extension
        }
    } else if (r.cmd_addr.read() == 0x302) {                // medeleg: [MRW] Machine exception delegation
        vb_rdata = r.medeleg.read();
        if (v_csr_wena) {
            // page 31. Read-only zero for exceptions that could not be delegated, especially Call from M-mode
            v.medeleg = (r.cmd_data.read() & 0x000000000000B3FF);
        }
    } else if (r.cmd_addr.read() == 0x303) {                // mideleg: [MRW] Machine interrupt delegation
        vb_rdata = r.mideleg.read();
        if (v_csr_wena) {
            // No need to delegate machine interrupts to supervisor (but possible)
            v.mideleg = (r.cmd_data.read()((IRQ_TOTAL - 1), 0) & 0x222);
        }
    } else if (r.cmd_addr.read() == 0x304) {                // mie: [MRW] Machine interrupt enable bit
        vb_rdata[(IRQ_SSIP - 1)] = r.xmode[iS].xsie.read();
        vb_rdata[(IRQ_MSIP - 1)] = r.xmode[iM].xsie.read();
        vb_rdata[(IRQ_STIP - 1)] = r.xmode[iS].xtie.read();
        vb_rdata[(IRQ_MTIP - 1)] = r.xmode[iM].xtie.read();
        vb_rdata[(IRQ_SEIP - 1)] = r.xmode[iS].xeie.read();
        vb_rdata[(IRQ_MEIP - 1)] = r.xmode[iM].xeie.read();
        if (v_csr_wena) {
            // Write only supported interrupts
            v.xmode[iS].xsie = r.cmd_data.read()[IRQ_SSIP];
            v.xmode[iM].xsie = r.cmd_data.read()[IRQ_MSIP];
            v.xmode[iS].xtie = r.cmd_data.read()[IRQ_STIP];
            v.xmode[iM].xtie = r.cmd_data.read()[IRQ_MTIP];
            v.xmode[iS].xeie = r.cmd_data.read()[IRQ_SEIP];
            v.xmode[iM].xeie = r.cmd_data.read()[IRQ_MEIP];
        }
    } else if (r.cmd_addr.read() == 0x305) {                // mtvec: [MRW] Machine trap-handler base address
        vb_rdata = r.xmode[iM].xtvec_off.read();
        vb_rdata(1, 0) = r.xmode[iM].xtvec_mode.read();
        if (v_csr_wena == 1) {
            v.xmode[iM].xtvec_off = (r.cmd_data.read()((RISCV_ARCH - 1), 2) << 2);
            v.xmode[iM].xtvec_mode = r.cmd_data.read()(1, 0);
        }
    } else if (r.cmd_addr.read() == 0x306) {                // mcounteren: [MRW] Machine counter enable
        vb_rdata = r.xmode[iM].xcounteren.read();
        if (v_csr_wena == 1) {
            v.xmode[iM].xcounteren = r.cmd_data.read()(15, 0);
        }
    } else if (r.cmd_addr.read() == 0x340) {                // mscratch: [MRW] Machine scratch register
        vb_rdata = r.xmode[iM].xscratch.read();
        if (v_csr_wena) {
            v.xmode[iM].xscratch = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x341) {                // mepc: [MRW] Machine program counter
        vb_rdata = r.xmode[iM].xepc.read();
        if (v_csr_wena) {
            v.xmode[iM].xepc = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x342) {                // mcause: [MRW] Machine trap cause
        vb_rdata[63] = r.xmode[iM].xcause_irq.read();
        vb_rdata(4, 0) = r.xmode[iM].xcause_code.read();
    } else if (r.cmd_addr.read() == 0x343) {                // mtval: [MRW] Machine bad address or instruction
        vb_rdata = r.xmode[iM].xtval.read();
        if (v_csr_wena) {
            v.xmode[iM].xtval = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x344) {                // mip: [MRW] Machine interrupt pending
        vb_rdata((IRQ_TOTAL - 1), 0) = r.irq_pending.read();
        if (v_csr_wena) {
            v.mip_ssip = r.cmd_data.read()[IRQ_SSIP];
            v.mip_stip = r.cmd_data.read()[IRQ_STIP];
            v.mip_seip = r.cmd_data.read()[IRQ_SEIP];
        }
    } else if (r.cmd_addr.read() == 0x34A) {                // mtinst: [MRW] Machine trap instruction (transformed)
    } else if (r.cmd_addr.read() == 0x34B) {                // mtval2: [MRW] Machine bad guest physical register
    } else if (r.cmd_addr.read() == 0x30A) {                // menvcfg: [MRW] Machine environment configuration register
    } else if (r.cmd_addr.read() == 0x747) {                // mseccfg: [MRW] Machine security configuration register
    } else if (r.cmd_addr.read()(11, 4) == 0x3A) {          // pmpcfg0..63: [MRW] Physical memory protection configuration
        if (r.cmd_addr.read()[0] == 1) {
            v.cmd_exception = 1;                            // RV32 only
        } else if (t_pmpcfgidx < CFG_PMP_TBL_SIZE) {
            for (int i = 0; i < 8; i++) {
                vb_rdata((8 * i) + 8 - 1, (8 * i)) = r.pmp[(t_pmpcfgidx + i)].cfg.read();
                if ((v_csr_wena == 1)
                        && (r.pmp[(t_pmpcfgidx + i)].cfg.read()[7] == 0)) {
                    // [7] Bit L = locked cannot be modified upto reset
                    v.pmp[(t_pmpcfgidx + i)].cfg = r.cmd_data.read()((8 * i) + 8 - 1, (8 * i));
                    vb_pmp_upd_ena[(t_pmpcfgidx + i)] = 1;
                }
            }
        }
    } else if ((r.cmd_addr.read() >= 0x3B0)
                && (r.cmd_addr.read() <= 0x3EF)) {
        // pmpaddr0..63: [MRW] Physical memory protection address register
        for (int i = 0; i < (RISCV_ARCH - 2); i++) {
            if ((r.cmd_data.read()[i] == 1) && (v_napot_shift == 0)) {
                vb_pmp_napot_mask = ((vb_pmp_napot_mask << 1) | 1);
            } else {
                v_napot_shift = 1;
            }
        }
        if (t_pmpdataidx < CFG_PMP_TBL_SIZE) {
            vb_rdata((RISCV_ARCH - 3), 0) = r.pmp[t_pmpdataidx].addr.read();
            if ((v_csr_wena == 1)
                    && (r.pmp[t_pmpdataidx].cfg.read()[7] == 0)) {
                v.pmp[t_pmpdataidx].addr = (r.cmd_data.read()((RISCV_ARCH - 3), 0) << 2);
                v.pmp[t_pmpdataidx].mask = vb_pmp_napot_mask;
            }
        }
    } else if (r.cmd_addr.read() <= 0x3EF) {                // pmpaddr63: [MRW] Physical memory protection address register
    } else if (r.cmd_addr.read() == 0xB00) {                // mcycle: [MRW] Machine cycle counter
        vb_rdata = r.mcycle_cnt.read();
        if (v_csr_wena) {
            v.mcycle_cnt = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0xB02) {                // minstret: [MRW] Machine instructions-retired counter
        vb_rdata = r.minstret_cnt.read();
        if (v_csr_wena) {
            v.minstret_cnt = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x320) {                // mcountinhibit: [MRW] Machine counter-inhibit register
        vb_rdata = r.mcountinhibit.read();
        if (v_csr_wena == 1) {
            v.mcountinhibit = r.cmd_data.read()(15, 0);
        }
    } else if (r.cmd_addr.read() == 0x323) {                // mpevent3: [MRW] Machine performance-monitoring event selector
    } else if (r.cmd_addr.read() == 0x324) {                // mpevent4: [MRW] Machine performance-monitoring event selector
    } else if (r.cmd_addr.read() == 0x33F) {                // mpevent31: [MRW] Machine performance-monitoring event selector
    } else if (r.cmd_addr.read() == 0x7A0) {                // tselect: [MRW] Debug/Trace trigger register select
    } else if (r.cmd_addr.read() == 0x7A1) {                // tdata1: [MRW] First Debug/Trace trigger data register
    } else if (r.cmd_addr.read() == 0x7A2) {                // tdata2: [MRW] Second Debug/Trace trigger data register
    } else if (r.cmd_addr.read() == 0x7A3) {                // tdata3: [MRW] Third Debug/Trace trigger data register
    } else if (r.cmd_addr.read() == 0x7A8) {                // mcontext: [MRW] Machine-mode context register
    } else if (r.cmd_addr.read() == 0x7B0) {                // dcsr: [DRW] Debug control and status register
        vb_rdata(31, 28) = 4;                               // xdebugver: 4=External debug supported
        vb_rdata[15] = r.dcsr_ebreakm.read();
        vb_rdata[11] = r.dcsr_stepie.read();                // interrupt dis/ena during step
        vb_rdata[10] = r.dcsr_stopcount.read();             // don't increment any counter
        vb_rdata[9] = r.dcsr_stoptimer.read();              // don't increment timer
        vb_rdata(8, 6) = r.halt_cause.read();
        vb_rdata[2] = r.dcsr_step.read();
        vb_rdata(1, 0) = 3;                                 // prv: privilege in debug mode: 3=machine
        if (v_csr_wena == 1) {
            v.dcsr_ebreakm = r.cmd_data.read()[15];
            v.dcsr_stepie = r.cmd_data.read()[11];
            v.dcsr_stopcount = r.cmd_data.read()[10];
            v.dcsr_stoptimer = r.cmd_data.read()[9];
            v.dcsr_step = r.cmd_data.read()[2];
        }
    } else if (r.cmd_addr.read() == 0x7B1) {                // dpc: [DRW] Debug PC
        // Upon entry into debug mode DPC must contains:
        //        cause        |   Address
        // --------------------|----------------
        //  ebreak             |  Address of ebreak instruction
        //  single step        |  Address of next instruction to be executed
        //  trigger (HW BREAK) |  if timing=0, cause isntruction, if timing=1 enxt instruction
        //  halt request       |  next instruction

        if (i_e_halted.read() == 1) {
            vb_rdata = r.dpc.read();
        } else {
            // make visible current pc for the debugger even in running state
            vb_rdata = i_e_pc.read();
        }
        if (v_csr_wena == 1) {
            v.dpc = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x7B2) {                // dscratch0: [DRW] Debug scratch register 0
        vb_rdata = r.dscratch0.read();
        if (v_csr_wena == 1) {
            v.dscratch0 = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0x7B3) {                // dscratch1: [DRW] Debug scratch register 1
        vb_rdata = r.dscratch1.read();
        if (v_csr_wena) {
            v.dscratch1 = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0xBC0) {                // mstackovr: [MRW] Machine Stack Overflow
        vb_rdata = r.mstackovr.read();
        if (v_csr_wena == 1) {
            v.mstackovr = r.cmd_data.read();
        }
    } else if (r.cmd_addr.read() == 0xBC1) {                // mstackund: [MRW] Machine Stack Underflow
        vb_rdata = r.mstackund.read();
        if (v_csr_wena == 1) {
            v.mstackund = r.cmd_data.read();
        }
    } else {
        // Not implemented CSR:
        if (r.state.read() == State_RW) {
            v.cmd_exception = 1;
        }
    }

    if (v_csr_rena == 1) {
        v.cmd_data = vb_rdata;
    }


    if (v_csr_trapreturn == 1) {
        if (r.mode.read() == r.cmd_addr.read()) {
            v.mode = vb_xpp;
            v.xmode[r.mode.read().to_int()].xpie = 1;       // xPIE is set to 1 always, page 21
            v.xmode[r.mode.read().to_int()].xpp = PRV_U;    // Set to least-privildged supported mode (U), page 21
        } else {
            v.cmd_exception = 1;                            // xret not matched to current mode
        }
        if (r.xmode[r.mode.read().to_int()].xpp.read() != PRV_M) {// see page 21
            v.mprv = 0;
        }
    }

    // Check MMU:
    v.mmu_ena = 0;
    if ((r.satp_sv39.read() == 1) || (r.satp_sv48.read() == 1)) {// Sv39 and Sv48 are implemented
        if (r.mode.read()[1] == 0) {
            // S and U modes
            v.mmu_ena = 1;
            v_flushpipeline = (!r.mmu_ena.read());          // Flush pipeline on MMU turning on
        } else if ((r.mprv.read() == 1) && (vb_xpp[1] == 0)) {
            // Previous state is S or U mode
            // Instruction address-translation and protection are unaffected
            v.mmu_ena = 1;
            v_flushpipeline = (!r.mmu_ena.read());          // Flush pipeline on MMU turning on
        }
    }

    if ((r.mode.read() <= PRV_S)
            && ((vb_e_emux & r.medeleg.read()) || (vb_e_imux & r.mideleg.read()))) {
        // Trap delegated to Supervisor mode
        v.xmode[iS].xpp = r.mode.read();
        v.xmode[iS].xpie = r.xmode[r.mode.read().to_int()].xie.read();
        v.xmode[iS].xie = 0;
        v.xmode[iS].xepc = i_e_pc.read();                   // current latched instruction not executed overwritten by exception/interrupt
        v.xmode[iS].xtval = vb_xtval;                       // trap value/bad address
        v.xmode[iS].xcause_code = wb_trap_cause;
        v.xmode[iS].xcause_irq = (!vb_e_emux.or_reduce());
        v.mode = PRV_S;
    } else if ((vb_e_emux.or_reduce() == 1) || (vb_e_imux.or_reduce() == 1)) {
        // By default all exceptions and interrupts handled in M-mode (not delegations)
        v.xmode[iM].xpp = r.mode.read();
        v.xmode[iM].xpie = r.xmode[r.mode.read().to_int()].xie.read();
        v.xmode[iM].xie = 0;
        v.xmode[iM].xepc = i_e_pc.read();                   // current latched instruction not executed overwritten by exception/interrupt
        v.xmode[iM].xtval = vb_xtval;                       // trap value/bad address
        v.xmode[iM].xcause_code = wb_trap_cause;
        v.xmode[iM].xcause_irq = (!vb_e_emux.or_reduce());
        v.mode = PRV_M;
        v.mmu_ena = 0;
    }

    // Step is not enabled or interrupt enabled during stepping
    if (((!r.dcsr_step.read()) || r.dcsr_stepie.read()) == 1) {
        if (r.xmode[iM].xie.read()) {
            // ALL not-delegated interrupts
            vb_irq_ena = (~r.mideleg.read());
        }
        if (r.xmode[iS].xie.read()) {
            // Delegated to S-mode:
            vb_irq_ena = (vb_irq_ena | r.mideleg.read());
        }
    }

    // The following pending interrupt could be set in mip:
    vb_pending[IRQ_MSIP] = (i_irq_pending.read()[IRQ_MSIP] && r.xmode[iM].xsie.read());
    vb_pending[IRQ_MTIP] = (i_irq_pending.read()[IRQ_MTIP] && r.xmode[iM].xtie.read());
    vb_pending[IRQ_MEIP] = (i_irq_pending.read()[IRQ_MEIP] && r.xmode[iM].xeie.read());
    vb_pending[IRQ_SSIP] = ((i_irq_pending.read()[IRQ_SSIP] || r.mip_ssip.read()) && r.xmode[iS].xsie.read());
    vb_pending[IRQ_STIP] = ((i_irq_pending.read()[IRQ_STIP] || r.mip_stip.read()) && r.xmode[iS].xtie.read());
    vb_pending[IRQ_SEIP] = ((i_irq_pending.read()[IRQ_SEIP] || r.mip_seip.read()) && r.xmode[iS].xeie.read());
    v.irq_pending = vb_pending;

    // Transmit data to MPU
    if (r.pmp_upd_ena.read().or_reduce() == 1) {
        vb_pmp_upd_ena[r.pmp_upd_cnt.read().to_int()] = 0;
        v.pmp_upd_cnt = (r.pmp_upd_cnt.read() + 1);
        v.pmp_we = r.pmp_upd_ena.read()[r.pmp_upd_cnt.read().to_int()];
        v.pmp_region = r.pmp_upd_cnt.read();
        if (r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()(4, 3) == 0) {
            // OFF: Null region (disabled)
            v.pmp_start_addr = 0;
            v.pmp_end_addr = r.pmp[r.pmp_upd_cnt.read().to_int()].addr.read();
            v.pmp_flags = 0;
        } else if (r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()(4, 3) == 1) {
            // TOR: Top of range
            if (r.pmp_end_addr.read()[0] == 1) {
                v.pmp_start_addr = (r.pmp_end_addr.read() + 1);
            } else {
                v.pmp_start_addr = r.pmp_end_addr.read();
            }
            v.pmp_end_addr = (r.pmp[r.pmp_upd_cnt.read().to_int()].addr.read() - 1);
            v.pmp_flags = (1, r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()[7], r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()(2, 0));
        } else if (r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()(4, 3) == 2) {
            // NA4: Naturally aligned four-bytes region
            v.pmp_start_addr = r.pmp[r.pmp_upd_cnt.read().to_int()].addr.read();
            v.pmp_end_addr = (r.pmp[r.pmp_upd_cnt.read().to_int()].addr.read() | 0x3);
            v.pmp_flags = (1, r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()[7], r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()(2, 0));
        } else {
            // NAPOT: Naturally aligned power-of-two region, >=8 bytes
            v.pmp_start_addr = (r.pmp[r.pmp_upd_cnt.read().to_int()].addr.read() & (~r.pmp[r.pmp_upd_cnt.read().to_int()].mask.read()));
            v.pmp_end_addr = (r.pmp[r.pmp_upd_cnt.read().to_int()].addr.read() | r.pmp[r.pmp_upd_cnt.read().to_int()].mask.read());
            v.pmp_flags = (1, r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()[7], r.pmp[r.pmp_upd_cnt.read().to_int()].cfg.read()(2, 0));
        }
    } else {
        v.pmp_upd_cnt = 0;
        v.pmp_start_addr = 0;
        v.pmp_end_addr = 0;
        v.pmp_flags = 0;
        v.pmp_we = 0;
    }
    v.pmp_upd_ena = vb_pmp_upd_ena;
    v.pmp_ena = ((!r.mode.read()[1]) | r.mprv.read());      // S,U mode or MPRV is set

    w_mstackovr = 0;
    if ((r.mstackovr.read().or_reduce() == 1) && (i_sp.read() < r.mstackovr.read())) {
        w_mstackovr = 1;
        v.mstackovr = 0;
    }
    w_mstackund = 0;
    if ((r.mstackund.read().or_reduce() == 1) && (i_sp.read() > r.mstackund.read())) {
        w_mstackund = 1;
        v.mstackund = 0;
    }

    // if (i_fpu_valid.read()) {
    //     v.ex_fpu_invalidop = i_ex_fpu_invalidop.read();
    //     v.ex_fpu_divbyzero = i_ex_fpu_divbyzero.read();
    //     v.ex_fpu_overflow = i_ex_fpu_overflow.read();
    //     v.ex_fpu_underflow = i_ex_fpu_underflow.read();
    //     v.ex_fpu_inexact = i_ex_fpu_inexact.read();
    // }

    // stopcount: do not increment any counters including cycle and instret
    if ((i_e_halted.read() == 0) && (r.dcsr_stopcount.read() == 0) && (r.mcountinhibit.read()[0] == 0)) {
        v.mcycle_cnt = (r.mcycle_cnt.read() + 1);
    }
    if ((i_e_valid.read() == 1)
            && (r.dcsr_stopcount.read() == 0)
            && (i_dbg_progbuf_ena.read() == 0)
            && (r.mcountinhibit.read()[2] == 0)) {
        v.minstret_cnt = (r.minstret_cnt.read() + 1);
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        CsrRegs_r_reset(v);
    }

    o_req_ready = v_req_ready;
    o_resp_valid = v_resp_valid;
    o_resp_data = r.cmd_data.read();
    o_resp_exception = r.cmd_exception.read();
    o_progbuf_end = (r.progbuf_end.read() && i_resp_ready.read());
    o_progbuf_error = (r.progbuf_err.read() && i_resp_ready.read());
    o_irq_pending = (r.irq_pending.read() & vb_irq_ena);
    o_wakeup = r.irq_pending.read().or_reduce();
    o_stack_overflow = w_mstackovr;
    o_stack_underflow = w_mstackund;
    o_executed_cnt = r.minstret_cnt.read();
    o_pmp_ena = r.pmp_ena.read();                           // S,U mode or MPRV is set
    o_pmp_we = r.pmp_we.read();
    o_pmp_region = r.pmp_region.read();
    o_pmp_start_addr = r.pmp_start_addr.read();
    o_pmp_end_addr = r.pmp_end_addr.read();
    o_pmp_flags = r.pmp_flags.read();
    o_mmu_ena = r.mmu_ena.read();
    o_mmu_sv39 = r.satp_sv39.read();
    o_mmu_sv48 = r.satp_sv48.read();
    o_mmu_ppn = r.satp_ppn.read();
    o_mprv = r.mprv.read();
    o_mxr = r.mxr.read();
    o_sum = r.sum.read();
    o_step = r.dcsr_step.read();
    o_flushd_valid = v_flushd;
    o_flushi_valid = v_flushi;
    o_flushmmu_valid = v_flushmmu;
    o_flushpipeline_valid = v_flushpipeline;
    o_flush_addr = r.cmd_data.read();
}

void CsrRegs::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        CsrRegs_r_reset(r);
    } else {
        for (int i = 0; i < 4; i++) {
            r.xmode[i].xepc = v.xmode[i].xepc.read();
            r.xmode[i].xpp = v.xmode[i].xpp.read();
            r.xmode[i].xpie = v.xmode[i].xpie.read();
            r.xmode[i].xie = v.xmode[i].xie.read();
            r.xmode[i].xsie = v.xmode[i].xsie.read();
            r.xmode[i].xtie = v.xmode[i].xtie.read();
            r.xmode[i].xeie = v.xmode[i].xeie.read();
            r.xmode[i].xtvec_off = v.xmode[i].xtvec_off.read();
            r.xmode[i].xtvec_mode = v.xmode[i].xtvec_mode.read();
            r.xmode[i].xtval = v.xmode[i].xtval.read();
            r.xmode[i].xcause_irq = v.xmode[i].xcause_irq.read();
            r.xmode[i].xcause_code = v.xmode[i].xcause_code.read();
            r.xmode[i].xscratch = v.xmode[i].xscratch.read();
            r.xmode[i].xcounteren = v.xmode[i].xcounteren.read();
        }
        for (int i = 0; i < CFG_PMP_TBL_SIZE; i++) {
            r.pmp[i].cfg = v.pmp[i].cfg.read();
            r.pmp[i].addr = v.pmp[i].addr.read();
            r.pmp[i].mask = v.pmp[i].mask.read();
        }
        r.state = v.state.read();
        r.fencestate = v.fencestate.read();
        r.irq_pending = v.irq_pending.read();
        r.cmd_type = v.cmd_type.read();
        r.cmd_addr = v.cmd_addr.read();
        r.cmd_data = v.cmd_data.read();
        r.cmd_exception = v.cmd_exception.read();
        r.progbuf_end = v.progbuf_end.read();
        r.progbuf_err = v.progbuf_err.read();
        r.mip_ssip = v.mip_ssip.read();
        r.mip_stip = v.mip_stip.read();
        r.mip_seip = v.mip_seip.read();
        r.medeleg = v.medeleg.read();
        r.mideleg = v.mideleg.read();
        r.mcountinhibit = v.mcountinhibit.read();
        r.mstackovr = v.mstackovr.read();
        r.mstackund = v.mstackund.read();
        r.mmu_ena = v.mmu_ena.read();
        r.satp_ppn = v.satp_ppn.read();
        r.satp_sv39 = v.satp_sv39.read();
        r.satp_sv48 = v.satp_sv48.read();
        r.mode = v.mode.read();
        r.mprv = v.mprv.read();
        r.mxr = v.mxr.read();
        r.sum = v.sum.read();
        r.tvm = v.tvm.read();
        r.ex_fpu_invalidop = v.ex_fpu_invalidop.read();
        r.ex_fpu_divbyzero = v.ex_fpu_divbyzero.read();
        r.ex_fpu_overflow = v.ex_fpu_overflow.read();
        r.ex_fpu_underflow = v.ex_fpu_underflow.read();
        r.ex_fpu_inexact = v.ex_fpu_inexact.read();
        r.trap_addr = v.trap_addr.read();
        r.mcycle_cnt = v.mcycle_cnt.read();
        r.minstret_cnt = v.minstret_cnt.read();
        r.dscratch0 = v.dscratch0.read();
        r.dscratch1 = v.dscratch1.read();
        r.dpc = v.dpc.read();
        r.halt_cause = v.halt_cause.read();
        r.dcsr_ebreakm = v.dcsr_ebreakm.read();
        r.dcsr_stopcount = v.dcsr_stopcount.read();
        r.dcsr_stoptimer = v.dcsr_stoptimer.read();
        r.dcsr_step = v.dcsr_step.read();
        r.dcsr_stepie = v.dcsr_stepie.read();
        r.stepping_mode_cnt = v.stepping_mode_cnt.read();
        r.ins_per_step = v.ins_per_step.read();
        r.pmp_upd_ena = v.pmp_upd_ena.read();
        r.pmp_upd_cnt = v.pmp_upd_cnt.read();
        r.pmp_ena = v.pmp_ena.read();
        r.pmp_we = v.pmp_we.read();
        r.pmp_region = v.pmp_region.read();
        r.pmp_start_addr = v.pmp_start_addr.read();
        r.pmp_end_addr = v.pmp_end_addr.read();
        r.pmp_flags = v.pmp_flags.read();
    }
}

}  // namespace debugger

