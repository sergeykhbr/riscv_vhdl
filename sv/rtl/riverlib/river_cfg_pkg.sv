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
package river_cfg_pkg;


localparam bit [31:0] CFG_VENDOR_ID = 32'h000000f1;
localparam bit [31:0] CFG_IMPLEMENTATION_ID = 32'h20220813;
localparam bit CFG_HW_FPU_ENABLE = 1'b1;
localparam bit CFG_TRACER_ENABLE = 1'b0;

// Architectural size definition
localparam int RISCV_ARCH = 64;

localparam int CFG_CPU_ADDR_BITS = 48;
localparam int CFG_CPU_ID_BITS = 1;
localparam int CFG_CPU_USER_BITS = 1;

// 
// 2**Number of CPU slots in the clusters. Some of them could be unavailable
// 
localparam int CFG_LOG2_CPU_MAX = 2;                        // 1=Dual-core (maximum); 2=Quad-core (maximum), 3=..
localparam int CFG_CPU_MAX = (2**CFG_LOG2_CPU_MAX);
// +1 ACP coherent port
localparam int CFG_SLOT_L1_TOTAL = (CFG_CPU_MAX + 1);

// Power-on start address can be free changed
localparam bit [CFG_CPU_ADDR_BITS-1:0] CFG_RESET_VECTOR = 48'h000000010000;

// 
// Branch Predictor Branch Target Buffer (BTB) size
// 
localparam int CFG_BTB_SIZE = 8;
// Branch predictor depth. It is better when it is equal to the pipeline depth excluding fetcher.
// Let it be equal to the decoder's history depth
localparam int CFG_BP_DEPTH = 5;

// 
// Decoded instructions history buffer size in Decoder
// 
localparam int CFG_DEC_DEPTH = (CFG_BP_DEPTH - 3);          // requested, fetching, fetched

// Valid size 0..16
localparam int CFG_PROGBUF_REG_TOTAL = 16;
// Must be at least 2 to support RV64I
localparam int CFG_DATA_REG_TOTAL = 4;
// Total number of dscratch registers
localparam int CFG_DSCRATCH_REG_TOTAL = 2;
// Number of elements each 2*CFG_ADDR_WIDTH in stack trace buffer:
localparam int CFG_LOG2_STACK_TRACE_ADDR = 5;
localparam int STACK_TRACE_BUF_SIZE = (2**CFG_LOG2_STACK_TRACE_ADDR);


// 
// L1 cache common parameters (suppose I$ and D$ have the same size)
// 
localparam int CFG_LOG2_L1CACHE_BYTES_PER_LINE = 5;         // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
localparam int L1CACHE_BYTES_PER_LINE = (2**CFG_LOG2_L1CACHE_BYTES_PER_LINE);
localparam int L1CACHE_LINE_BITS = (8 * L1CACHE_BYTES_PER_LINE);
// D-Cache flags:
localparam int TAG_FL_VALID = 0;                            // always 0
localparam int DTAG_FL_DIRTY = 1;
localparam int DTAG_FL_SHARED = 2;
localparam int DTAG_FL_RESERVED = 3;
localparam int DTAG_FL_TOTAL = 4;
// I-Cache flags:
localparam int ITAG_FL_TOTAL = 1;

localparam int SNOOP_REQ_TYPE_READDATA = 0;                 // 0=check flags; 1=data transfer
localparam int SNOOP_REQ_TYPE_READCLEAN = 1;                // 0=do nothing; 1=read and invalidate line
localparam int SNOOP_REQ_TYPE_BITS = 2;


// 
// L2 cache config (River 16 KB, 2 ways by default, Wasserfall 64 KB, 4 ways)
// 
localparam int CFG_L2_LOG2_BYTES_PER_LINE = 5;              // [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
localparam int L2CACHE_BYTES_PER_LINE = (2**CFG_L2_LOG2_BYTES_PER_LINE);
localparam int L2CACHE_LINE_BITS = (8 * L2CACHE_BYTES_PER_LINE);

localparam int L2TAG_FL_DIRTY = 1;
localparam int L2TAG_FL_TOTAL = 2;

