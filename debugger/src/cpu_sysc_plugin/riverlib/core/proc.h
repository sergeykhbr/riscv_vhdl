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
#include "regfbank.h"
#include "tracer.h"
#include <fstream>

namespace debugger {

SC_MODULE(Processor) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset. Active LOW
    // Control path:
    sc_in<bool> i_req_ctrl_ready;                       // ICache is ready to accept request
    sc_out<bool> o_req_ctrl_valid;                      // Request to ICache is valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_ctrl_addr;    // Requesting address to ICache
    sc_in<bool> i_resp_ctrl_valid;                      // ICache response is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_resp_ctrl_addr;    // Response address must be equal to the latest request address
    sc_in<sc_uint<32>> i_resp_ctrl_data;                // Read value
    sc_in<bool> i_resp_ctrl_load_fault;
    sc_in<bool> i_resp_ctrl_executable;                 // MPU flag
    sc_out<bool> o_resp_ctrl_ready;                     // Core is ready to accept response from ICache
    // Data path:
    sc_in<bool> i_req_data_ready;                       // DCache is ready to accept request
    sc_out<bool> o_req_data_valid;                      // Request to DCache is valid
    sc_out<bool> o_req_data_write;                      // Read/Write transaction
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_data_addr;    // Requesting address to DCache
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_req_data_wdata;   // Writing value
    sc_out<sc_uint<BUS_DATA_BYTES>> o_req_data_wstrb;   // 8-bytes aligned strobs
    sc_in<bool> i_resp_data_valid;                      // DCache response is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_resp_data_addr;    // DCache response address must be equal to the latest request address
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_resp_data_data;    // Read value
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_resp_data_store_fault_addr;  // write-error address (B-channel)
    sc_in<bool> i_resp_data_load_fault;                 // Bus response with SLVERR or DECERR on read
    sc_in<bool> i_resp_data_store_fault;                // Bus response with SLVERR or DECERR on write
    sc_in<bool> i_resp_data_er_mpu_load;
    sc_in<bool> i_resp_data_er_mpu_store;
    sc_out<bool> o_resp_data_ready;                     // Core is ready to accept response from DCache
    // External interrupt pin
    sc_in<bool> i_ext_irq;                              // PLIC interrupt accordingly with spec
    sc_out<sc_uint<64>> o_time;                         // Clock/Step counter depending attribute "GenerateRef"
    sc_out<sc_uint<64>> o_exec_cnt;
    // MPU interface
    sc_out<bool> o_mpu_region_we;
    sc_out<sc_uint<CFG_MPU_TBL_WIDTH>> o_mpu_region_idx;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_mpu_region_addr;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_mpu_region_mask;
    sc_out<sc_uint<CFG_MPU_FL_TOTAL>> o_mpu_region_flags;  // {ena, cachable, r, w, x}
    // Debug interface
    sc_in<bool> i_dport_valid;                          // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<2>> i_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_in<sc_uint<12>> i_dport_addr;                    // Register idx
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_ready;                         // Response is ready
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    sc_out<bool> o_halted;                              // CPU halted via debug interface
    // Cache debug signals:
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_flush_address;    // Address of instruction to remove from ICache
    sc_out<bool> o_flush_valid;                         // Remove address from ICache is valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_data_flush_address;    // Address of instruction to remove from D$
    sc_out<bool> o_data_flush_valid;                         // Remove address from D$ is valid
    sc_in<sc_uint<4>> i_istate;                         // ICache transaction state
    sc_in<sc_uint<4>> i_dstate;                         // DCache transaction state
    sc_in<sc_uint<2>> i_cstate;                         // CacheTop state machine value

    void comb();
    void negedge_proc();
    void dbg_print();
    void generateRef(bool v);
    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

    SC_HAS_PROCESS(Processor);

    Processor(sc_module_name name_, uint32_t hartid, bool async_reset,
             bool tracer_ena);
    virtual ~Processor();

private:
    struct FetchType {
        sc_signal<bool> req_fire;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;
        sc_signal<bool> valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> imem_req_valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> imem_req_addr;
        sc_signal<bool> pipeline_hold;
    };

