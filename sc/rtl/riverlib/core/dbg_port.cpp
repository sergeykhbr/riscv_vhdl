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

#include "dbg_port.h"
#include "api_core.h"

namespace debugger {

DbgPort::DbgPort(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_dport_req_valid("i_dport_req_valid"),
    i_dport_type("i_dport_type"),
    i_dport_addr("i_dport_addr"),
    i_dport_wdata("i_dport_wdata"),
    i_dport_size("i_dport_size"),
    o_dport_req_ready("o_dport_req_ready"),
    i_dport_resp_ready("i_dport_resp_ready"),
    o_dport_resp_valid("o_dport_resp_valid"),
    o_dport_resp_error("o_dport_resp_error"),
    o_dport_rdata("o_dport_rdata"),
    o_csr_req_valid("o_csr_req_valid"),
    i_csr_req_ready("i_csr_req_ready"),
    o_csr_req_type("o_csr_req_type"),
    o_csr_req_addr("o_csr_req_addr"),
    o_csr_req_data("o_csr_req_data"),
    i_csr_resp_valid("i_csr_resp_valid"),
    o_csr_resp_ready("o_csr_resp_ready"),
    i_csr_resp_data("i_csr_resp_data"),
    i_csr_resp_exception("i_csr_resp_exception"),
    i_progbuf("i_progbuf"),
    o_progbuf_ena("o_progbuf_ena"),
    o_progbuf_pc("o_progbuf_pc"),
    o_progbuf_instr("o_progbuf_instr"),
    i_csr_progbuf_end("i_csr_progbuf_end"),
    i_csr_progbuf_error("i_csr_progbuf_error"),
    o_ireg_addr("o_ireg_addr"),
    o_ireg_wdata("o_ireg_wdata"),
    o_ireg_ena("o_ireg_ena"),
    o_ireg_write("o_ireg_write"),
    i_ireg_rdata("i_ireg_rdata"),
    o_mem_req_valid("o_mem_req_valid"),
    i_mem_req_ready("i_mem_req_ready"),
    i_mem_req_error("i_mem_req_error"),
    o_mem_req_write("o_mem_req_write"),
    o_mem_req_addr("o_mem_req_addr"),
    o_mem_req_size("o_mem_req_size"),
    o_mem_req_wdata("o_mem_req_wdata"),
    i_mem_resp_valid("i_mem_resp_valid"),
    i_mem_resp_error("i_mem_resp_error"),
    i_mem_resp_rdata("i_mem_resp_rdata"),
    i_e_pc("i_e_pc"),
    i_e_npc("i_e_npc"),
    i_e_call("i_e_call"),
    i_e_ret("i_e_ret"),
    i_e_memop_valid("i_e_memop_valid"),
    i_m_valid("i_m_valid") {

    async_reset_ = async_reset;
    trbuf0 = 0;

    // generate
    if (CFG_LOG2_STACK_TRACE_ADDR != 0) {
        trbuf0 = new StackTraceBuffer("trbuf0");
        trbuf0->i_clk(i_clk);
        trbuf0->i_raddr(wb_stack_raddr);
        trbuf0->o_rdata(wb_stack_rdata);
        trbuf0->i_we(w_stack_we);
        trbuf0->i_waddr(wb_stack_waddr);
        trbuf0->i_wdata(wb_stack_wdata);
    }

    // endgenerate


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dport_req_valid;
    sensitive << i_dport_type;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << i_dport_size;
    sensitive << i_dport_resp_ready;
    sensitive << i_csr_req_ready;
    sensitive << i_csr_resp_valid;
    sensitive << i_csr_resp_data;
    sensitive << i_csr_resp_exception;
    sensitive << i_progbuf;
    sensitive << i_csr_progbuf_end;
    sensitive << i_csr_progbuf_error;
    sensitive << i_ireg_rdata;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_req_error;
    sensitive << i_mem_resp_valid;
    sensitive << i_mem_resp_error;
    sensitive << i_mem_resp_rdata;
    sensitive << i_e_pc;
    sensitive << i_e_npc;
    sensitive << i_e_call;
    sensitive << i_e_ret;
    sensitive << i_e_memop_valid;
    sensitive << i_m_valid;
    sensitive << wb_stack_raddr;
    sensitive << wb_stack_rdata;
    sensitive << w_stack_we;
    sensitive << wb_stack_waddr;
    sensitive << wb_stack_wdata;
    sensitive << r.dport_write;
    sensitive << r.dport_addr;
    sensitive << r.dport_wdata;
    sensitive << r.dport_rdata;
    sensitive << r.dport_size;
    sensitive << r.dstate;
    sensitive << r.rdata;
    sensitive << r.stack_trace_cnt;
    sensitive << r.req_accepted;
    sensitive << r.resp_error;
    sensitive << r.progbuf_ena;
    sensitive << r.progbuf_pc;
    sensitive << r.progbuf_instr;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

DbgPort::~DbgPort() {
    if (trbuf0) {
        delete trbuf0;
    }
}

void DbgPort::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_dport_req_valid, i_dport_req_valid.name());
        sc_trace(o_vcd, i_dport_type, i_dport_type.name());
        sc_trace(o_vcd, i_dport_addr, i_dport_addr.name());
        sc_trace(o_vcd, i_dport_wdata, i_dport_wdata.name());
        sc_trace(o_vcd, i_dport_size, i_dport_size.name());
        sc_trace(o_vcd, o_dport_req_ready, o_dport_req_ready.name());
        sc_trace(o_vcd, i_dport_resp_ready, i_dport_resp_ready.name());
        sc_trace(o_vcd, o_dport_resp_valid, o_dport_resp_valid.name());
        sc_trace(o_vcd, o_dport_resp_error, o_dport_resp_error.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());
        sc_trace(o_vcd, o_csr_req_valid, o_csr_req_valid.name());
        sc_trace(o_vcd, i_csr_req_ready, i_csr_req_ready.name());
        sc_trace(o_vcd, o_csr_req_type, o_csr_req_type.name());
        sc_trace(o_vcd, o_csr_req_addr, o_csr_req_addr.name());
        sc_trace(o_vcd, o_csr_req_data, o_csr_req_data.name());
        sc_trace(o_vcd, i_csr_resp_valid, i_csr_resp_valid.name());
        sc_trace(o_vcd, o_csr_resp_ready, o_csr_resp_ready.name());
        sc_trace(o_vcd, i_csr_resp_data, i_csr_resp_data.name());
        sc_trace(o_vcd, i_csr_resp_exception, i_csr_resp_exception.name());
        sc_trace(o_vcd, i_progbuf, i_progbuf.name());
        sc_trace(o_vcd, o_progbuf_ena, o_progbuf_ena.name());
        sc_trace(o_vcd, o_progbuf_pc, o_progbuf_pc.name());
        sc_trace(o_vcd, o_progbuf_instr, o_progbuf_instr.name());
        sc_trace(o_vcd, i_csr_progbuf_end, i_csr_progbuf_end.name());
        sc_trace(o_vcd, i_csr_progbuf_error, i_csr_progbuf_error.name());
        sc_trace(o_vcd, o_ireg_addr, o_ireg_addr.name());
        sc_trace(o_vcd, o_ireg_wdata, o_ireg_wdata.name());
        sc_trace(o_vcd, o_ireg_ena, o_ireg_ena.name());
        sc_trace(o_vcd, o_ireg_write, o_ireg_write.name());
        sc_trace(o_vcd, i_ireg_rdata, i_ireg_rdata.name());
        sc_trace(o_vcd, o_mem_req_valid, o_mem_req_valid.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, i_mem_req_error, i_mem_req_error.name());
        sc_trace(o_vcd, o_mem_req_write, o_mem_req_write.name());
        sc_trace(o_vcd, o_mem_req_addr, o_mem_req_addr.name());
        sc_trace(o_vcd, o_mem_req_size, o_mem_req_size.name());
        sc_trace(o_vcd, o_mem_req_wdata, o_mem_req_wdata.name());
        sc_trace(o_vcd, i_mem_resp_valid, i_mem_resp_valid.name());
        sc_trace(o_vcd, i_mem_resp_error, i_mem_resp_error.name());
        sc_trace(o_vcd, i_mem_resp_rdata, i_mem_resp_rdata.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_npc, i_e_npc.name());
        sc_trace(o_vcd, i_e_call, i_e_call.name());
        sc_trace(o_vcd, i_e_ret, i_e_ret.name());
        sc_trace(o_vcd, i_e_memop_valid, i_e_memop_valid.name());
        sc_trace(o_vcd, i_m_valid, i_m_valid.name());
        sc_trace(o_vcd, r.dport_write, pn + ".r_dport_write");
        sc_trace(o_vcd, r.dport_addr, pn + ".r_dport_addr");
        sc_trace(o_vcd, r.dport_wdata, pn + ".r_dport_wdata");
        sc_trace(o_vcd, r.dport_rdata, pn + ".r_dport_rdata");
        sc_trace(o_vcd, r.dport_size, pn + ".r_dport_size");
        sc_trace(o_vcd, r.dstate, pn + ".r_dstate");
        sc_trace(o_vcd, r.rdata, pn + ".r_rdata");
        sc_trace(o_vcd, r.stack_trace_cnt, pn + ".r_stack_trace_cnt");
        sc_trace(o_vcd, r.req_accepted, pn + ".r_req_accepted");
        sc_trace(o_vcd, r.resp_error, pn + ".r_resp_error");
        sc_trace(o_vcd, r.progbuf_ena, pn + ".r_progbuf_ena");
        sc_trace(o_vcd, r.progbuf_pc, pn + ".r_progbuf_pc");
        sc_trace(o_vcd, r.progbuf_instr, pn + ".r_progbuf_instr");
    }

}

