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

namespace debugger {

/** (SignalIdx, 2'mode) */
static const uint64_t NMI_TABLE_ADDR[4*EXCEPTIONS_Total] = {
    // Exceptions:
    // User mode                    SuperUser mode                   Hypervisor mode               Machine Mode
    CFG_NMI_INSTR_UNALIGNED_ADDR,  CFG_NMI_INSTR_UNALIGNED_ADDR,  CFG_NMI_INSTR_UNALIGNED_ADDR,  CFG_NMI_INSTR_UNALIGNED_ADDR,
    CFG_NMI_INSTR_FAULT_ADDR,      CFG_NMI_INSTR_FAULT_ADDR,      CFG_NMI_INSTR_FAULT_ADDR,      CFG_NMI_INSTR_FAULT_ADDR,
    CFG_NMI_INSTR_ILLEGAL_ADDR,    CFG_NMI_INSTR_ILLEGAL_ADDR,    CFG_NMI_INSTR_ILLEGAL_ADDR,    CFG_NMI_INSTR_ILLEGAL_ADDR,
    CFG_NMI_BREAKPOINT_ADDR,       CFG_NMI_BREAKPOINT_ADDR,       CFG_NMI_BREAKPOINT_ADDR,       CFG_NMI_BREAKPOINT_ADDR,
    CFG_NMI_LOAD_UNALIGNED_ADDR,   CFG_NMI_LOAD_UNALIGNED_ADDR,   CFG_NMI_LOAD_UNALIGNED_ADDR,   CFG_NMI_LOAD_UNALIGNED_ADDR,
    CFG_NMI_LOAD_FAULT_ADDR,       CFG_NMI_LOAD_FAULT_ADDR,       CFG_NMI_LOAD_FAULT_ADDR,       CFG_NMI_LOAD_FAULT_ADDR,
    CFG_NMI_STORE_UNALIGNED_ADDR,  CFG_NMI_STORE_UNALIGNED_ADDR,  CFG_NMI_STORE_UNALIGNED_ADDR,  CFG_NMI_STORE_UNALIGNED_ADDR,
    CFG_NMI_STORE_FAULT_ADDR,      CFG_NMI_STORE_FAULT_ADDR,      CFG_NMI_STORE_FAULT_ADDR,      CFG_NMI_STORE_FAULT_ADDR,
    CFG_NMI_CALL_FROM_UMODE_ADDR,  CFG_NMI_CALL_FROM_SMODE_ADDR,  CFG_NMI_CALL_FROM_HMODE_ADDR,  CFG_NMI_CALL_FROM_MMODE_ADDR,
    CFG_NMI_INSTR_PAGE_FAULT_ADDR, CFG_NMI_INSTR_PAGE_FAULT_ADDR, CFG_NMI_INSTR_PAGE_FAULT_ADDR, CFG_NMI_INSTR_PAGE_FAULT_ADDR,
    CFG_NMI_LOAD_PAGE_FAULT_ADDR,  CFG_NMI_LOAD_PAGE_FAULT_ADDR,  CFG_NMI_LOAD_PAGE_FAULT_ADDR,  CFG_NMI_LOAD_PAGE_FAULT_ADDR,
    CFG_NMI_14_ADDR,               CFG_NMI_14_ADDR,               CFG_NMI_14_ADDR,               CFG_NMI_14_ADDR,
    CFG_NMI_STORE_PAGE_FAULT_ADDR, CFG_NMI_STORE_PAGE_FAULT_ADDR, CFG_NMI_STORE_PAGE_FAULT_ADDR, CFG_NMI_STORE_PAGE_FAULT_ADDR,
    CFG_NMI_STACK_OVERFLOW_ADDR,   CFG_NMI_STACK_OVERFLOW_ADDR,   CFG_NMI_STACK_OVERFLOW_ADDR,   CFG_NMI_STACK_OVERFLOW_ADDR,
    CFG_NMI_STACK_UNDERFLOW_ADDR,  CFG_NMI_STACK_UNDERFLOW_ADDR,  CFG_NMI_STACK_UNDERFLOW_ADDR,  CFG_NMI_STACK_UNDERFLOW_ADDR
};

static const uint64_t NMI_CAUSE_IDX[4*EXCEPTIONS_Total] = {
    EXCEPTION_InstrMisalign,    EXCEPTION_InstrMisalign,    EXCEPTION_InstrMisalign,    EXCEPTION_InstrMisalign,
    EXCEPTION_InstrFault,       EXCEPTION_InstrFault,       EXCEPTION_InstrFault,       EXCEPTION_InstrFault,
    EXCEPTION_InstrIllegal,     EXCEPTION_InstrIllegal,     EXCEPTION_InstrIllegal,     EXCEPTION_InstrIllegal,
    EXCEPTION_Breakpoint,       EXCEPTION_Breakpoint,       EXCEPTION_Breakpoint,       EXCEPTION_Breakpoint,
    EXCEPTION_LoadMisalign,     EXCEPTION_LoadMisalign,     EXCEPTION_LoadMisalign,     EXCEPTION_LoadMisalign,
    EXCEPTION_LoadFault,        EXCEPTION_LoadFault,        EXCEPTION_LoadFault,        EXCEPTION_LoadFault,
    EXCEPTION_StoreMisalign,    EXCEPTION_StoreMisalign,    EXCEPTION_StoreMisalign,    EXCEPTION_StoreMisalign,
    EXCEPTION_StoreFault,       EXCEPTION_StoreFault,       EXCEPTION_StoreFault,       EXCEPTION_StoreFault,
    EXCEPTION_CallFromUmode,    EXCEPTION_CallFromSmode,    EXCEPTION_CallFromHmode,    EXCEPTION_CallFromMmode,
    EXCEPTION_InstrPageFault,   EXCEPTION_InstrPageFault,   EXCEPTION_InstrPageFault,   EXCEPTION_InstrPageFault,
    EXCEPTION_LoadPageFault,    EXCEPTION_LoadPageFault,    EXCEPTION_LoadPageFault,    EXCEPTION_LoadPageFault,
    EXCEPTION_rsrv14,           EXCEPTION_rsrv14,           EXCEPTION_rsrv14,           EXCEPTION_rsrv14,
    EXCEPTION_StorePageFault,   EXCEPTION_StorePageFault,   EXCEPTION_StorePageFault,   EXCEPTION_StorePageFault,
    EXCEPTION_StackOverflow,    EXCEPTION_StackOverflow,    EXCEPTION_StackOverflow,    EXCEPTION_StackOverflow,
    EXCEPTION_StackUnderflow,   EXCEPTION_StackUnderflow,   EXCEPTION_StackUnderflow,   EXCEPTION_StackUnderflow,
};

CsrRegs::CsrRegs(sc_module_name name_, uint32_t hartid, bool async_reset)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_sp("i_sp"),
    i_req_valid("i_req_valid"),
    o_req_ready("o_req_ready"),
    i_req_type("i_req_type"),
    i_req_addr("i_req_addr"),
    i_req_data("i_req_data"),
    o_resp_valid("o_resp_valid"),
    i_resp_ready("i_resp_ready"),
    o_resp_data("o_resp_data"),
    o_resp_exception("o_resp_exception"),
    i_e_pc("i_e_pc"),
    i_e_npc("i_e_npc"),
    i_irq_external("i_irq_external"),
    o_irq_software("o_irq_software"),
    o_irq_timer("o_irq_timer"),
    o_irq_external("o_irq_external"),
    i_e_valid("i_e_valid"),
    o_executed_cnt("o_executed_cnt"),
    o_dbg_pc_write("o_dbg_pc_write"),
    o_dbg_pc("o_dbg_pc"),
    o_progbuf_ena("o_progbuf_ena"),
    o_progbuf_pc("o_progbuf_pc"),
    o_progbuf_data("o_progbuf_data"),
    o_flushi_ena("o_flushi_ena"),
    o_flushi_addr("o_flushi_addr"),
    o_mpu_region_we("o_mpu_region_we"),
    o_mpu_region_idx("o_mpu_region_idx"),
    o_mpu_region_addr("o_mpu_region_addr"),
    o_mpu_region_mask("o_mpu_region_mask"),
    o_mpu_region_flags("o_mpu_region_flags"),
    o_halt("o_halt") {
    hartid_ = hartid;
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_sp;
    sensitive << i_req_valid;
    sensitive << i_req_type;
    sensitive << i_req_addr;
    sensitive << i_req_data;
    sensitive << i_resp_ready;
    sensitive << i_e_pc;
    sensitive << i_e_npc;
    sensitive << i_irq_external;
    sensitive << i_e_valid;
    sensitive << r.state;
    sensitive << r.cmd_type;
    sensitive << r.cmd_addr;
    sensitive << r.cmd_data;
    sensitive << r.mtvec;
    sensitive << r.mscratch;
    sensitive << r.mstackovr;
    sensitive << r.mstackund;
    sensitive << r.mbadaddr;
    sensitive << r.mode;
    sensitive << r.uie;
    sensitive << r.mie;
    sensitive << r.mpie;
    sensitive << r.mstackovr_ena;
    sensitive << r.mstackund_ena;
    sensitive << r.mpp;
    sensitive << r.mepc;
    sensitive << r.uepc;
    sensitive << r.ext_irq;
    sensitive << r.ex_fpu_invalidop;
    sensitive << r.ex_fpu_divbyzero;
    sensitive << r.ex_fpu_overflow;
    sensitive << r.ex_fpu_underflow;
    sensitive << r.ex_fpu_inexact;
    sensitive << r.trap_irq;
    sensitive << r.trap_code;
    sensitive << r.trap_addr;
    sensitive << r.break_event;
    sensitive << r.hold_data_store_fault;
    sensitive << r.hold_data_load_fault;
    sensitive << r.hold_mbadaddr;
    sensitive << r.timer;
    sensitive << r.cycle_cnt;
    sensitive << r.executed_cnt;
    sensitive << r.break_mode;
    sensitive << r.halt;
    sensitive << r.halt_cause;
    sensitive << r.progbuf_ena;
    sensitive << r.progbuf_data;
    sensitive << r.progbuf_data_out;
    sensitive << r.progbuf_data_pc;
    sensitive << r.progbuf_data_npc;
    sensitive << r.progbuf_err;
    sensitive << r.stepping_mode;
    sensitive << r.stepping_mode_cnt;
    sensitive << r.ins_per_step;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void CsrRegs::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_sp, i_sp.name());
        sc_trace(o_vcd, i_req_valid, i_req_valid.name());
        sc_trace(o_vcd, o_req_ready, o_req_ready.name());
        sc_trace(o_vcd, i_req_type, i_req_type.name());
        sc_trace(o_vcd, i_req_addr, i_req_addr.name());
        sc_trace(o_vcd, i_req_data, i_req_data.name());
        sc_trace(o_vcd, o_resp_valid, o_resp_valid.name());
        sc_trace(o_vcd, i_resp_ready, i_resp_ready.name());
        sc_trace(o_vcd, o_resp_data, o_resp_data.name());
        sc_trace(o_vcd, o_resp_exception, o_resp_exception.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_npc, i_e_npc.name());
        sc_trace(o_vcd, i_irq_external, i_irq_external.name());
        sc_trace(o_vcd, o_irq_software, o_irq_software.name());
        sc_trace(o_vcd, o_irq_timer, o_irq_timer.name());
        sc_trace(o_vcd, o_irq_external, o_irq_external.name());
        sc_trace(o_vcd, o_executed_cnt, o_executed_cnt.name());
        sc_trace(o_vcd, o_dbg_pc_write, o_dbg_pc_write.name());
        sc_trace(o_vcd, o_dbg_pc, o_dbg_pc.name());
        sc_trace(o_vcd, o_progbuf_ena, o_progbuf_ena.name());
        sc_trace(o_vcd, o_progbuf_pc, o_progbuf_pc.name());
        sc_trace(o_vcd, o_progbuf_data, o_progbuf_data.name());
        sc_trace(o_vcd, o_flushi_ena, o_flushi_ena.name());
        sc_trace(o_vcd, o_flushi_addr, o_flushi_addr.name());
        sc_trace(o_vcd, o_halt, o_halt.name());

