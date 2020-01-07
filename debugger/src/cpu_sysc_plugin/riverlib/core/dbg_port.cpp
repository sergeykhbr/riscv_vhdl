/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "dbg_port.h"

namespace debugger {

DbgPort::DbgPort(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_dport_valid("i_dport_valid"),
    i_dport_write("i_dport_write"),
    i_dport_region("i_dport_region"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_ready("o_dport_ready"),
    o_dport_rdata("o_dport_rdata"),
    o_csr_addr("o_csr_addr"),
    o_reg_addr("o_reg_addr"),
    o_core_wdata("o_core_wdata"),
    o_csr_ena("o_csr_ena"),
    o_csr_write("o_csr_write"),
    i_csr_rdata("i_csr_rdata"),
    o_ireg_ena("o_ireg_ena"),
    o_ireg_write("o_ireg_write"),
    o_npc_write("o_npc_write"),
    i_ireg_rdata("i_ireg_rdata"),
    i_pc("i_pc"),
    i_npc("i_npc"),
    i_e_next_ready("i_e_next_ready"),
    i_e_valid("i_e_valid"),
    i_e_call("i_e_call"),
    i_e_ret("i_e_ret"),
    o_clock_cnt("o_clock_cnt"),
    o_executed_cnt("o_executed_cnt"),
    o_halt("o_halt"),
    i_ebreak("i_ebreak"),
    o_break_mode("o_break_mode"),
    o_br_fetch_valid("o_br_fetch_valid"),
    o_br_address_fetch("o_br_address_fetch"),
    o_br_instr_fetch("o_br_instr_fetch"),
    o_flush_address("o_flush_address"),
    o_flush_valid("o_flush_valid"),
    i_istate("i_istate"),
    i_dstate("i_dstate"),
    i_cstate("i_cstate") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dport_valid;
    sensitive << i_dport_write;
    sensitive << i_dport_region;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << i_ireg_rdata;
    sensitive << i_csr_rdata;
    sensitive << i_pc;
    sensitive << i_npc;
    sensitive << i_e_call;
    sensitive << i_e_ret;
    sensitive << i_e_next_ready;
    sensitive << i_e_valid;
    sensitive << i_ebreak;
    sensitive << i_istate;
    sensitive << i_dstate;
    sensitive << i_cstate;
    sensitive << r.ready;
    sensitive << r.rdata;
    sensitive << r.halt;
    sensitive << r.breakpoint;
    sensitive << r.stepping_mode;
    sensitive << r.clock_cnt;
    sensitive << r.executed_cnt;
    sensitive << r.stepping_mode_cnt;
    sensitive << r.trap_on_break;
    sensitive << r.flush_address;
    sensitive << r.flush_valid;
    sensitive << wb_stack_rdata;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    if (CFG_STACK_TRACE_BUF_SIZE != 0) {
        trbuf0 = new StackTraceBuffer("trbuf0");
        trbuf0->i_clk(i_clk);
        trbuf0->i_raddr(wb_stack_raddr);
        trbuf0->o_rdata(wb_stack_rdata);
        trbuf0->i_we(w_stack_we);
        trbuf0->i_waddr(wb_stack_waddr);
        trbuf0->i_wdata(wb_stack_wdata);
    }
};

DbgPort::~DbgPort() {
    if (CFG_STACK_TRACE_BUF_SIZE != 0) {
        delete trbuf0;
    }
}

void DbgPort::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_dport_valid, i_dport_valid.name());
        sc_trace(o_vcd, i_dport_write, i_dport_write.name());
        sc_trace(o_vcd, i_dport_region, i_dport_region.name());
        sc_trace(o_vcd, i_dport_addr, i_dport_addr.name());
        sc_trace(o_vcd, i_dport_wdata, i_dport_wdata.name());
        sc_trace(o_vcd, o_dport_ready, o_dport_ready.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());
        sc_trace(o_vcd, o_csr_addr, o_csr_addr.name());
        sc_trace(o_vcd, o_reg_addr, o_reg_addr.name());
        sc_trace(o_vcd, o_core_wdata, o_core_wdata.name());
        sc_trace(o_vcd, o_csr_ena, o_csr_ena.name());
        sc_trace(o_vcd, o_csr_write, o_csr_write.name());
        sc_trace(o_vcd, i_csr_rdata, i_csr_rdata.name());
        sc_trace(o_vcd, o_ireg_ena, o_ireg_ena.name());
        sc_trace(o_vcd, o_ireg_write, o_ireg_write.name());
        sc_trace(o_vcd, o_npc_write, o_npc_write.name());
        sc_trace(o_vcd, i_ireg_rdata, i_ireg_rdata.name());
        sc_trace(o_vcd, i_pc, i_pc.name());
        sc_trace(o_vcd, i_npc, i_npc.name());
        sc_trace(o_vcd, i_e_next_ready, i_e_next_ready.name());
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_e_call, i_e_call.name());
        sc_trace(o_vcd, i_e_ret, i_e_ret.name());
        sc_trace(o_vcd, o_clock_cnt, o_clock_cnt.name());
        sc_trace(o_vcd, o_executed_cnt, o_executed_cnt.name());
        sc_trace(o_vcd, o_halt, o_halt.name());
        sc_trace(o_vcd, i_ebreak, i_ebreak.name());
        sc_trace(o_vcd, o_break_mode, o_break_mode.name());
        sc_trace(o_vcd, o_br_fetch_valid, o_br_fetch_valid.name());
        sc_trace(o_vcd, o_br_address_fetch, o_br_address_fetch.name());
        sc_trace(o_vcd, o_br_instr_fetch, o_br_instr_fetch.name());
        sc_trace(o_vcd, o_flush_address, o_flush_address.name());
        sc_trace(o_vcd, o_flush_valid, o_flush_valid.name());
        sc_trace(o_vcd, i_istate, i_istate.name());
        sc_trace(o_vcd, i_dstate, i_dstate.name());
        sc_trace(o_vcd, i_cstate, i_cstate.name());

        std::string pn(name());
        sc_trace(o_vcd, r.clock_cnt, pn + ".r_clock_cnt");
        sc_trace(o_vcd, r.stepping_mode_cnt, pn + ".r_stepping_mode_cnt");
        sc_trace(o_vcd, r.stepping_mode, pn + ".r_stepping_mode");
        sc_trace(o_vcd, r.breakpoint, pn + ".r_breakpoint");
    }
    if (CFG_STACK_TRACE_BUF_SIZE != 0) {
        trbuf0->generateVCD(i_vcd, o_vcd);
    }
}

