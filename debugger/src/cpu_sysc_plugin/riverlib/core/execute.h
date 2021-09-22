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
#include "riscv-isa.h"
#include "arith/alu_logic.h"
#include "arith/int_addsub.h"
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
    sc_in<sc_uint<12>> i_d_csr_addr;            // decoded CSR address
    sc_in<sc_uint<RISCV_ARCH>> i_d_imm;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_d_pc;      // Instruction pointer on decoded instruction
    sc_in<sc_uint<32>> i_d_instr;               // Decoded instruction value
    sc_in<bool> i_d_progbuf_ena;                // instruction from progbuf passed decoder
    sc_in<bool> i_dbg_progbuf_ena;              // progbuf mode enabled
    sc_in<sc_uint<6>> i_wb_waddr;               // write back address
    sc_in<bool> i_memop_store;                  // Store to memory operation
    sc_in<bool> i_memop_load;                   // Load from memoru operation
    sc_in<bool> i_memop_sign_ext;               // Load memory value with sign extending
    sc_in<sc_uint<2>> i_memop_size;             // Memory transaction size
    sc_in<bool> i_unsigned_op;                  // Unsigned operands
    sc_in<bool> i_rv32;                         // 32-bits instruction
    sc_in<bool> i_compressed;                   // C-extension (2-bytes length)
    sc_in<bool> i_amo;                          // A-extension (atomic)
    sc_in<bool> i_f64;                          // D-extension (FPU)
    sc_in<sc_bv<ISA_Total>> i_isa_type;         // Type of the instruction's structure (ISA spec.)
    sc_in<sc_bv<Instr_Total>> i_ivec;           // One pulse per supported instruction.
    sc_in<bool> i_unsup_exception;              // Unsupported instruction exception
    sc_in<bool> i_instr_load_fault;             // fault instruction's address. Bus returned ERR on read transaction
    sc_in<bool> i_instr_executable;             // MPU flag 'executable' not set for this memory region
    sc_in<bool> i_mem_ex_load_fault;                    // Memoryaccess: Bus response with SLVERR or DECERR on read data
    sc_in<bool> i_mem_ex_store_fault;                   // Memoryaccess: Bus response with SLVERR or DECERR on write data
    sc_in<bool> i_mem_ex_mpu_store;                     // Memoryaccess: MPU access error on storing data
    sc_in<bool> i_mem_ex_mpu_load;                      // Memoryaccess: MPU access error on load data
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_mem_ex_addr;    // Memoryaccess: exception address

    sc_in<bool> i_irq_software;                 // software interrupt request from CSR register xSIP
    sc_in<bool> i_irq_timer;                    // interrupt request from wallclock timer
    sc_in<bool> i_irq_external;                 // interrupt request from PLIC
    sc_in<bool> i_halt;                         // halt request from debug unit
    sc_in<bool> i_dport_npc_write;              // Write npc value from debug port
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_dport_npc; // Debug port npc value to write

    sc_in<sc_uint<RISCV_ARCH>> i_rdata1;        // Integer/Float register value 1
    sc_in<sc_uint<CFG_REG_TAG_WITH>> i_rtag1;
    sc_in<sc_uint<RISCV_ARCH>> i_rdata2;        // Integer/Float register value 2
    sc_in<sc_uint<CFG_REG_TAG_WITH>> i_rtag2;
    sc_out<sc_uint<6>> o_radr1;
    sc_out<sc_uint<6>> o_radr2;
    sc_out<bool> o_reg_wena;
    sc_out<sc_uint<6>> o_reg_waddr;             // Address to store result of the instruction (0=do not store)
    sc_out<sc_uint<CFG_REG_TAG_WITH>> o_reg_wtag;
    sc_out<sc_uint<RISCV_ARCH>> o_reg_wdata;    // Value to store
    sc_out<bool> o_d_ready;                     // Hold pipeline while 'writeback' not done or multi-clock instruction.

    sc_out<bool> o_csr_req_valid;               // Access to CSR request
    sc_in<bool> i_csr_req_ready;                // CSR module is ready to accept request
    sc_out<sc_uint<CsrReq_TotalBits>> o_csr_req_type;// Request type: [0]-read csr; [1]-write csr; [2]-change mode
    sc_out<sc_uint<12>> o_csr_req_addr;         // Requested CSR address
    sc_out<sc_uint<RISCV_ARCH>> o_csr_req_data; // CSR new value
    sc_in<bool> i_csr_resp_valid;               // CSR module Response is valid
    sc_out<bool> o_csr_resp_ready;              // Executor is ready to accept response
    sc_in<sc_uint<RISCV_ARCH>> i_csr_resp_data; // Responded CSR data
    sc_in<bool> i_csr_resp_exception;           // Raise exception on CSR access

    sc_out<bool> o_memop_valid;                 // Request to memory is valid
    sc_out<bool> o_memop_sign_ext;              // Load data with sign extending
    sc_out<sc_uint<MemopType_Total>> o_memop_type;  // [0]: 1=store/0=Load data
    sc_out<sc_uint<2>> o_memop_size;            // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_memop_memaddr;// Memory access address
    sc_out<sc_uint<RISCV_ARCH>> o_memop_wdata;
    sc_in<bool> i_memop_ready;

    sc_out<bool> o_valid;                       // Output is valid
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_pc;    // Valid instruction pointer
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_npc;   // Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    sc_out<sc_uint<32>> o_instr;                // Valid instruction value
    sc_in<bool> i_flushd_end;
    sc_out<bool> o_flushd;
    sc_out<bool> o_flushi;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_flushi_addr;
    sc_out<bool> o_call;                        // CALL pseudo instruction detected
    sc_out<bool> o_ret;                         // RET pseudoinstruction detected
    sc_out<bool> o_halted;

    void comb();
    void registers();

    SC_HAS_PROCESS(InstrExecute);

    InstrExecute(sc_module_name name_, bool async_reset, bool fpu_ena);
    virtual ~InstrExecute();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:

    enum EResTypes {
        Res_Zero,
        Res_Reg2,
        Res_Npc,
        Res_Ra,     // return address
        Res_Csr,
        Res_Alu,
        Res_AddSub,
        Res_Shifter,
        Res_IMul,
        Res_IDiv,
        Res_FPU,
        Res_Total
    };

    static const unsigned State_Idle = 0;
    static const unsigned State_WaitMemAcces = 1;
    static const unsigned State_WaitMulti = 2;
    static const unsigned State_WaitFlushingAccept = 3;     // memaccess should accept flushing request
    static const unsigned State_Flushing_I = 4;
    static const unsigned State_Amo = 5;
    static const unsigned State_Csr = 6;
    static const unsigned State_Halted = 7;

    static const unsigned CsrState_Idle = 0;
    static const unsigned CsrState_Req = 1;
    static const unsigned CsrState_Resp = 2;

    static const unsigned AmoState_WaitMemAccess = 0;
    static const unsigned AmoState_Read = 1;
    static const unsigned AmoState_Modify = 2;
    static const unsigned AmoState_Write = 3;

    struct select_type {
        sc_signal<bool> ena[Res_Total];
        sc_signal<bool> valid[Res_Total];
        sc_signal<sc_uint<RISCV_ARCH>> res[Res_Total];
    };

    struct input_mux_type {
        sc_uint<6> radr1;
        sc_uint<6> radr2;
        sc_uint<6> waddr;
        sc_uint<RISCV_ARCH> imm;
        sc_uint<CFG_CPU_ADDR_BITS> pc;
        sc_uint<32> instr;
        sc_uint<MemopType_Total> memop_type;
        bool memop_sign_ext;
        sc_uint<2> memop_size;
        bool unsigned_op;
        bool rv32;
        bool compressed;
        bool f64;
        sc_bv<Instr_Total> ivec;
        sc_bv<ISA_Total> isa_type;
    };

    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<2>> csrstate;
        sc_signal<sc_uint<2>> amostate;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> npc;
        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
        sc_signal<sc_bv<Instr_Total>> ivec;
        sc_signal<sc_bv<ISA_Total>> isa_type;
        sc_signal<sc_uint<RISCV_ARCH>> imm;

        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_biguint<CFG_REG_TAG_WITH*REGS_TOTAL>> tagcnt_rd;      // N-bits tag per register (expected)
        sc_signal<sc_biguint<CFG_REG_TAG_WITH*REGS_TOTAL>> tagcnt_wr;      // N-bits tag per register (written)

        sc_signal<bool> reg_write;
        sc_signal<sc_uint<6>> reg_waddr;
        sc_signal<sc_uint<CFG_REG_TAG_WITH>> reg_wtag;

        sc_signal<bool> csr_req_rmw;                    // csr read-modify-write request
        sc_signal<bool> csr_req_pc;                     // csr request instruction pointer
        sc_signal<sc_uint<CsrReq_TotalBits>> csr_req_type;
        sc_signal<sc_uint<12>> csr_req_addr;
        sc_signal<sc_uint<RISCV_ARCH>> csr_req_data;

        sc_signal<bool> memop_valid;
        sc_signal<sc_uint<MemopType_Total>> memop_type;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> memop_memaddr;
        sc_signal<sc_uint<RISCV_ARCH>> memop_wdata;

        sc_signal<bool> unsigned_op;
        sc_signal<bool> rv32;
        sc_signal<bool> compressed;
        sc_signal<bool> f64;

        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> res_npc;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> res_ra;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> res_csr;
        sc_signal<sc_uint<Res_Total>> select;
        sc_signal<bool> valid;
        sc_signal<bool> call;
        sc_signal<bool> ret;
        sc_signal<bool> flushd;
        sc_signal<bool> flushi;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> flushi_addr;
        sc_signal<sc_uint<32>> progbuf_npc;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = State_Idle;
        iv.csrstate = CsrState_Idle;
        iv.amostate = AmoState_WaitMemAccess;
        iv.pc = 0;
        iv.npc = CFG_NMI_RESET_VECTOR;
        iv.radr1 = 0;
        iv.radr2 = 0;
        iv.waddr = 0;
        iv.rdata1 = 0;
        iv.rdata2 = 0;
        iv.ivec = 0;
        iv.isa_type = 0;
        iv.imm = 0;
        iv.instr = 0;
        iv.tagcnt_rd = 0;
        iv.tagcnt_wr = 0;
        iv.reg_write = 0;
        iv.reg_waddr = 0;
        iv.reg_wtag = 0;
        iv.csr_req_rmw = 0;
        iv.csr_req_pc = 0;
        iv.csr_req_type = 0;
        iv.csr_req_addr = 0;
        iv.csr_req_data = 0;
        iv.memop_valid = 0;
        iv.memop_type = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = 0;
        iv.memop_memaddr = 0;
        iv.memop_wdata = 0;

        iv.unsigned_op = 0;
        iv.rv32 = 0;
        iv.compressed = 0;
        iv.f64 = 0;

        iv.res_npc = 0;
        iv.res_ra = 0;
        iv.res_csr = 0;
        iv.select = 0;

        iv.valid = 0;
        iv.call = 0;
        iv.ret = 0;
        iv.flushd = 0;
        iv.flushi = 0;
        iv.flushi_addr = 0;
        iv.progbuf_npc = 0;
    }

    select_type wb_select;
    sc_signal<sc_uint<3>> wb_alu_mode;
    sc_signal<sc_uint<7>> wb_addsub_mode;
    sc_signal<sc_uint<4>> wb_shifter_mode;
    sc_signal<bool> w_arith_residual_high;
    sc_signal<bool> w_mul_hsu;
    sc_signal<sc_bv<Instr_FPU_Total>> wb_fpu_vec;
    sc_signal<bool> w_ex_fpu_invalidop;            // FPU Exception: invalid operation
    sc_signal<bool> w_ex_fpu_divbyzero;            // FPU Exception: divide by zero
    sc_signal<bool> w_ex_fpu_overflow;             // FPU Exception: overflow
    sc_signal<bool> w_ex_fpu_underflow;            // FPU Exception: underflow
    sc_signal<bool> w_ex_fpu_inexact;              // FPU Exception: inexact

    bool w_hazard1;
    bool w_hazard2;

    sc_signal<sc_uint<RISCV_ARCH>> wb_rdata1;
    sc_signal<sc_uint<RISCV_ARCH>> wb_rdata2;

    sc_signal<sc_uint<RISCV_ARCH>> wb_shifter_a1;      // Shifters operand 1
    sc_signal<sc_uint<6>> wb_shifter_a2;               // Shifters operand 2

    sc_uint<CFG_REG_TAG_WITH> tag_expected[Reg_Total];

    AluLogic *alu0;
    IntAddSub *addsub0;
    IntMul *mul0;
    IntDiv *div0;
    Shifter *sh0;
    FpuTop *fpu0;

    bool async_reset_;
    bool fpu_ena_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
