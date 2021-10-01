/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_RIVERLIB_PROC_H__
#define __DEBUGGER_RIVERLIB_PROC_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "fetch.h"
#include "decoder.h"
#include "execute.h"
#include "memaccess.h"
#include "execute.h"
#include "regibank.h"
#include "csr.h"
#include "br_predic.h"
#include "dbg_port.h"
#include "tracer.h"
#include "ic_csr_m2_s1.h"
#include <fstream>

namespace debugger {

SC_MODULE(Processor) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset. Active LOW
    // Control path:
    sc_in<bool> i_req_ctrl_ready;                       // ICache is ready to accept request
    sc_out<bool> o_req_ctrl_valid;                      // Request to ICache is valid
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_ctrl_addr;    // Requesting address to ICache
    sc_in<bool> i_resp_ctrl_valid;                      // ICache response is valid
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_resp_ctrl_addr;    // Response address must be equal to the latest request address
    sc_in<sc_uint<32>> i_resp_ctrl_data;                // Read value
    sc_in<bool> i_resp_ctrl_load_fault;
    sc_in<bool> i_resp_ctrl_executable;                 // MPU flag
    sc_out<bool> o_resp_ctrl_ready;                     // Core is ready to accept response from ICache
    // Data path:
    sc_in<bool> i_req_data_ready;                       // DCache is ready to accept request
    sc_out<bool> o_req_data_valid;                      // Request to DCache is valid
    sc_out<sc_uint<MemopType_Total>> o_req_data_type;   // Read/Write transaction plus additional flags
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_data_addr; // Requesting address to DCache
    sc_out<sc_uint<64>> o_req_data_wdata;               // Writing value
    sc_out<sc_uint<8>> o_req_data_wstrb;                // 8-bytes aligned strobs
    sc_out<sc_uint<2>> o_req_data_size;                                     // memory operation 1,2,4 or 8 bytes
    sc_in<bool> i_resp_data_valid;                      // DCache response is valid
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_resp_data_addr;    // DCache response address must be equal to the latest request address
    sc_in<sc_uint<64>> i_resp_data_data;                // Read value
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_resp_data_fault_addr;  // write-error address (B-channel)
    sc_in<bool> i_resp_data_load_fault;                 // Bus response with SLVERR or DECERR on read
    sc_in<bool> i_resp_data_store_fault;                // Bus response with SLVERR or DECERR on write
    sc_in<bool> i_resp_data_er_mpu_load;
    sc_in<bool> i_resp_data_er_mpu_store;
    sc_out<bool> o_resp_data_ready;                     // Core is ready to accept response from DCache
    // External interrupt pin
    sc_in<bool> i_ext_irq;                              // PLIC interrupt accordingly with spec
    // MPU interface
    sc_out<bool> o_mpu_region_we;
    sc_out<sc_uint<CFG_MPU_TBL_WIDTH>> o_mpu_region_idx;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_region_addr;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_region_mask;
    sc_out<sc_uint<CFG_MPU_FL_TOTAL>> o_mpu_region_flags;  // {ena, cachable, r, w, x}
    // Debug interface
    sc_in<bool> i_dport_req_valid;                      // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<CFG_DPORT_ADDR_BITS>> i_dport_addr;   // dport address
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_req_ready;
    sc_in<bool> i_dport_resp_ready;                     // ready to accepd response
    sc_out<bool> o_dport_resp_valid;                    // Response is valid
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    sc_out<bool> o_halted;                              // CPU halted via debug interface
    // Cache debug signals:
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_flush_address;    // Address of instruction to remove from ICache
    sc_out<bool> o_flush_valid;                         // Remove address from ICache is valid
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_data_flush_address;    // Address of instruction to remove from D$
    sc_out<bool> o_data_flush_valid;                         // Remove address from D$ is valid
    sc_in<bool> i_data_flush_end;

    void comb();
    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

    SC_HAS_PROCESS(Processor);

    Processor(sc_module_name name_, uint32_t hartid, bool async_reset,
             bool fpu_ena, bool tracer_ena);
    virtual ~Processor();

private:
    struct FetchType {
        sc_signal<bool> req_fire;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;
        sc_signal<bool> valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> imem_req_valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> imem_req_addr;
        sc_signal<bool> pipeline_hold;
    };

    struct InstructionDecodeType {
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> instr_valid;
        sc_signal<bool> memop_store;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> rv32;                       // 32-bits instruction
        sc_signal<bool> compressed;                 // C-extension
        sc_signal<bool> amo;                        // A-extension
        sc_signal<bool> f64;                        // D-extension (FPU)
        sc_signal<bool> unsigned_op;                // Unsigned operands
        sc_signal<sc_bv<ISA_Total>> isa_type;
        sc_signal<sc_bv<Instr_Total>> instr_vec;
        sc_signal<bool> exception;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;
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
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> npc;

        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<bool> reg_wena;
        sc_signal<sc_uint<6>> reg_waddr;
        sc_signal<sc_uint<CFG_REG_TAG_WITH>> reg_wtag;
        sc_signal<sc_uint<RISCV_ARCH>> reg_wdata;
        sc_signal<bool> csr_req_valid;               // Access to CSR request
        sc_signal<sc_uint<CsrReq_TotalBits>> csr_req_type;// Request type: [0]-read csr; [1]-write csr; [2]-change mode
        sc_signal<sc_uint<12>> csr_req_addr;         // Requested CSR address
        sc_signal<sc_uint<RISCV_ARCH>> csr_req_data; // CSR new value
        sc_signal<bool> csr_resp_ready;              // Executor is ready to accept response