localparam int L2_REQ_TYPE_WRITE = 0;
localparam int L2_REQ_TYPE_CACHED = 1;
localparam int L2_REQ_TYPE_UNIQUE = 2;
localparam int L2_REQ_TYPE_SNOOP = 3;                       // Use data received through snoop channel (no memory request)
localparam int L2_REQ_TYPE_BITS = 4;

// PMP config
localparam int CFG_PMP_TBL_WIDTH = 3;                       // [1:0]  log2(MPU_TBL_SIZE)
localparam int CFG_PMP_TBL_SIZE = (2**CFG_PMP_TBL_WIDTH);

localparam int CFG_PMP_FL_R = 0;
localparam int CFG_PMP_FL_W = 1;
localparam int CFG_PMP_FL_X = 2;
localparam int CFG_PMP_FL_L = 3;
localparam int CFG_PMP_FL_V = 4;
localparam int CFG_PMP_FL_TOTAL = 5;

// MMU config. Fetch and Data pathes have its own MMU block
localparam int CFG_MMU_TLB_AWIDTH = 6;                      // TLB memory address bus width
localparam int CFG_MMU_TLB_SIZE = (2**CFG_MMU_TLB_AWIDTH);  // Number of PTE entries in a table
localparam int CFG_MMU_PTE_DWIDTH = ((2 * RISCV_ARCH) - 12);// PTE entry size in bits
localparam int CFG_MMU_PTE_DBYTES = (CFG_MMU_PTE_DWIDTH / 8);// PTE entry size in bytes


localparam bit [1:0] MEMOP_8B = 2'h3;
localparam bit [1:0] MEMOP_4B = 2'h2;
localparam bit [1:0] MEMOP_2B = 2'h1;
localparam bit [1:0] MEMOP_1B = 2'h0;

// Integer Registers specified by ISA
localparam int REG_ZERO = 0;
localparam int REG_RA = 1;                                  // [1] Return address
localparam int REG_SP = 2;                                  // [2] Stack pointer
localparam int REG_GP = 3;                                  // [3] Global pointer
localparam int REG_TP = 4;                                  // [4] Thread pointer
localparam int REG_T0 = 5;                                  // [5] Temporaries 0 s3
localparam int REG_T1 = 6;                                  // [6] Temporaries 1 s4
localparam int REG_T2 = 7;                                  // [7] Temporaries 2 s5
localparam int REG_S0 = 8;                                  // [8] s0/fp Saved register/frame pointer
localparam int REG_S1 = 9;                                  // [9] Saved register 1
localparam int REG_A0 = 10;                                 // [10] Function argumentes 0
localparam int REG_A1 = 11;                                 // [11] Function argumentes 1
localparam int REG_A2 = 12;                                 // [12] Function argumentes 2
localparam int REG_A3 = 13;                                 // [13] Function argumentes 3
localparam int REG_A4 = 14;                                 // [14] Function argumentes 4
localparam int REG_A5 = 15;                                 // [15] Function argumentes 5
localparam int REG_A6 = 16;                                 // [16] Function argumentes 6
localparam int REG_A7 = 17;                                 // [17] Function argumentes 7
localparam int REG_S2 = 18;                                 // [18] Saved register 2
localparam int REG_S3 = 19;                                 // [19] Saved register 3
localparam int REG_S4 = 20;                                 // [20] Saved register 4
localparam int REG_S5 = 21;                                 // [21] Saved register 5
localparam int REG_S6 = 22;                                 // [22] Saved register 6
localparam int REG_S7 = 23;                                 // [23] Saved register 7
localparam int REG_S8 = 24;                                 // [24] Saved register 8
localparam int REG_S9 = 25;                                 // [25] Saved register 9
localparam int REG_S10 = 26;                                // [26] Saved register 10
localparam int REG_S11 = 27;                                // [27] Saved register 11
localparam int REG_T3 = 28;                                 // [28]
localparam int REG_T4 = 29;                                 // [29]
localparam int REG_T5 = 30;                                 // [30]
localparam int REG_T6 = 31;                                 // [31]

