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
    sensitive << i_wadr;
    sensitive << r.ra_updated;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
    }
};


void RegIntBank::comb() {
    v = r;

    v.ra_updated = 0;
    if (i_wena.read()) {
        if (i_wadr.read()) {
            v.mem[i_wadr.read()] = i_wdata;
        }
        if (i_wadr.read() == Reg_ra) {
            v.ra_updated = 1;
        }
    }

    if (!i_nrst.read()) {
        v.ra_updated = false;
        for (int i = 0; i < Reg_Total; i++) {
            v.mem[i] = 0;
        }
    }

    o_rdata1 = r.mem[i_radr1.read()];
    o_rdata2 = r.mem[i_radr2.read()];
    o_ra = r.mem[Reg_ra];
}

void RegIntBank::registers() {
    r = v;
}

}  // namespace debugger