        std::string pn(name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.mode, pn + ".r_mode");
        sc_trace(o_vcd, r.mie, pn + ".r_mie");
        sc_trace(o_vcd, r.mepc, pn + ".r_mepc");
        sc_trace(o_vcd, r.mbadaddr, pn + ".r_mbadaddr");
        sc_trace(o_vcd, r.ext_irq, pn + ".r_ext_irq");
        sc_trace(o_vcd, r.trap_code, pn + ".r_trap_code");
        sc_trace(o_vcd, r.hold_data_load_fault, pn + ".r_hold_data_load_fault");
        sc_trace(o_vcd, r.break_mode, pn + ".r_break_mode");
        sc_trace(o_vcd, r.halt, pn + ".r_halt");
        sc_trace(o_vcd, r.halt_cause, pn + ".r_halt_cause");
        sc_trace(o_vcd, r.progbuf_ena, pn + ".r_progbuf_ena");
        //sc_trace(o_vcd, r.progbuf_data, pn + ".r_progbuf_data");
        sc_trace(o_vcd, r.progbuf_data_out, pn + ".r_progbuf_data_out");
        sc_trace(o_vcd, r.progbuf_data_pc, pn + ".r_progbuf_data_pc");
        sc_trace(o_vcd, r.progbuf_data_npc, pn + ".r_progbuf_data_npc");
        sc_trace(o_vcd, r.progbuf_err, pn + ".r_progbuf_err");
        sc_trace(o_vcd, r.stepping_mode, pn + ".r_stepping_mode");
        sc_trace(o_vcd, r.stepping_mode_cnt, pn + ".r_stepping_mode_cnt");
        sc_trace(o_vcd, r.ins_per_step, pn + ".r_ins_per_step");
        sc_trace(o_vcd, r.flushi_ena, pn + ".r_flushi_ena");
        sc_trace(o_vcd, r.flushi_addr, pn + ".r_flushi_addr");
    }
}

