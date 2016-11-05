/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CSR registers module.
 */

#include "csr.h"
#include "riscv-isa.h"

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
    case CSR_misa:
        break;
    case CSR_mvendorid:
        break;
    case CSR_marchid:
        break;
    case CSR_mimplementationid:
        break;
    case CSR_mhartid:
        break;
    case CSR_uepc:// - User mode program counter
        break;
    case CSR_mstatus:// - Machine mode status register
        break;
    case CSR_medeleg:// - Machine exception delegation
        break;
    case CSR_mideleg:// - Machine itnerrupt delegation
        break;
    case CSR_mie:// - Machine interrupt enable bit
        break;
    case CSR_mtvec:
        wb_rdata = r.mvec;
        break;
    case CSR_mtimecmp:// - Machine wall-clock timer compare value
        break;
    case CSR_mscratch:// - Machine scrathc register
        break;
    case CSR_mepc:// - Machine program counter
        wb_rdata = r.mepc;
        if (i_wena.read()) {
            v.mepc = i_wdata;
        }
        break;
    case CSR_mcause:// - Machine trap cause
        break;
    case CSR_mbadaddr:// - Machine bad address
        break;
    case CSR_mip:// - Machine interrupt pending
        break;
    default:;
    }

    if (!i_nrst.read()) {
        v.mvec = 0;
        v.mode = PRV_M;
        v.ie = 0;
        v.mepc = 0;
    }


    o_rdata = wb_rdata;
    o_ie = r.ie;
    o_mode = r.mode;
}

void CsrRegs::registers() {
    r = v;
}

}  // namespace debugger

