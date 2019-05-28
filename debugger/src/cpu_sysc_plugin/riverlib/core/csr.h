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

#ifndef __DEBUGGER_RIVERLIB_CSR_H__
#define __DEBUGGER_RIVERLIB_CSR_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(CsrRegs) {
    sc_in<bool> i_clk;                      // Clock signal
    sc_in<bool> i_nrst;                     // Reset (active low)
    sc_in<bool> i_mret;                     // mret instruction signals mode switching
    sc_in<bool> i_uret;                     // uret instruction signals mode switching
    sc_in<sc_uint<12>> i_addr;              // CSR address, if xret=1 switch mode accordingly
    sc_in<bool> i_wena;                     // Write enable
    sc_in<sc_uint<RISCV_ARCH>> i_wdata;     // CSR writing value
    sc_out<sc_uint<RISCV_ARCH>> o_rdata;    // CSR read value
    sc_in<bool> i_e_pre_valid;              // execute stage valid signal
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_ex_pc;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_ex_npc;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_ex_data_addr;  // Data path: address must be equal to the latest request address
    sc_in<bool> i_ex_data_load_fault;       // Data path: Bus response with SLVERR or DECERR on read
    sc_in<bool> i_ex_data_store_fault;      // Data path: Bus response with SLVERR or DECERR on write
    sc_in<bool> i_ex_ctrl_load_fault;
    sc_in<bool> i_ex_illegal_instr;
    sc_in<bool> i_ex_unalign_store;
    sc_in<bool> i_ex_unalign_load;
    sc_in<bool> i_ex_breakpoint;
    sc_in<bool> i_ex_ecall;
    sc_in<bool> i_irq_external;
    sc_out<bool> o_trap_valid;              // Trap pulse
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_trap_pc;

    sc_in<bool> i_break_mode;               // Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
    sc_out<bool> o_break_event;             // Breakpoint event to raise status flag in dport interfae

    sc_in<bool> i_dport_ena;                  // Debug port request is enabled
    sc_in<bool> i_dport_write;                // Debug port Write enable
    sc_in<sc_uint<12>> i_dport_addr;          // Debug port CSR address
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata; // Debug port CSR writing value
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;// Debug port CSR read value

    void comb();
    void registers();

    SC_HAS_PROCESS(CsrRegs);

    CsrRegs(sc_module_name name_, uint32_t hartid);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    struct RegistersType {
        sc_signal<sc_uint<RISCV_ARCH>> mtvec;
        sc_signal<sc_uint<RISCV_ARCH>> mscratch;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> mbadaddr;
        sc_signal<sc_uint<2>> mode;
        sc_signal<bool> uie;                    // User level interrupts ena for current priv. mode
        sc_signal<bool> mie;                    // Machine level interrupts ena for current priv. mode
        sc_signal<bool> mpie;                   // Previous MIE value
        sc_signal<sc_uint<2>> mpp;              // Previous mode
        sc_signal<sc_uint<RISCV_ARCH>> mepc;

        sc_signal<bool> trap_irq;
        sc_signal<sc_uint<4>> trap_code;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> trap_addr;
        sc_signal<bool> break_event;            // 1 clock pulse
    } v, r;
    uint32_t hartid_;

    void procedure_RegAccess(uint64_t iaddr, bool iwena,
                             sc_uint<RISCV_ARCH> iwdata,
                             RegistersType &ir, RegistersType *ov,
                             sc_uint<RISCV_ARCH> *ordata);
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CSR_H__