void CsrRegs::comb() {
    bool w_ie;
    bool w_ext_irq;
    bool w_trap_valid;
    //sc_uint<CFG_CPU_ADDR_BITS> wb_trap_pc;
    bool v_dbg_pc_write;
    sc_uint<CFG_CPU_ADDR_BITS> vb_dbg_pc;
    bool w_trap_irq;
    bool w_exception_xret;
    sc_uint<5> wb_trap_code;
    sc_uint<CFG_CPU_ADDR_BITS> wb_mbadaddr;
    bool w_mstackovr;
    bool w_mstackund;
    bool v_csr_rena;
    bool v_csr_wena;
    bool v_csr_trapreturn;
    sc_uint<RISCV_ARCH> vb_rdata;
    bool v_cur_halt;
    bool v_req_halt;
    bool v_req_resume;
    bool v_req_progbuf;
    bool v_clear_progbuferr;
    bool v_req_ready;
    bool v_resp_valid;

    v = r;

    vb_rdata = 0;
    v_dbg_pc_write = 0;
    vb_dbg_pc = 0;
    v_cur_halt = 0;
    v_req_halt = 0;
    v_req_resume = 0;
    v_req_progbuf = 0;
    v_clear_progbuferr = 0;
    v.flushi_ena = 0;
    v.flushi_addr = 0;
    v_req_ready = 0;
    v_resp_valid = 0;
    v_csr_rena = 0;
    v_csr_wena = 0;
    v_csr_trapreturn = 0;
    w_exception_xret = 0;
    w_trap_valid = 0;
    w_trap_irq = 0;
    wb_trap_code = 0;


    switch (r.state.read()) {
    case State_Idle:
        v_req_ready = 1;
        if (i_req_valid) {
            v.cmd_type = i_req_type;
            v.cmd_addr = i_req_addr;
            v.cmd_data = i_req_data;
            if (i_req_type.read()[CsrReq_ExceptionBit]) {
                v.state = State_Exception;
            } else if (i_req_type.read()[CsrReq_InterruptBit]) {
                v.state = State_Interrupt;
            } else if (i_req_type.read()[CsrReq_TrapReturnBit]) {
                v.state = State_TrapReturn;
            } else {
                v.state = State_RW;
            }
        }
        break;
    case State_Exception:
        v.state = State_Response;
        w_trap_valid = 1;
        wb_mbadaddr = r.cmd_data;
        wb_trap_code = NMI_CAUSE_IDX[4*r.cmd_addr.read() + r.mode.read().to_int()];
        w_trap_irq = 0;
        if (r.progbuf_ena.read() == 1) {
            v.progbuf_err = PROGBUF_ERR_EXCEPTION;
        }
        v.cmd_data = NMI_TABLE_ADDR[4*r.cmd_addr.read() + r.mode.read().to_int()];
        break;
    case State_Interrupt:
        v.state = State_Response;
        w_trap_valid = 1;
        wb_mbadaddr = r.cmd_data;
        wb_trap_code = 4*r.cmd_addr.read() + r.mode.read().to_int();
        w_trap_irq = 1;
        v.cmd_data = r.mtvec;
        break;
    case State_TrapReturn:
        v.state = State_Response;
        v_csr_trapreturn = 1;
        if (r.cmd_addr.read() == CSR_mepc) {
            v.cmd_data = r.mepc;
        } else if (r.cmd_addr.read() == CSR_hepc) {
            v.cmd_data = 0;
        } else if (r.cmd_addr.read() == CSR_sepc) {
            v.cmd_data = 0;
        } else if (r.cmd_addr.read() == CSR_uepc) {
            v.cmd_data = r.uepc;
        } else {
            v.cmd_data = 0;
        }
        break;
    case State_RW:
        v.state = State_Response;
        v_csr_rena = r.cmd_type.read()[CsrReq_ReadBit];
        v_csr_wena = r.cmd_type.read()[CsrReq_WriteBit];
        break;
    case State_Response:
        v_resp_valid = 1;
        if (i_resp_ready) {
            v.state = State_Idle;
        }
        break;
    default:;
    }

    if (v_csr_trapreturn) {
        if (r.mode.read() == PRV_M && r.cmd_addr.read() == CSR_mepc) {
            v.mie = r.mpie;
            v.mpie = 1;
            v.mode = r.mpp;
            v.mpp = PRV_U;
        } else if (r.mode.read() == PRV_U && r.cmd_addr.read() == CSR_uepc) {
            v.mie = r.mpie;
            v.mpie = 0;     // Interrupts in a user mode actually not supported
            v.mode = r.mpp;
            v.mpp = PRV_U;
        } else {
            wb_mbadaddr = i_e_pc.read();
            wb_trap_code = EXCEPTION_InstrIllegal;
            w_trap_irq = 0;
            if (r.progbuf_ena.read() == 1) {
                v.progbuf_err = PROGBUF_ERR_EXCEPTION;
            }
            v.cmd_data = NMI_TABLE_ADDR[4*EXCEPTION_InstrIllegal];
        }
    }

    // Behaviour on EBREAK instruction defined by 'i_break_mode':
    //     0 = halt;
    //     1 = generate trap
    if (w_trap_valid) {// && (r.break_mode.read() || !i_ex_breakpoint.read())) {
        v.mie = 0;
        v.mpp = r.mode;
        v.mepc = i_e_npc.read();
        v.mbadaddr = wb_mbadaddr;
        v.trap_code = wb_trap_code;
        v.trap_irq = w_trap_irq;
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


    if (v_csr_rena) {
        switch (r.cmd_addr.read()) {
        case CSR_fflags:
            vb_rdata[0] = r.ex_fpu_inexact;
            vb_rdata[1] = r.ex_fpu_underflow;
            vb_rdata[2] = r.ex_fpu_overflow;
            vb_rdata[3] = r.ex_fpu_divbyzero;
            vb_rdata[4] = r.ex_fpu_invalidop;
            break;
        case CSR_frm:
            if (CFG_HW_FPU_ENABLE) {
                vb_rdata(2, 0) = 0x4;  // Round mode: round to Nearest (RMM)
            }
            break;
        case CSR_fcsr:
            vb_rdata[0] = r.ex_fpu_inexact;
            vb_rdata[1] = r.ex_fpu_underflow;
            vb_rdata[2] = r.ex_fpu_overflow;
            vb_rdata[3] = r.ex_fpu_divbyzero;
            vb_rdata[4] = r.ex_fpu_invalidop;
            if (CFG_HW_FPU_ENABLE) {
                vb_rdata(7, 5) = 0x4;  // Round mode: round to Nearest (RMM)
            }
            break; 
        case CSR_misa:
            /** Base[XLEN-1:XLEN-2]
             *      1 = 32
             *      2 = 64
             *      3 = 128
             */
            vb_rdata(RISCV_ARCH-1, RISCV_ARCH-2) = 2;
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
            vb_rdata['A' - 'A'] = 1;
            vb_rdata['I' - 'A'] = 1;
            vb_rdata['M' - 'A'] = 1;
            vb_rdata['U' - 'A'] = 1;
            vb_rdata['C' - 'A'] = 1;
            if (CFG_HW_FPU_ENABLE) {
                vb_rdata['D' - 'A'] = 1;
            }
            break;
        case CSR_mvendorid:
            vb_rdata = CFG_VENDOR_ID;
            break;
        case CSR_marchid:
            break;
        case CSR_mimplementationid:
            vb_rdata = CFG_IMPLEMENTATION_ID;
            break;
        case CSR_mhartid:
            vb_rdata(63, 0) = hartid_;
            break;
        case CSR_uepc:// - User mode program counter
            vb_rdata = r.uepc;
            break;
        case CSR_mstatus:// - Machine mode status register
            vb_rdata[0] = r.uie;
            vb_rdata[3] = r.mie;
            vb_rdata[7] = r.mpie;
            vb_rdata(12, 11) = r.mpp;
            if (CFG_HW_FPU_ENABLE) {
                vb_rdata(14, 13) = 0x1;   // FS field: Initial state
            }
            vb_rdata(33, 32) = 0x2;       // UXL: User mode supported 64-bits
            break;
        case CSR_medeleg:// - Machine exception delegation
            break;
        case CSR_mideleg:// - Machine interrupt delegation
            break;
        case CSR_mie:// - Machine interrupt enable bit
            break;
        case CSR_mtvec:
            vb_rdata = r.mtvec;
            break;
        case CSR_mscratch:// - Machine scratch register
            vb_rdata = r.mscratch;
            break;
        case CSR_mepc:// - Machine program counter
            vb_rdata = r.mepc;
            break;
        case CSR_mcause:// - Machine trap cause
            vb_rdata = 0;
            vb_rdata[63] = r.trap_irq;
            vb_rdata(4, 0) = r.trap_code;
            break;
        case CSR_mbadaddr:// - Machine bad address
            vb_rdata = r.mbadaddr;
            break;
        case CSR_mip:// - Machine interrupt pending
            break;
        case CSR_cycle:
            vb_rdata = r.cycle_cnt;
            break;
        case CSR_time:
            vb_rdata = r.timer;
            break;
        case CSR_insret:
            vb_rdata = r.executed_cnt;
            break;
        case CSR_mstackovr:// - Machine Stack Overflow
            vb_rdata = r.mstackovr;
            break;
        case CSR_mstackund:// - Machine Stack Underflow
            vb_rdata = r.mstackund;
            break;
        case CSR_mpu_addr:  // [WO] MPU address
            break;
        case CSR_mpu_mask:  // [WO] MPU mask
            break;
        case CSR_mpu_ctrl:  // [WO] MPU flags and write ena
            vb_rdata = CFG_MPU_TBL_SIZE << 8;
            break;
        case CSR_runcontrol:
            break;
        case CSR_insperstep:
            vb_rdata = r.ins_per_step;
            break;
        case CSR_progbuf:
            break;
        case CSR_abstractcs:
            vb_rdata(28,24) = CFG_PROGBUF_REG_TOTAL;
            vb_rdata[12] = r.progbuf_ena;       // busy
            vb_rdata(10,8) = r.progbuf_err;
            vb_rdata(3,0) = CFG_DATA_REG_TOTAL;
            break;
        case CSR_dcsr:
            vb_rdata(31,28) = 4;          // xdebugver: 4=External debug supported
            vb_rdata(8,6) = r.halt_cause;    // cause:
            vb_rdata[2] = r.stepping_mode;   // step:
            vb_rdata(1,0) = 3;            // prv: privilege in debug mode: 3=machine
            break;
        case CSR_dpc:
            // Upon entry into debug mode DPC must contains:
            //       cause        |   Address
            // -------------------|----------------
            // ebreak             |  Address of ebreak instruction
            // single step        |  Address of next instruction to be executed
            // trigger (HW BREAK) |  if timing=0, cause isntruction, if timing=1 enxt instruction
            // halt request       |  next instruction
            //
            if (r.halt_cause.read() == HALT_CAUSE_EBREAK) {
                vb_rdata(CFG_CPU_ADDR_BITS-1, 0) = i_e_pc.read();
            } else {
                vb_rdata(CFG_CPU_ADDR_BITS-1, 0) = i_e_npc.read();
            }
            break;
        default:;
        }
        v.cmd_data = vb_rdata;
    }
    if (v_csr_wena) {
        switch (r.cmd_addr.read()) {
        case CSR_uepc:// - User mode program counter
            v.uepc = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0);
            break;
        case CSR_mstatus:// - Machine mode status register
            v.uie = r.cmd_data.read()[0];
            v.mie = r.cmd_data.read()[3];
            v.mpie = r.cmd_data.read()[7];
            v.mpp = r.cmd_data.read()(12, 11);
            break;
        case CSR_medeleg:// - Machine exception delegation
            break;
        case CSR_mideleg:// - Machine interrupt delegation
            break;
        case CSR_mie:// - Machine interrupt enable bit
            break;
        case CSR_mtvec:
            v.mtvec = r.cmd_data.read();
            break;
        case CSR_mscratch:// - Machine scratch register
            v.mscratch = r.cmd_data.read();
            break;
        case CSR_mepc:// - Machine program counter
            v.mepc = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0);
            break;
        case CSR_mip:// - Machine interrupt pending
            break;
        case CSR_mstackovr:// - Machine Stack Overflow
            v.mstackovr = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0);
            v.mstackovr_ena = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0).or_reduce();
            break;
        case CSR_mstackund:// - Machine Stack Underflow
            v.mstackund = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0);
            v.mstackund_ena = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0).or_reduce();
            break;
        case CSR_mpu_addr:  // [WO] MPU address
            v.mpu_addr = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0);
            break;
        case CSR_mpu_mask:  // [WO] MPU mask
            v.mpu_mask = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0);
            break;
        case CSR_mpu_ctrl:  // [WO] MPU flags and write ena
            v.mpu_idx = r.cmd_data.read()(8+CFG_MPU_TBL_WIDTH-1, 8);
            v.mpu_flags = r.cmd_data.read()(CFG_MPU_FL_TOTAL-1, 0);
            v.mpu_we = 1;
            break;
        case CSR_runcontrol:
            v_req_halt = r.cmd_data.read()[31];
            v_req_resume = r.cmd_data.read()[30];
            if (r.cmd_data.read()[27] == 1) {
                if (r.halt.read() == 1) {
                    v_req_progbuf = 1;
                } else {
                    v.progbuf_err = PROGBUF_ERR_HALT_RESUME;
                }
            }
            break;
        case CSR_insperstep:
            v.ins_per_step = r.cmd_data.read();
            if (r.cmd_data.read().or_reduce() == 0) {
                v.ins_per_step = 1;  // cannot be zero
            }
            if (r.halt.read() == 1) {
                v.stepping_mode_cnt = r.cmd_data.read();
            }
            break;
        case CSR_progbuf:
            if (v_csr_wena == 1) {
                int tidx = r.cmd_data.read()(35,32);
                sc_biguint<CFG_PROGBUF_REG_TOTAL*32> t2 = r.progbuf_data;
                t2(32*tidx+31, 32*tidx) = r.cmd_data.read()(31,0);
                v.progbuf_data = t2;
            }
            break;
        case CSR_abstractcs:
            v_clear_progbuferr = r.cmd_data.read()[8];   // W1C err=1
            break;
        case CSR_flushi:
            v.flushi_ena = 1;
            v.flushi_addr = r.cmd_data.read()(CFG_CPU_ADDR_BITS-1, 0);
            break;
        case CSR_dcsr:
            v.stepping_mode = r.cmd_data.read()[2];
            if (r.cmd_data.read()[2] == 1) {
                v.stepping_mode_cnt = r.ins_per_step;  // default =1
            }
            break;
        case CSR_dpc:
            v_dbg_pc_write = 1;
            vb_dbg_pc = r.cmd_data.read();
            break;
        default:;
        }
    }

    if (r.mpu_we.read() == 1) {
        v.mpu_we = 0;
    }

    w_ie = 0;
    if ((r.mode.read() != PRV_M) || r.mie.read()) {
        w_ie = 1;
    }
    w_ext_irq = i_irq_external.read() && w_ie;
    //if (i_trap_ready.read()) {
    //    v.ext_irq = w_ext_irq;
    //}

    w_mstackovr = 0;
    if (i_sp.read()(CFG_CPU_ADDR_BITS-1, 0) < r.mstackovr.read()) {
        w_mstackovr = 1;
    }

    w_mstackund = 0;
    if (i_sp.read()(CFG_CPU_ADDR_BITS-1, 0) > r.mstackund.read()) {
        w_mstackund = 1;
    }

