/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "csr.h"
#include "riscv-isa.h"

namespace debugger {

CsrRegs::CsrRegs(sc_module_name name_, uint32_t hartid)
    : sc_module(name_) {
    hartid_ = hartid;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xret;
    sensitive << i_addr;
    sensitive << i_wena;
    sensitive << i_wdata;
    sensitive << i_break_mode;
    sensitive << i_breakpoint;
    sensitive << i_trap_ena;
    sensitive << i_trap_code;
    sensitive << i_trap_pc;
    sensitive << i_dport_ena;
    sensitive << i_dport_write;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << r.mtvec;
    sensitive << r.mscratch;
    sensitive << r.mbadaddr;
    sensitive << r.mode;
    sensitive << r.uie;
    sensitive << r.mie;
    sensitive << r.mpie;
    sensitive << r.mpp;
    sensitive << r.mepc;
    sensitive << r.trap_irq;
    sensitive << r.trap_code;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void CsrRegs::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_xret, "/top/proc0/csr0/i_xret");
        sc_trace(o_vcd, i_addr, "/top/proc0/csr0/i_addr");
        sc_trace(o_vcd, i_wena, "/top/proc0/csr0/i_wena");
        sc_trace(o_vcd, i_wdata, "/top/proc0/csr0/i_wdata");
        sc_trace(o_vcd, i_trap_ena, "/top/proc0/csr0/i_trap_ena");
        sc_trace(o_vcd, i_trap_code, "/top/proc0/csr0/i_trap_code");
        sc_trace(o_vcd, i_trap_pc, "/top/proc0/csr0/i_trap_pc");
        sc_trace(o_vcd, o_mode, "/top/proc0/csr0/o_mode");
        sc_trace(o_vcd, o_ie, "/top/proc0/csr0/o_ie");
        sc_trace(o_vcd, o_rdata, "/top/proc0/csr0/o_rdata");
        sc_trace(o_vcd, o_mtvec, "/top/proc0/csr0/o_mtvec");
        sc_trace(o_vcd, i_dport_ena, "/top/proc0/csr0/i_dport_ena");
        sc_trace(o_vcd, i_dport_write, "/top/proc0/csr0/i_dport_write");
        sc_trace(o_vcd, i_dport_addr, "/top/proc0/csr0/i_dport_addr");
        sc_trace(o_vcd, i_dport_wdata, "/top/proc0/csr0/i_dport_wdata");
        sc_trace(o_vcd, o_dport_rdata, "/top/proc0/csr0/o_dport_rdata");
    }
}

