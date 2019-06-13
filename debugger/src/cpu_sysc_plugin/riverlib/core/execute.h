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
#include "fpu_d/fadd_d.h"

namespace debugger {

SC_MODULE(InstrExecute) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                         // Reset active LOW
    sc_in<bool> i_pipeline_hold;                // Hold execution by any reason
    sc_in<bool> i_d_valid;                      // Decoded instruction is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_d_pc;      // Instruction pointer on decoded instruction
    sc_in<sc_uint<32>> i_d_instr;               // Decoded instruction value
    sc_in<bool> i_wb_done;                      // write back done (Used to clear hazardness)
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
    sc_in<bool> i_dport_npc_write;              // Write npc value from debug port
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_dport_npc; // Debug port npc value to write

    sc_out<sc_uint<6>> o_radr1;                 // Integer/float register index 1
    sc_in<sc_uint<RISCV_ARCH>> i_rdata1;        // Integer register value 1
    sc_out<sc_uint<6>> o_radr2;                 // Integer/float register index 2
    sc_in<sc_uint<RISCV_ARCH>> i_rdata2;        // Integer register value 2
    sc_in<sc_uint<RISCV_ARCH>> i_rfdata1;       // Float register value 1
    sc_in<sc_uint<RISCV_ARCH>> i_rfdata2;       // Float register value 2
    sc_out<sc_uint<6>> o_res_addr;              // Address to store result of the instruction (0=do not store)
    sc_out<sc_uint<RISCV_ARCH>> o_res_data;     // Value to store
    sc_out<bool> o_pipeline_hold;               // Hold pipeline while 'writeback' not done or multi-clock instruction.
    sc_out<sc_uint<12>> o_csr_addr;             // CSR address. 0 if not a CSR instruction with xret signals mode switching
    sc_out<bool> o_csr_wena;                    // Write new CSR value
    sc_in<sc_uint<RISCV_ARCH>> i_csr_rdata;     // CSR current value
    sc_out<sc_uint<RISCV_ARCH>> o_csr_wdata;    // CSR new value
    sc_in<bool> i_trap_valid;                   // async trap event
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_trap_pc;   // jump to address

    // exceptions:
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_ex_npc;   // npc on before trap
    sc_out<bool> o_ex_illegal_instr;
    sc_out<bool> o_ex_unalign_store;
    sc_out<bool> o_ex_unalign_load;
    sc_out<bool> o_ex_breakpoint;
    sc_out<bool> o_ex_ecall;

    sc_out<bool> o_memop_sign_ext;              // Load data with sign extending
    sc_out<bool> o_memop_load;                  // Load data instruction
    sc_out<bool> o_memop_store;                 // Store data instruction
    sc_out<sc_uint<2>> o_memop_size;            // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_memop_addr;// Memory access address

    sc_out<bool> o_pre_valid;                   // pre-latch of valid
    sc_out<bool> o_valid;                       // Output is valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_pc;       // Valid instruction pointer
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_npc;      // Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    sc_out<sc_uint<32>> o_instr;                // Valid instruction value
    sc_out<bool> o_call;                        // CALL pseudo instruction detected
    sc_out<bool> o_ret;                         // RET pseudoinstruction detected
    sc_out<bool> o_mret;                        // MRET.
    sc_out<bool> o_uret;                        // MRET.

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrExecute);

    InstrExecute(sc_module_name name_);
    virtual ~InstrExecute();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    enum EMultiCycleInstruction {
        Multi_MUL,
        Multi_DIV,
        Multi_FADD_D,
        Multi_Total
    };

    struct multi_arith_type {
        sc_signal<sc_uint<RISCV_ARCH>> arr[Multi_Total];
    };

    struct RegistersType {
        sc_signal<bool> d_valid;                        // Valid decoded instruction latch
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> npc;
        sc_signal<sc_uint<32>> instr;
        sc_uint<6> res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> res_val;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_store;
        bool memop_sign_ext;
        sc_uint<2> memop_size;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> memop_addr;

        sc_signal<sc_uint<6>> multi_res_addr;           // latched output reg. address while multi-cycle instruction
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> multi_pc;    // latched pc-value while multi-cycle instruction
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> multi_npc;   // latched npc-value while multi-cycle instruction
        sc_signal<sc_uint<32>> multi_instr;             // Multi-cycle instruction is under processing
        sc_signal<bool> multi_ena[Multi_Total];         // Enable pulse for Operation that takes more than 1 clock
        sc_signal<bool> multi_rv32;                     // Long operation with 32-bits operands
        sc_signal<bool> multi_f64;                      // Long float operation
        sc_signal<bool> multi_unsigned;                 // Long operation with unsiged operands
        sc_signal<bool> multi_residual_high;            // Flag for Divider module: 0=divsion output; 1=residual output
                                                        // Flag for multiplier: 0=usual; 1=get high bits
        sc_signal<bool> multi_fadd_d;
        sc_signal<bool> multi_fsub_d;
        sc_signal<bool> multi_feq_d;
        sc_signal<bool> multi_fle_d;
        sc_signal<bool> multi_flt_d;
        sc_signal<bool> multi_fmax_d;
        sc_signal<bool> multi_fmin_d;
        sc_signal<bool> multiclock_ena;
        sc_signal<sc_uint<RISCV_ARCH>> multi_a1;        // Multi-cycle operand 1
        sc_signal<sc_uint<RISCV_ARCH>> multi_a2;        // Multi-cycle operand 2

        sc_signal<sc_uint<6>> hazard_addr0;             // Updated register address on previous step
        sc_signal<sc_uint<6>> hazard_addr1;             // Updated register address on pre-previous step
        sc_signal<sc_uint<2>> hazard_depth;             // Number of modificated registers that wasn't done yet

        sc_signal<bool> call;
        sc_signal<bool> ret;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.d_valid = false;
        iv.pc = 0;
        iv.npc = CFG_NMI_RESET_VECTOR;
        iv.instr = 0;
        iv.res_addr = 0;
        iv.res_val = 0;
        iv.memop_load = 0;
        iv.memop_store = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = 0;
        iv.memop_addr = 0;

        iv.multi_res_addr = 0;
        iv.multi_pc = 0;
        iv.multi_npc = 0;
        iv.multi_instr = 0;
        iv.multi_ena[Multi_MUL] = 0;
        iv.multi_ena[Multi_DIV] = 0;
        iv.multi_ena[Multi_FADD_D] = 0;
        iv.multi_rv32 = 0;
        iv.multi_f64 = 0;
        iv.multi_unsigned = 0;
        iv.multi_residual_high = 0;
        iv.multi_fadd_d = 0;
        iv.multi_fsub_d = 0;
        iv.multi_feq_d = 0;
        iv.multi_flt_d = 0;
        iv.multi_fle_d = 0;
        iv.multi_fmax_d = 0;
        iv.multi_fmin_d = 0;
        iv.multiclock_ena = 0;
        iv.multi_a1 = 0;
        iv.multi_a2 = 0;
        iv.hazard_addr0 = 0;
        iv.hazard_addr1 = 0;
        iv.hazard_depth = 0;
        iv.call = 0;
        iv.ret = 0;
    }

    sc_signal<bool> w_hazard_detected;
    multi_arith_type wb_arith_res;
    sc_signal<bool> w_arith_valid[Multi_Total];
    sc_signal<bool> w_arith_busy[Multi_Total];
    sc_signal<bool> w_exception_fadd_d;
    bool w_exception_store;
    bool w_exception_load;

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
    DoubleAdd *fadd_d0;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
