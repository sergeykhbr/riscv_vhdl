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
#include <riscv-isa.h>

namespace debugger {

DbgPort::DbgPort(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_dport_req_valid("i_dport_req_valid"),
    i_dport_write("i_dport_write"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_req_ready("o_dport_req_ready"),
    i_dport_resp_ready("i_dport_resp_ready"),
    o_dport_resp_valid("o_dport_resp_valid"),
    o_dport_rdata("o_dport_rdata"),
    o_csr_addr("o_csr_addr"),
    o_reg_addr("o_reg_addr"),
    o_core_wdata("o_core_wdata"),
    o_csr_ena("o_csr_ena"),
    o_csr_write("o_csr_write"),
    i_csr_valid("i_csr_valid"),
    i_csr_rdata("i_csr_rdata"),
    o_ireg_ena("o_ireg_ena"),
    o_ireg_write("o_ireg_write"),
    i_ireg_rdata("i_ireg_rdata"),
    i_pc("i_pc"),
    i_npc("i_npc"),
    i_e_call("i_e_call"),
    i_e_ret("i_e_ret") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dport_req_valid;
    sensitive << i_dport_write;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << i_dport_resp_ready;
    sensitive << i_ireg_rdata;
    sensitive << i_csr_valid;
    sensitive << i_csr_rdata;
    sensitive << i_pc;
    sensitive << i_npc;
    sensitive << i_e_call;
    sensitive << i_e_ret;
    sensitive << r.dport_write;
    sensitive << r.dport_addr;
    sensitive << r.dport_wdata;
    sensitive << r.dport_rdata;
    sensitive << r.dstate;
    sensitive << r.rdata;
    sensitive << wb_stack_rdata;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();

    if (CFG_LOG2_STACK_TRACE_ADDR != 0) {
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
    if (CFG_LOG2_STACK_TRACE_ADDR != 0) {
        delete trbuf0;
    }
}

void DbgPort::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_dport_req_valid, i_dport_req_valid.name());
        sc_trace(o_vcd, i_dport_write, i_dport_write.name());
        sc_trace(o_vcd, i_dport_addr, i_dport_addr.name());
        sc_trace(o_vcd, i_dport_wdata, i_dport_wdata.name());
        sc_trace(o_vcd, o_dport_req_ready, o_dport_req_ready.name());
        sc_trace(o_vcd, i_dport_resp_ready, i_dport_resp_ready.name());
        sc_trace(o_vcd, o_dport_resp_valid, o_dport_resp_valid.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());
        sc_trace(o_vcd, o_csr_addr, o_csr_addr.name());
        sc_trace(o_vcd, o_reg_addr, o_reg_addr.name());
        sc_trace(o_vcd, o_core_wdata, o_core_wdata.name());
        sc_trace(o_vcd, o_csr_ena, o_csr_ena.name());
        sc_trace(o_vcd, o_csr_write, o_csr_write.name());
        sc_trace(o_vcd, i_csr_valid, i_csr_valid.name());
        sc_trace(o_vcd, i_csr_rdata, i_csr_rdata.name());
        sc_trace(o_vcd, o_ireg_ena, o_ireg_ena.name());
        sc_trace(o_vcd, o_ireg_write, o_ireg_write.name());
        sc_trace(o_vcd, i_ireg_rdata, i_ireg_rdata.name());
        sc_trace(o_vcd, i_pc, i_pc.name());
        sc_trace(o_vcd, i_npc, i_npc.name());
        sc_trace(o_vcd, i_e_call, i_e_call.name());
        sc_trace(o_vcd, i_e_ret, i_e_ret.name());
    }
    if (CFG_LOG2_STACK_TRACE_ADDR != 0) {
        trbuf0->generateVCD(i_vcd, o_vcd);
    }
}

