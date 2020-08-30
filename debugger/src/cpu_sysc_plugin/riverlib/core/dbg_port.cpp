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
    i_dport_region("i_dport_region"),
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
    i_e_ret("i_e_ret"),
    o_progbuf_ena("o_progbuf_ena"),
    o_progbuf_pc("o_progbuf_pc"),
    o_progbuf_data("o_progbuf_data"),
    o_br_fetch_valid("o_br_fetch_valid"),
    o_br_address_fetch("o_br_address_fetch"),
    o_br_instr_fetch("o_br_instr_fetch"),
    o_flush_address("o_flush_address"),
    o_flush_valid("o_flush_valid") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dport_req_valid;
    sensitive << i_dport_write;
    sensitive << i_dport_region;
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
    sensitive << r.dport_region;
    sensitive << r.dport_addr;
    sensitive << r.dport_wdata;
    sensitive << r.dport_rdata;
    sensitive << r.dstate;
    sensitive << r.rdata;
    sensitive << r.trap_on_break;
    sensitive << r.flush_address;
    sensitive << r.flush_valid;
    sensitive << r.progbuf_ena;
    sensitive << r.progbuf_data;
    sensitive << r.progbuf_data_out;
    sensitive << r.progbuf_data_pc;
    sensitive << r.progbuf_data_npc;
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
        sc_trace(o_vcd, i_dport_region, i_dport_region.name());
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
        sc_trace(o_vcd, o_br_fetch_valid, o_br_fetch_valid.name());
        sc_trace(o_vcd, o_br_address_fetch, o_br_address_fetch.name());
        sc_trace(o_vcd, o_br_instr_fetch, o_br_instr_fetch.name());
        sc_trace(o_vcd, o_progbuf_ena, o_progbuf_ena.name());
        sc_trace(o_vcd, o_progbuf_pc, o_progbuf_pc.name());
        sc_trace(o_vcd, o_progbuf_data, o_progbuf_data.name());
        sc_trace(o_vcd, o_flush_address, o_flush_address.name());
        sc_trace(o_vcd, o_flush_valid, o_flush_valid.name());
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
    int tidx;

    v = r;

    wb_o_csr_addr = 0;
    wb_o_reg_addr = 0;
    wb_o_core_wdata = 0;
    wb_idx = i_dport_addr.read();
    w_o_csr_ena = 0;
    w_o_csr_write = 0;
    w_o_ireg_ena = 0;
    w_o_ireg_write = 0;
    v.br_fetch_valid = 0;
    v.flush_valid = 0;
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
            v.dport_region = i_dport_region;
            v.dport_addr = i_dport_addr;
            v.dport_wdata = i_dport_wdata;
            if (i_dport_region.read() == 0x0) {
                v.dstate = csr_region;
            } else if (i_dport_region.read() == 0x1) {
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
            } else if (i_dport_region.read() == 0x2) {
                v.dstate = control;
            } else {
                v.dstate = wait_to_accept;
            }
        }
        break;
    case csr_region:
        w_o_csr_ena = 1;
        wb_o_csr_addr = r.dport_addr;
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
    case control:
        switch (r.dport_addr.read()) {
        case 4:
            //! Trap on instruction:
            //!      0 = Halt pipeline on ECALL instruction
            //!      1 = Generate trap on ECALL instruction
            vrdata = 0;
            vrdata[0] = r.trap_on_break;
            if (r.dport_write.read() == 1) {
                v.trap_on_break = r.dport_wdata.read()[0];
            }
            break;
        case 7:
            vrdata = 0;
            vrdata(CFG_CPU_ADDR_BITS-1, 0) = r.br_address_fetch;
            if (r.dport_write.read() == 1) {
                v.br_address_fetch = r.dport_wdata.read()(CFG_CPU_ADDR_BITS-1, 0);
            }
            break;
        case 8:
            vrdata = 0;
            vrdata(31, 0) = r.br_instr_fetch;
            if (r.dport_write == 1) {
                v.br_fetch_valid = 1;
                v.br_instr_fetch = r.dport_wdata.read()(31, 0);
            }
            break;
        case 9:
            vrdata = 0;
            vrdata(CFG_CPU_ADDR_BITS-1, 0) = r.flush_address;
            if (r.dport_write == 1) {
                v.flush_valid = 1;
                v.flush_address = r.dport_wdata.read()(CFG_CPU_ADDR_BITS-1, 0);
            }
            break;
        case 11:
            // Read or write access starts execution from progbuf
            if (r.dport_write == 1) {
                v.progbuf_ena = 1;
                v.progbuf_data_out = r.progbuf_data.read()(31,0).to_uint();
                v.progbuf_data_pc = 0;
                if (r.progbuf_data.read()(1,0).to_uint() == 0x3) {
                    v.progbuf_data_npc = 2;
                } else {
                    v.progbuf_data_npc = 1;
                }
                //v.halt = 0;
            }
            break;
        default:
            vrdata = 0;
            if (r.dport_addr.read() >= 0x010 && r.dport_addr.read() < 0x020) {
                tidx = r.dport_addr.read() - 0x010;
                vrdata(31, 0) =
                    r.progbuf_data.read()(32*tidx+31, 32*tidx).to_uint();
                if (r.dport_write == 1) {
                    sc_biguint<16*32> t2 = r.progbuf_data;
                    t2(32*tidx+31, 32*tidx) = r.dport_wdata.read()(31,0);
                    v.progbuf_data = t2;
                }
            }
            break;
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
    o_progbuf_ena = r.progbuf_ena;
    o_progbuf_pc = r.progbuf_data_pc.read() << 1;
    o_progbuf_data = r.progbuf_data_out;
    o_br_fetch_valid = r.br_fetch_valid;
    o_br_address_fetch = r.br_address_fetch;
    o_br_instr_fetch = r.br_instr_fetch;
    o_flush_address = r.flush_address;
    o_flush_valid = r.flush_valid;

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

