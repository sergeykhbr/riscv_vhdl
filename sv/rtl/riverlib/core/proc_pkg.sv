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
package proc_pkg;

import river_cfg_pkg::*;

localparam string trace_file = "trace_river_sysc";

typedef struct {
    logic instr_load_fault;
    logic instr_page_fault_x;
    logic [RISCV_ARCH-1:0] requested_pc;                    // requested but responded address
    logic [RISCV_ARCH-1:0] fetching_pc;                     // receiving from cache before latch
    logic [RISCV_ARCH-1:0] pc;
    logic [63:0] instr;
    logic imem_req_valid;
    logic [RISCV_ARCH-1:0] imem_req_addr;
    logic imem_resp_ready;
} FetchType;

typedef struct {
    logic req_ready;
    logic valid;
    logic [RISCV_ARCH-1:0] addr;
    logic [63:0] data;
    logic load_fault;
    logic store_fault;
    logic page_fault_x;
    logic page_fault_r;
    logic page_fault_w;
} MmuType;

typedef struct {
    logic [RISCV_ARCH-1:0] pc;
    logic [31:0] instr;
    logic memop_store;
    logic memop_load;
    logic memop_sign_ext;
    logic [1:0] memop_size;
    logic rv32;                                             // 32-bits instruction
    logic compressed;                                       // C-extension
    logic amo;                                              // A-extension
    logic f64;                                              // D-extension (FPU)
    logic unsigned_op;                                      // Unsigned operands
    logic [ISA_Total-1:0] isa_type;
    logic [Instr_Total-1:0] instr_vec;
    logic exception;
    logic instr_load_fault;
    logic page_fault_x;
    logic [5:0] radr1;
    logic [5:0] radr2;
    logic [5:0] waddr;
    logic [11:0] csr_addr;
    logic [RISCV_ARCH-1:0] imm;
    logic progbuf_ena;
} InstructionDecodeType;

typedef struct {
    logic valid;
    logic [31:0] instr;
    logic [RISCV_ARCH-1:0] pc;
    logic [RISCV_ARCH-1:0] npc;
    logic [5:0] radr1;
    logic [5:0] radr2;
    logic reg_wena;
    logic [5:0] reg_waddr;
    logic [CFG_REG_TAG_WIDTH-1:0] reg_wtag;
    logic [RISCV_ARCH-1:0] reg_wdata;
    logic csr_req_valid;                                    // Access to CSR request
    logic [CsrReq_TotalBits-1:0] csr_req_type;              // Request type: [0]-read csr; [1]-write csr; [2]-change mode
    logic [11:0] csr_req_addr;                              // Requested CSR address
    logic [RISCV_ARCH-1:0] csr_req_data;                    // CSR new value
    logic csr_resp_ready;                                   // Executor is ready to accept response
    logic memop_valid;
    logic memop_debug;
    logic memop_sign_ext;
    logic [MemopType_Total-1:0] memop_type;
    logic [1:0] memop_size;
    logic [RISCV_ARCH-1:0] memop_addr;
    logic [RISCV_ARCH-1:0] memop_wdata;
    logic call;                                             // pseudo-instruction CALL
    logic ret;                                              // pseudo-instruction RET
    logic jmp;                                              // jump was executed
    logic halted;
    logic dbg_mem_req_ready;
    logic dbg_mem_req_error;
} ExecuteType;

typedef struct {
    logic memop_ready;
    logic flushd;
    logic [RISCV_ARCH-1:0] pc;
    logic valid;
    logic idle;
    logic debug_valid;
    logic dmmu_ena;
    logic dmmu_sv39;
    logic dmmu_sv48;
    logic req_data_valid;
    logic [MemopType_Total-1:0] req_data_type;
    logic [RISCV_ARCH-1:0] req_data_addr;
    logic [63:0] req_data_wdata;
    logic [7:0] req_data_wstrb;
    logic [1:0] req_data_size;
    logic resp_data_ready;
} MemoryType;

typedef struct {
    logic wena;
    logic [5:0] waddr;
    logic [RISCV_ARCH-1:0] wdata;
    logic [CFG_REG_TAG_WIDTH-1:0] wtag;
} WriteBackType;