void DbgPort::comb() {
    sc_uint<12> wb_o_csr_addr;
    sc_uint<6> wb_o_reg_addr;
    sc_uint<RISCV_ARCH> wb_o_core_wdata;
    sc_uint<12> wb_idx;
    bool w_o_csr_ena;
    bool w_o_csr_write;
    bool w_o_ireg_ena;
    bool w_o_ireg_write;
    bool v_req_ready;
    bool v_resp_valid;
    sc_uint<64> vrdata;

    v = r;

    wb_o_csr_addr = 0;
    wb_o_reg_addr = 0;
    wb_o_core_wdata = 0;
    wb_idx = i_dport_addr.read()(11, 0);
    w_o_csr_ena = 0;
    w_o_csr_write = 0;
    w_o_ireg_ena = 0;
    w_o_ireg_write = 0;
    wb_stack_raddr = 0;
    w_stack_we = 0;
    wb_stack_waddr = 0;
    wb_stack_wdata = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    vrdata = r.dport_rdata;

    if (CFG_LOG2_STACK_TRACE_ADDR != 0) {
        if (i_e_call.read() && 
            r.stack_trace_cnt.read() != (STACK_TRACE_BUF_SIZE - 1)) {
            w_stack_we = 1;
            wb_stack_waddr = r.stack_trace_cnt.read();
            wb_stack_wdata = (i_npc, i_pc);
            v.stack_trace_cnt = r.stack_trace_cnt.read() + 1;
        } else if (i_e_ret.read() && r.stack_trace_cnt.read() != 0) {
            v.stack_trace_cnt = r.stack_trace_cnt.read() - 1;
        }
    }

    switch (r.dstate.read()) {
    case idle:
        v_req_ready = 1;
        vrdata = 0;
        if (i_dport_req_valid.read() == 1) {
            v.dport_write = i_dport_write;
            v.dport_addr = i_dport_addr;
            v.dport_wdata = i_dport_wdata;
            if (i_dport_addr.read()(CFG_DPORT_ADDR_BITS-1, 12) == 0x0) {
                v.dstate = csr_region;
            } else if (i_dport_addr.read()(CFG_DPORT_ADDR_BITS-1, 12) == 0x1) {
                if (wb_idx < 64) {
                    v.dstate = reg_bank;
                } else if (wb_idx == 64) {
                    v.dstate = reg_stktr_cnt;
                } else if ((wb_idx >= 128) && (wb_idx < (128 + 2 * STACK_TRACE_BUF_SIZE))) {
                    v.dstate = reg_stktr_buf_adr;
                } else {
                    vrdata = 0;
                    v.dstate = wait_to_accept;
                }
            } else {
                v.dstate = wait_to_accept;
            }
        }
        break;
    case csr_region:
        w_o_csr_ena = 1;
        wb_o_csr_addr = r.dport_addr.read()(11,0);
        if (r.dport_write == 1) {
             w_o_csr_write   = 1;
             wb_o_core_wdata = r.dport_wdata;
        }
        if (i_csr_valid.read() == 1) {
            vrdata = i_csr_rdata.read();
            v.dstate = wait_to_accept;
        }
        break;
    case reg_bank:
        w_o_ireg_ena = 1;
        wb_o_reg_addr = r.dport_addr.read()(5,0);
        vrdata = i_ireg_rdata.read();
        if (r.dport_write.read() == 1) {
            w_o_ireg_write  = 1;
            wb_o_core_wdata = r.dport_wdata;
        }
        v.dstate = wait_to_accept;
        break;
    case reg_stktr_cnt:
        vrdata = 0;
        vrdata(CFG_LOG2_STACK_TRACE_ADDR-1, 0) = r.stack_trace_cnt;
        if (r.dport_write == 1) {
            v.stack_trace_cnt = r.dport_wdata.read();
        }
        v.dstate = wait_to_accept;
        break;
    case reg_stktr_buf_adr:
        wb_stack_raddr = r.dport_addr.read()(CFG_LOG2_STACK_TRACE_ADDR, 1);
        v.dstate = reg_stktr_buf_dat;
        break;
    case reg_stktr_buf_dat:
        if (r.dport_addr.read()[0] == 0) {
            vrdata(CFG_CPU_ADDR_BITS-1, 0) =
                     wb_stack_rdata.read()(CFG_CPU_ADDR_BITS-1, 0);
        } else {
            vrdata(CFG_CPU_ADDR_BITS-1, 0) =
                     wb_stack_rdata.read()(2*CFG_CPU_ADDR_BITS-1, CFG_CPU_ADDR_BITS);
        }
        v.dstate = wait_to_accept;
        break;
    case wait_to_accept:
        v_resp_valid = 1;
        if (i_dport_resp_ready.read() == 1) {
            v.dstate = idle;
        }
        break;
    default:;
    }

    v.dport_rdata = vrdata;

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

    o_dport_req_ready  = v_req_ready;
    o_dport_resp_valid = v_resp_valid;
    o_dport_rdata      = r.dport_rdata;
}

void DbgPort::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

