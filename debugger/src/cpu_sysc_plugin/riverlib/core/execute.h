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

#ifndef __DEBUGGER_RIVERLIB_EXECUTE_H__
#define __DEBUGGER_RIVERLIB_EXECUTE_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "arith/int_div.h"
#include "arith/int_mul.h"
#include "arith/shift.h"
#include "fpu_d/fpu_top.h"

namespace debugger {

SC_MODULE(InstrExecute) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                         // Reset active LOW
    sc_in<bool> i_d_valid;                      // Decoded instruction is valid
    sc_in<sc_uint<6>> i_d_radr1;
    sc_in<sc_uint<6>> i_d_radr2;
    sc_in<sc_uint<6>> i_d_waddr;
    sc_in<sc_uint<RISCV_ARCH>> i_d_imm;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_d_pc;      // Instruction pointer on decoded instruction
    sc_in<sc_uint<32>> i_d_instr;               // Decoded instruction value
    sc_in<sc_uint<6>> i_wb_waddr;               // write back address
    sc_in<bool> i_memop_store;                  // Store to memory operation
    sc_in<bool> i_memop_load;                   // Load from memoru operation
    sc_in<bool> i_memop_sign_ext;               // Load memory value with sign extending
    sc_in<sc_uint<2>> i_memop_size;             // Memory transaction size
    sc_in<bool> i_unsigned_op;                  // Unsigned operands
    sc_in<bool> i_rv32;                         // 32-bits instruction
    sc_in<bool> i_compressed;                   // C-extension (2-bytes length)
    sc_in<bool> i_f64;                          // D-extension (FPU)
    sc_in<sc_bv<ISA_Total>> i_isa_type;         // Type of the instruction's structure (ISA spec.)
    sc_in<sc_bv<Instr_Total>> i_ivec;           // One pulse per supported instruction.
    sc_in<bool> i_unsup_exception;              // Unsupported instruction exception
    sc_in<bool> i_instr_load_fault;             // fault instruction's address
    sc_in<bool> i_instr_executable;             // MPU flag

    sc_in<bool> i_dport_npc_write;              // Write npc value from debug port
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_dport_npc; // Debug port npc value to write

    sc_in<sc_uint<RISCV_ARCH>> i_rdata1;        // Integer/Float register value 1
    sc_in<bool> i_rhazard1;
    sc_in<sc_uint<RISCV_ARCH>> i_rdata2;        // Integer/Float register value 2
    sc_in<bool> i_rhazard2;
    sc_in<sc_uint<4>> i_wtag;
    sc_out<bool> o_wena;
    sc_out<sc_uint<6>> o_waddr;                 // Address to store result of the instruction (0=do not store)
    sc_out<bool> o_whazard;
    sc_out<sc_uint<RISCV_ARCH>> o_wdata;        // Value to store
    sc_out<sc_uint<4>> o_wtag;
    sc_out<bool> o_d_ready;                     // Hold pipeline while 'writeback' not done or multi-clock instruction.
    sc_out<sc_uint<12>> o_csr_addr;             // CSR address. 0 if not a CSR instruction with xret signals mode switching
    sc_out<bool> o_csr_wena;                    // Write new CSR value
    sc_in<sc_uint<RISCV_ARCH>> i_csr_rdata;     // CSR current value
    sc_out<sc_uint<RISCV_ARCH>> o_csr_wdata;    // CSR new value
    sc_in<bool> i_trap_valid;                   // async trap event
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_trap_pc;   // jump to address

    // exceptions:
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_ex_npc;   // npc on before trap
    sc_out<bool> o_ex_instr_load_fault;         // fault instruction's address
    sc_out<bool> o_ex_instr_not_executable;     // MPU prohibit this instruction
    sc_out<bool> o_ex_illegal_instr;
    sc_out<bool> o_ex_unalign_store;
    sc_out<bool> o_ex_unalign_load;
    sc_out<bool> o_ex_breakpoint;
    sc_out<bool> o_ex_ecall;
    sc_out<bool> o_ex_fpu_invalidop;            // FPU Exception: invalid operation
    sc_out<bool> o_ex_fpu_divbyzero;            // FPU Exception: divide by zero
    sc_out<bool> o_ex_fpu_overflow;             // FPU Exception: overflow
    sc_out<bool> o_ex_fpu_underflow;            // FPU Exception: underflow
    sc_out<bool> o_ex_fpu_inexact;              // FPU Exception: inexact
    sc_out<bool> o_fpu_valid;                   // FPU output is valid