    struct InstructionDecodeType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> instr_valid;
        sc_signal<bool> memop_store;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> rv32;                       // 32-bits instruction
        sc_signal<bool> compressed;                 // C-extension
        sc_signal<bool> f64;                        // D-extension (FPU)
        sc_signal<bool> unsigned_op;                // Unsigned operands
        sc_signal<sc_bv<ISA_Total>> isa_type;
        sc_signal<sc_bv<Instr_Total>> instr_vec;
        sc_signal<bool> exception;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;
    };

    struct ExecuteType {
        sc_signal<bool> trap_ready;
        sc_signal<bool> valid;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> npc;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> ex_npc;

        sc_signal<sc_uint<6>> radr1;
        sc_signal<sc_uint<6>> radr2;
        sc_signal<sc_uint<6>> res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> res_data;
        sc_signal<bool> mret;
        sc_signal<bool> uret;
        sc_signal<sc_uint<12>> csr_addr;
        sc_signal<bool> csr_wena;
        sc_signal<sc_uint<RISCV_ARCH>> csr_wdata;
        sc_signal<bool> ex_instr_load_fault;
        sc_signal<bool> ex_instr_not_executable;
        sc_signal<bool> ex_illegal_instr;
        sc_signal<bool> ex_unalign_load;
        sc_signal<bool> ex_unalign_store;
        sc_signal<bool> ex_breakpoint;
        sc_signal<bool> ex_ecall;
        sc_signal<bool> ex_fpu_invalidop;            // FPU Exception: invalid operation
        sc_signal<bool> ex_fpu_divbyzero;            // FPU Exception: divide by zero
        sc_signal<bool> ex_fpu_overflow;             // FPU Exception: overflow
        sc_signal<bool> ex_fpu_underflow;            // FPU Exception: underflow
        sc_signal<bool> ex_fpu_inexact;              // FPU Exception: inexact
        sc_signal<bool> fpu_valid;

        sc_signal<bool> memop_sign_ext;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_store;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> memop_addr;
        sc_signal<bool> pipeline_hold;           // Hold pipeline from Execution stage
        sc_signal<bool> call;                       // pseudo-instruction CALL
        sc_signal<bool> ret;                        // pseudo-instruction RET
    };

    struct MemoryType {
        sc_signal<bool> memop_ready;;
        sc_signal<bool> valid;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<bool> pipeline_hold;
    };

    struct WriteBackType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<bool> wena;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
    };

    struct IntRegsType {
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
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
        sc_signal<sc_uint<RISCV_ARCH>> rdata;
        sc_signal<sc_uint<RISCV_ARCH>> dport_rdata;
        sc_signal<bool> trap_valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> trap_pc;
        sc_signal<bool> break_event;             // ebreak detected 1 clock pulse
    } csr;

    struct DebugType {
        sc_signal<sc_uint<12>> core_addr;           // Address of the sub-region register
        sc_signal<sc_uint<RISCV_ARCH>> core_wdata;  // Write data
        sc_signal<bool> csr_ena;                    // Region 0: Access to CSR bank is enabled.
        sc_signal<bool> csr_write;                  // Region 0: CSR write enable
        sc_signal<bool> ireg_ena;                   // Region 1: Access to integer register bank is enabled
        sc_signal<bool> ireg_write;                 // Region 1: Integer registers bank write pulse
        sc_signal<bool> freg_ena;                   // Region 1: Access to float register bank is enabled
        sc_signal<bool> freg_write;                 // Region 1: Float registers bank write pulse
        sc_signal<bool> npc_write;                  // Region 1: npc write enable
        sc_signal<bool> halt;                       // Halt signal is equal to hold pipeline
        sc_signal<sc_uint<64>> clock_cnt;           // Number of clocks excluding halt state
        sc_signal<sc_uint<64>> executed_cnt;        // Number of executed instruction
        sc_signal<bool> break_mode;                          // Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
        sc_signal<bool> br_fetch_valid;                      // Fetch injection address/instr are valid
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> br_address_fetch; // Fetch injection address to skip ebreak instruciton only once
        sc_signal<sc_uint<32>> br_instr_fetch;               // Real instruction value that was replaced by ebreak
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> flush_address;    // Address of instruction to remove from ICache
        sc_signal<bool> flush_valid;                         // Remove address from ICache is valid
    } dbg;

    struct BranchPredictorType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> npc;
    } bp;

    /** 5-stages CPU pipeline */
    struct PipelineType {
        FetchType f;                            // Fetch instruction stage
        InstructionDecodeType d;                // Decode instruction stage
        ExecuteType e;                          // Execute instruction
        MemoryType m;                           // Memory load/store
        WriteBackType w;                        // Write back registers value
    } w;

    sc_signal<sc_uint<5>> wb_ireg_dport_addr;
    sc_signal<sc_uint<5>> wb_freg_dport_addr;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_exec_dport_npc;

    sc_signal<bool> w_fetch_pipeline_hold;
    sc_signal<bool> w_any_pipeline_hold;
    sc_signal<bool> w_exec_pipeline_hold;

    InstrFetch *fetch0;
    InstrDecoder *dec0;
    InstrExecute *exec0;
    MemAccess *mem0;

    BranchPredictor *predic0;
    RegIntBank *iregs0;
    RegFloatBank *fregs0;
    CsrRegs *csr0;
    Tracer *trace0;

    DbgPort *dbg0;

    /** Used only for reference trace generation to compare with
        functional model */
    bool generate_ref_;
    sc_event print_event_;
    char tstr[1024];
    ofstream *reg_dbg;
    ofstream *mem_dbg;
    bool mem_dbg_write_flag;
    uint64_t dbg_mem_value_mask;
    uint64_t dbg_mem_write_value;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_PROC_H__