        sc_signal<bool> memop_valid;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<MemopType_Total>> memop_type;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> memop_addr;
        sc_signal<sc_uint<RISCV_ARCH>> memop_wdata;
        sc_signal<bool> d_ready;           // Hold pipeline from Execution stage
        sc_signal<bool> flushd;
        sc_signal<bool> flushi;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> flushi_addr;
        sc_signal<bool> call;                       // pseudo-instruction CALL
        sc_signal<bool> ret;                        // pseudo-instruction RET
        sc_signal<bool> halted;
    };

    struct MemoryType {
        sc_signal<bool> memop_ready;
        sc_signal<bool> flushd;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<bool> valid;
    };

    struct WriteBackType {
        sc_signal<bool> wena;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
        sc_signal<sc_uint<CFG_REG_TAG_WITH>> wtag;
    };

    struct IntRegsType {
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<CFG_REG_TAG_WITH>> rtag1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
        sc_signal<sc_uint<CFG_REG_TAG_WITH>> rtag2;
        sc_signal<sc_uint<RISCV_ARCH>> dport_rdata;
        sc_signal<sc_uint<RISCV_ARCH>> ra;      // Return address
        sc_signal<sc_uint<RISCV_ARCH>> sp;      // Stack pointer
    } ireg;

    struct FloatRegsType {
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
        sc_signal<sc_uint<RISCV_ARCH>> dport_rdata;
    } freg;

    struct CsrType {
        sc_signal<bool> req_ready;                  // CSR module is ready to accept request
        sc_signal<bool> resp_valid;                 // CSR module Response is valid
        sc_signal<sc_uint<RISCV_ARCH>> resp_data;   // Responded CSR data
        sc_signal<bool> resp_exception;             // Exception of CSR access
        sc_signal<bool> progbuf_ena;                // execute instruction from progbuf
        sc_signal<sc_uint<32>> progbuf_pc;          // progbuf instruction counter
        sc_signal<sc_uint<32>> progbuf_data;        // progbuf instruction to execute
        sc_signal<bool> flushi_ena;                 // clear specified addr in ICache without execution of fence.i
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> flushi_addr;
        sc_signal<sc_uint<64>> executed_cnt;        // Number of executed instruction
        sc_signal<bool> dbg_pc_write;               // modify npc value strob
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> dbg_pc;
        sc_signal<bool> halt;                       // Halt signal is equal to hold pipeline
        sc_signal<bool> irq_software;
        sc_signal<bool> irq_timer;
        sc_signal<bool> irq_external;
        sc_signal<bool> stack_overflow;
        sc_signal<bool> stack_underflow;
    } csr;

    struct DebugType {
        sc_signal<bool> csr_req_valid;
        sc_signal<sc_uint<CsrReq_TotalBits>> csr_req_type;
        sc_signal<sc_uint<12>> csr_req_addr;           // Address of the sub-region register
        sc_signal<sc_uint<RISCV_ARCH>> csr_req_data;
        sc_signal<bool> csr_resp_ready;
        sc_signal<sc_uint<6>> reg_addr;
        sc_signal<sc_uint<RISCV_ARCH>> core_wdata;  // Write data
        sc_signal<bool> ireg_ena;                   // Region 1: Access to integer register bank is enabled
        sc_signal<bool> ireg_write;                 // Region 1: Integer registers bank write pulse
    } dbg;

    struct BranchPredictorType {
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> npc;
    } bp;

    /** 5-stages CPU pipeline */
    struct PipelineType {
        FetchType f;                            // Fetch instruction stage
        InstructionDecodeType d;                // Decode instruction stage
        ExecuteType e;                          // Execute instruction
        MemoryType m;                           // Memory load/store
        WriteBackType w;                        // Write back registers value
    } w;

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

    sc_signal<bool> w_fetch_pipeline_hold;
    sc_signal<bool> w_any_pipeline_hold;
    sc_signal<bool> w_flush_pipeline;

    InstrFetch *fetch0;
    InstrDecoder *dec0;
    InstrExecute *exec0;
    MemAccess *mem0;

    BranchPredictor *predic0;
    RegIntBank *iregs0;
    ic_csr_m2_s1 *iccsr0;
    CsrRegs *csr0;
    Tracer *trace0;

    DbgPort *dbg0;

    sc_signal<bool> w_writeback_ready;
    sc_signal<bool> w_reg_wena;
    sc_signal<sc_uint<6>> wb_reg_waddr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_reg_wdata;
    sc_signal<sc_uint<CFG_REG_TAG_WITH>> wb_reg_wtag;
    sc_signal<bool> w_reg_inorder;
    sc_signal<bool> w_reg_ignored;

    bool fpu_ena_;
    bool tracer_ena_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_PROC_H__