localparam int REG_F0 = 0;                                  // ft0 temporary register
localparam int REG_F1 = 1;                                  // ft1
localparam int REG_F2 = 2;                                  // ft2
localparam int REG_F3 = 3;                                  // ft3
localparam int REG_F4 = 4;                                  // ft4
localparam int REG_F5 = 5;                                  // ft5
localparam int REG_F6 = 6;                                  // ft6
localparam int REG_F7 = 7;                                  // ft7
localparam int REG_F8 = 8;                                  // fs0 saved register
localparam int REG_F9 = 9;                                  // fs1
localparam int REG_F10 = 10;                                // fa0 argument/return value
localparam int REG_F11 = 11;                                // fa1 argument/return value
localparam int REG_F12 = 12;                                // fa2 argument register
localparam int REG_F13 = 13;                                // fa3
localparam int REG_F14 = 14;                                // fa4
localparam int REG_F15 = 15;                                // fa5
localparam int REG_F16 = 16;                                // fa6
localparam int REG_F17 = 17;                                // fa7
localparam int REG_F18 = 18;                                // fs2 saved register
localparam int REG_F19 = 19;                                // fs3
localparam int REG_F20 = 20;                                // fs4
localparam int REG_F21 = 21;                                // fs5
localparam int REG_F22 = 22;                                // fs6
localparam int REG_F23 = 23;                                // fs7
localparam int REG_F24 = 24;                                // fs8
localparam int REG_F25 = 25;                                // fs9
localparam int REG_F26 = 26;                                // fs10
localparam int REG_F27 = 27;                                // fs11
localparam int REG_F28 = 28;                                // ft8 temporary register
localparam int REG_F29 = 29;                                // ft9
localparam int REG_F30 = 30;                                // ft10
localparam int REG_F31 = 31;                                // ft11

localparam int INTREGS_TOTAL = 32;
localparam int FPUREGS_OFFSET = INTREGS_TOTAL;
localparam int FPUREGS_TOTAL = 32;

localparam int REGS_BUS_WIDTH = 6;
localparam int REGS_TOTAL = (2**REGS_BUS_WIDTH);            // INTREGS_TOTAL + FPUREGS_TOTAL


// Instruction formats specified by ISA specification
localparam int ISA_R_type = 0;
localparam int ISA_I_type = 1;
localparam int ISA_S_type = 2;
localparam int ISA_SB_type = 3;
localparam int ISA_U_type = 4;
localparam int ISA_UJ_type = 5;
localparam int ISA_Total = 6;