typedef struct {
    logic [RISCV_ARCH-1:0] rdata1;
    logic [CFG_REG_TAG_WIDTH-1:0] rtag1;
    logic [RISCV_ARCH-1:0] rdata2;
    logic [CFG_REG_TAG_WIDTH-1:0] rtag2;
    logic [RISCV_ARCH-1:0] dport_rdata;
    logic [RISCV_ARCH-1:0] ra;                              // Return address
    logic [RISCV_ARCH-1:0] sp;                              // Stack pointer
    logic [RISCV_ARCH-1:0] gp;
    logic [RISCV_ARCH-1:0] tp;
    logic [RISCV_ARCH-1:0] t0;
    logic [RISCV_ARCH-1:0] t1;
    logic [RISCV_ARCH-1:0] t2;
    logic [RISCV_ARCH-1:0] fp;
    logic [RISCV_ARCH-1:0] s1;
    logic [RISCV_ARCH-1:0] a0;
    logic [RISCV_ARCH-1:0] a1;
    logic [RISCV_ARCH-1:0] a2;
    logic [RISCV_ARCH-1:0] a3;
    logic [RISCV_ARCH-1:0] a4;
    logic [RISCV_ARCH-1:0] a5;
    logic [RISCV_ARCH-1:0] a6;
    logic [RISCV_ARCH-1:0] a7;
    logic [RISCV_ARCH-1:0] s2;
    logic [RISCV_ARCH-1:0] s3;
    logic [RISCV_ARCH-1:0] s4;
    logic [RISCV_ARCH-1:0] s5;
    logic [RISCV_ARCH-1:0] s6;
    logic [RISCV_ARCH-1:0] s7;
    logic [RISCV_ARCH-1:0] s8;
    logic [RISCV_ARCH-1:0] s9;
    logic [RISCV_ARCH-1:0] s10;
    logic [RISCV_ARCH-1:0] s11;
    logic [RISCV_ARCH-1:0] t3;
    logic [RISCV_ARCH-1:0] t4;
    logic [RISCV_ARCH-1:0] t5;
    logic [RISCV_ARCH-1:0] t6;
} IntRegsType;

typedef struct {
    logic req_ready;                                        // CSR module is ready to accept request
    logic resp_valid;                                       // CSR module Response is valid
    logic [RISCV_ARCH-1:0] resp_data;                       // Responded CSR data
    logic resp_exception;                                   // Exception of CSR access
    logic flushd_valid;                                     // clear specified addr in D$
    logic flushi_valid;                                     // clear specified addr in I$
    logic flushmmu_valid;                                   // clear specified leaf in xMMU
    logic flushpipeline_valid;                              // clear pipeline
    logic [RISCV_ARCH-1:0] flush_addr;
    logic [63:0] executed_cnt;                              // Number of executed instruction
    logic [IRQ_TOTAL-1:0] irq_pending;
    logic o_wakeup;                                         // There's pending bit even if interrupts globally disabled
    logic stack_overflow;
    logic stack_underflow;
    logic step;
    logic mmu_ena;                                          // MMU enabled in U and S modes. Sv48 only.
    logic mmu_sv39;
    logic mmu_sv48;
    logic [43:0] mmu_ppn;                                   // Physical Page Number
    logic mprv;
    logic mxr;
    logic sum;
    logic progbuf_end;
    logic progbuf_error;
} CsrType;

typedef struct {
    logic csr_req_valid;
    logic [CsrReq_TotalBits-1:0] csr_req_type;
    logic [11:0] csr_req_addr;                              // Address of the sub-region register
    logic [RISCV_ARCH-1:0] csr_req_data;
    logic csr_resp_ready;
    logic [5:0] ireg_addr;
    logic [RISCV_ARCH-1:0] ireg_wdata;                      // Write data
    logic ireg_ena;                                         // Region 1: Access to integer register bank is enabled
    logic ireg_write;                                       // Region 1: Integer registers bank write pulse
    logic mem_req_valid;                                    // Type 2: request is valid
    logic mem_req_write;                                    // Type 2: is write
    logic [RISCV_ARCH-1:0] mem_req_addr;                    // Type 2: Debug memory request
    logic [1:0] mem_req_size;                               // Type 2: memory operation size: 0=1B; 1=2B; 2=4B; 3=8B
    logic [RISCV_ARCH-1:0] mem_req_wdata;                   // Type 2: memory write data
    logic progbuf_ena;                                      // execute instruction from progbuf
    logic [RISCV_ARCH-1:0] progbuf_pc;                      // progbuf instruction counter
    logic [63:0] progbuf_instr;                             // progbuf instruction to execute
} DebugType;

typedef struct {
    logic f_valid;
    logic [RISCV_ARCH-1:0] f_pc;
} BranchPredictorType;

typedef struct {
    FetchType f;                                            // Fetch instruction stage
    InstructionDecodeType d;                                // Decode instruction stage
    ExecuteType e;                                          // Execute instruction
    MemoryType m;                                           // Memory load/store
    WriteBackType w;                                        // Write back registers value
} PipelineType;


endpackage: proc_pkg
