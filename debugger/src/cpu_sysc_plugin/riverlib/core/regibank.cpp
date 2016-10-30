/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Multi-port CPU Integer Registers memory.
 */

#include "regibank.h"

namespace debugger {

RegIntBank::RegIntBank(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_radr1;
    sensitive << i_radr2;
    sensitive << i_wena;
    sensitive << i_wdata;
    sensitive << i_waddr;
    sensitive << r.update;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
        sc_trace(vcd, i_wena, "/top/proc0/regs/i_wena");
        sc_trace(vcd, i_waddr, "/top/proc0/regs/i_waddr");
        sc_trace(vcd, i_wdata, "/top/proc0/regs/i_wdata");
        sc_trace(vcd, r.mem[4], "/top/proc0/regs/r4");
        sc_trace(vcd, o_rdata1, "/top/proc0/regs/o_rdata1");
        sc_trace(vcd, o_rdata2, "/top/proc0/regs/o_rdata2");
    }
};


void RegIntBank::comb() {
    v = r;

    if (i_wena.read()) {
        if (i_waddr.read() != 0) {
            v.mem[i_waddr.read()] = i_wdata;
        }
    }

    if (!i_nrst.read()) {
        for (int i = 0; i < Reg_Total; i++) {
            v.mem[i] = 0;
        }
        v.update = 0;
    }
    v.update = !r.update;

    o_rdata1 = r.mem[i_radr1.read()];
    o_rdata2 = r.mem[i_radr2.read()];
    o_ra = r.mem[Reg_ra];
}

void RegIntBank::registers() {
    r = v;
}

}  // namespace debugger

