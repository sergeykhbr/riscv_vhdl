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

#include "memaccess.h"

namespace debugger {

MemAccess::MemAccess(sc_module_name name_, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_e_valid("i_e_valid"),
    i_e_pc("i_e_pc"),
    i_e_instr("i_e_instr"),
    i_res_addr("i_res_addr"),
    i_res_data("i_res_data"),
    i_memop_sign_ext("i_memop_sign_ext"),
    i_memop_load("i_memop_load"),
    i_memop_store("i_memop_store"),
    i_memop_size("i_memop_size"),
    i_memop_addr("i_memop_addr"),
    o_wena("o_wena"),
    o_memop_ready("o_memop_ready"),
    o_waddr("o_waddr"),
    o_wdata("o_wdata"),
    i_mem_req_ready("i_mem_req_ready"),
    o_mem_valid("o_mem_valid"),
    o_mem_write("o_mem_write"),
    o_mem_addr("o_mem_addr"),
    o_mem_wdata("o_mem_wdata"),
    o_mem_wstrb("o_mem_wstrb"),
    i_mem_data_valid("i_mem_data_valid"),
    i_mem_data_addr("i_mem_data_addr"),
    i_mem_data("i_mem_data"),
    o_mem_resp_ready("o_mem_resp_ready"),
    o_hold("o_hold"),
    o_valid("o_valid"),
    o_pc("o_pc"),
    o_instr("o_instr") {
    async_reset_ = async_reset;

    SC_METHOD(main);
    sensitive << i_e_valid;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_res_addr;
    sensitive << i_res_data;
    sensitive << i_memop_sign_ext;
    sensitive << i_memop_load;
    sensitive << i_memop_store;
    sensitive << i_memop_size;
    sensitive << i_memop_addr;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mem_req_ready;
    sensitive << i_mem_data_valid;
    sensitive << i_mem_data_addr;
    sensitive << i_mem_data;
    sensitive << queue_data_o;
    sensitive << queue_nempty;
    sensitive << queue_full;
    sensitive << r.state;
    sensitive << r.memop_r;
    sensitive << r.memop_addr;
    sensitive << r.pc;
    sensitive << r.instr;
    sensitive << r.res_addr;
    sensitive << r.res_data;
    sensitive << r.memop_sign_ext;
    sensitive << r.memop_size;
    sensitive << r.memop_wdata;
    sensitive << r.memop_wstrb;
    sensitive << r.wena;

    SC_METHOD(qproc);
    sensitive << i_nrst;
    sensitive << queue_we;
    sensitive << queue_re;
    sensitive << queue_data_i;
    sensitive << qr.wcnt;
    for(int i = 0; i < QUEUE_DEPTH; i++) {
        sensitive << qr.mem[i];
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void MemAccess::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_instr, i_e_instr.name());
        sc_trace(o_vcd, i_memop_addr, i_memop_addr.name());
        sc_trace(o_vcd, i_memop_store, i_memop_store.name());
        sc_trace(o_vcd, i_memop_load, i_memop_load.name());
        sc_trace(o_vcd, i_memop_size, i_memop_size.name());
        sc_trace(o_vcd, o_mem_valid, o_mem_valid.name());
        sc_trace(o_vcd, o_mem_write, o_mem_write.name());
        sc_trace(o_vcd, i_mem_req_ready, i_mem_req_ready.name());
        sc_trace(o_vcd, o_mem_addr, o_mem_addr.name());
        sc_trace(o_vcd, o_mem_wdata, o_mem_wdata.name());
        sc_trace(o_vcd, o_mem_wstrb, o_mem_wstrb.name());
        sc_trace(o_vcd, i_mem_data_valid, i_mem_data_valid.name());
        sc_trace(o_vcd, i_mem_data_addr, i_mem_data_addr.name());
        sc_trace(o_vcd, i_mem_data, i_mem_data.name());
        sc_trace(o_vcd, o_mem_resp_ready, o_mem_resp_ready.name());

        sc_trace(o_vcd, o_hold, o_hold.name());
        sc_trace(o_vcd, o_valid, o_valid.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_wena, o_wena.name());
        sc_trace(o_vcd, o_memop_ready, o_memop_ready.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_wdata, o_wdata.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".state");
        sc_trace(o_vcd, queue_data_i, pn + ".queue_data_i");
        sc_trace(o_vcd, queue_data_o, pn + ".queue_data_o");
        sc_trace(o_vcd, queue_nempty, pn + ".queue_nempty");
        sc_trace(o_vcd, queue_full, pn + ".queue_full");
    }
}

void MemAccess::main() {
    sc_uint<BUS_DATA_WIDTH> vb_memop_wdata;
    sc_uint<BUS_DATA_BYTES> vb_memop_wstrb;

    switch (i_memop_size.read()) {
    case 0:
        vb_memop_wdata = (i_res_data.read()(7, 0),
            i_res_data.read()(7, 0), i_res_data.read()(7, 0),
            i_res_data.read()(7, 0), i_res_data.read()(7, 0),
            i_res_data.read()(7, 0), i_res_data.read()(7, 0),
            i_res_data.read()(7, 0));
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
        vb_memop_wdata = (i_res_data.read()(15, 0),
            i_res_data.read()(15, 0), i_res_data.read()(15, 0),
            i_res_data.read()(15, 0));
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
        vb_memop_wdata = (i_res_data.read()(31, 0),
                    i_res_data.read()(31, 0));
        if (i_memop_addr.read()[2]) {
            vb_memop_wstrb = 0xF0;
        } else {
            vb_memop_wstrb = 0x0F;
        }
        break;
    case 3:
        vb_memop_wdata = i_res_data;
        vb_memop_wstrb = 0xFF;
        break;
    default:;
    }


    queue_data_i = (vb_memop_wdata, vb_memop_wstrb,
                    i_res_data, i_res_addr, i_e_instr, i_e_pc,
                    i_memop_size, i_memop_sign_ext, i_memop_store,
                    (i_memop_load | i_memop_store), i_memop_addr);
    queue_we = i_e_valid;
}

void MemAccess::qproc() {
    bool nempty;
    sc_biguint<QUEUE_WIDTH> vb_data_o;
    bool full;

    qv = qr;

    full = 0;
    if (qr.wcnt.read() == QUEUE_DEPTH) {
        full = 1;
    }

    vb_data_o = qr.mem[0].read();
    if (queue_re == 1 && queue_we == 1) {
        if (qr.wcnt.read() == 0) {
            vb_data_o = queue_data_i;
        } else {
            qv.mem[0] = queue_data_i;
        }
    } else if (queue_re == 0 && queue_we == 1) {
        if (full == 0) {
            qv.wcnt = qr.wcnt.read() + 1;
            qv.mem[0] = queue_data_i;
        }
    } else if (queue_re == 1 && queue_we == 0) {
        if (qr.wcnt.read() != 0) {
            qv.wcnt = qr.wcnt.read() - 1;
        }
    }

    nempty = 0;
    if (queue_we == 1 || qr.wcnt.read() != 0) {
        nempty = 1;
    }

    if (!async_reset_ && i_nrst == 0) {
        qv.wcnt = 0;
        for (int k = 0; k < QUEUE_DEPTH; k++) {
            qv.mem[k] =  0;
        }
    }

    queue_nempty = nempty;
    queue_full = full;
    queue_data_o = vb_data_o;
}

void MemAccess::comb() {
    bool w_mem_access;
    bool w_mem_valid;
    bool w_mem_write;
    bool w_mem_sign_ext;
    sc_uint<2> wb_mem_sz;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<BUS_DATA_WIDTH> vb_wdata;
    bool w_hold;
    bool w_valid;
    bool w_queue_re;
    //sc_uint<RISCV_ARCH> wb_mem_data_signext;
    sc_uint<BUS_DATA_WIDTH> vb_mem_wdata;
    sc_uint<BUS_DATA_BYTES> vb_mem_wstrb;
    sc_uint<BUS_DATA_WIDTH> vb_mem_resp_shifted;
    sc_uint<BUS_DATA_WIDTH> vb_mem_data_unsigned;
    sc_uint<BUS_DATA_WIDTH> vb_mem_data_signed;
    sc_uint<RISCV_ARCH> vb_res_data;
    sc_uint<6> wb_res_addr;
    sc_uint<BUS_ADDR_WIDTH> wb_e_pc;
    sc_uint<32> wb_e_instr;

    v = r;

    w_mem_valid = 0;
    w_hold = 0;
    w_valid = 0;
    w_queue_re = 0;

    vb_mem_wdata = queue_data_o.read()(2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+BUS_DATA_WIDTH+43-1,
                                      2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+43);
    vb_mem_wstrb = queue_data_o.read()(2*BUS_ADDR_WIDTH+RISCV_ARCH+BUS_DATA_BYTES+43-1,
                                      2*BUS_ADDR_WIDTH+RISCV_ARCH+43);
    vb_res_data = queue_data_o.read()(2*BUS_ADDR_WIDTH+RISCV_ARCH+43-1,
                                      2*BUS_ADDR_WIDTH+43);
    wb_res_addr = queue_data_o.read()(2*BUS_ADDR_WIDTH+43-1,
                                      2*BUS_ADDR_WIDTH+37);
    wb_e_instr = queue_data_o.read()(2*BUS_ADDR_WIDTH+37-1,
                                     2*BUS_ADDR_WIDTH+5);
    wb_e_pc = queue_data_o.read()(2*BUS_ADDR_WIDTH+5-1, BUS_ADDR_WIDTH+5);
    wb_mem_sz = queue_data_o.read()(BUS_ADDR_WIDTH+4, BUS_ADDR_WIDTH+3);
    w_mem_sign_ext = queue_data_o.read()[BUS_ADDR_WIDTH+2];
    w_mem_write = queue_data_o.read()[BUS_ADDR_WIDTH+1];
    w_mem_access = queue_data_o.read()[BUS_ADDR_WIDTH];
    wb_mem_addr = queue_data_o.read()(BUS_ADDR_WIDTH-1, 0);

    switch (r.state.read()) {
    case State_Idle:
        w_queue_re = 1;
        if (queue_nempty == 1) {
            if (w_mem_access == 1) {
                w_mem_valid = 1;
                if (i_mem_req_ready.read() == 1) {
                    v.state = State_WaitResponse;
                } else {
                    w_hold = 1;
                    v.state = State_WaitReqAccept;
                }
            } else {
                v.state = State_RegForward;
            }
        }
        break;
    case State_WaitReqAccept:
        w_mem_valid = 1;
        w_mem_write = !r.memop_r.read();
        wb_mem_sz = r.memop_size.read();
        wb_mem_addr = r.memop_addr.read();
        vb_mem_wdata = r.memop_wdata.read();
        vb_mem_wstrb = r.memop_wstrb.read();
        vb_res_data = r.res_data.read();
        w_hold = 1;
        if (i_mem_req_ready.read() == 1) {
            v.state = State_WaitResponse;
        }
        break;
    case State_WaitResponse:
        w_valid = 1;
        w_queue_re = 1;
        if (i_mem_data_valid.read() == 0) {
            w_queue_re = 0;
            w_valid = 0;
            w_hold = 1;
        } else if (queue_nempty == 1) {
            if (w_mem_access == 1) {
                w_mem_valid = 1;
                if (i_mem_req_ready.read() == 1) {
                    v.state = State_WaitResponse;
                } else {
                    w_hold = 1;
                    v.state = State_WaitReqAccept;
                }
            } else {
                v.state = State_RegForward;
            }
        } else {
            v.state = State_Idle;
        }
        break;
    case State_RegForward:
        w_valid = 1;
        w_queue_re = 1;
        if (queue_nempty == 1) {
            if (w_mem_access == 1) {
                w_mem_valid = 1;
                if (i_mem_req_ready.read() == 1) {
                    v.state = State_WaitResponse;
                } else {
                    w_hold = 1;
                    v.state = State_WaitReqAccept;
                }
            } else {
                v.state = State_RegForward;
            }
        } else {
            v.state = State_Idle;
        }
        break;
    default:;
    }

    if (w_queue_re == 1) {
        v.pc = wb_e_pc;
        v.instr = wb_e_instr;
        v.res_addr = wb_res_addr;
        v.res_data = vb_res_data;
        v.memop_wdata = vb_mem_wdata;
        v.memop_wstrb = vb_mem_wstrb;
        if (wb_res_addr == 0) {
            v.wena = 0;
        } else {
            v.wena = 1;
        }
        v.memop_addr = wb_mem_addr;
        v.memop_r = w_mem_access && !w_mem_write;
        v.memop_sign_ext = w_mem_sign_ext;
        v.memop_size = wb_mem_sz;
    }


    vb_mem_resp_shifted = 0;
    vb_mem_data_unsigned = 0;
    vb_mem_data_signed = 0;
    switch (r.memop_addr.read()(2, 0)) {
    case 1:
        vb_mem_resp_shifted = i_mem_data.read()(63, 8);
        break;
    case 2:
        vb_mem_resp_shifted = i_mem_data.read()(63, 16);
        break;
    case 3:
        vb_mem_resp_shifted = i_mem_data.read()(63, 24);
        break;
    case 4:
        vb_mem_resp_shifted = i_mem_data.read()(63, 32);
        break;
    case 5:
        vb_mem_resp_shifted = i_mem_data.read()(63, 40);
        break;
    case 6:
        vb_mem_resp_shifted = i_mem_data.read()(63, 48);
        break;
    case 7:
        vb_mem_resp_shifted = i_mem_data.read()(63, 56);
        break;
    default:
        vb_mem_resp_shifted = i_mem_data.read();
    } 

    switch (r.memop_size.read()) {
    case MEMOP_1B:
        vb_mem_data_unsigned = vb_mem_resp_shifted(7, 0);
        vb_mem_data_signed = vb_mem_resp_shifted(7, 0);
        if (vb_mem_resp_shifted[7]) {
            vb_mem_data_signed(63, 8) = ~0;
        }
        break;
    case MEMOP_2B:
        vb_mem_data_unsigned = vb_mem_resp_shifted(15, 0);
        vb_mem_data_signed = vb_mem_resp_shifted(15, 0);
        if (vb_mem_resp_shifted[15]) {
            vb_mem_data_signed(63, 16) = ~0;
        }
        break;
    case MEMOP_4B:
        vb_mem_data_unsigned = vb_mem_resp_shifted(31, 0);
        vb_mem_data_signed = vb_mem_resp_shifted(31, 0);
        if (i_mem_data.read()[31]) {
            vb_mem_data_signed(63, 32) = ~0;
        }
        break;
    default:
        vb_mem_data_unsigned = vb_mem_resp_shifted;
        vb_mem_data_signed = vb_mem_resp_shifted;
    }

    if (r.memop_r.read() == 1) {
        if (r.memop_sign_ext.read() == 1) {
            vb_wdata = vb_mem_data_signed;
        } else {
            vb_wdata = vb_mem_data_unsigned;
        }
    } else {
        vb_wdata = r.res_data;
    }


    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    queue_re = w_queue_re;

    o_mem_resp_ready = 1;

    o_mem_valid = w_mem_valid;
    o_mem_write = w_mem_write;
    o_mem_addr = wb_mem_addr & ~LOG2_DATA_BYTES_MASK;
    o_mem_wdata = vb_mem_wdata;
    o_mem_wstrb = vb_mem_wstrb;

    o_memop_ready = !queue_full.read();
    o_wena = r.wena.read() && w_valid;
    o_waddr = r.res_addr;
    o_wdata = vb_wdata;
    o_hold = w_hold;
    /** the following signal used to executation instruction count and debug */
    o_valid = w_valid;
    o_pc = r.pc;
    o_instr = r.instr;
}

void MemAccess::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
        qr.wcnt = 0;
        for (int k = 0; k < QUEUE_DEPTH; k++) {
            qr.mem[k] = 0;
        }
    } else {
        r = v;
        qr = qv;
    }
}

}  // namespace debugger