void CsrRegs::procedure_RegAccess(uint64_t iaddr, bool iwena,
                                 sc_uint<RISCV_ARCH> iwdata,
                                 RegistersType &ir, RegistersType *ov,
                                 sc_uint<RISCV_ARCH> *ordata) {
    *ordata = 0;
    switch (iaddr) {
    case CSR_misa:
        /** Base[XLEN-1:XLEN-2]
         *      1 = 32
         *      2 = 64
         *      3 = 128
         */
        (*ordata)(RISCV_ARCH-1, RISCV_ARCH-2) = 2;
        /** BitCharacterDescription
         * 0  A Atomic extension
         * 1  B Tentatively reserved for Bit operations extension
         * 2  C Compressed extension
         * 3  D Double-precision Foating-point extension
         * 4  E RV32E base ISA (embedded)
         * 5  F Single-precision Foating-point extension
         * 6  G Additional standard extensions present
         * 7  H Hypervisor mode implemented
         * 8  I RV32I/64I/128I base ISA
         * 9  J Reserved
         * 10 K Reserved
         * 11 L Tentatively reserved for Decimal Floating-Point extension
         * 12 M Integer Multiply/Divide extension
         * 13 N User-level interrupts supported
         * 14 O Reserved
         * 15 P Tentatively reserved for Packed-SIMD extension
         * 16 Q Quad-precision Foating-point extension
         * 17 R Reserved
         * 18 S Supervisor mode implemented
         * 19 T Tentatively reserved for Transactional Memory extension
         * 20 U User mode implemented
         * 21 V Tentatively reserved for Vector extension
         * 22 W Reserved
         * 23 X Non-standard extensions present
         * 24 Y Reserved
         * 25 Z Reserve
         */
        //(*ordata)['A' - 'A'] = 1;
        (*ordata)['I' - 'A'] = 1;
        (*ordata)['M' - 'A'] = 1;
        (*ordata)['U' - 'A'] = 1;
        (*ordata)['C' - 'A'] = 1;
        break;
    case CSR_mvendorid:
        (*ordata) = CFG_VENDOR_ID;
        break;
    case CSR_marchid:
        break;
    case CSR_mimplementationid:
        (*ordata) = CFG_IMPLEMENTATION_ID;
        break;
    case CSR_mhartid:
        (*ordata)(63, 0) = hartid_;
        break;
    case CSR_uepc:// - User mode program counter
        break;
    case CSR_mstatus:// - Machine mode status register
        (*ordata)[0] = ir.uie;
        (*ordata)[3] = ir.mie;
        (*ordata)[7] = ir.mpie;
        (*ordata)(12, 11) = ir.mpp;
        if (iwena) {
            ov->uie = iwdata[0];
            ov->mie = iwdata[3];
            ov->mpie = iwdata[7];
            ov->mpp = iwdata(12, 11);
        }
        break;
    case CSR_medeleg:// - Machine exception delegation
        break;
    case CSR_mideleg:// - Machine itnerrupt delegation
        break;
    case CSR_mie:// - Machine interrupt enable bit
        break;
    case CSR_mtvec:
        (*ordata) = ir.mtvec;
        if (iwena) {
            ov->mtvec = iwdata;
        }
        break;
    case CSR_mtimecmp:// - Machine wall-clock timer compare value
        break;
    case CSR_mscratch:// - Machine scratch register
        (*ordata) = ir.mscratch;
        if (iwena) {
            ov->mscratch = iwdata;
        }
        break;
    case CSR_mepc:// - Machine program counter
        (*ordata) = ir.mepc;
        if (iwena) {
            ov->mepc = iwdata;
        }
        break;
    case CSR_mcause:// - Machine trap cause
        (*ordata) = 0;
        (*ordata)[63] = ir.trap_irq;
        (*ordata)(3, 0) = ir.trap_code;
        break;
    case CSR_mbadaddr:// - Machine bad address
        (*ordata) = ir.mbadaddr;
        break;
    case CSR_mip:// - Machine interrupt pending
        break;
    default:;
    }

}

void CsrRegs::comb() {
    sc_uint<RISCV_ARCH> wb_rdata = 0;
    sc_uint<RISCV_ARCH> wb_dport_rdata = 0;
    bool w_ie;
    bool w_dport_wena;

    v = r;

    w_dport_wena = i_dport_ena & i_dport_write;

    procedure_RegAccess(i_addr.read(), i_wena.read(), i_wdata.read(),
                        r, &v, &wb_rdata);

    procedure_RegAccess(i_dport_addr.read(), w_dport_wena,
                        i_dport_wdata.read(), r, &v, &wb_dport_rdata);


    if (i_addr.read() == CSR_mepc && i_xret.read()) {
        // Switch to previous mode
        v.mie = r.mpie;
        v.mpie = 1;
        v.mode = r.mpp;
        v.mpp = PRV_U;
    }

    if (i_trap_ena.read() && (i_break_mode.read() || !i_breakpoint.read())) {
        v.mie = 0;
        v.mpp = r.mode;
        v.mepc = i_trap_pc.read();
        v.mbadaddr = i_trap_pc.read();
        v.trap_code = i_trap_code.read()(3, 0);
        v.trap_irq = i_trap_code.read()[4];
        v.mode = PRV_M;
        switch (r.mode.read()) {
        case PRV_U:
            v.mpie = r.uie;
            break;
        case PRV_M:
            v.mpie = r.mie;
            break;
        default:;
        }
    }

    w_ie = 0;
    if ((r.mode.read() != PRV_M) || r.mie.read()) {
        w_ie = 1;
    }

    if (!i_nrst.read()) {
        v.mtvec = 0;
        v.mscratch = 0;
        v.mbadaddr = 0;
        v.mode = PRV_M;
        v.uie = 0;
        v.mie = 0;
        v.mpie = 0;
        v.mpp = 0;
        v.mepc = 0;
        v.trap_code = 0;
        v.trap_irq = 0;
    }


    o_rdata = wb_rdata;
    o_ie = w_ie;
    o_mode = r.mode;
    o_mtvec = r.mtvec.read()(BUS_ADDR_WIDTH - 1, 0);
    o_dport_rdata = wb_dport_rdata;
}

void CsrRegs::registers() {
    r = v;
}

}  // namespace debugger