void DbgPort::comb() {
    sc_uint<6> wb_o_reg_addr;
    sc_uint<12> wb_o_csr_addr;
    sc_uint<RISCV_ARCH> wb_o_core_wdata;
    sc_uint<64> wb_rdata;
    sc_uint<12> wb_idx;
    sc_uint<64> wb_o_rdata;
    bool w_o_csr_ena;
    bool w_o_csr_write;
    bool w_o_ireg_ena;
    bool w_o_ireg_write;
    bool w_o_npc_write;
    bool w_cur_halt;

    v = r;

    wb_o_reg_addr = 0;
    wb_o_csr_addr = 0;
    wb_o_core_wdata = 0;
    wb_rdata = 0;
    wb_o_rdata = 0;
    wb_idx = i_dport_addr.read();
    w_o_csr_ena = 0;
    w_o_csr_write = 0;
    w_o_ireg_ena = 0;
    w_o_ireg_write = 0;
    w_o_npc_write = 0;
    v.br_fetch_valid = 0;
    v.flush_valid = 0;
    v.rd_trbuf_ena = 0;
    wb_stack_raddr = 0;
    w_stack_we = 0;
    wb_stack_waddr = 0;
    wb_stack_wdata = 0;

    v.ready = i_dport_valid.read();

    w_cur_halt = 0;
    if (i_e_next_ready.read()) {
        if (r.stepping_mode_cnt.read() != 0) {
            v.stepping_mode_cnt = r.stepping_mode_cnt.read() - 1;
            if (r.stepping_mode_cnt.read() == 1) {
                v.halt = 1;
                w_cur_halt = 1;
                v.stepping_mode = 0;
            }
        }
    }

    if (r.halt == 0) {
        v.clock_cnt = r.clock_cnt.read() + 1;
    }
    if (i_e_valid.read()) {
        v.executed_cnt = r.executed_cnt.read() + 1;
    }
    if (i_ebreak.read()) {
        v.breakpoint = 1;
        if (!r.trap_on_break) {
            v.halt = 1;
        }
    }

    if (CFG_STACK_TRACE_BUF_SIZE != 0) {
        if (i_e_call.read() && 
            r.stack_trace_cnt.read() != (CFG_STACK_TRACE_BUF_SIZE - 1)) {
            w_stack_we = 1;
            wb_stack_waddr = r.stack_trace_cnt.read();
            wb_stack_wdata = (i_npc, i_pc);
            v.stack_trace_cnt = r.stack_trace_cnt.read() + 1;
        } else if (i_e_ret.read() && r.stack_trace_cnt.read() != 0) {
            v.stack_trace_cnt = r.stack_trace_cnt.read() - 1;
        }
    }

    if (i_dport_valid.read()) {
        switch (i_dport_region.read()) {
        case 0:
            w_o_csr_ena = 1;
            wb_o_csr_addr = i_dport_addr;
            wb_rdata = i_csr_rdata;
            if (i_dport_write.read()) {
                w_o_csr_write = 1;
                wb_o_core_wdata = i_dport_wdata;
            }
            break;
        case 1:
            if (wb_idx < 32) {
                w_o_ireg_ena = 1;
                wb_o_reg_addr = (0, i_dport_addr.read()(4, 0));
                wb_rdata = i_ireg_rdata;
                if (i_dport_write.read()) {
                    w_o_ireg_write = 1;
                    wb_o_core_wdata = i_dport_wdata;
                }
            } else if (wb_idx == 32) {
                /** Read only register */
                wb_rdata = i_pc;
            } else if (wb_idx == 33) {
                wb_rdata = i_npc;
                if (i_dport_write.read()) {
                    w_o_npc_write = 1;
                    wb_o_core_wdata = i_dport_wdata;
                }
            } else if (wb_idx == 34) {
                wb_rdata = r.stack_trace_cnt;
                if (i_dport_write.read()) {
                    v.stack_trace_cnt = i_dport_wdata.read()(4, 0);
                }
            } else if (wb_idx >= 64 && wb_idx < 96) {
                w_o_ireg_ena = 1;
                wb_o_reg_addr = (1, i_dport_addr.read()(4, 0));
                wb_rdata = i_ireg_rdata;
                if (i_dport_write.read()) {
                    w_o_ireg_write = 1;
                    wb_o_core_wdata = i_dport_wdata;
                }
            } else if (wb_idx >= 128 
                && wb_idx < (128 + 2 * CFG_STACK_TRACE_BUF_SIZE)) {
                    v.rd_trbuf_ena = 1;
                    v.rd_trbuf_addr0 = wb_idx & 0x1;
                    wb_stack_raddr = (wb_idx - 128) / 2;
            }
            break;
        case 2:
            switch (wb_idx) {
            case 0:
                wb_rdata[0] = r.halt;
                wb_rdata[2] = r.breakpoint;
                wb_rdata(35, 32) = i_istate.read();
                wb_rdata(39, 36) = i_dstate.read();
                wb_rdata(41, 40) = i_cstate.read();
                if (i_dport_write.read()) {
                    v.halt = i_dport_wdata.read()[0];
                    v.stepping_mode = i_dport_wdata.read()[1];
                    if (i_dport_wdata.read()[1]) {
                        v.stepping_mode_cnt = r.stepping_mode_steps;
                    }
                }
                break;
            case 1:
                wb_rdata = r.stepping_mode_steps;
                if (i_dport_write.read()) {
                    v.stepping_mode_steps = i_dport_wdata.read();
                }
                break;
            case 2:
                wb_rdata = r.clock_cnt;
                break;
            case 3:
                wb_rdata = r.executed_cnt;
                break;
            case 4:
                /** Trap on instruction:
                    *      0 = Halt pipeline on ECALL instruction
                    *      1 = Generate trap on ECALL instruction
                    */
                wb_rdata[0] = r.trap_on_break;
                if (i_dport_write.read()) {
                    v.trap_on_break = i_dport_wdata.read()[0];
                }
                break;
            case 5:
                // todo: add hardware breakpoint
                break;
            case 6:
                // todo: remove hardware breakpoint
                break;
            case 7:
                wb_rdata(BUS_ADDR_WIDTH-1, 0) = r.br_address_fetch;
                if (i_dport_write.read()) {
                    v.br_address_fetch = i_dport_wdata.read()(BUS_ADDR_WIDTH-1, 0);
                }
                break;
            case 8:
                wb_rdata(31, 0) = r.br_instr_fetch;
                if (i_dport_write.read()) {
                    v.br_fetch_valid = 1;
                    v.breakpoint = 0;
                    v.br_instr_fetch = i_dport_wdata.read()(31, 0);
                }
                break;
            case 9:
                wb_rdata(BUS_ADDR_WIDTH-1, 0) = r.flush_address;
                if (i_dport_write.read()) {
                    v.flush_valid = 1;
                    v.flush_address =
                        i_dport_wdata.read()(BUS_ADDR_WIDTH-1, 0);
                }
                break;
            default:;
            }
            break;
        default:;
        }
    }
    v.rdata = wb_rdata;
    if (r.rd_trbuf_ena.read()) {
        if (r.rd_trbuf_addr0.read() == 0) {
            wb_o_rdata = wb_stack_rdata.read()(BUS_ADDR_WIDTH-1, 0);
        } else {
            wb_o_rdata = wb_stack_rdata.read()(2*BUS_ADDR_WIDTH-1,
                                               BUS_ADDR_WIDTH);
        }
    } else {
        wb_o_rdata = r.rdata;
    }

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_csr_addr = wb_o_csr_addr;
    o_reg_addr = wb_o_reg_addr;
    o_core_wdata = wb_o_core_wdata;
    o_csr_ena = w_o_csr_ena;
    o_csr_write = w_o_csr_write;
    o_ireg_ena = w_o_ireg_ena;
    o_ireg_write = w_o_ireg_write;
    o_npc_write = w_o_npc_write;
    o_clock_cnt = r.clock_cnt;
    o_executed_cnt = r.executed_cnt;
    o_halt = r.halt | w_cur_halt;
    o_break_mode = r.trap_on_break;
    o_br_fetch_valid = r.br_fetch_valid;
    o_br_address_fetch = r.br_address_fetch;
    o_br_instr_fetch = r.br_instr_fetch;
    o_flush_address = r.flush_address;
    o_flush_valid = r.flush_valid;

    o_dport_ready = r.ready;
    o_dport_rdata = wb_o_rdata;
}

void DbgPort::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