// Implemented instruction list and its indexes
localparam int Instr_ADD = 0;
localparam int Instr_ADDI = 1;
localparam int Instr_ADDIW = 2;
localparam int Instr_ADDW = 3;
localparam int Instr_AND = 4;
localparam int Instr_ANDI = 5;
localparam int Instr_AUIPC = 6;
localparam int Instr_BEQ = 7;
localparam int Instr_BGE = 8;
localparam int Instr_BGEU = 9;
localparam int Instr_BLT = 10;
localparam int Instr_BLTU = 11;
localparam int Instr_BNE = 12;
localparam int Instr_JAL = 13;
localparam int Instr_JALR = 14;
localparam int Instr_LB = 15;
localparam int Instr_LH = 16;
localparam int Instr_LW = 17;
localparam int Instr_LD = 18;
localparam int Instr_LBU = 19;
localparam int Instr_LHU = 20;
localparam int Instr_LWU = 21;
localparam int Instr_LUI = 22;
localparam int Instr_OR = 23;
localparam int Instr_ORI = 24;
localparam int Instr_SLLI = 25;
localparam int Instr_SLT = 26;
localparam int Instr_SLTI = 27;
localparam int Instr_SLTU = 28;
localparam int Instr_SLTIU = 29;
localparam int Instr_SLL = 30;
localparam int Instr_SLLW = 31;
localparam int Instr_SLLIW = 32;
localparam int Instr_SRA = 33;
localparam int Instr_SRAW = 34;
localparam int Instr_SRAI = 35;
localparam int Instr_SRAIW = 36;
localparam int Instr_SRL = 37;
localparam int Instr_SRLI = 38;
localparam int Instr_SRLIW = 39;
localparam int Instr_SRLW = 40;
localparam int Instr_SB = 41;
localparam int Instr_SH = 42;
localparam int Instr_SW = 43;
localparam int Instr_SD = 44;
localparam int Instr_SUB = 45;
localparam int Instr_SUBW = 46;
localparam int Instr_XOR = 47;
localparam int Instr_XORI = 48;
localparam int Instr_CSRRW = 49;
localparam int Instr_CSRRS = 50;
localparam int Instr_CSRRC = 51;
localparam int Instr_CSRRWI = 52;
localparam int Instr_CSRRCI = 53;
localparam int Instr_CSRRSI = 54;
localparam int Instr_URET = 55;
localparam int Instr_SRET = 56;
localparam int Instr_HRET = 57;
localparam int Instr_MRET = 58;
localparam int Instr_FENCE = 59;
localparam int Instr_FENCE_I = 60;
localparam int Instr_WFI = 61;
localparam int Instr_SFENCE_VMA = 62;
localparam int Instr_DIV = 63;
localparam int Instr_DIVU = 64;
localparam int Instr_DIVW = 65;
localparam int Instr_DIVUW = 66;
localparam int Instr_MUL = 67;
localparam int Instr_MULW = 68;
localparam int Instr_MULH = 69;
localparam int Instr_MULHSU = 70;
localparam int Instr_MULHU = 71;
localparam int Instr_REM = 72;
localparam int Instr_REMU = 73;
localparam int Instr_REMW = 74;
localparam int Instr_REMUW = 75;
localparam int Instr_AMOADD_W = 76;
localparam int Instr_AMOXOR_W = 77;
localparam int Instr_AMOOR_W = 78;
localparam int Instr_AMOAND_W = 79;
localparam int Instr_AMOMIN_W = 80;
localparam int Instr_AMOMAX_W = 81;
localparam int Instr_AMOMINU_W = 82;
localparam int Instr_AMOMAXU_W = 83;
localparam int Instr_AMOSWAP_W = 84;
localparam int Instr_LR_W = 85;
localparam int Instr_SC_W = 86;
localparam int Instr_AMOADD_D = 87;
localparam int Instr_AMOXOR_D = 88;
localparam int Instr_AMOOR_D = 89;
localparam int Instr_AMOAND_D = 90;
localparam int Instr_AMOMIN_D = 91;
localparam int Instr_AMOMAX_D = 92;
localparam int Instr_AMOMINU_D = 93;
localparam int Instr_AMOMAXU_D = 94;
localparam int Instr_AMOSWAP_D = 95;
localparam int Instr_LR_D = 96;
localparam int Instr_SC_D = 97;
localparam int Instr_ECALL = 98;
localparam int Instr_EBREAK = 99;
localparam int Instr_FADD_D = 100;
localparam int Instr_FCVT_D_W = 101;
localparam int Instr_FCVT_D_WU = 102;
localparam int Instr_FCVT_D_L = 103;
localparam int Instr_FCVT_D_LU = 104;
localparam int Instr_FCVT_W_D = 105;
localparam int Instr_FCVT_WU_D = 106;
localparam int Instr_FCVT_L_D = 107;
localparam int Instr_FCVT_LU_D = 108;
localparam int Instr_FDIV_D = 109;
localparam int Instr_FEQ_D = 110;
localparam int Instr_FLD = 111;
localparam int Instr_FLE_D = 112;
localparam int Instr_FLT_D = 113;
localparam int Instr_FMAX_D = 114;
localparam int Instr_FMIN_D = 115;
localparam int Instr_FMOV_D_X = 116;
localparam int Instr_FMOV_X_D = 117;
localparam int Instr_FMUL_D = 118;
localparam int Instr_FSD = 119;
localparam int Instr_FSUB_D = 120;
localparam int Instr_Total = 121;

localparam int Instr_FPU_First = Instr_FADD_D;
localparam int Instr_FPU_Last = Instr_FSUB_D;
localparam int Instr_FPU_Total = ((Instr_FSUB_D - Instr_FADD_D) + 1);


