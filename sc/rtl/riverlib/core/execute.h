// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "../river_cfg.h"
#include "arith/alu_logic.h"
#include "arith/int_addsub.h"
#include "arith/int_mul.h"
#include "arith/int_div.h"
#include "arith/shift.h"
#include "fpu_d/fpu_top.h"

namespace debugger {

SC_MODULE(InstrExecute) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<6>> i_d_radr1;                            // rs1 address
    sc_in<sc_uint<6>> i_d_radr2;                            // rs2 address
    sc_in<sc_uint<6>> i_d_waddr;                            // rd address
    sc_in<sc_uint<12>> i_d_csr_addr;                        // decoded CSR address
    sc_in<sc_uint<RISCV_ARCH>> i_d_imm;                     // immediate value
    sc_in<sc_uint<RISCV_ARCH>> i_d_pc;                      // Instruction pointer on decoded instruction
    sc_in<sc_uint<32>> i_d_instr;                           // Decoded instruction value
    sc_in<bool> i_d_progbuf_ena;                            // instruction from progbuf passed decoder
    sc_in<sc_uint<6>> i_wb_waddr;                           // write back address
    sc_in<bool> i_memop_store;                              // Store to memory operation
    sc_in<bool> i_memop_load;                               // Load from memoru operation
    sc_in<bool> i_memop_sign_ext;                           // Load memory value with sign extending
    sc_in<sc_uint<2>> i_memop_size;                         // Memory transaction size
    sc_in<bool> i_unsigned_op;                              // Unsigned operands
    sc_in<bool> i_rv32;                                     // 32-bits instruction
    sc_in<bool> i_compressed;                               // C-extension (2-bytes length)
    sc_in<bool> i_amo;                                      // A-extension (atomic)
    sc_in<bool> i_f64;                                      // D-extension (FPU)
    sc_in<sc_uint<ISA_Total>> i_isa_type;                   // Type of the instruction's structure (ISA spec.)
    sc_in<sc_biguint<Instr_Total>> i_ivec;                  // One pulse per supported instruction.
    sc_in<bool> i_stack_overflow;                           // exception stack overflow
    sc_in<bool> i_stack_underflow;                          // exception stack overflow
    sc_in<bool> i_unsup_exception;                          // Unsupported instruction exception
    sc_in<bool> i_instr_load_fault;                         // fault instruction's address. Bus returned ERR on read transaction
    sc_in<bool> i_mem_valid;                                // memory operation done (need for AMO)
    sc_in<sc_uint<RISCV_ARCH>> i_mem_rdata;                 // memory operation read data (need for AMO)
    sc_in<bool> i_mem_ex_debug;                             // Memoryaccess: Debug requested processed with error. Ignore it.
    sc_in<bool> i_mem_ex_load_fault;                        // Memoryaccess: Bus response with SLVERR or DECERR on read data
    sc_in<bool> i_mem_ex_store_fault;                       // Memoryaccess: Bus response with SLVERR or DECERR on write data
    sc_in<bool> i_page_fault_x;                             // IMMU execute page fault signal
    sc_in<bool> i_page_fault_r;                             // DMMU read access page fault
    sc_in<bool> i_page_fault_w;                             // DMMU write access page fault
    sc_in<sc_uint<RISCV_ARCH>> i_mem_ex_addr;               // Memoryaccess: exception address
    sc_in<sc_uint<IRQ_TOTAL>> i_irq_pending;                // Per Hart pending interrupts pins
    sc_in<bool> i_wakeup;                                   // There's pending bit even if interrupts globally disabled
    sc_in<bool> i_haltreq;                                  // halt request from debug unit
    sc_in<bool> i_resumereq;                                // resume request from debug unit
    sc_in<bool> i_step;                                     // resume with step
    sc_in<bool> i_dbg_progbuf_ena;                          // progbuf mode enabled
    sc_in<sc_uint<RISCV_ARCH>> i_rdata1;                    // Integer/Float register value 1
    sc_in<sc_uint<CFG_REG_TAG_WIDTH>> i_rtag1;
    sc_in<sc_uint<RISCV_ARCH>> i_rdata2;                    // Integer/Float register value 2
    sc_in<sc_uint<CFG_REG_TAG_WIDTH>> i_rtag2;
    sc_out<sc_uint<6>> o_radr1;
    sc_out<sc_uint<6>> o_radr2;
    sc_out<bool> o_reg_wena;
    sc_out<sc_uint<6>> o_reg_waddr;                         // Address to store result of the instruction (0=do not store)
    sc_out<sc_uint<CFG_REG_TAG_WIDTH>> o_reg_wtag;
    sc_out<sc_uint<RISCV_ARCH>> o_reg_wdata;                // Value to store
    sc_out<bool> o_csr_req_valid;                           // Access to CSR request
    sc_in<bool> i_csr_req_ready;                            // CSR module is ready to accept request
    sc_out<sc_uint<CsrReq_TotalBits>> o_csr_req_type;       // Request type: [0]-read csr; [1]-write csr; [2]-change mode
    sc_out<sc_uint<12>> o_csr_req_addr;                     // Requested CSR address
    sc_out<sc_uint<RISCV_ARCH>> o_csr_req_data;             // CSR new value
    sc_in<bool> i_csr_resp_valid;                           // CSR module Response is valid
    sc_out<bool> o_csr_resp_ready;                          // Executor is ready to accept response
    sc_in<sc_uint<RISCV_ARCH>> i_csr_resp_data;             // Responded CSR data
    sc_in<bool> i_csr_resp_exception;                       // Raise exception on CSR access
    sc_out<bool> o_memop_valid;                             // Request to memory is valid
    sc_out<bool> o_memop_debug;                             // Debug Request shouldn't modify registers in write back stage
    sc_out<bool> o_memop_sign_ext;                          // Load data with sign extending
    sc_out<sc_uint<MemopType_Total>> o_memop_type;          // [0]: 1=store/0=Load data
    sc_out<sc_uint<2>> o_memop_size;                        // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_out<sc_uint<RISCV_ARCH>> o_memop_memaddr;            // Memory access address
    sc_out<sc_uint<RISCV_ARCH>> o_memop_wdata;
    sc_in<bool> i_memop_ready;                              // memaccess is ready to accept memop on next clock
    sc_in<bool> i_memop_idle;                               // No memory operations in progress
    sc_in<bool> i_dbg_mem_req_valid;                        // Debug Request to memory is valid
    sc_in<bool> i_dbg_mem_req_write;                        // 0=read; 1=write
    sc_in<sc_uint<2>> i_dbg_mem_req_size;                   // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_in<sc_uint<RISCV_ARCH>> i_dbg_mem_req_addr;          // Memory access address
    sc_in<sc_uint<RISCV_ARCH>> i_dbg_mem_req_wdata;
    sc_out<bool> o_dbg_mem_req_ready;                       // Debug emmory request was accepted
    sc_out<bool> o_dbg_mem_req_error;                       // Debug memory reques misaliged
    sc_out<bool> o_valid;                                   // Output is valid
    sc_out<sc_uint<RISCV_ARCH>> o_pc;                       // Valid instruction pointer
    sc_out<sc_uint<RISCV_ARCH>> o_npc;                      // Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    sc_out<sc_uint<32>> o_instr;                            // Valid instruction value
    sc_out<bool> o_call;                                    // CALL pseudo instruction detected
    sc_out<bool> o_ret;                                     // RET pseudoinstruction detected (hw stack tracing)
    sc_out<bool> o_jmp;                                     // Jump was executed
    sc_out<bool> o_halted;

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrExecute);

    InstrExecute(sc_module_name name,
                 bool async_reset,
                 bool fpu_ena);
    virtual ~InstrExecute();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    bool fpu_ena_;

    static const int Res_Zero = 0;
    static const int Res_Reg2 = 1;
    static const int Res_Npc = 2;
    static const int Res_Ra = 3;
    static const int Res_Csr = 4;
    static const int Res_Alu = 5;
    static const int Res_AddSub = 6;
    static const int Res_Shifter = 7;
    static const int Res_IMul = 8;
    static const int Res_IDiv = 9;
    static const int Res_FPU = 10;
    static const int Res_Total = 11;
    
    static const uint8_t State_Idle = 0;
    static const uint8_t State_WaitMemAcces = 1;
    static const uint8_t State_WaitMulti = 2;
    static const uint8_t State_Amo = 5;
    static const uint8_t State_Csr = 6;
    static const uint8_t State_Halted = 7;
    static const uint8_t State_DebugMemRequest = 8;
    static const uint8_t State_DebugMemError = 9;
    static const uint8_t State_Wfi = 0xf;
    
    static const uint8_t CsrState_Idle = 0;
    static const uint8_t CsrState_Req = 1;
    static const uint8_t CsrState_Resp = 2;
    
    static const uint8_t AmoState_WaitMemAccess = 0;
    static const uint8_t AmoState_Read = 1;
    static const uint8_t AmoState_Modify = 2;
    static const uint8_t AmoState_Write = 3;

    sc_uint<4> irq2idx(sc_uint<IRQ_TOTAL> irqbus);

    struct select_type {
        sc_signal<bool> ena;
        sc_signal<bool> valid;
        sc_signal<sc_uint<RISCV_ARCH>> res;
    };

    struct input_mux_type {
        sc_uint<6> radr1;
        sc_uint<6> radr2;
        sc_uint<6> waddr;
        sc_uint<RISCV_ARCH> imm;
        sc_uint<RISCV_ARCH> pc;
        sc_uint<32> instr;
        sc_uint<MemopType_Total> memop_type;
        bool memop_sign_ext;
        sc_uint<2> memop_size;
        bool unsigned_op;
        bool rv32;
        bool compressed;
        bool f64;
        sc_biguint<Instr_Total> ivec;
        sc_uint<ISA_Total> isa_type;
    };


    struct InstrExecute_registers {
        sc_signal<sc_uint<4>> state;
        sc_signal<sc_uint<2>> csrstate;
        sc_signal<sc_uint<2>> amostate;
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<sc_uint<RISCV_ARCH>> npc;
        sc_signal<sc_uint<RISCV_ARCH>> dnpc;
        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
        sc_signal<sc_uint<RISCV_ARCH>> rdata1_amo;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2_amo;
        sc_signal<sc_biguint<Instr_Total>> ivec;
        sc_signal<sc_uint<ISA_Total>> isa_type;
        sc_signal<sc_uint<RISCV_ARCH>> imm;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_biguint<(CFG_REG_TAG_WIDTH * REGS_TOTAL)>> tagcnt;// N-bits tag per register (expected)
        sc_signal<bool> reg_write;
        sc_signal<sc_uint<6>> reg_waddr;
        sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> reg_wtag;
        sc_signal<bool> csr_req_rmw;                        // csr read-modify-write request
        sc_signal<sc_uint<CsrReq_TotalBits>> csr_req_type;
        sc_signal<sc_uint<12>> csr_req_addr;
        sc_signal<sc_uint<RISCV_ARCH>> csr_req_data;
        sc_signal<bool> memop_valid;
        sc_signal<bool> memop_debug;                        // request from dport
        sc_signal<bool> memop_halted;                       // 0=return to Idle; 1=return to halt state
        sc_signal<sc_uint<MemopType_Total>> memop_type;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<sc_uint<RISCV_ARCH>> memop_memaddr;
        sc_signal<sc_uint<RISCV_ARCH>> memop_wdata;
        sc_signal<bool> unsigned_op;
        sc_signal<bool> rv32;
        sc_signal<bool> compressed;
        sc_signal<bool> f64;
        sc_signal<bool> stack_overflow;
        sc_signal<bool> stack_underflow;
        sc_signal<bool> mem_ex_load_fault;
        sc_signal<bool> mem_ex_store_fault;
        sc_signal<bool> page_fault_r;
        sc_signal<bool> page_fault_w;
        sc_signal<sc_uint<RISCV_ARCH>> mem_ex_addr;
        sc_signal<sc_uint<RISCV_ARCH>> res_npc;
        sc_signal<sc_uint<RISCV_ARCH>> res_ra;
        sc_signal<sc_uint<RISCV_ARCH>> res_csr;
        sc_signal<sc_uint<Res_Total>> select;
        sc_signal<bool> valid;
        sc_signal<bool> call;
        sc_signal<bool> ret;
        sc_signal<bool> jmp;
        sc_signal<bool> stepdone;
    } v, r;

    void InstrExecute_r_reset(InstrExecute_registers &iv) {
        iv.state = State_Idle;
        iv.csrstate = CsrState_Idle;
        iv.amostate = AmoState_WaitMemAccess;
        iv.pc = 0ull;
        iv.npc = CFG_RESET_VECTOR;
        iv.dnpc = 0ull;
        iv.radr1 = 0;
        iv.radr2 = 0;
        iv.waddr = 0;
        iv.rdata1 = 0ull;
        iv.rdata2 = 0ull;
        iv.rdata1_amo = 0ull;
        iv.rdata2_amo = 0ull;
        iv.ivec = 0ull;
        iv.isa_type = 0;
        iv.imm = 0ull;
        iv.instr = 0;
        iv.tagcnt = 0ull;
        iv.reg_write = 0;
        iv.reg_waddr = 0;
        iv.reg_wtag = 0;
        iv.csr_req_rmw = 0;
        iv.csr_req_type = 0;
        iv.csr_req_addr = 0;
        iv.csr_req_data = 0ull;
        iv.memop_valid = 0;
        iv.memop_debug = 0;
        iv.memop_halted = 0;
        iv.memop_type = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = 0;
        iv.memop_memaddr = 0ull;
        iv.memop_wdata = 0ull;
        iv.unsigned_op = 0;
        iv.rv32 = 0;
        iv.compressed = 0;
        iv.f64 = 0;
        iv.stack_overflow = 0;
        iv.stack_underflow = 0;
        iv.mem_ex_load_fault = 0;
        iv.mem_ex_store_fault = 0;
        iv.page_fault_r = 0;
        iv.page_fault_w = 0;
        iv.mem_ex_addr = 0ull;
        iv.res_npc = 0ull;
        iv.res_ra = 0ull;
        iv.res_csr = 0ull;
        iv.select = 0;
        iv.valid = 0;
        iv.call = 0;
        iv.ret = 0;
        iv.jmp = 0;
        iv.stepdone = 0;
    }

    select_type wb_select[Res_Total];
    sc_signal<sc_uint<3>> wb_alu_mode;
    sc_signal<sc_uint<7>> wb_addsub_mode;
    sc_signal<sc_uint<4>> wb_shifter_mode;
    sc_signal<bool> w_arith_residual_high;
    sc_signal<bool> w_mul_hsu;
    sc_signal<sc_uint<Instr_FPU_Total>> wb_fpu_vec;
    sc_signal<bool> w_ex_fpu_invalidop;                     // FPU Exception: invalid operation
    sc_signal<bool> w_ex_fpu_divbyzero;                     // FPU Exception: divide by zero
    sc_signal<bool> w_ex_fpu_overflow;                      // FPU Exception: overflow
    sc_signal<bool> w_ex_fpu_underflow;                     // FPU Exception: underflow
    sc_signal<bool> w_ex_fpu_inexact;                       // FPU Exception: inexact
    sc_signal<bool> w_hazard1;
    sc_signal<bool> w_hazard2;
    sc_signal<sc_uint<RISCV_ARCH>> wb_rdata1;
    sc_signal<sc_uint<RISCV_ARCH>> wb_rdata2;
    sc_signal<sc_uint<RISCV_ARCH>> wb_shifter_a1;           // Shifters operand 1
    sc_signal<sc_uint<6>> wb_shifter_a2;                    // Shifters operand 2
    sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> tag_expected[INTREGS_TOTAL];

    AluLogic *alu0;
    IntAddSub *addsub0;
    IntMul *mul0;
    IntDiv *div0;
    Shifter *sh0;
    FpuTop *fpu0;

};

}  // namespace debugger