#if 0
    if (i_fpu_valid.read()) {
        v.ex_fpu_invalidop = i_ex_fpu_invalidop.read();
        v.ex_fpu_divbyzero = i_ex_fpu_divbyzero.read();
        v.ex_fpu_overflow = i_ex_fpu_overflow.read();
        v.ex_fpu_underflow = i_ex_fpu_underflow.read();
        v.ex_fpu_inexact = i_ex_fpu_inexact.read();
    }

    w_trap_valid = 0;
    w_trap_irq = 0;
    wb_trap_code = 0;
    v.break_event = 0;
    wb_trap_pc = r.mtvec.read()(CFG_CPU_ADDR_BITS-1, 0);
    wb_mbadaddr = i_e_npc.read();

    if (i_ex_instr_load_fault.read() == 1) {
        w_trap_valid = 1;
        wb_trap_pc = CFG_NMI_INSTR_FAULT_ADDR;
        wb_trap_code = EXCEPTION_InstrFault;
        // illegal address instruction can generate any other exceptions
        v.hold_data_load_fault = 0;
        v.hold_data_store_fault = 0;
    } else if (i_ex_illegal_instr.read() == 1 || w_exception_xret == 1) {
        w_trap_valid = 1;
        wb_trap_pc = CFG_NMI_INSTR_ILLEGAL_ADDR;
        wb_trap_code = EXCEPTION_InstrIllegal;
        // illegal instruction can generate any other exceptions
        v.hold_data_load_fault = 0;
        v.hold_data_store_fault = 0;
    } else if (i_ex_breakpoint.read() == 1) {
        v.break_event = 1;
        w_trap_valid = 1;
        wb_trap_code = EXCEPTION_Breakpoint;
        if (r.break_mode.read() == 0) {
            wb_trap_pc = i_e_npc;
        } else {
            wb_trap_pc = CFG_NMI_BREAKPOINT_ADDR;
        }
    } else if (i_ex_unalign_load.read() == 1) {
        w_trap_valid = 1;
        wb_trap_pc = CFG_NMI_LOAD_UNALIGNED_ADDR;
        wb_trap_code = EXCEPTION_LoadMisalign;
    } else if (i_ex_data_load_fault.read() == 1 ||
                r.hold_data_load_fault.read() == 1) {
        w_trap_valid = 1;
        v.hold_data_load_fault = 0;
        if (i_trap_ready.read() == 0) {
            v.hold_data_load_fault = 1;
        }
        wb_trap_pc = CFG_NMI_LOAD_FAULT_ADDR;
        if (i_ex_data_load_fault.read() == 1) {
            wb_mbadaddr = i_ex_data_addr.read();    // miss-access read
            v.hold_mbadaddr = i_ex_data_addr.read();
        } else {
            wb_mbadaddr = r.hold_mbadaddr;
        }
        wb_trap_code = EXCEPTION_LoadFault;
    } else if (i_ex_unalign_store.read() == 1) {
        w_trap_valid = 1;
        wb_trap_pc = CFG_NMI_STORE_UNALIGNED_ADDR;
        wb_trap_code = EXCEPTION_StoreMisalign;
    } else if (i_ex_data_store_fault.read() == 1
             || r.hold_data_store_fault.read() == 1) {
        w_trap_valid = 1;
        v.hold_data_store_fault = 0;
        if (i_trap_ready.read() == 0) {
            v.hold_data_store_fault = 1;
        }
        wb_trap_pc = CFG_NMI_STORE_FAULT_ADDR;
        if (i_ex_data_store_fault.read() == 1) {
            wb_mbadaddr = i_ex_data_store_fault_addr.read();  // miss-access write
            v.hold_mbadaddr = i_ex_data_store_fault_addr.read();
        } else {
            wb_mbadaddr = r.hold_mbadaddr;
        }
        wb_trap_code = EXCEPTION_StoreFault;
    } else if (i_ex_ecall.read() == 1) {
        w_trap_valid = 1;
        if (r.mode.read() == PRV_M) {
            wb_trap_pc = CFG_NMI_CALL_FROM_MMODE_ADDR;
            wb_trap_code = EXCEPTION_CallFromMmode;
        } else {
            wb_trap_pc = CFG_NMI_CALL_FROM_UMODE_ADDR;
            wb_trap_code = EXCEPTION_CallFromUmode;
        }
    } else if (i_ex_instr_not_executable.read() == 1) {
        w_trap_valid = 1;
        wb_trap_pc = CFG_NMI_INSTR_PAGE_FAULT_ADDR;
        wb_trap_code = EXCEPTION_InstrPageFault;
    } else if (r.mstackovr_ena.read() == 1 && w_mstackovr == 1) {
        w_trap_valid = 1;
        wb_trap_pc = CFG_NMI_STACK_OVERFLOW_ADDR;
        wb_trap_code = EXCEPTION_StackOverflow;
        if (i_trap_ready.read() == 1) {
            v.mstackovr = 0;
            v.mstackovr_ena = 0;
        }
    } else if (r.mstackund_ena.read() == 1 && w_mstackund == 1) {
        w_trap_valid = 1;
        wb_trap_pc = CFG_NMI_STACK_UNDERFLOW_ADDR;
        wb_trap_code = EXCEPTION_StackUnderflow;
        if (i_trap_ready.read() == 1) {
            v.mstackund = 0;
            v.mstackund_ena = 0;
        }
    } else if (w_ext_irq == 1 && r.ext_irq.read() == 0) {
        w_trap_valid = 1;
        wb_trap_pc = r.mtvec.read()(CFG_CPU_ADDR_BITS-1, 0);
        wb_trap_code = 0xB;
        w_trap_irq = 1;
    }


    // Behaviour on EBREAK instruction defined by 'i_break_mode':
    //     0 = halt;
    //     1 = generate trap
    if (w_trap_valid && i_trap_ready.read() &&
        (r.break_mode.read() || !i_ex_breakpoint.read())) {
        v.mie = 0;
        v.mpp = r.mode;
        v.mepc = i_ex_npc.read();
        v.mbadaddr = wb_mbadaddr;
        v.trap_code = wb_trap_code;
        v.trap_irq = w_trap_irq;
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
#endif


    if (r.halt.read() == 0) {
        v.cycle_cnt = r.cycle_cnt.read() + 1;
    }
    if (i_e_valid.read()) {
        v.executed_cnt = r.executed_cnt.read() + 1;
    }
    v.timer = r.timer.read() + 1;

    if (i_e_valid.read()) {
        if (r.progbuf_ena.read() == 1) {
            int t1 = 16*r.progbuf_data_npc.read();
            v.progbuf_data_out = r.progbuf_data.read()(t1 + 31, t1).to_uint();
            v.progbuf_data_pc = r.progbuf_data_npc.read();
            if (r.progbuf_data.read()(t1 + 1, t1) == 0x3) {
                v.progbuf_data_npc = r.progbuf_data_npc.read() + 2;
            } else {
                v.progbuf_data_npc = r.progbuf_data_npc.read() + 1;
            }
            if (r.progbuf_data_pc.read()(4, 1).and_reduce() == 1) {  // use end of buffer as a watchdog
                v.progbuf_ena = 0;
                v.halt        = 1;
            }
        } else if (r.stepping_mode_cnt.read().or_reduce() == 1) {
            v.stepping_mode_cnt = r.stepping_mode_cnt.read() - 1;
            if (r.stepping_mode_cnt.read()(RISCV_ARCH-1, 1).or_reduce() == 0) {
                v.halt = 1;
                v_cur_halt = 1;
                v.stepping_mode = 0;
                v.halt_cause = HALT_CAUSE_STEP;
            }
        }
    }

    if (r.break_event.read() == 1) {
        if (r.progbuf_ena.read() == 1) {
            v.halt = 1;  // do not modify halt cause in debug mode
            v.progbuf_ena = 0;
        } else {
            if (r.break_mode.read() == 0) {
                v.halt = 1;
                v.halt_cause = HALT_CAUSE_EBREAK;
            }
        }
    } else if (v_req_halt == 1 && r.halt.read() == 0) {
        if (r.progbuf_ena.read() == 0 && r.stepping_mode == 0) {
            v.halt = 1;
            v.halt_cause = HALT_CAUSE_HALTREQ;
        }
    } else if (v_req_progbuf == 1) {
        v.progbuf_ena = 1;
        v.progbuf_data_out = r.progbuf_data.read()(31,0).to_uint();
        v.progbuf_data_pc = 0;
        if (r.progbuf_data.read()(1,0).to_uint() == 0x3) {
            v.progbuf_data_npc = 2;
        } else {
            v.progbuf_data_npc = 1;
        }
        v.halt = 0;
    } else if (v_req_resume == 1 && r.halt.read() == 1) {
        v.halt = 0;
    }

    if (v_clear_progbuferr == 1) {
        v.progbuf_err = PROGBUF_ERR_NONE;
    }
    //else if (r.progbuf_ena.read() == 1) {
    //    if (i_ex_data_load_fault.read() == 1
    //        || i_ex_data_store_fault.read() == 1) {
    //        v.progbuf_err = PROGBUF_ERR_EXCEPTION;
    //    } else if (i_ex_unalign_store.read() == 1
    //            || i_ex_unalign_load.read() == 1
    //            || i_ex_mpu_store.read() == 1
    //            || i_ex_mpu_load.read() == 1) {
    //        v.progbuf_err = PROGBUF_ERR_BUS;
    //    }
    //}

    if (!async_reset_ && !i_nrst.read()) {
        R_RESET(v);
    }

    o_req_ready = v_req_ready;
    o_resp_valid = v_resp_valid;
    o_resp_data = r.cmd_data;
    o_resp_exception = 0;

    o_irq_software = 0;
    o_irq_timer = 0;
    o_irq_external = 0;

    o_executed_cnt = r.executed_cnt;
    o_dbg_pc_write = v_dbg_pc_write;
    o_dbg_pc = vb_dbg_pc;
    o_mpu_region_we = r.mpu_we;
    o_mpu_region_idx = r.mpu_idx;
    o_mpu_region_addr = r.mpu_addr;
    o_mpu_region_mask = r.mpu_mask;
    o_mpu_region_flags = r.mpu_flags;
    o_progbuf_ena = r.progbuf_ena;
    o_progbuf_pc = r.progbuf_data_pc.read() << 1;
    o_progbuf_data = r.progbuf_data_out;
    o_flushi_ena = r.flushi_ena;
    o_flushi_addr = r.flushi_addr;
    o_halt = r.halt | v_cur_halt;
}

void CsrRegs::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        R_RESET(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