// @name PRV bits possible values:
// @{
// User-mode
localparam bit [1:0] PRV_U = 2'h0;
// super-visor mode
localparam bit [1:0] PRV_S = 2'h1;
// hyper-visor mode
localparam bit [1:0] PRV_H = 2'h2;
// machine mode
localparam bit [1:0] PRV_M = 2'h3;
// @}

// Dport request types:
localparam int DPortReq_Write = 0;
localparam int DPortReq_RegAccess = 1;
localparam int DPortReq_MemAccess = 2;
localparam int DPortReq_MemVirtual = 3;
localparam int DPortReq_Progexec = 4;
localparam int DPortReq_Total = 5;

// DCSR register halt causes:
localparam bit [2:0] HALT_CAUSE_EBREAK = 3'h1;              // software breakpoint
localparam bit [2:0] HALT_CAUSE_TRIGGER = 3'h2;             // hardware breakpoint
localparam bit [2:0] HALT_CAUSE_HALTREQ = 3'h3;             // halt request via debug interface
localparam bit [2:0] HALT_CAUSE_STEP = 3'h4;                // step done
localparam bit [2:0] HALT_CAUSE_RESETHALTREQ = 3'h5;        // not implemented

localparam bit [2:0] PROGBUF_ERR_NONE = 3'h0;               // no error
localparam bit [2:0] PROGBUF_ERR_BUSY = 3'h1;               // abstract command in progress
localparam bit [2:0] PROGBUF_ERR_NOT_SUPPORTED = 3'h2;      // Request command not supported
localparam bit [2:0] PROGBUF_ERR_EXCEPTION = 3'h3;          // Exception occurs while executing progbuf
localparam bit [2:0] PROGBUF_ERR_HALT_RESUME = 3'h4;        // Command cannot be executed because of wrong CPU state
localparam bit [2:0] PROGBUF_ERR_BUS = 3'h5;                // Bus error occurs
localparam bit [2:0] PROGBUF_ERR_OTHER = 3'h7;              // Other reason

localparam int EXCEPTION_InstrMisalign = 0;                 // Instruction address misaligned
localparam int EXCEPTION_InstrFault = 1;                    // Instruction access fault
localparam int EXCEPTION_InstrIllegal = 2;                  // Illegal instruction
localparam int EXCEPTION_Breakpoint = 3;                    // Breakpoint
localparam int EXCEPTION_LoadMisalign = 4;                  // Load address misaligned
localparam int EXCEPTION_LoadFault = 5;                     // Load access fault
localparam int EXCEPTION_StoreMisalign = 6;                 // Store/AMO address misaligned
localparam int EXCEPTION_StoreFault = 7;                    // Store/AMO access fault
localparam int EXCEPTION_CallFromUmode = 8;                 // Environment call from U-mode
localparam int EXCEPTION_CallFromSmode = 9;                 // Environment call from S-mode
localparam int EXCEPTION_CallFromHmode = 10;                // Environment call from H-mode
localparam int EXCEPTION_CallFromMmode = 11;                // Environment call from M-mode
localparam int EXCEPTION_InstrPageFault = 12;               // Instruction page fault
localparam int EXCEPTION_LoadPageFault = 13;                // Load page fault
localparam int EXCEPTION_rsrv14 = 14;                       // reserved
localparam int EXCEPTION_StorePageFault = 15;               // Store/AMO page fault
localparam int EXCEPTION_StackOverflow = 16;                // Stack overflow
localparam int EXCEPTION_StackUnderflow = 17;               // Stack underflow
localparam int EXCEPTIONS_Total = 64;

localparam int EXCEPTION_CallFromXMode = EXCEPTION_CallFromUmode;

// Per Hart Interrupt bus
localparam int IRQ_SSIP = 1;                                // Supervisor software pening interrupt
localparam int IRQ_MSIP = 3;                                // Machine software pening interrupt
localparam int IRQ_STIP = 5;                                // Supervisor timer pening interrupt
localparam int IRQ_MTIP = 7;                                // Machine timer pening interrupt
localparam int IRQ_SEIP = 9;                                // Supervisor external pening interrupt
localparam int IRQ_MEIP = 11;                               // Machine external pening interrupt
localparam int IRQ_TOTAL = 16;                              // Total number of direct core interrupt requests
// Depth of the fifo between Executor and MemoryAccess modules.
localparam int CFG_MEMACCESS_QUEUE_DEPTH = 2;
// Register's tag used to detect reg hazard and it should be higher than available
// slots in the fifo Executor => Memaccess.
localparam int CFG_REG_TAG_WIDTH = 3;