    sc_out<bool> o_memop_sign_ext;              // Load data with sign extending
    sc_out<bool> o_memop_load;                  // Load data instruction
    sc_out<bool> o_memop_store;                 // Store data instruction
    sc_out<sc_uint<2>> o_memop_size;            // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_memop_addr;// Memory access address
    sc_out<sc_uint<RISCV_ARCH>> o_memop_wdata;
    sc_out<sc_uint<6>> o_memop_waddr;
    sc_out<sc_uint<4>> o_memop_wtag;
    sc_in<bool> i_memop_ready;

    sc_out<bool> o_trap_ready;                  // trap branch request accepted
    sc_out<bool> o_valid;                       // Output is valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_pc;       // Valid instruction pointer
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_npc;      // Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    sc_out<sc_uint<32>> o_instr;                // Valid instruction value
    sc_out<bool> o_fence;                                                // 
    sc_out<bool> o_fencei;                                               // 
    sc_out<bool> o_call;                        // CALL pseudo instruction detected
    sc_out<bool> o_ret;                         // RET pseudoinstruction detected
    sc_out<bool> o_mret;                        // MRET.
    sc_out<bool> o_uret;                        // MRET.
    sc_out<bool> o_multi_ready;

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrExecute);

    InstrExecute(sc_module_name name_, bool async_reset, bool fpu_ena);
    virtual ~InstrExecute();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    enum EMultiCycleInstruction {
        Multi_MUL,
        Multi_DIV,
        Multi_FPU,
        Multi_Total
    };

    struct multi_arith_type {
        sc_signal<sc_uint<RISCV_ARCH>> arr[Multi_Total];
    };

    struct RegistersType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> npc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<6>> memop_waddr;
        sc_signal<sc_uint<4>> memop_wtag;
        sc_signal<sc_uint<RISCV_ARCH>> wval;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_store;
        bool memop_sign_ext;
        sc_uint<2> memop_size;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> memop_addr;
        sc_signal<sc_uint<RISCV_ARCH>> memop_wdata;

        sc_signal<bool> valid;
        sc_signal<bool> call;
        sc_signal<bool> ret;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.pc = 0;
        iv.npc = CFG_NMI_RESET_VECTOR;
        iv.instr = 0;
        iv.memop_waddr = 0;
        iv.memop_wtag = 0;
        iv.wval = 0;
        iv.memop_load = 0;
        iv.memop_store = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = 0;
        iv.memop_addr = 0;
        iv.memop_wdata = 0;

        iv.valid = 0;
        iv.call = 0;
        iv.ret = 0;
    }

    multi_arith_type wb_arith_res;
    sc_signal<bool> w_arith_ena[Multi_Total];
    sc_signal<bool> w_arith_valid[Multi_Total];
    sc_signal<bool> w_arith_busy[Multi_Total];
    sc_signal<bool> w_arith_residual_high;
    sc_signal<sc_bv<Instr_FPU_Total>> wb_fpu_vec;
    bool w_exception_store;
    bool w_exception_load;
    bool w_next_ready;
    bool w_multi_ena;
    bool w_multi_ready;
    bool w_multi_busy;
    bool w_hold_multi;
    bool w_hold_hazard;
    bool w_hold_memop;

    sc_signal<sc_uint<RISCV_ARCH>> wb_rdata1;
    sc_signal<sc_uint<RISCV_ARCH>> wb_rdata2;

    sc_signal<sc_uint<RISCV_ARCH>> wb_shifter_a1;      // Shifters operand 1
    sc_signal<sc_uint<6>> wb_shifter_a2;               // Shifters operand 2
    sc_signal<sc_uint<RISCV_ARCH>> wb_sll;
    sc_signal<sc_uint<RISCV_ARCH>> wb_sllw;
    sc_signal<sc_uint<RISCV_ARCH>> wb_srl;
    sc_signal<sc_uint<RISCV_ARCH>> wb_srlw;
    sc_signal<sc_uint<RISCV_ARCH>> wb_sra;
    sc_signal<sc_uint<RISCV_ARCH>> wb_sraw;

    IntMul *mul0;
    IntDiv *div0;
    Shifter *sh0;
    FpuTop *fpu0;

    bool async_reset_;
    bool fpu_ena_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
