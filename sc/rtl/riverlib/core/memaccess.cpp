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

#include "memaccess.h"
#include "api_core.h"

namespace debugger {

MemAccess::MemAccess(sc_module_name name,
                     bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_e_pc("i_e_pc"),
    i_e_instr("i_e_instr"),
    i_flushd_valid("i_flushd_valid"),
    i_flushd_addr("i_flushd_addr"),
    o_flushd("o_flushd"),
    i_mmu_ena("i_mmu_ena"),
    i_mmu_sv39("i_mmu_sv39"),
    i_mmu_sv48("i_mmu_sv48"),
    o_mmu_ena("o_mmu_ena"),
    o_mmu_sv39("o_mmu_sv39"),
    o_mmu_sv48("o_mmu_sv48"),
    i_reg_waddr("i_reg_waddr"),
    i_reg_wtag("i_reg_wtag"),
    i_memop_valid("i_memop_valid"),
    i_memop_debug("i_memop_debug"),
    i_memop_wdata("i_memop_wdata"),
    i_memop_sign_ext("i_memop_sign_ext"),
    i_memop_type("i_memop_type"),
    i_memop_size("i_memop_size"),
    i_memop_addr("i_memop_addr"),
    o_memop_ready("o_memop_ready"),
    o_wb_wena("o_wb_wena"),
    o_wb_waddr("o_wb_waddr"),
    o_wb_wdata("o_wb_wdata"),
    o_wb_wtag("o_wb_wtag"),
    i_wb_ready("i_wb_ready"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_valid("o_mem_valid"),
    o_mem_type("o_mem_type"),
    o_mem_addr("o_mem_addr"),
    o_mem_wdata("o_mem_wdata"),
    o_mem_wstrb("o_mem_wstrb"),
    o_mem_size("o_mem_size"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data_addr("i_mem_data_addr"),
    i_mem_data("i_mem_data"),
    o_mem_resp_ready("o_mem_resp_ready"),
    o_pc("o_pc"),
    o_valid("o_valid"),
    o_idle("o_idle"),
    o_debug_valid("o_debug_valid") {

    async_reset_ = async_reset;
    queue0 = 0;

    queue0 = new Queue<CFG_MEMACCESS_QUEUE_DEPTH,
                       QUEUE_WIDTH>("queue0", async_reset);
    queue0->i_clk(i_clk);
    queue0->i_nrst(i_nrst);
    queue0->i_re(queue_re);
    queue0->i_we(queue_we);
    queue0->i_wdata(queue_data_i);
    queue0->o_rdata(queue_data_o);
    queue0->o_full(queue_full);
    queue0->o_nempty(queue_nempty);


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_flushd_valid;
    sensitive << i_flushd_addr;
    sensitive << i_mmu_ena;
    sensitive << i_mmu_sv39;
    sensitive << i_mmu_sv48;
    sensitive << i_reg_waddr;
    sensitive << i_reg_wtag;
    sensitive << i_memop_valid;
    sensitive << i_memop_debug;
    sensitive << i_memop_wdata;
    sensitive << i_memop_sign_ext;
    sensitive << i_memop_type;
    sensitive << i_memop_size;
    sensitive << i_memop_addr;
    sensitive << i_wb_ready;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data;
    sensitive << r.state;
    sensitive << r.mmu_ena;
    sensitive << r.mmu_sv39;
    sensitive << r.mmu_sv48;
    sensitive << r.memop_type;
    sensitive << r.memop_addr;
    sensitive << r.memop_wdata;
    sensitive << r.memop_wstrb;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.memop_debug;
    sensitive << r.memop_res_pc;
    sensitive << r.memop_res_instr;
    sensitive << r.memop_res_addr;
    sensitive << r.memop_res_wtag;
    sensitive << r.memop_res_data;
    sensitive << r.memop_res_wena;
    sensitive << r.hold_rdata;
    sensitive << r.pc;
    sensitive << r.valid;
    sensitive << queue_we;
    sensitive << queue_re;
    sensitive << queue_data_i;
    sensitive << queue_data_o;
    sensitive << queue_nempty;
    sensitive << queue_full;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

MemAccess::~MemAccess() {
    if (queue0) {
        delete queue0;
    }
}

void MemAccess::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_instr, i_e_instr.name());
        sc_trace(o_vcd, i_flushd_valid, i_flushd_valid.name());
        sc_trace(o_vcd, i_flushd_addr, i_flushd_addr.name());
        sc_trace(o_vcd, o_flushd, o_flushd.name());
        sc_trace(o_vcd, i_mmu_ena, i_mmu_ena.name());
        sc_trace(o_vcd, i_mmu_sv39, i_mmu_sv39.name());
        sc_trace(o_vcd, i_mmu_sv48, i_mmu_sv48.name());
        sc_trace(o_vcd, o_mmu_ena, o_mmu_ena.name());
        sc_trace(o_vcd, o_mmu_sv39, o_mmu_sv39.name());
        sc_trace(o_vcd, o_mmu_sv48, o_mmu_sv48.name());
        sc_trace(o_vcd, i_reg_waddr, i_reg_waddr.name());
        sc_trace(o_vcd, i_reg_wtag, i_reg_wtag.name());
        sc_trace(o_vcd, i_memop_valid, i_memop_valid.name());
        sc_trace(o_vcd, i_memop_debug, i_memop_debug.name());
        sc_trace(o_vcd, i_memop_wdata, i_memop_wdata.name());
        sc_trace(o_vcd, i_memop_sign_ext, i_memop_sign_ext.name());
        sc_trace(o_vcd, i_memop_type, i_memop_type.name());
        sc_trace(o_vcd, i_memop_size, i_memop_size.name());
        sc_trace(o_vcd, i_memop_addr, i_memop_addr.name());
        sc_trace(o_vcd, o_memop_ready, o_memop_ready.name());
        sc_trace(o_vcd, o_wb_wena, o_wb_wena.name());
        sc_trace(o_vcd, o_wb_waddr, o_wb_waddr.name());
        sc_trace(o_vcd, o_wb_wdata, o_wb_wdata.name());
        sc_trace(o_vcd, o_wb_wtag, o_wb_wtag.name());
        sc_trace(o_vcd, i_wb_ready, i_wb_ready.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_mem_valid, o_mem_valid.name());
        sc_trace(o_vcd, o_mem_type, o_mem_type.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, o_mem_wdata, o_mem_wdata.name());
        sc_trace(o_vcd, o_mem_wstrb, o_mem_wstrb.name());
        sc_trace(o_vcd, o_mem_size, o_mem_size.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data_addr, i_mem_data_addr.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_idle, o_idle.name());
        sc_trace(o_vcd, o_debug_valid, o_debug_valid.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.mmu_ena, pn + ".r_mmu_ena");
        sc_trace(o_vcd, r.mmu_sv39, pn + ".r_mmu_sv39");
        sc_trace(o_vcd, r.mmu_sv48, pn + ".r_mmu_sv48");
        sc_trace(o_vcd, r.memop_type, pn + ".r_memop_type");
        sc_trace(o_vcd, r.memop_addr, pn + ".r_memop_addr");
        sc_trace(o_vcd, r.memop_wdata, pn + ".r_memop_wdata");
        sc_trace(o_vcd, r.memop_wstrb, pn + ".r_memop_wstrb");
        sc_trace(o_vcd, r.memop_sign_ext, pn + ".r_memop_sign_ext");
        sc_trace(o_vcd, r.memop_size, pn + ".r_memop_size");
        sc_trace(o_vcd, r.memop_debug, pn + ".r_memop_debug");
        sc_trace(o_vcd, r.memop_res_pc, pn + ".r_memop_res_pc");
        sc_trace(o_vcd, r.memop_res_instr, pn + ".r_memop_res_instr");
        sc_trace(o_vcd, r.memop_res_addr, pn + ".r_memop_res_addr");
        sc_trace(o_vcd, r.memop_res_wtag, pn + ".r_memop_res_wtag");
        sc_trace(o_vcd, r.memop_res_data, pn + ".r_memop_res_data");
        sc_trace(o_vcd, r.memop_res_wena, pn + ".r_memop_res_wena");
        sc_trace(o_vcd, r.hold_rdata, pn + ".r_hold_rdata");
        sc_trace(o_vcd, r.pc, pn + ".r_pc");
        sc_trace(o_vcd, r.valid, pn + ".r_valid");
    }

}

void MemAccess::comb() {
    sc_uint<RISCV_ARCH> vb_req_addr;
    sc_uint<64> vb_memop_wdata;
    sc_uint<8> vb_memop_wstrb;
    bool v_mem_valid;
    bool v_mem_debug;
    sc_uint<MemopType_Total> vb_mem_type;
    bool v_mem_sign_ext;
    sc_uint<2> vb_mem_sz;
    sc_uint<RISCV_ARCH> vb_mem_addr;
    sc_uint<64> vb_mem_rdata;
    bool v_queue_re;
    bool v_flushd;
    bool v_mmu_ena;
    bool v_mmu_sv39;
    bool v_mmu_sv48;
    sc_uint<CFG_REG_TAG_WIDTH> vb_res_wtag;
    sc_uint<64> vb_mem_wdata;
    sc_uint<8> vb_mem_wstrb;
    sc_uint<64> vb_mem_resp_shifted;
    sc_uint<64> vb_mem_data_unsigned;
    sc_uint<64> vb_mem_data_signed;
    sc_uint<RISCV_ARCH> vb_res_data;
    sc_uint<6> vb_res_addr;
    sc_uint<RISCV_ARCH> vb_e_pc;
    sc_uint<32> vb_e_instr;
    bool v_memop_ready;
    bool v_o_wena;
    sc_uint<6> vb_o_waddr;
    sc_uint<RISCV_ARCH> vb_o_wdata;
    sc_uint<CFG_REG_TAG_WIDTH> vb_o_wtag;
    bool v_valid;
    bool v_idle;
    sc_uint<1> t_memop_debug;

    vb_req_addr = 0;
    vb_memop_wdata = 0;
    vb_memop_wstrb = 0;
    v_mem_valid = 0;
    v_mem_debug = 0;
    vb_mem_type = 0;
    v_mem_sign_ext = 0;
    vb_mem_sz = 0;
    vb_mem_addr = 0;
    vb_mem_rdata = 0;
    v_queue_re = 0;
    v_flushd = 0;
    v_mmu_ena = 0;
    v_mmu_sv39 = 0;
    v_mmu_sv48 = 0;
    vb_res_wtag = 0;
    vb_mem_wdata = 0;
    vb_mem_wstrb = 0;
    vb_mem_resp_shifted = 0;
    vb_mem_data_unsigned = 0;
    vb_mem_data_signed = 0;
    vb_res_data = 0;
    vb_res_addr = 0;
    vb_e_pc = 0;
    vb_e_instr = 0;
    v_memop_ready = 0;
    v_o_wena = 0;
    vb_o_waddr = 0;
    vb_o_wdata = 0;
    vb_o_wtag = 0;
    v_valid = 0;
    v_idle = 0;
    t_memop_debug = 0;

    v = r;

    v.valid = 0;                                            // valid on next clock

    if (i_flushd_valid.read() == 1) {
        vb_req_addr = i_flushd_addr;
    } else {
        vb_req_addr = i_memop_addr;
    }

    switch (i_memop_size.read()) {
    case 0:
        vb_memop_wdata = (i_memop_wdata.read()(7, 0),
                i_memop_wdata.read()(7, 0),
                i_memop_wdata.read()(7, 0),
                i_memop_wdata.read()(7, 0),
                i_memop_wdata.read()(7, 0),
                i_memop_wdata.read()(7, 0),
                i_memop_wdata.read()(7, 0),
                i_memop_wdata.read()(7, 0));
        if (i_memop_addr.read()(2, 0) == 0x0) {
            vb_memop_wstrb = 0x01;
        } else if (i_memop_addr.read()(2, 0) == 0x1) {
            vb_memop_wstrb = 0x02;
        } else if (i_memop_addr.read()(2, 0) == 0x2) {
            vb_memop_wstrb = 0x04;
        } else if (i_memop_addr.read()(2, 0) == 0x3) {
            vb_memop_wstrb = 0x08;
        } else if (i_memop_addr.read()(2, 0) == 0x4) {
            vb_memop_wstrb = 0x10;
        } else if (i_memop_addr.read()(2, 0) == 0x5) {
            vb_memop_wstrb = 0x20;
        } else if (i_memop_addr.read()(2, 0) == 0x6) {
            vb_memop_wstrb = 0x40;
        } else if (i_memop_addr.read()(2, 0) == 0x7) {
            vb_memop_wstrb = 0x80;
        }
        break;
    case 1:
        vb_memop_wdata = (i_memop_wdata.read()(15, 0),
                i_memop_wdata.read()(15, 0),
                i_memop_wdata.read()(15, 0),
                i_memop_wdata.read()(15, 0));
        if (i_memop_addr.read()(2, 1) == 0) {
            vb_memop_wstrb = 0x03;
        } else if (i_memop_addr.read()(2, 1) == 1) {
            vb_memop_wstrb = 0x0C;
        } else if (i_memop_addr.read()(2, 1) == 2) {
            vb_memop_wstrb = 0x30;
        } else {
            vb_memop_wstrb = 0xC0;
        }
        break;
    case 2:
        vb_memop_wdata = (i_memop_wdata.read()(31, 0),
                i_memop_wdata.read()(31, 0));
        if (i_memop_addr.read()[2] == 1) {
            vb_memop_wstrb = 0xF0;
        } else {
            vb_memop_wstrb = 0x0F;
        }
        break;
    case 3:
        vb_memop_wdata = i_memop_wdata;
        vb_memop_wstrb = 0xFF;
        break;
    default:
        break;
    }

    // Form Queue inputs:
    t_memop_debug = i_memop_debug;                          // looks like bug in systemc, cannot handle bool properly
    queue_data_i = (t_memop_debug,
            i_flushd_valid,
            i_mmu_ena,
            i_mmu_sv39,
            i_mmu_sv48,
            i_reg_wtag,
            vb_memop_wdata,
            vb_memop_wstrb,
            i_memop_wdata,
            i_reg_waddr,
            i_e_instr,
            i_e_pc,
            i_memop_size,
            i_memop_sign_ext,
            i_memop_type,
            vb_req_addr);
    queue_we = ((i_memop_valid.read() | i_flushd_valid.read()) & (!queue_full));

    // Split Queue outputs:
    v_mem_debug = queue_data_o.read()[316];
    v_flushd = queue_data_o.read()[315];
    v_mmu_ena = queue_data_o.read()[314];
    v_mmu_sv39 = queue_data_o.read()[313];
    v_mmu_sv48 = queue_data_o.read()[312];
    vb_res_wtag = queue_data_o.read()(311, 309);
    vb_mem_wdata = queue_data_o.read()(308, 245);
    vb_mem_wstrb = queue_data_o.read()(244, 237);
    vb_res_data = queue_data_o.read()(236, 173);
    vb_res_addr = queue_data_o.read()(172, 167);
    vb_e_instr = queue_data_o.read()(166, 135);
    vb_e_pc = queue_data_o.read()(134, 71);
    vb_mem_sz = queue_data_o.read()(70, 69);
    v_mem_sign_ext = queue_data_o.read()[68];
    vb_mem_type = queue_data_o.read()(67, 64);
    vb_mem_addr = queue_data_o.read()(63, 0);

    switch (r.memop_addr.read()(2, 0)) {
    case 1:
        vb_mem_resp_shifted(55, 0) = i_mem_data.read()(63, 8);
        break;
    case 2:
        vb_mem_resp_shifted(47, 0) = i_mem_data.read()(63, 16);
        break;
    case 3:
        vb_mem_resp_shifted(39, 0) = i_mem_data.read()(63, 24);
        break;
    case 4:
        vb_mem_resp_shifted(31, 0) = i_mem_data.read()(63, 32);
        break;
    case 5:
        vb_mem_resp_shifted(23, 0) = i_mem_data.read()(63, 40);
        break;
    case 6:
        vb_mem_resp_shifted(15, 0) = i_mem_data.read()(63, 48);
        break;
    case 7:
        vb_mem_resp_shifted(7, 0) = i_mem_data.read()(63, 56);
        break;
    default:
        vb_mem_resp_shifted = i_mem_data;
        break;
    }
    switch (r.memop_size.read()) {
    case MEMOP_1B:
        vb_mem_data_unsigned(7, 0) = vb_mem_resp_shifted(7, 0);
        vb_mem_data_signed(7, 0) = vb_mem_resp_shifted(7, 0);
        if (vb_mem_resp_shifted[7] == 1) {
            vb_mem_data_signed(63, 8) = ~0ull;
        }
        break;
    case MEMOP_2B:
        vb_mem_data_unsigned(15, 0) = vb_mem_resp_shifted(15, 0);
        vb_mem_data_signed(15, 0) = vb_mem_resp_shifted(15, 0);
        if (vb_mem_resp_shifted[15] == 1) {
            vb_mem_data_signed(63, 16) = ~0ull;
        }
        break;
    case MEMOP_4B:
        vb_mem_data_unsigned(31, 0) = vb_mem_resp_shifted(31, 0);
        vb_mem_data_signed(31, 0) = vb_mem_resp_shifted(31, 0);
        if (vb_mem_resp_shifted[31] == 1) {
            vb_mem_data_signed(63, 32) = ~0ull;
        }
        break;
    default:
        vb_mem_data_unsigned = vb_mem_resp_shifted;
        vb_mem_data_signed = vb_mem_resp_shifted;
        break;
    }

    if ((r.memop_type.read()[MemopType_Store] == 0) || (r.memop_type.read()[MemopType_Release] == 1)) {
        if (r.memop_sign_ext.read() == 1) {
            vb_mem_rdata = vb_mem_data_signed;
        } else {
            vb_mem_rdata = vb_mem_data_unsigned;
        }
    } else {
        vb_mem_rdata = r.memop_res_data;
    }
    switch (r.state.read()) {
    case State_Idle:
        v_queue_re = 1;
        if (queue_nempty.read() == 1) {
            v.pc = vb_e_pc;
            v_mem_valid = (!v_flushd);
            v.mmu_ena = v_mmu_ena;
            v.mmu_sv39 = v_mmu_sv39;
            v.mmu_sv48 = v_mmu_sv48;
            v.memop_res_pc = vb_e_pc;
            v.memop_res_instr = vb_e_instr;
            v.memop_res_addr = vb_res_addr;
            v.memop_res_wtag = vb_res_wtag;
            v.memop_res_data = vb_res_data;
            if ((vb_res_addr.or_reduce() == 1)
                    && (((!vb_mem_type[MemopType_Store]) || vb_mem_type[MemopType_Release]) == 1)) {
                v.memop_res_wena = 1;
            } else {
                v.memop_res_wena = 0;
            }
            v.memop_addr = vb_mem_addr;
            v.memop_wdata = vb_mem_wdata;
            v.memop_wstrb = vb_mem_wstrb;
            v.memop_type = vb_mem_type;
            v.memop_debug = v_mem_debug;
            v.memop_sign_ext = v_mem_sign_ext;
            v.memop_size = vb_mem_sz;
            if (v_flushd == 1) {
                // do nothing
                v.valid = 1;
            } else if (i_mem_req_ready.read() == 1) {
                v.state = State_WaitResponse;
            } else {
                v.state = State_WaitReqAccept;
            }
        }
        break;
    case State_WaitReqAccept:
        v_mem_valid = 1;
        v_mmu_ena = r.mmu_ena;
        v_mmu_sv39 = r.mmu_sv39;
        v_mmu_sv48 = r.mmu_sv48;
        vb_mem_type = r.memop_type;
        vb_mem_sz = r.memop_size;
        vb_mem_addr = r.memop_addr;
        vb_mem_wdata = r.memop_wdata;
        vb_mem_wstrb = r.memop_wstrb;
        vb_res_data = r.memop_res_data;
        if (i_mem_req_ready.read() == 1) {
            v.state = State_WaitResponse;
        }
        break;
    case State_WaitResponse:
        if (i_mem_data_valid.read() == 0) {
            // Do nothing
        } else {
            v_o_wena = r.memop_res_wena;
            vb_o_waddr = r.memop_res_addr;
            vb_o_wdata = vb_mem_rdata;
            vb_o_wtag = r.memop_res_wtag;

            v_queue_re = 1;
            if ((r.memop_res_wena.read() == 1) && (r.memop_debug.read() == 0) && (i_wb_ready.read() == 0)) {
                // Inject only one clock hold-on and wait a couple of clocks while writeback finished
                v_queue_re = 0;
                v.state = State_Hold;
                v.hold_rdata = vb_mem_rdata;
            } else if (queue_nempty.read() == 1) {
                v_valid = 1;
                v.pc = vb_e_pc;
                v_mem_valid = (!v_flushd);
                v.mmu_ena = v_mmu_ena;
                v.mmu_sv39 = v_mmu_sv39;
                v.mmu_sv48 = v_mmu_sv48;
                v.memop_res_pc = vb_e_pc;
                v.memop_res_instr = vb_e_instr;
                v.memop_res_addr = vb_res_addr;
                v.memop_res_wtag = vb_res_wtag;
                v.memop_res_data = vb_res_data;
                if ((vb_res_addr.or_reduce() == 1)
                        && (((!vb_mem_type[MemopType_Store]) || vb_mem_type[MemopType_Release]) == 1)) {
                    v.memop_res_wena = 1;
                } else {
                    v.memop_res_wena = 0;
                }
                v.memop_addr = vb_mem_addr;
                v.memop_wdata = vb_mem_wdata;
                v.memop_wstrb = vb_mem_wstrb;
                v.memop_type = vb_mem_type;
                v.memop_sign_ext = v_mem_sign_ext;
                v.memop_size = vb_mem_sz;
                v.memop_debug = v_mem_debug;

                if (v_flushd == 1) {
                    v.state = State_Idle;
                    v.valid = 1;
                } else if (i_mem_req_ready.read() == 1) {
                    v.state = State_WaitResponse;
                } else {
                    v.state = State_WaitReqAccept;
                }
            } else {
                v.state = State_Idle;
                v_valid = 1;
            }
        }
        break;
    case State_Hold:
        v_o_wena = r.memop_res_wena;
        vb_o_waddr = r.memop_res_addr;
        vb_o_wdata = r.hold_rdata;
        vb_o_wtag = r.memop_res_wtag;
        if (i_wb_ready.read() == 1) {
            v_valid = 1;
            v_queue_re = 1;
            if (queue_nempty.read() == 1) {
                v.pc = vb_e_pc;
                v_mem_valid = (!v_flushd);
                v.mmu_ena = v_mmu_ena;
                v.mmu_sv39 = v_mmu_sv39;
                v.mmu_sv48 = v_mmu_sv48;
                v.memop_res_pc = vb_e_pc;
                v.memop_res_instr = vb_e_instr;
                v.memop_res_addr = vb_res_addr;
                v.memop_res_wtag = vb_res_wtag;
                v.memop_res_data = vb_res_data;
                if ((vb_res_addr.or_reduce() == 1)
                        && (((!vb_mem_type[MemopType_Store]) || vb_mem_type[MemopType_Release]) == 1)) {
                    v.memop_res_wena = 1;
                } else {
                    v.memop_res_wena = 0;
                }
                v.memop_addr = vb_mem_addr;
                v.memop_wdata = vb_mem_wdata;
                v.memop_wstrb = vb_mem_wstrb;
                v.memop_type = vb_mem_type;
                v.memop_sign_ext = v_mem_sign_ext;
                v.memop_size = vb_mem_sz;
                v.memop_debug = v_mem_debug;

                if (v_flushd == 1) {
                    v.state = State_Idle;
                    v.valid = 1;
                } else if (i_mem_req_ready.read() == 1) {
                    v.state = State_WaitResponse;
                } else {
                    v.state = State_WaitReqAccept;
                }
            } else {
                v.state = State_Idle;
            }
        }
        break;
    default:
        break;
    }

    v_memop_ready = 1;
    if (queue_full.read() == 1) {
        v_memop_ready = 0;
    }

    if ((queue_nempty.read() == 0) && (r.state.read() == State_Idle)) {
        v_idle = 1;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        MemAccess_r_reset(v);
    }

    queue_re = v_queue_re;
    o_flushd = (queue_nempty && v_flushd && v_queue_re);
    o_mmu_ena = v_mmu_ena;
    o_mmu_sv39 = v_mmu_sv39;
    o_mmu_sv48 = v_mmu_sv48;
    o_mem_resp_ready = 1;
    o_mem_valid = v_mem_valid;
    o_mem_type = vb_mem_type;
    o_mem_addr = vb_mem_addr;
    o_mem_wdata = vb_mem_wdata;
    o_mem_wstrb = vb_mem_wstrb;
    o_mem_size = vb_mem_sz;
    o_memop_ready = v_memop_ready;
    o_wb_wena = (v_o_wena && (!r.memop_debug));
    o_wb_waddr = vb_o_waddr;
    o_wb_wdata = vb_o_wdata;
    o_wb_wtag = vb_o_wtag;
    o_pc = r.pc;
    o_valid = ((r.valid || v_valid) && (!r.memop_debug));
    o_idle = v_idle;
    o_debug_valid = ((r.valid || v_valid) && r.memop_debug);
}

void MemAccess::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        MemAccess_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

