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
#include "fetch.h"
#include "decoder.h"
#include "execute.h"
#include "memaccess.h"
#include "mmu.h"
#include "bp.h"
#include "regibank.h"
#include "ic_csr_m2_s1.h"
#include "csr.h"
#include "tracer.h"
#include "dbg_port.h"

namespace debugger {

SC_MODULE(Processor) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<64>> i_mtimer;                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    // Control path:
    sc_in<bool> i_req_ctrl_ready;                           // ICache is ready to accept request
    sc_out<bool> o_req_ctrl_valid;                          // Request to ICache is valid
    sc_out<sc_uint<RISCV_ARCH>> o_req_ctrl_addr;            // Requesting address to ICache
    sc_in<bool> i_resp_ctrl_valid;                          // ICache response is valid
    sc_in<sc_uint<RISCV_ARCH>> i_resp_ctrl_addr;            // Response address must be equal to the latest request address
    sc_in<sc_uint<64>> i_resp_ctrl_data;                    // Read value
    sc_in<bool> i_resp_ctrl_load_fault;
    sc_out<bool> o_resp_ctrl_ready;                         // Core is ready to accept response from ICache
    // Data path:
    sc_in<bool> i_req_data_ready;                           // DCache is ready to accept request
    sc_out<bool> o_req_data_valid;                          // Request to DCache is valid
    sc_out<sc_uint<MemopType_Total>> o_req_data_type;       // Read/Write transaction plus additional flags
    sc_out<sc_uint<RISCV_ARCH>> o_req_data_addr;            // Requesting address to DCache
    sc_out<sc_uint<64>> o_req_data_wdata;                   // Writing value
    sc_out<sc_uint<8>> o_req_data_wstrb;                    // 8-bytes aligned strobs
    sc_out<sc_uint<2>> o_req_data_size;                     // memory operation 1,2,4 or 8 bytes
    sc_in<bool> i_resp_data_valid;                          // DCache response is valid
    sc_in<sc_uint<RISCV_ARCH>> i_resp_data_addr;            // DCache response address must be equal to the latest request address
    sc_in<sc_uint<64>> i_resp_data_data;                    // Read value
    sc_in<bool> i_resp_data_load_fault;                     // Bus response with SLVERR or DECERR on read
    sc_in<bool> i_resp_data_store_fault;                    // Bus response with SLVERR or DECERR on write
    sc_out<bool> o_resp_data_ready;                         // Core is ready to accept response from DCache
    // Interrupt line from external interrupts controller (PLIC):
    sc_in<sc_uint<IRQ_TOTAL>> i_irq_pending;                // Per Hart pending interrupts pins
    // PMP interface
    sc_out<bool> o_pmp_ena;                                 // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    sc_out<bool> o_pmp_we;                                  // write enable into PMP
    sc_out<sc_uint<CFG_PMP_TBL_WIDTH>> o_pmp_region;        // selected PMP region
    sc_out<sc_uint<RISCV_ARCH>> o_pmp_start_addr;           // PMP region start address
    sc_out<sc_uint<RISCV_ARCH>> o_pmp_end_addr;             // PMP region end address (inclusive)
    sc_out<sc_uint<CFG_PMP_FL_TOTAL>> o_pmp_flags;          // {ena, lock, r, w, x}
    // Debug interface:
    sc_in<bool> i_haltreq;                                  // DMI: halt request from debug unit
    sc_in<bool> i_resumereq;                                // DMI: resume request from debug unit
    sc_in<bool> i_dport_req_valid;                          // Debug access from DSU is valid
    sc_in<sc_uint<DPortReq_Total>> i_dport_type;            // Debug access type
    sc_in<sc_uint<RISCV_ARCH>> i_dport_addr;                // dport address
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;               // Write value
    sc_in<sc_uint<3>> i_dport_size;                         // reg/mem access size:0=1B;...,4=128B;
    sc_out<bool> o_dport_req_ready;
    sc_in<bool> i_dport_resp_ready;                         // ready to accepd response
    sc_out<bool> o_dport_resp_valid;                        // Response is valid
    sc_out<bool> o_dport_resp_error;                        // Something wrong during command execution
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;              // Response value
    sc_in<sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)>> i_progbuf;// progam buffer
    sc_out<bool> o_halted;                                  // CPU halted via debug interface
    // Cache debug signals:
    sc_out<bool> o_flushi_valid;                            // Remove address from ICache is valid
    sc_out<sc_uint<RISCV_ARCH>> o_flushi_addr;              // Address of instruction to remove from ICache
    sc_out<bool> o_flushd_valid;                            // Remove address from D$ is valid
    sc_out<sc_uint<RISCV_ARCH>> o_flushd_addr;              // Address of instruction to remove from D$
    sc_in<bool> i_flushd_end;

    void comb();

    SC_HAS_PROCESS(Processor);

    Processor(sc_module_name name,
              bool async_reset,
              uint32_t hartid,
              bool fpu_ena,
              bool tracer_ena);
    virtual ~Processor();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t hartid_;
    bool fpu_ena_;
    bool tracer_ena_;

    struct FetchType {
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_page_fault_x;
        sc_signal<sc_uint<RISCV_ARCH>> requested_pc;        // requested but responded address
        sc_signal<sc_uint<RISCV_ARCH>> fetching_pc;         // receiving from cache before latch
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<sc_uint<64>> instr;
        sc_signal<bool> imem_req_valid;
        sc_signal<sc_uint<RISCV_ARCH>> imem_req_addr;
        sc_signal<bool> imem_resp_ready;
    };

    struct MmuType {
        sc_signal<bool> req_ready;
        sc_signal<bool> valid;
        sc_signal<sc_uint<RISCV_ARCH>> addr;
        sc_signal<sc_uint<64>> data;
        sc_signal<bool> load_fault;
        sc_signal<bool> store_fault;
        sc_signal<bool> page_fault_x;
        sc_signal<bool> page_fault_r;
        sc_signal<bool> page_fault_w;
    };

    struct InstructionDecodeType {
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> memop_store;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> rv32;                               // 32-bits instruction
        sc_signal<bool> compressed;                         // C-extension
        sc_signal<bool> amo;                                // A-extension
        sc_signal<bool> f64;                                // D-extension (FPU)
        sc_signal<bool> unsigned_op;                        // Unsigned operands
        sc_signal<sc_uint<ISA_Total>> isa_type;
        sc_signal<sc_biguint<Instr_Total>> instr_vec;
        sc_signal<bool> exception;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> page_fault_x;
        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<12>> csr_addr;
        sc_signal<sc_uint<RISCV_ARCH>> imm;
        sc_signal<bool> progbuf_ena;
    };

    struct ExecuteType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<sc_uint<RISCV_ARCH>> npc;
        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<bool> reg_wena;
        sc_signal<sc_uint<6>> reg_waddr;
        sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> reg_wtag;
        sc_signal<sc_uint<RISCV_ARCH>> reg_wdata;
        sc_signal<bool> csr_req_valid;                      // Access to CSR request
        sc_signal<sc_uint<CsrReq_TotalBits>> csr_req_type;  // Request type: [0]-read csr; [1]-write csr; [2]-change mode
        sc_signal<sc_uint<12>> csr_req_addr;                // Requested CSR address
        sc_signal<sc_uint<RISCV_ARCH>> csr_req_data;        // CSR new value
        sc_signal<bool> csr_resp_ready;                     // Executor is ready to accept response
        sc_signal<bool> memop_valid;
        sc_signal<bool> memop_debug;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<MemopType_Total>> memop_type;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<sc_uint<RISCV_ARCH>> memop_addr;
        sc_signal<sc_uint<RISCV_ARCH>> memop_wdata;
        sc_signal<bool> call;                               // pseudo-instruction CALL
        sc_signal<bool> ret;                                // pseudo-instruction RET
        sc_signal<bool> jmp;                                // jump was executed
        sc_signal<bool> halted;
        sc_signal<bool> dbg_mem_req_ready;
        sc_signal<bool> dbg_mem_req_error;
    };

    struct MemoryType {
        sc_signal<bool> memop_ready;
        sc_signal<bool> flushd;
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<bool> valid;
        sc_signal<bool> idle;
        sc_signal<bool> debug_valid;
        sc_signal<bool> dmmu_ena;
        sc_signal<bool> dmmu_sv39;
        sc_signal<bool> dmmu_sv48;
        sc_signal<bool> req_data_valid;
        sc_signal<sc_uint<MemopType_Total>> req_data_type;
        sc_signal<sc_uint<RISCV_ARCH>> req_data_addr;
        sc_signal<sc_uint<64>> req_data_wdata;
        sc_signal<sc_uint<8>> req_data_wstrb;
        sc_signal<sc_uint<2>> req_data_size;
        sc_signal<bool> resp_data_ready;
    };

    struct WriteBackType {
        sc_signal<bool> wena;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
        sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> wtag;
    };

    struct IntRegsType {
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> rtag1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
        sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> rtag2;
        sc_signal<sc_uint<RISCV_ARCH>> dport_rdata;
        sc_signal<sc_uint<RISCV_ARCH>> ra;                  // Return address
        sc_signal<sc_uint<RISCV_ARCH>> sp;                  // Stack pointer
        sc_signal<sc_uint<RISCV_ARCH>> gp;
        sc_signal<sc_uint<RISCV_ARCH>> tp;
        sc_signal<sc_uint<RISCV_ARCH>> t0;
        sc_signal<sc_uint<RISCV_ARCH>> t1;
        sc_signal<sc_uint<RISCV_ARCH>> t2;
        sc_signal<sc_uint<RISCV_ARCH>> fp;
        sc_signal<sc_uint<RISCV_ARCH>> s1;
        sc_signal<sc_uint<RISCV_ARCH>> a0;
        sc_signal<sc_uint<RISCV_ARCH>> a1;
        sc_signal<sc_uint<RISCV_ARCH>> a2;
        sc_signal<sc_uint<RISCV_ARCH>> a3;
        sc_signal<sc_uint<RISCV_ARCH>> a4;
        sc_signal<sc_uint<RISCV_ARCH>> a5;
        sc_signal<sc_uint<RISCV_ARCH>> a6;
        sc_signal<sc_uint<RISCV_ARCH>> a7;
        sc_signal<sc_uint<RISCV_ARCH>> s2;
        sc_signal<sc_uint<RISCV_ARCH>> s3;
        sc_signal<sc_uint<RISCV_ARCH>> s4;
        sc_signal<sc_uint<RISCV_ARCH>> s5;
        sc_signal<sc_uint<RISCV_ARCH>> s6;
        sc_signal<sc_uint<RISCV_ARCH>> s7;
        sc_signal<sc_uint<RISCV_ARCH>> s8;
        sc_signal<sc_uint<RISCV_ARCH>> s9;
        sc_signal<sc_uint<RISCV_ARCH>> s10;
        sc_signal<sc_uint<RISCV_ARCH>> s11;
        sc_signal<sc_uint<RISCV_ARCH>> t3;
        sc_signal<sc_uint<RISCV_ARCH>> t4;
        sc_signal<sc_uint<RISCV_ARCH>> t5;
        sc_signal<sc_uint<RISCV_ARCH>> t6;
    };

    struct CsrType {
        sc_signal<bool> req_ready;                          // CSR module is ready to accept request
        sc_signal<bool> resp_valid;                         // CSR module Response is valid
        sc_signal<sc_uint<RISCV_ARCH>> resp_data;           // Responded CSR data
        sc_signal<bool> resp_exception;                     // Exception of CSR access
        sc_signal<bool> flushd_valid;                       // clear specified addr in D$
        sc_signal<bool> flushi_valid;                       // clear specified addr in I$
        sc_signal<bool> flushmmu_valid;                     // clear specified leaf in xMMU
        sc_signal<bool> flushpipeline_valid;                // clear pipeline
        sc_signal<sc_uint<RISCV_ARCH>> flush_addr;
        sc_signal<sc_uint<64>> executed_cnt;                // Number of executed instruction
        sc_signal<sc_uint<IRQ_TOTAL>> irq_pending;
        sc_signal<bool> o_wakeup;                           // There's pending bit even if interrupts globally disabled
        sc_signal<bool> stack_overflow;
        sc_signal<bool> stack_underflow;
        sc_signal<bool> step;
        sc_signal<bool> mmu_ena;                            // MMU enabled in U and S modes. Sv48 only.
        sc_signal<bool> mmu_sv39;
        sc_signal<bool> mmu_sv48;
        sc_signal<sc_uint<44>> mmu_ppn;                     // Physical Page Number
        sc_signal<bool> mprv;
        sc_signal<bool> mxr;
        sc_signal<bool> sum;
        sc_signal<bool> progbuf_end;
        sc_signal<bool> progbuf_error;
    };

    struct DebugType {
        sc_signal<bool> csr_req_valid;
        sc_signal<sc_uint<CsrReq_TotalBits>> csr_req_type;
        sc_signal<sc_uint<12>> csr_req_addr;                // Address of the sub-region register
        sc_signal<sc_uint<RISCV_ARCH>> csr_req_data;
        sc_signal<bool> csr_resp_ready;
        sc_signal<sc_uint<6>> ireg_addr;
        sc_signal<sc_uint<RISCV_ARCH>> ireg_wdata;          // Write data
        sc_signal<bool> ireg_ena;                           // Region 1: Access to integer register bank is enabled
        sc_signal<bool> ireg_write;                         // Region 1: Integer registers bank write pulse
        sc_signal<bool> mem_req_valid;                      // Type 2: request is valid
        sc_signal<bool> mem_req_write;                      // Type 2: is write
        sc_signal<sc_uint<RISCV_ARCH>> mem_req_addr;        // Type 2: Debug memory request
        sc_signal<sc_uint<2>> mem_req_size;                 // Type 2: memory operation size: 0=1B; 1=2B; 2=4B; 3=8B
        sc_signal<sc_uint<RISCV_ARCH>> mem_req_wdata;       // Type 2: memory write data
        sc_signal<bool> progbuf_ena;                        // execute instruction from progbuf
        sc_signal<sc_uint<RISCV_ARCH>> progbuf_pc;          // progbuf instruction counter
        sc_signal<sc_uint<64>> progbuf_instr;               // progbuf instruction to execute
    };

    struct BranchPredictorType {
        sc_signal<bool> f_valid;
        sc_signal<sc_uint<RISCV_ARCH>> f_pc;
    };

    struct PipelineType {
        FetchType f;                                        // Fetch instruction stage
        InstructionDecodeType d;                            // Decode instruction stage
        ExecuteType e;                                      // Execute instruction
        MemoryType m;                                       // Memory load/store
        WriteBackType w;                                    // Write back registers value
    };


    PipelineType w;                                         // 5-stages CPU pipeline
    MmuType immu;
    MmuType dmmu;
    IntRegsType ireg;
    CsrType csr;
    DebugType dbg;
    BranchPredictorType bp;
    // csr bridge to executor unit
    sc_signal<bool> iccsr_m0_req_ready;
    sc_signal<bool> iccsr_m0_resp_valid;
    sc_signal<sc_uint<RISCV_ARCH>> iccsr_m0_resp_data;
    sc_signal<bool> iccsr_m0_resp_exception;
    // csr bridge to debug unit
    sc_signal<bool> iccsr_m1_req_ready;
    sc_signal<bool> iccsr_m1_resp_valid;
    sc_signal<sc_uint<RISCV_ARCH>> iccsr_m1_resp_data;
    sc_signal<bool> iccsr_m1_resp_exception;
    // csr bridge to CSR module
    sc_signal<bool> iccsr_s0_req_valid;
    sc_signal<sc_uint<CsrReq_TotalBits>> iccsr_s0_req_type;
    sc_signal<sc_uint<12>> iccsr_s0_req_addr;
    sc_signal<sc_uint<RISCV_ARCH>> iccsr_s0_req_data;
    sc_signal<bool> iccsr_s0_resp_ready;
    sc_signal<bool> iccsr_s0_resp_exception;
    sc_signal<bool> w_mem_resp_error;
    sc_signal<bool> w_writeback_ready;
    sc_signal<bool> w_reg_wena;
    sc_signal<sc_uint<6>> wb_reg_waddr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_reg_wdata;
    sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> wb_reg_wtag;
    sc_signal<bool> w_reg_inorder;
    sc_signal<bool> w_reg_ignored;
    sc_signal<bool> w_f_flush_ready;
    sc_signal<sc_uint<MemopType_Total>> unused_immu_mem_req_type;
    sc_signal<sc_uint<64>> unused_immu_mem_req_wdata;
    sc_signal<sc_uint<8>> unused_immu_mem_req_wstrb;
    sc_signal<sc_uint<2>> unused_immu_mem_req_size;
    sc_signal<bool> w_immu_core_req_fetch;                  // assign to 1: fetch instruction
    sc_signal<bool> w_dmmu_core_req_fetch;                  // assign to 0: data
    sc_signal<sc_uint<MemopType_Total>> unused_immu_core_req_type;
    sc_signal<sc_uint<64>> unused_immu_core_req_wdata;
    sc_signal<sc_uint<8>> unused_immu_core_req_wstrb;
    sc_signal<sc_uint<2>> unused_immu_core_req_size;
    sc_signal<bool> unused_immu_mem_resp_store_fault;

    InstrFetch *fetch0;
    InstrDecoder *dec0;
    InstrExecute *exec0;
    MemAccess *mem0;
    Mmu *immu0;
    Mmu *dmmu0;
    BranchPredictor *predic0;
    RegIntBank *iregs0;
    ic_csr_m2_s1 *iccsr0;
    CsrRegs *csr0;
    Tracer *trace0;
    DbgPort *dbg0;

};

}  // namespace debugger

