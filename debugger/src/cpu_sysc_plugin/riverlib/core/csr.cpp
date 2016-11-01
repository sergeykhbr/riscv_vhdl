/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CSR registers module.
 */

#include "csr.h"

namespace debugger {

CsrRegs::CsrRegs(sc_module_name name_, sc_trace_file *vcd) 
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_addr;
    sensitive << i_wena;
    sensitive << i_wdata;
    sensitive << r.mode;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    if (vcd) {
    }
};


void CsrRegs::comb() {
    sc_uint<RISCV_ARCH> wb_rdata = 0;

    v = r;

    switch (i_addr.read()) {
    case 0xf10: // misa
        break;
    case 0xf11: // mvendorid
        break;
    case 0xf12: // marchid
        break;
    case 0xf13: // mimplementationid
        break;
    case 0xf14: // mhartid
        break;
    case 0x041: // uepc - User mode program counter
        break;
    case 0x300: // mstatus - Machine mode status register
        break;
    case 0x302: // medeleg - Machine exception delegation
        break;
    case 0x303: // mideleg - Machine itnerrupt delegation
        break;
    case 0x304: // mie - Machine interrupt enable bit
        break;
    case 0x305: // mvec
        wb_rdata = r.mvec;
        break;
    case 0x321: // mtimecmp - Machine wall-clock timer compare value
        break;
    case 0x340: // mscratch - Machine scrathc register
        break;
    case 0x341: // mepc - Machine program counter
        break;
    case 0x342: // mcause - Machine trap cause
        break;
    case 0x343: // mbadaddr - Machine bad address
        break;
    case 0x344: // mip - Machine interrupt pending
        break;
    default:;
    }

    if (!i_nrst.read()) {
        v.mvec = 0;
        v.mode = PRV_M;
        v.ie = 0;
    }


    o_rdata = wb_rdata;
    o_ie = r.ie;
    o_mode = r.mode;
}

void CsrRegs::registers() {
    r = v;
}

}  // namespace debugger