void DbgPort::comb() {
    sc_uint<CFG_LOG2_STACK_TRACE_ADDR> vb_stack_raddr;
    bool v_stack_we;
    sc_uint<CFG_LOG2_STACK_TRACE_ADDR> vb_stack_waddr;
    sc_biguint<(2 * RISCV_ARCH)> vb_stack_wdata;
    bool v_csr_req_valid;
    bool v_csr_resp_ready;
    sc_uint<CsrReq_TotalBits> vb_csr_req_type;
    sc_uint<12> vb_csr_req_addr;
    sc_uint<RISCV_ARCH> vb_csr_req_data;
    sc_uint<6> vb_o_ireg_addr;
    sc_uint<RISCV_ARCH> vb_o_ireg_wdata;
    sc_uint<12> vb_idx;
    bool v_o_ireg_ena;
    bool v_o_ireg_write;
    bool v_mem_req_valid;
    bool v_req_ready;
    bool v_resp_valid;
    sc_uint<64> vrdata;
    sc_uint<5> t_idx;

    vb_stack_raddr = 0;
    v_stack_we = 0;
    vb_stack_waddr = 0;
    vb_stack_wdata = 0;
    v_csr_req_valid = 0;
    v_csr_resp_ready = 0;
    vb_csr_req_type = 0;
    vb_csr_req_addr = 0;
    vb_csr_req_data = 0;
    vb_o_ireg_addr = 0;
    vb_o_ireg_wdata = 0;
    vb_idx = 0;
    v_o_ireg_ena = 0;
    v_o_ireg_write = 0;
    v_mem_req_valid = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    vrdata = 0;
    t_idx = 0;

    v = r;

    vb_idx = i_dport_addr.read()(11, 0);
    vrdata = r.dport_rdata;

    if (CFG_LOG2_STACK_TRACE_ADDR != 0) {
        if ((i_e_call.read() == 1) && (r.stack_trace_cnt.read() != (STACK_TRACE_BUF_SIZE - 1))) {
            v_stack_we = 1;
            vb_stack_waddr = r.stack_trace_cnt;
            vb_stack_wdata = (i_e_npc.read(), i_e_pc.read());
            v.stack_trace_cnt = (r.stack_trace_cnt.read() + 1);
        } else if ((i_e_ret.read() == 1) && (r.stack_trace_cnt.read().or_reduce() == 1)) {
            v.stack_trace_cnt = (r.stack_trace_cnt.read() - 1);
        }
    }

    switch (r.dstate.read()) {
    case idle:
        v_req_ready = 1;
        vrdata = 0;
        v.req_accepted = 0;
        v.resp_error = 0;
        v.progbuf_ena = 0;
        if (i_dport_req_valid.read() == 1) {
            if (i_dport_type.read()[DPortReq_RegAccess] == 1) {
                v.dport_write = i_dport_type.read()[DPortReq_Write];
                v.dport_addr = i_dport_addr;
                v.dport_wdata = i_dport_wdata;
                if (i_dport_addr.read()(15, 12) == 0x0) {
                    v.dstate = csr_region;
                } else if (i_dport_addr.read()(15, 12) == 0x1) {
                    v.dstate = reg_bank;
                } else if (i_dport_addr.read()(15, 12) == 0xC) {
                    // non-standard extension
                    if (vb_idx == 64) {
                        v.dstate = reg_stktr_cnt;
                    } else if ((vb_idx >= 128) && (vb_idx < (128 + (2 * STACK_TRACE_BUF_SIZE)))) {
                        v.dstate = reg_stktr_buf_adr;
                    }
                } else {
                    v.dstate = wait_to_accept;
                }
            } else if (i_dport_type.read()[DPortReq_Progexec] == 1) {
                v.dstate = exec_progbuf_start;
            } else if (i_dport_type.read()[DPortReq_MemAccess] == 1) {
                v.dstate = abstract_mem_request;
                v.dport_write = i_dport_type.read()[DPortReq_Write];
                v.dport_addr = i_dport_addr;
                v.dport_wdata = i_dport_wdata;
                v.dport_size = i_dport_size.read()(1, 0);
            } else {
                // Unsupported request
                v.dstate = wait_to_accept;
                v.resp_error = 1;
            }
        }
        break;
    case csr_region:
        v_csr_req_valid = (!r.req_accepted);
        v_csr_resp_ready = r.req_accepted;
        if ((r.req_accepted.read() == 0) && (i_csr_req_ready.read() == 1)) {
            v.req_accepted = 1;
        }
        if (r.dport_write.read() == 1) {
            vb_csr_req_type = CsrReq_WriteCmd;
        } else {
            vb_csr_req_type = CsrReq_ReadCmd;
        }
        vb_csr_req_addr = r.dport_addr.read()(11, 0);
        vb_csr_req_data = r.dport_wdata;
        if ((r.req_accepted && i_csr_resp_valid) == 1) {
            vrdata = i_csr_resp_data;
            v.dstate = wait_to_accept;
        }
        break;
    case reg_bank:
        v_o_ireg_ena = 1;
        vb_o_ireg_addr = r.dport_addr.read()(5, 0);
        vrdata = i_ireg_rdata;
        if (r.dport_write.read() == 1) {
            v_o_ireg_write = 1;
            vb_o_ireg_wdata = r.dport_wdata;
        }
        v.dstate = wait_to_accept;
        break;
    case reg_stktr_cnt:
        vrdata = 0;
        vrdata((CFG_LOG2_STACK_TRACE_ADDR - 1), 0) = r.stack_trace_cnt;
        if (r.dport_write.read() == 1) {
            v.stack_trace_cnt = r.dport_wdata.read()((CFG_LOG2_STACK_TRACE_ADDR - 1), 0);
        }
        v.dstate = wait_to_accept;
        break;
    case reg_stktr_buf_adr:
        vb_stack_raddr = r.dport_addr.read()(CFG_LOG2_STACK_TRACE_ADDR, 1);
        v.dstate = reg_stktr_buf_dat;
        break;
    case reg_stktr_buf_dat:
        if (r.dport_addr.read()[0] == 0) {
            vrdata = wb_stack_rdata.read()((RISCV_ARCH - 1), 0);
        } else {
            vrdata = wb_stack_rdata.read()(((2 * RISCV_ARCH) - 1), RISCV_ARCH);
        }
        v.dstate = wait_to_accept;
        break;
    case exec_progbuf_start:
        v.progbuf_ena = 1;
        v.progbuf_pc = 0;
        v.progbuf_instr = i_progbuf.read()(63, 0).to_uint64();
        v.dstate = exec_progbuf_next;
        break;
    case exec_progbuf_next:
        if (i_csr_progbuf_end.read() == 1) {
            v.progbuf_ena = 0;
            v.resp_error = i_csr_progbuf_error;
            v.dstate = wait_to_accept;
        } else if (i_e_memop_valid.read() == 1) {
            v.dstate = exec_progbuf_waitmemop;
        } else {
            t_idx = i_e_npc.read()(5, 2);
            v.progbuf_pc = (0ull, (i_e_npc.read()(5, 2) << 2));
            if (t_idx == 0xf) {
                v.progbuf_instr = (0, i_progbuf.read()(255, 224)).to_uint64();
            } else {
                v.progbuf_instr = i_progbuf.read()((32 * t_idx) + 64 - 1, (32 * t_idx)).to_uint64();
            }
        }
        break;
    case exec_progbuf_waitmemop:
        if (i_m_valid.read() == 1) {
            v.dstate = exec_progbuf_next;
        }
        break;
    case abstract_mem_request:
        v_mem_req_valid = 1;
        if (i_mem_req_ready.read() == 1) {
            if (i_mem_req_error.read() == 1) {
                v.dstate = wait_to_accept;
                v.resp_error = 1;
                vrdata = ~0ull;
            } else {
                v.dstate = abstract_mem_response;
            }
        }
        break;
    case abstract_mem_response:
        vrdata = i_mem_resp_rdata;
        if (i_mem_resp_valid.read() == 1) {
            v.dstate = wait_to_accept;
            v.resp_error = i_mem_resp_error;
        }
        break;
    case wait_to_accept:
        v_resp_valid = 1;
        if (i_dport_resp_ready.read() == 1) {
            v.dstate = idle;
        }
        break;
    default:
        break;
    }

    v.dport_rdata = vrdata;

    if (!async_reset_ && i_nrst.read() == 0) {
        DbgPort_r_reset(v);
    }

    wb_stack_raddr = vb_stack_raddr;
    w_stack_we = v_stack_we;
    wb_stack_waddr = vb_stack_waddr;
    wb_stack_wdata = vb_stack_wdata;

    o_csr_req_valid = v_csr_req_valid;
    o_csr_req_type = vb_csr_req_type;
    o_csr_req_addr = vb_csr_req_addr;
    o_csr_req_data = vb_csr_req_data;
    o_csr_resp_ready = v_csr_resp_ready;
    o_ireg_addr = vb_o_ireg_addr;
    o_ireg_wdata = vb_o_ireg_wdata;
    o_ireg_ena = v_o_ireg_ena;
    o_ireg_write = v_o_ireg_write;
    o_mem_req_valid = v_mem_req_valid;
    o_mem_req_write = r.dport_write;
    o_mem_req_addr = r.dport_addr;
    o_mem_req_wdata = r.dport_wdata;
    o_mem_req_size = r.dport_size;
    o_progbuf_ena = r.progbuf_ena;
    o_progbuf_pc = r.progbuf_pc;
    o_progbuf_instr = r.progbuf_instr;
    o_dport_req_ready = v_req_ready;
    o_dport_resp_valid = v_resp_valid;
    o_dport_resp_error = r.resp_error;
    o_dport_rdata = r.dport_rdata;
}

void DbgPort::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        DbgPort_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