// Request type: [0]-read csr; [1]-write csr; [2]-change mode
localparam int CsrReq_ReadBit = 0;
localparam int CsrReq_WriteBit = 1;
localparam int CsrReq_TrapReturnBit = 2;
localparam int CsrReq_ExceptionBit = 3;                     // return instruction pointer
localparam int CsrReq_InterruptBit = 4;                     // return instruction pointer
localparam int CsrReq_BreakpointBit = 5;
localparam int CsrReq_HaltBit = 6;
localparam int CsrReq_ResumeBit = 7;
localparam int CsrReq_WfiBit = 8;                           // wait for interrupt
localparam int CsrReq_FenceBit = 9;                         // one of Fence instructions
localparam int CsrReq_TotalBits = 10;

localparam bit [CsrReq_TotalBits-1:0] CsrReq_ReadCmd = (2**CsrReq_ReadBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_WriteCmd = (2**CsrReq_WriteBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_TrapReturnCmd = (2**CsrReq_TrapReturnBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_ExceptionCmd = (2**CsrReq_ExceptionBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_InterruptCmd = (2**CsrReq_InterruptBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_BreakpointCmd = (2**CsrReq_BreakpointBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_HaltCmd = (2**CsrReq_HaltBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_ResumeCmd = (2**CsrReq_ResumeBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_WfiCmd = (2**CsrReq_WfiBit);
localparam bit [CsrReq_TotalBits-1:0] CsrReq_FenceCmd = (2**CsrReq_FenceBit);

localparam int MemopType_Store = 0;                         // 0=load; 1=store
localparam int MemopType_Locked = 1;                        // AMO instructions
localparam int MemopType_Reserve = 2;                       // LS load with reserve
localparam int MemopType_Release = 3;                       // SC store with release
localparam int MemopType_Total = 4;

localparam int REQ_MEM_TYPE_WRITE = 0;
localparam int REQ_MEM_TYPE_CACHED = 1;
localparam int REQ_MEM_TYPE_UNIQUE = 2;
localparam int REQ_MEM_TYPE_BITS = 3;

function automatic logic [REQ_MEM_TYPE_BITS-1:0] ReadNoSnoop();
logic [REQ_MEM_TYPE_BITS-1:0] ret;
begin
    ret = '0;
    return ret;
end
endfunction: ReadNoSnoop


function automatic logic [REQ_MEM_TYPE_BITS-1:0] ReadShared();
logic [REQ_MEM_TYPE_BITS-1:0] ret;
begin
    ret = '0;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    return ret;
end
endfunction: ReadShared


function automatic logic [REQ_MEM_TYPE_BITS-1:0] ReadMakeUnique();
logic [REQ_MEM_TYPE_BITS-1:0] ret;
begin
    ret = '0;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    ret[REQ_MEM_TYPE_UNIQUE] = 1;
    return ret;
end
endfunction: ReadMakeUnique


function automatic logic [REQ_MEM_TYPE_BITS-1:0] WriteNoSnoop();
logic [REQ_MEM_TYPE_BITS-1:0] ret;
begin
    ret = '0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    return ret;
end
endfunction: WriteNoSnoop


function automatic logic [REQ_MEM_TYPE_BITS-1:0] WriteLineUnique();
logic [REQ_MEM_TYPE_BITS-1:0] ret;
begin
    ret = '0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    ret[REQ_MEM_TYPE_UNIQUE] = 1;
    return ret;
end
endfunction: WriteLineUnique


function automatic logic [REQ_MEM_TYPE_BITS-1:0] WriteBack();
logic [REQ_MEM_TYPE_BITS-1:0] ret;
begin
    ret = '0;
    ret[REQ_MEM_TYPE_WRITE] = 1;
    ret[REQ_MEM_TYPE_CACHED] = 1;
    return ret;
end
endfunction: WriteBack


endpackage: river_cfg_pkg
