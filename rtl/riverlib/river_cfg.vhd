--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
--! Unless required by applicable law or agreed to in writing, software
--! distributed under the License is distributed on an "AS IS" BASIS,
--! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--! See the License for the specific language governing permissions and
--! limitations under the License.
--!

--! Standard library.
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;

--! @brief   Library global parameters.
package river_cfg is

  constant CFG_VENDOR_ID : std_logic_vector(31 downto 0) := X"000000F1";
  constant CFG_IMPLEMENTATION_ID : std_logic_vector(31 downto 0) := X"20191108";
  constant CFG_HW_FPU_ENABLE : boolean := true;


  --! Architecture size difinition.
  constant RISCV_ARCH : integer := 64;

  --! @name System bus parameters
  --! @brief Constants specify AXI bus global settigns
  --! @{

  --! @brief   Address bus bit-size.
  constant BUS_ADDR_WIDTH : integer := 32;
  --! @brief   Data bus bit-size.
  constant CFG_LOG2_DATA_BYTES : integer := 3;
  constant BUS_DATA_BYTES : integer := (2**CFG_LOG2_DATA_BYTES);
  constant BUS_DATA_WIDTH : integer := 8 * BUS_DATA_BYTES;
  constant LOG2_DATA_BYTES_MASK : integer := (2**CFG_LOG2_DATA_BYTES) - 1;
  --! @}


  -- 
  -- ICacheLru config (16 KB by default)
  --
  constant CFG_ILOG2_BYTES_PER_LINE : integer := 5;    -- [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
  constant CFG_ILOG2_LINES_PER_WAY  : integer := 7;
  constant CFG_ILOG2_NWAYS          : integer := 2;

  -- Derivatives I$ constants:
  constant ICACHE_BYTES_PER_LINE    : integer := 2**CFG_ILOG2_BYTES_PER_LINE;
  constant ICACHE_LINES_PER_WAY     : integer := 2**CFG_ILOG2_LINES_PER_WAY;
  constant ICACHE_WAYS              : integer := 2**CFG_ILOG2_NWAYS;

  constant ICACHE_LOG2_BURST_LEN    : integer :=
                  CFG_ILOG2_BYTES_PER_LINE - CFG_LOG2_DATA_BYTES;
  constant ICACHE_BURST_LEN         : integer := 2**ICACHE_LOG2_BURST_LEN;
  constant ICACHE_LINE_BITS         : integer := 8*ICACHE_BYTES_PER_LINE;

  -- Information: To define the CACHE SIZE in Bytes use the following:
  constant ICACHE_SIZE_BYTES : integer :=
      ICACHE_WAYS * ICACHE_LINES_PER_WAY * ICACHE_BYTES_PER_LINE;

  constant ITAG_FL_TOTAL      : integer := 1;


  -- 
  -- DCacheLru config (16 KB by default)
  --
  constant CFG_DLOG2_BYTES_PER_LINE : integer := 5;    -- [4:0] 32 Bytes = 4x8 B log2(Bytes per line)
  constant CFG_DLOG2_LINES_PER_WAY  : integer := 7;
  constant CFG_DLOG2_NWAYS          : integer := 2;

  -- Derivatives D$ constants:
  constant DCACHE_BYTES_PER_LINE    : integer := 2**CFG_DLOG2_BYTES_PER_LINE;
  constant DCACHE_LINES_PER_WAY     : integer := 2**CFG_DLOG2_LINES_PER_WAY;
  constant DCACHE_WAYS              : integer := 2**CFG_DLOG2_NWAYS;

  constant DCACHE_LOG2_BURST_LEN    : integer :=
                CFG_DLOG2_BYTES_PER_LINE - CFG_LOG2_DATA_BYTES;
  constant DCACHE_BURST_LEN         : integer := 2**DCACHE_LOG2_BURST_LEN;
  constant DCACHE_LINE_BITS         : integer := 8*DCACHE_BYTES_PER_LINE;

  -- Information: To define the CACHE SIZE in Bytes use the following:
  constant DCACHE_SIZE_BYTES : integer :=
      DCACHE_WAYS * DCACHE_LINES_PER_WAY * DCACHE_BYTES_PER_LINE;

  constant TAG_FL_VALID       : integer := 0;    -- always 0
  constant DTAG_FL_DIRTY      : integer := 1;
  constant DTAG_FL_LOAD_FAULT : integer := 2;
  constant DTAG_FL_TOTAL      : integer := 3;

  -- MPU config:
  constant CFG_MPU_TBL_WIDTH   : integer := 2;    -- [1:0]  log2(MPU_TBL_SIZE)
  constant CFG_MPU_TBL_SIZE    : integer := 2**CFG_MPU_TBL_WIDTH;

  constant CFG_MPU_FL_WR       : integer := 0;
  constant CFG_MPU_FL_RD       : integer := 1;
  constant CFG_MPU_FL_EXEC     : integer := 2;
  constant CFG_MPU_FL_CACHABLE : integer := 3;
  constant CFG_MPU_FL_ENA      : integer := 4;
  constant CFG_MPU_FL_TOTAL    : integer := 5;


  --! @name   Encoded Memory operation size values
  --! @{

  constant MEMOP_8B : std_logic_vector(1 downto 0) := "11";
  constant MEMOP_4B : std_logic_vector(1 downto 0) := "10";
  constant MEMOP_2B : std_logic_vector(1 downto 0) := "01";
  constant MEMOP_1B : std_logic_vector(1 downto 0) := "00";
  --! @}
  
  --! Non-maskable interrupts (exceptions) table.
  --!  It can be freely changed to optimize memory consumption/performance
  --!
  constant CFG_NMI_RESET_VECTOR         : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000000";
  constant CFG_NMI_INSTR_UNALIGNED_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000008";
  constant CFG_NMI_INSTR_FAULT_ADDR     : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000010";
  constant CFG_NMI_INSTR_ILLEGAL_ADDR   : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000018";
  constant CFG_NMI_BREAKPOINT_ADDR      : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000020";
  constant CFG_NMI_LOAD_UNALIGNED_ADDR  : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000028";
  constant CFG_NMI_LOAD_FAULT_ADDR      : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000030";
  constant CFG_NMI_STORE_UNALIGNED_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000038";
  constant CFG_NMI_STORE_FAULT_ADDR     : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000040";
  constant CFG_NMI_CALL_FROM_UMODE_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000048";
  constant CFG_NMI_CALL_FROM_SMODE_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000050";
  constant CFG_NMI_CALL_FROM_HMODE_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000058";
  constant CFG_NMI_CALL_FROM_MMODE_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000060";
  constant CFG_NMI_INSTR_PAGE_FAULT_ADDR: std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000068";
  constant CFG_NMI_LOAD_PAGE_FAULT_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000070";
  constant CFG_NMI_STORE_PAGE_FAULT_ADDR: std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000078";
  constant CFG_NMI_STACK_OVERFLOW_ADDR  : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000080";
  constant CFG_NMI_STACK_UNDERFLOW_ADDR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00000088";

  --! Number of elements each 2*CFG_ADDR_WIDTH in stack trace buffer, 0 = disabled
  constant CFG_STACK_TRACE_BUF_SIZE : integer := 32;

  --! @name   Integer Registers specified by ISA
  --! @{
    constant Reg_Zero : integer := 0;
    constant Reg_ra : integer := 1;       -- [1] Return address
    constant Reg_sp : integer := 2;       -- [2] Stack pointer
    constant Reg_gp : integer := 3;       -- [3] Global pointer
    constant Reg_tp : integer := 4;       -- [4] Thread pointer
    constant Reg_t0 : integer := 5;       -- [5] Temporaries 0 s3
    constant Reg_t1 : integer := 6;       -- [6] Temporaries 1 s4
    constant Reg_t2 : integer := 7;       -- [7] Temporaries 2 s5
    constant Reg_s0 : integer := 8;       -- [8] s0/fp Saved register/frame pointer
    constant Reg_s1 : integer := 9;       -- [9] Saved register 1
    constant Reg_a0 : integer := 10;      -- [10] Function argumentes 0
    constant Reg_a1 : integer := 11;      -- [11] Function argumentes 1
    constant Reg_a2 : integer := 12;      -- [12] Function argumentes 2
    constant Reg_a3 : integer := 13;      -- [13] Function argumentes 3
    constant Reg_a4 : integer := 14;      -- [14] Function argumentes 4
    constant Reg_a5 : integer := 15;      -- [15] Function argumentes 5
    constant Reg_a6 : integer := 16;      -- [16] Function argumentes 6
    constant Reg_a7 : integer := 17;      -- [17] Function argumentes 7
    constant Reg_s2 : integer := 18;      -- [18] Saved register 2
    constant Reg_s3 : integer := 19;      -- [19] Saved register 3
    constant Reg_s4 : integer := 20;      -- [20] Saved register 4
    constant Reg_s5 : integer := 21;      -- [21] Saved register 5
    constant Reg_s6 : integer := 22;      -- [22] Saved register 6
    constant Reg_s7 : integer := 23;      -- [23] Saved register 7
    constant Reg_s8 : integer := 24;      -- [24] Saved register 8
    constant Reg_s9 : integer := 25;      -- [25] Saved register 9
    constant Reg_s10 : integer := 26;     -- [26] Saved register 10
    constant Reg_s11 : integer := 27;     -- [27] Saved register 11
    constant Reg_t3 : integer := 28;      -- [28] 
    constant Reg_t4 : integer := 29;      -- [29] 
    constant Reg_t5 : integer := 30;      -- [30] 
    constant Reg_t6 : integer := 31;      -- [31] 
    constant Reg_Total : integer := 32;
  --! @}

  --! @name   Floating Point Unit Registers specified by ISA
  --! @{
    constant Reg_f0 : integer := 0;      -- ft0 temporary register
    constant Reg_f1 : integer := 1;      -- ft1
    constant Reg_f2 : integer := 2;      -- ft2
    constant Reg_f3 : integer := 3;      -- ft3
    constant Reg_f4 : integer := 4;      -- ft4
    constant Reg_f5 : integer := 5;      -- ft5
    constant Reg_f6 : integer := 6;      -- ft6
    constant Reg_f7 : integer := 7;      -- ft7
    constant Reg_f8 : integer := 8;      -- fs0 saved register
    constant Reg_f9 : integer := 9;      -- fs1
    constant Reg_f10 : integer := 10;    -- fa0 argument/return value
    constant Reg_f11 : integer := 11;    -- fa1 argument/return value
    constant Reg_f12 : integer := 12;    -- fa2 argument register
    constant Reg_f13 : integer := 13;    -- fa3
    constant Reg_f14 : integer := 14;    -- fa4
    constant Reg_f15 : integer := 15;    -- fa5
    constant Reg_f16 : integer := 16;    -- fa6
    constant Reg_f17 : integer := 17;    -- fa7
    constant Reg_f18 : integer := 18;    -- fs2 saved register
    constant Reg_f19 : integer := 19;    -- fs3
    constant Reg_f20 : integer := 20;    -- fs4
    constant Reg_f21 : integer := 21;    -- fs5
    constant Reg_f22 : integer := 22;    -- fs6
    constant Reg_f23 : integer := 23;    -- fs7
    constant Reg_f24 : integer := 24;    -- fs8
    constant Reg_f25 : integer := 25;    -- fs9
    constant Reg_f26 : integer := 26;    -- fs10
    constant Reg_f27 : integer := 27;    -- fs11
    constant Reg_f28 : integer := 28;    -- ft8 temporary register
    constant Reg_f29 : integer := 29;    -- ft9
    constant Reg_f30 : integer := 30;    -- ft10
    constant Reg_f31 : integer := 31;    -- ft11
    constant RegFpu_Total  : integer := 32;
  --! @}

  --! @name   Instruction formats specified by ISA specification
  --! @{
  constant ISA_R_type : integer := 0;
  constant ISA_I_type : integer := 1;
  constant ISA_S_type : integer := 2;
  constant ISA_SB_type : integer := 3;
  constant ISA_U_type : integer := 4;
  constant ISA_UJ_type : integer := 5;
  constant ISA_Total : integer := 6;
  --! @}


  --! @name   Implemented instruction list and its indexes
  --! @{
  constant Instr_ADD : integer := 0;
  constant Instr_ADDI : integer := 1;
  constant Instr_ADDIW : integer := 2;
  constant Instr_ADDW : integer := 3;
  constant Instr_AND : integer := 4;
  constant Instr_ANDI : integer := 5;
  constant Instr_AUIPC : integer := 6;
  constant Instr_BEQ : integer := 7;
  constant Instr_BGE : integer := 8;
  constant Instr_BGEU : integer := 9;
  constant Instr_BLT : integer := 10;
  constant Instr_BLTU : integer := 11;
  constant Instr_BNE : integer := 12;
  constant Instr_JAL : integer := 13;
  constant Instr_JALR : integer := 14;
  constant Instr_LB : integer := 15;
  constant Instr_LH : integer := 16;
  constant Instr_LW : integer := 17;
  constant Instr_LD : integer := 18;
  constant Instr_LBU : integer := 19;
  constant Instr_LHU : integer := 20;
  constant Instr_LWU : integer := 21;
  constant Instr_LUI : integer := 22;
  constant Instr_OR : integer := 23;
  constant Instr_ORI : integer := 24;
  constant Instr_SLLI : integer := 25;
  constant Instr_SLT : integer := 26;
  constant Instr_SLTI : integer := 27;
  constant Instr_SLTU : integer := 28;
  constant Instr_SLTIU : integer := 29;
  constant Instr_SLL : integer := 30;
  constant Instr_SLLW : integer := 31;
  constant Instr_SLLIW : integer := 32;
  constant Instr_SRA : integer := 33;
  constant Instr_SRAW : integer := 34;
  constant Instr_SRAI : integer := 35;
  constant Instr_SRAIW : integer := 36;
  constant Instr_SRL : integer := 37;
  constant Instr_SRLI : integer := 38;
  constant Instr_SRLIW : integer := 39;
  constant Instr_SRLW : integer := 40;
  constant Instr_SB : integer := 41;
  constant Instr_SH : integer := 42;
  constant Instr_SW : integer := 43;
  constant Instr_SD : integer := 44;
  constant Instr_SUB : integer := 45;
  constant Instr_SUBW : integer := 46;
  constant Instr_XOR : integer := 47;
  constant Instr_XORI : integer := 48;
  constant Instr_CSRRW : integer := 49;
  constant Instr_CSRRS : integer := 50;
  constant Instr_CSRRC : integer := 51;
  constant Instr_CSRRWI : integer := 52;
  constant Instr_CSRRCI : integer := 53;
  constant Instr_CSRRSI : integer := 54;
  constant Instr_URET : integer := 55;
  constant Instr_SRET : integer := 56;
  constant Instr_HRET : integer := 57;
  constant Instr_MRET : integer := 58;
  constant Instr_FENCE : integer := 59;
  constant Instr_FENCE_I : integer := 60;
  constant Instr_DIV : integer := 61;
  constant Instr_DIVU : integer := 62;
  constant Instr_DIVW : integer := 63;
  constant Instr_DIVUW : integer := 64;
  constant Instr_MUL : integer := 65;
  constant Instr_MULW : integer := 66;
  constant Instr_REM : integer := 67;
  constant Instr_REMU : integer := 68;
  constant Instr_REMW : integer := 69;
  constant Instr_REMUW : integer := 70;
  constant Instr_ECALL : integer := 71;
  constant Instr_EBREAK : integer := 72;
  constant Instr_FADD_D : integer := 73;
  constant Instr_FCVT_D_W : integer := 74;
  constant Instr_FCVT_D_WU : integer := 75;
  constant Instr_FCVT_D_L : integer := 76;
  constant Instr_FCVT_D_LU : integer := 77;
  constant Instr_FCVT_W_D : integer := 78;
  constant Instr_FCVT_WU_D : integer := 79;
  constant Instr_FCVT_L_D : integer := 80;
  constant Instr_FCVT_LU_D : integer := 81;
  constant Instr_FDIV_D : integer := 82;
  constant Instr_FEQ_D : integer := 83;
  constant Instr_FLD : integer := 84;
  constant Instr_FLE_D : integer := 85;
  constant Instr_FLT_D : integer := 86;
  constant Instr_FMAX_D : integer := 87;
  constant Instr_FMIN_D : integer := 88;
  constant Instr_FMOV_D_X : integer := 89;
  constant Instr_FMOV_X_D : integer := 90;
  constant Instr_FMUL_D : integer := 91;
  constant Instr_FSD : integer := 92;
  constant Instr_FSUB_D : integer := 93;
  constant Instr_Total : integer := 94;

  constant Instr_FPU_Total : integer := Instr_FSUB_D - Instr_FADD_D + 1;
  --! @}

  --! @name PRV bits possible values:
  --!
  --! @{
  --! User-mode
  constant PRV_U : std_logic_vector(1 downto 0) := "00";
  --! super-visor mode
  constant PRV_S : std_logic_vector(1 downto 0) := "01";
  --! hyper-visor mode
  constant PRV_H : std_logic_vector(1 downto 0) := "10";
  --! machine mode
  constant PRV_M : std_logic_vector(1 downto 0) := "11";
  --! @}


  --! @name CSR registers.
  --!
  --! @{
  
  -- FPU Accrued Exceptions fields from FCSR
  constant CSR_fflags            : std_logic_vector(11 downto 0) := X"001";
  -- FPU dynamic Rounding Mode fields from FCSR
  constant CSR_frm               : std_logic_vector(11 downto 0) := X"002";
  -- FPU Control and Status register (frm + fflags)
  constant CSR_fcsr              : std_logic_vector(11 downto 0) := X"003";
  -- ISA and extensions supported.
  constant CSR_misa              : std_logic_vector(11 downto 0) := X"f10";
  -- Vendor ID.
  constant CSR_mvendorid         : std_logic_vector(11 downto 0) := X"f11";
  -- Architecture ID.
  constant CSR_marchid           : std_logic_vector(11 downto 0) := X"f12";
  -- Vendor ID.
  constant CSR_mimplementationid : std_logic_vector(11 downto 0) := X"f13";
  -- Thread id (the same as core).
  constant CSR_mhartid           : std_logic_vector(11 downto 0) := X"f14";
  -- Machine wall-clock time
  constant CSR_mtime         : std_logic_vector(11 downto 0) := X"701";
  -- Software reset.
  constant CSR_mreset        : std_logic_vector(11 downto 0) := X"782";

  -- machine mode status read/write register.
  constant CSR_mstatus       : std_logic_vector(11 downto 0) := X"300";
  -- Machine exception delegation
  constant CSR_medeleg       : std_logic_vector(11 downto 0) := X"302";
  -- Machine interrupt delegation
  constant CSR_mideleg       : std_logic_vector(11 downto 0) := X"303";
  -- Machine interrupt enable
  constant CSR_mie           : std_logic_vector(11 downto 0) := X"304";
  -- The base address of the M-mode trap vector.
  constant CSR_mtvec         : std_logic_vector(11 downto 0) := X"305";
  -- Machine wall-clock timer compare value.
  constant CSR_mtimecmp      : std_logic_vector(11 downto 0) := X"321";
  -- Scratch register for machine trap handlers.
  constant CSR_mscratch      : std_logic_vector(11 downto 0) := X"340";
  -- Exception program counters.
  constant CSR_uepc          : std_logic_vector(11 downto 0) := X"041";
  constant CSR_sepc          : std_logic_vector(11 downto 0) := X"141";
  constant CSR_hepc          : std_logic_vector(11 downto 0) := X"241";
  constant CSR_mepc          : std_logic_vector(11 downto 0) := X"341";
  -- Machine trap cause
  constant CSR_mcause        : std_logic_vector(11 downto 0) := X"342";
  -- Machine bad address.
  constant CSR_mbadaddr      : std_logic_vector(11 downto 0) := X"343";
  -- Machine interrupt pending
  constant CSR_mip           : std_logic_vector(11 downto 0) := X"344";
  -- Machine stack overflow
  constant CSR_mstackovr     : std_logic_vector(11 downto 0) := X"350";
  -- Machine stack underflow
  constant CSR_mstackund     : std_logic_vector(11 downto 0) := X"351";
  -- MPU region address (non-standard CSR).
  constant CSR_mpu_addr      : std_logic_vector(11 downto 0) := X"352";
  -- MPU region mask (non-standard CSR).
  constant CSR_mpu_mask      : std_logic_vector(11 downto 0) := X"353";
  -- MPU region control (non-standard CSR).
  constant CSR_mpu_ctrl      : std_logic_vector(11 downto 0) := X"354";
  --! @}

  --! @name   Exceptions
  --! @{
  -- Instruction address misaligned
  constant EXCEPTION_InstrMisalign   : std_logic_vector(4 downto 0) := "00000";
  -- Instruction access fault
  constant EXCEPTION_InstrFault      : std_logic_vector(4 downto 0) := "00001";
  -- Illegal instruction
  constant EXCEPTION_InstrIllegal    : std_logic_vector(4 downto 0) := "00010";
  -- Breakpoint
  constant EXCEPTION_Breakpoint      : std_logic_vector(4 downto 0) := "00011";
  -- Load address misaligned
  constant EXCEPTION_LoadMisalign    : std_logic_vector(4 downto 0) := "00100";
  -- Load access fault
  constant EXCEPTION_LoadFault       : std_logic_vector(4 downto 0) := "00101";
  -- Store/AMO address misaligned
  constant EXCEPTION_StoreMisalign   : std_logic_vector(4 downto 0) := "00110";
  -- Store/AMO access fault
  constant EXCEPTION_StoreFault      : std_logic_vector(4 downto 0) := "00111";
  -- Environment call from U-mode
  constant EXCEPTION_CallFromUmode   : std_logic_vector(4 downto 0) := "01000";
  -- Environment call from S-mode
  constant EXCEPTION_CallFromSmode   : std_logic_vector(4 downto 0) := "01001";
  -- Environment call from H-mode
  constant EXCEPTION_CallFromHmode   : std_logic_vector(4 downto 0) := "01010";
  -- Environment call from M-mode
  constant EXCEPTION_CallFromMmode   : std_logic_vector(4 downto 0) := "01011";
  -- Instruction page fault
  constant EXCEPTION_InstrPageFault  : std_logic_vector(4 downto 0) := "01100";
  -- Load page fault
  constant EXCEPTION_LoadPageFault   : std_logic_vector(4 downto 0) := "01101";
  -- reserved
  constant EXCEPTION_rsrv14          : std_logic_vector(4 downto 0) := "01110";
  -- Store/AMO page fault
  constant EXCEPTION_StorePageFault  : std_logic_vector(4 downto 0) := "01111";
  -- Stack overflow
  constant EXCEPTION_StackOverflow   : std_logic_vector(4 downto 0) := "10000";
  -- Stack underflow
  constant EXCEPTION_StackUnderflow  : std_logic_vector(4 downto 0) := "10001";
  --! @}

  --! @name   Interrupts
  --! @{
  -- User software interrupt
  constant INTERRUPT_USoftware       : std_logic_vector(4 downto 0) := "00000";
  -- Superuser software interrupt
  constant INTERRUPT_SSoftware       : std_logic_vector(4 downto 0) := "00001";
  -- Hypervisor software itnerrupt
  constant INTERRUPT_HSoftware       : std_logic_vector(4 downto 0) := "00010";
  -- Machine software interrupt
  constant INTERRUPT_MSoftware       : std_logic_vector(4 downto 0) := "00011";
  -- User timer interrupt
  constant INTERRUPT_UTimer          : std_logic_vector(4 downto 0) := "00100";
  -- Superuser timer interrupt
  constant INTERRUPT_STimer          : std_logic_vector(4 downto 0) := "00101";
  -- Hypervisor timer interrupt
  constant INTERRUPT_HTimer          : std_logic_vector(4 downto 0) := "00110";
  -- Machine timer interrupt
  constant INTERRUPT_MTimer          : std_logic_vector(4 downto 0) := "00111";
  -- User external interrupt
  constant INTERRUPT_UExternal       : std_logic_vector(4 downto 0) := "01000";
  -- Superuser external interrupt
  constant INTERRUPT_SExternal       : std_logic_vector(4 downto 0) := "01001";
  -- Hypervisor external interrupt
  constant INTERRUPT_HExternal       : std_logic_vector(4 downto 0) := "01010";
  -- Machine external interrupt (from PLIC)
  constant INTERRUPT_MExternal       : std_logic_vector(4 downto 0) := "01011";
  --! @}


  --! @param[in] i_clk             CPU clock
  --! @param[in] i_nrst            Reset. Active LOW.
  --! @param[in] i_req_mem_fire    Memory request was accepted
  --! @param[in] i_resp_mem_valid  Memory response from ICache is valid
  --! @param[in] i_resp_mem_addr   Memory response address
  --! @param[in] i_resp_mem_data   Memory response value
  --! @param[in] i_e_npc           Valid instruction value awaited by 'Executor'
  --! @param[in] i_ra              Return address register value
  --! @param[out] o_npc_predic     Predicted next instruction address
  --! @param[out] o_predict        Mark requested address as predicted
  component BranchPredictor is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_mem_fire : in std_logic;
    i_resp_mem_valid : in std_logic;
    i_resp_mem_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_mem_data : in std_logic_vector(31 downto 0);
    i_e_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_ra : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_npc_predict : out std_logic_vector(31 downto 0)
  );
  end component; 

  --! @param[in] i_clk          CPU clock
  --! @param[in] i_nrst         Reset. Active LOW.
  --! @param[in] i_xret         XRet instruction signals mode switching
  --! @param[in] i_addr         CSR address, if xret=1 switch mode accordingly
  --! @param[in] i_wena         Write enable
  --! @param[in] i_wdata        CSR writing value
  --! @param[out] o_rdata       CSR read value
  --! @param[in] i_trap_ready   Trap branch request was accepted
  --! @param[in] i_ex_data_addr    Data path: address must be equal to the latest request address
  --! @param[in] i_ex_data_load_fault Data path: Bus response with SLVERR or DECERR on read
  --! @param[in] i_ex_data_store_fault Data path: Bus response with SLVERR or DECERR on write
  --! @param[in] i_break_mode   Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
  --! @param[out] o_break_event ebreak detected1 clock event
  --! @param[in] i_dport_ena    Debug port request is enabled
  --! @param[in] i_dport_write  Debug port Write enable
  --! @param[in] i_dport_addr   Debug port CSR address
  --! @param[in] i_dport_wdata  Debug port CSR writing value
  --! @param[out] o_dport_rdata Debug port CSR read value
  component CsrRegs is
  generic (
    hartid : integer;
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_mret : in std_logic;
    i_uret : in std_logic;
    i_sp : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_addr : in std_logic_vector(11 downto 0);
    i_wena : in std_logic;
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_trap_ready : in std_logic;
    i_ex_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_ex_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_ex_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_ex_data_load_fault : in std_logic;
    i_ex_data_store_fault : in std_logic;
    i_ex_data_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_ex_instr_load_fault : in std_logic;
    i_ex_illegal_instr : in std_logic;
    i_ex_unalign_store : in std_logic;
    i_ex_unalign_load : in std_logic;
    i_ex_breakpoint : in std_logic;
    i_ex_ecall : in std_logic;
    i_ex_fpu_invalidop : in std_logic;
    i_ex_fpu_divbyzero : in std_logic;
    i_ex_fpu_overflow : in std_logic;
    i_ex_fpu_underflow : in std_logic;
    i_ex_fpu_inexact : in std_logic;
    i_fpu_valid : in std_logic;
    i_irq_external : in std_logic;
    o_trap_valid : out std_logic;
    o_trap_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_break_mode : in std_logic;
    o_break_event : out std_logic;
    o_mpu_region_we : out std_logic;
    o_mpu_region_idx : out std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    o_mpu_region_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_mpu_region_mask : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_mpu_region_flags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    i_dport_ena : in std_logic;
    i_dport_write : in std_logic;
    i_dport_addr : in std_logic_vector(11 downto 0);
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0)
  );
  end component; 

  --! @param[in] i_clk CPU clock
  --! @param[in] i_nrst Reset. Active LOW.
  --! @param[in] i_any_hold Hold pipeline by any reason
  --! @param[in] i_f_valid Fetch input valid
  --! @param[in] i_f_pc Fetched pc
  --! @param[in] i_f_instr Fetched instruction value
  --! @param[in] i_instr_load_fault Instruction fetched from fault address
  --! @param[out] o_valid Current output values are valid
  --! @param[out] o_pc Current instruction pointer value
  --! @param[out] o_instr Current instruction value
  --! @param[out] o_memop_store Store to memory operation
  --! @param[out] o_memop_load Load from memoru operation
  --! @param[out] o_memop_sign_ext Load memory value with sign extending
  --! @param[out] o_memop_size Memory transaction size
  --! @param[out] o_rv32 32-bits instruction
  --! @param[out] o_f64  64-bits FPU (D-extension)
  --! @param[out] o_compressed 16-bits instruction (C-extension)
  --! @param[out] o_insigned_op Unsigned operands
  --! @param[out] o_isa_type Instruction format accordingly with ISA
  --! @param[out] o_instr_vec One bit per decoded instruction bus
  --! @param[out] o_exception Unimplemented instruction
  --! @param[out] o_instr_load_fault Instruction fetched from fault address
  component InstrDecoder is generic (
    async_reset : boolean;
    fpu_ena : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_any_hold : in std_logic;                               -- Hold pipeline by any reason
    i_f_valid : in std_logic;                                -- Fetch input valid
    i_f_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Fetched pc
    i_f_instr : in std_logic_vector(31 downto 0);            -- Fetched instruction value
    i_instr_load_fault : in std_logic;                       -- Instruction fetched from fault address
    i_instr_executable : in std_logic;                       -- MPU flag

    o_radr1 : out std_logic_vector(5 downto 0);
    o_radr2 : out std_logic_vector(5 downto 0);
    o_waddr : out std_logic_vector(5 downto 0);
    o_imm : out std_logic_vector(RISCV_ARCH-1 downto 0);

    i_e_ready : in std_logic;
    i_e_fencei : in std_logic;
    o_valid : out std_logic;                                 -- Current output values are valid
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- Current instruction pointer value
    o_instr : out std_logic_vector(31 downto 0);             -- Current instruction value
    o_memop_store : out std_logic;                           -- Store to memory operation
    o_memop_load : out std_logic;                            -- Load from memoru operation
    o_memop_sign_ext : out std_logic;                        -- Load memory value with sign extending
    o_memop_size : out std_logic_vector(1 downto 0);         -- Memory transaction size
    o_rv32 : out std_logic;                                  -- 32-bits instruction
    o_compressed : out std_logic;                            -- 16-bits opcode (C-extension)
    o_f64 : out std_logic;                                   -- 64-bits FPU (D-extension)
    o_unsigned_op : out std_logic;                           -- Unsigned operands
    o_isa_type : out std_logic_vector(ISA_Total-1 downto 0); -- Instruction format accordingly with ISA
    o_instr_vec : out std_logic_vector(Instr_Total-1 downto 0); -- One bit per decoded instruction bus
    o_exception : out std_logic;                             -- Unimplemented instruction
    o_instr_load_fault : out std_logic;                      -- Instruction fetched from fault address
    o_instr_executable : out std_logic                       -- MPU flag
  );
  end component; 


  --! @param[in] i_clk  
  --! @param[in] i_nrst Reset active LOW
  --! @param[in] i_pipeline_hold Hold execution by any reason
  --! @param[in] i_d_valid Decoded instruction is valid
  --! @param[in] i_d_pc Instruction pointer on decoded instruction
  --! @param[in] i_d_instr Decoded instruction value
  --! @param[in] i_wb_done write back done (Used to clear hazardness)
  --! @param[in] i_memop_store Store to memory operation
  --! @param[in] i_memop_load Load from memoru operation
  --! @param[in] i_memop_sign_ext Load memory value with sign extending
  --! @param[in] i_memop_size Memory transaction size
  --! @param[in] i_unsigned_op Unsigned operands
  --! @param[in] i_rv32 32-bits instruction
  --! @param[in] i_compressed 16-bits instruction (C-extension)
  --! @param[in] i_f64        D-extension (FPU)
  --! @param[in] i_isa_type   Type of the instruction's structure (ISA spec.)
  --! @param[in] i_break_mode        Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
  --! @param[in] i_unsup_exception   Unsupported instruction exception
  --! @param[in] i_instr_load_fault  Instruction fetched from fault address
  --! @param[in] i_ext_irq           External interrupt from PLIC (todo: timer & software interrupts)
  --! @param[in] i_dport_npc_write   Write npc value from debug port
  --! @param[in] i_dport_npc         Debug port npc value to write
  --! @param[out] o_radr1 Integer/float register index 1
  --! @param[in] i_rdata1 Integer register value 1
  --! @param[out] o_radr2 Integer/float register index 2
  --! @param[in] i_rdata2 Integer register value 2
  --! @param[out] o_res_addr Address to store result of the instruction (0=do not store)
  --! @param[out] o_res_data Value to store
  --! @param[out] o_pipeline_hold Hold pipeline while 'writeback' not done or multi-clock instruction.
  --! @param[out] o_csr_addr CSR address. 0 if not a CSR instruction with xret signals mode switching
  --! @param[out] o_csr_wena Write new CSR value
  --! @param[in] i_csr_rdata CSR current value
  --! @param[out] o_csr_wdata CSR new value
  --! @param[out] o_ex_npc    exception npc
  --! @param[out] o_ex_instr_load_fault Instruction fetched from fault address
  --! @param[out] o_ex_illegal_instr Exception: illegal instruction
  --! @param[out] o_ex_unalign_store Exception: Unaligned store
  --! @param[out] o_ex_unalign_load  Exception: Unaligned load
  --! @param[out] o_ex_breakpoint    Exception: BREAK
  --! @param[out] o_ex_ecall         Exception: ECALL
  --! @param[out] o_ex_fpu_invalidop FPU Exception: invalid operation
  --! @param[out] o_ex_fpu_divbyzero FPU Exception: divide by zero
  --! @param[out] o_ex_fpu_overflow  FPU Exception: overflow
  --! @param[out] o_ex_fpu_underflow FPU Exception: underflow
  --! @param[out] o_ex_fpu_inexact   FPU Exception: inexact
  --! @param[out] o_fpu_valid        FPU output is valid
  --! @param[out] o_memop_sign_ext Load data with sign extending
  --! @param[out] o_memop_load Load data instruction
  --! @param[out] o_memop_store Store data instruction
  --! @param[out] o_memop_size 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
  --! @param[out] o_memop_addr  Memory access address
  --! @param[out] o_trap_ready  Trap branch request was accepted
  --! @param[out] o_valid       Output is valid
  --! @param[out] o_pc          Valid instruction pointer
  --! @param[out] o_npc         Next instruction pointer. Next decoded pc must match to this value or will be ignored.
  --! @param[out] o_instr       Valid instruction value
  --! @param[out] o_call        CALL pseudo instruction detected
  --! @param[out] o_ret         RET pseudoinstruction detected
  --! @param[out] o_mret        MRET detected
  --! @param[out] o_uret        URET detected
  component InstrExecute is generic (
    async_reset : boolean;
    fpu_ena : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;                                      -- Reset active LOW
    i_d_valid : in std_logic;                                   -- Decoded instruction is valid
    i_d_radr1 : in std_logic_vector(5 downto 0);
    i_d_radr2 : in std_logic_vector(5 downto 0);
    i_d_waddr : in std_logic_vector(5 downto 0);
    i_d_imm : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_d_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Instruction pointer on decoded instruction
    i_d_instr : in std_logic_vector(31 downto 0);               -- Decoded instruction value
    i_wb_valid : in std_logic;                                  -- End of write back operation
    i_wb_waddr : in std_logic_vector(5 downto 0);               -- Write back address
    i_memop_store : in std_logic;                               -- Store to memory operation
    i_memop_load : in std_logic;                                -- Load from memoru operation
    i_memop_sign_ext : in std_logic;                            -- Load memory value with sign extending
    i_memop_size : in std_logic_vector(1 downto 0);             -- Memory transaction size
    i_unsigned_op : in std_logic;                               -- Unsigned operands
    i_rv32 : in std_logic;                                      -- 32-bits instruction
    i_compressed : in std_logic;                                -- C-extension (2-bytes length)
    i_f64 : in std_logic;                                       -- D-extension (FPU)
    i_isa_type : in std_logic_vector(ISA_Total-1 downto 0);     -- Type of the instruction's structure (ISA spec.)
    i_ivec : in std_logic_vector(Instr_Total-1 downto 0);       -- One pulse per supported instruction.
    i_unsup_exception : in std_logic;                           -- Unsupported instruction exception
    i_instr_load_fault : in std_logic;                          -- Instruction fetched from fault address
    i_instr_executable : in std_logic;                          -- MPU flag
    i_dport_npc_write : in std_logic;                           -- Write npc value from debug port
    i_dport_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Debug port npc value to write

    i_rdata1 : in std_logic_vector(RISCV_ARCH-1 downto 0);      -- Integer register value 1
    i_rdata2 : in std_logic_vector(RISCV_ARCH-1 downto 0);      -- Integer register value 2
    o_res_addr : out std_logic_vector(5 downto 0);              -- Address to store result of the instruction (0=do not store)
    o_res_data : out std_logic_vector(RISCV_ARCH-1 downto 0);   -- Value to store
    o_d_ready : out std_logic;                                  -- Hold pipeline while 'writeback' not done or multi-clock instruction.
    o_csr_addr : out std_logic_vector(11 downto 0);             -- CSR address. 0 if not a CSR instruction with xret signals mode switching
    o_csr_wena : out std_logic;                                 -- Write new CSR value
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- CSR current value
    o_csr_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);  -- CSR new value
    i_trap_valid : in std_logic;
    i_trap_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -- exceptions:
    o_ex_npc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_ex_instr_load_fault : out std_logic;                      -- Instruction fetched from fault address
    o_ex_instr_not_executable : out std_logic;                  -- MPU prohibit this instruction
    o_ex_illegal_instr : out std_logic;
    o_ex_unalign_store : out std_logic;
    o_ex_unalign_load : out std_logic;
    o_ex_breakpoint : out std_logic;
    o_ex_ecall : out std_logic;
    o_ex_fpu_invalidop : out std_logic;            -- FPU Exception: invalid operation
    o_ex_fpu_divbyzero : out std_logic;            -- FPU Exception: divide by zero
    o_ex_fpu_overflow : out std_logic;             -- FPU Exception: overflow
    o_ex_fpu_underflow : out std_logic;            -- FPU Exception: underflow
    o_ex_fpu_inexact : out std_logic;              -- FPU Exception: inexact
    o_fpu_valid : out std_logic;                   -- FPU output is valid

    o_memop_sign_ext : out std_logic;                           -- Load data with sign extending
    o_memop_load : out std_logic;                               -- Load data instruction
    o_memop_store : out std_logic;                              -- Store data instruction
    o_memop_size : out std_logic_vector(1 downto 0);            -- 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    o_memop_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Memory access address
    i_memop_ready : in std_logic;

    o_trap_ready : out std_logic;                               -- Trap branch request was accepted
    o_valid : out std_logic;                                    -- Output is valid
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);     -- Valid instruction pointer
    o_npc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    o_instr : out std_logic_vector(31 downto 0);                -- Valid instruction value
    o_fence : out std_logic;
    o_fencei : out std_logic;
    o_call : out std_logic;                                     -- CALL pseudo instruction detected
    o_ret : out std_logic;                                      -- RET pseudoinstruction detected
    o_mret : out std_logic;                                     -- MRET instruction
    o_uret : out std_logic                                      -- URET instruction
  );
  end component; 

  --! @param[in] i_clk
  --! @param[in] i_nrst
  --! @param[in] i_pipeline_hold
  --! @param[in] i_mem_ready
  --! @param[out] o_mem_addr_valid
  --! @param[out] o_mem_addr
  --! @param[in] i_mem_data_valid
  --! @param[in] i_mem_data_addr
  --! @param[in] i_mem_data
  --! @param[out] o_mem_ready
  --! @param[in] i_predict_npc
  --! @param[out] o_mem_req_fire    Used by branch predictor to form new npc value
  --! @param[out] o_valid
  --! @param[out] o_pc
  --! @param[out] o_instr
  --! @param[out] o_hold            Hold due no response from icache yet
  --! @param[in] i_br_fetch_valid   Fetch injection address/instr are valid
  --! @param[in] i_br_address_fetch Fetch injection address to skip ebreak instruciton only once
  --! @param[in] i_br_instr_fetch   Real instruction value that was replaced by ebreak
  component InstrFetch is generic (
    async_reset : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_pipeline_hold : in std_logic;
    i_mem_req_ready : in std_logic;
    o_mem_addr_valid : out std_logic;
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data : in std_logic_vector(31 downto 0);
    i_mem_load_fault : in std_logic;
    i_mem_executable : in std_logic;
    o_mem_resp_ready : out std_logic;
    i_e_fencei : in std_logic;

    i_predict_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);

    o_mem_req_fire : out std_logic;                    -- used by branch predictor to form new npc value
    o_instr_load_fault : out std_logic;                -- fault instruction's address
    o_instr_executable : out std_logic;
    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0);
    o_hold : out std_logic;                                -- Hold due no response from icache yet
    i_br_fetch_valid : in std_logic;                       -- Fetch injection address/instr are valid
    i_br_address_fetch : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Fetch injection address to skip ebreak instruciton only once
    i_br_instr_fetch : in std_logic_vector(31 downto 0)   -- Real instruction value that was replaced by ebreak
  );
  end component; 

  --! @param[in] i_clk
  --! @param[in] i_nrst
  --! @param[in] i_e_valid Execution stage outputs are valid
  --! @param[in] i_e_pc Execution stage instruction pointer
  --! @param[in] i_e_instr Execution stage instruction value
  --! @param[in] i_res_addr Register address to be written (0=no writing)
  --! @param[in] i_res_data Register value to be written
  --! @param[in] i_memop_sign_ext Load data with sign extending (if less than 8 Bytes)
  --! @param[in] i_memop_load Load data from memory and write to i_res_addr
  --! @param[in] i_memop_store Store i_res_data value into memory
  --! @param[in] i_memop_size Encoded memory transaction size in bytes:
  --!                         0=1B; 1=2B; 2=4B; 3=8B
  --! @param[in] i_memop_addr Memory access address
  --! @param[out] o_wena Write enable signal
  --! @param[out] o_waddr Output register address (0 = x0 = no write)
  --! @param[out] o_wdata Register value
  --! @param[in] i_mem_req_read Memory request is acceptable
  --! @param[out] o_mem_valid Memory request is valid
  --! @param[out] o_mem_write Memory write request
  --! @param[out] o_mem_sz Encoded data size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
  --! @param[out] o_mem_addr Data path requested address
  --! @param[out] o_mem_data Data path requested data (write transaction)
  --! @param[in] i_mem_data_valid Data path memory response is valid
  --! @param[in] i_mem_data_addr Data path memory response address
  --! @param[in] i_mem_data Data path memory response value
  --! @param[out] o_mem_resp_ready Data from DCache was accepted
  --! @param[out] o_hold Hold-on pipeline while memory operation not finished
  --! @param[out] o_valid Output is valid
  --! @param[out] o_pc Valid instruction pointer
  --! @param[out] o_instr Valid instruction value
  component MemAccess is generic (
    async_reset : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_e_valid : in std_logic;                                         -- Execution stage outputs are valid
    i_e_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);          -- Execution stage instruction pointer
    i_e_instr : in std_logic_vector(31 downto 0);                     -- Execution stage instruction value

    i_res_addr : in std_logic_vector(5 downto 0);                     -- Register address to be written (0=no writing)
    i_res_data : in std_logic_vector(RISCV_ARCH-1 downto 0);          -- Register value to be written
    i_memop_sign_ext : in std_logic;                                  -- Load data with sign extending (if less than 8 Bytes)
    i_memop_load : in std_logic;                                      -- Load data from memory and write to i_res_addr
    i_memop_store : in std_logic;                                     -- Store i_res_data value into memory
    i_memop_size : in std_logic_vector(1 downto 0);                   -- Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    i_memop_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Memory access address
    o_memop_ready : out std_logic;                                    -- Ready to accept memop request
    o_wena : out std_logic;                                           -- Write enable signal
    o_waddr : out std_logic_vector(5 downto 0);                       -- Output register address (0 = x0 = no write)
    o_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);            -- Register value

    -- Memory interface:
    i_mem_req_ready : in std_logic;
    o_mem_valid : out std_logic;                                      -- Memory request is valid
    o_mem_write : out std_logic;                                      -- Memory write request
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);     -- Data path requested address
    o_mem_wdata : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);     -- Data path requested data (write transaction)
    o_mem_wstrb : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);    -- 8-bytes aligned strobs
    i_mem_data_valid : in std_logic;                                  -- Data path memory response is valid
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Data path memory response address
    i_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);      -- Data path memory response value
    o_mem_resp_ready : out std_logic;

    o_hold : out std_logic;                                           -- Memory operation is more than 1 clock
    o_valid : out std_logic;                                          -- Output is valid
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);           -- Valid instruction pointer
    o_instr : out std_logic_vector(31 downto 0);                      -- Valid instruction value
    o_wb_memop : out std_logic                                        -- memory operation write-back (for tracer only)
  );
  end component; 

  --! @param[in] i_clk CPU clock
  --! @param[in] i_nrst Reset. Active LOW.
  --! @param[in] i_radr1 Port 1 read address
  --! @param[out] o_rdata1 Port 1 read value
  --! @param[in] i_radr2 Port 2 read address
  --! @param[out] o_rdata2 Port 2 read value
  --! @param[in] i_waddr Writing value
  --! @param[in] i_wena Writing is enabled
  --! @param[in] i_wdata Writing value
  --! @param[in] i_dport_addr    Debug port address
  --! @param[in] i_dport_ena     Debug port is enabled
  --! @param[in] i_dport_write   Debug port write is enabled
  --! @param[in] i_dport_wdata   Debug port write value
  --! @param[out] o_dport_rdata  Debug port read value
  --! @param[out] o_ra           Return address for branch predictor
  --! @param[out] o_sp           Stack Pointer for the borders control
  component RegBank is generic (
    async_reset : boolean;
    fpu_ena : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_radr1 : in std_logic_vector(5 downto 0);
    o_rdata1 : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_radr2 : in std_logic_vector(5 downto 0);
    o_rdata2 : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_waddr : in std_logic_vector(5 downto 0);
    i_wena : in std_logic;
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_dport_addr : in std_logic_vector(4 downto 0);
    i_dport_ena : in std_logic;
    i_dport_write : in std_logic;
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_ra : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_sp : out std_logic_vector(RISCV_ARCH-1 downto 0)
  );
  end component; 

  component RegFloatBank is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;                                   -- CPU clock
    i_nrst : in std_logic;                                  -- Reset. Active LOW.

    i_radr1 : in std_logic_vector(5 downto 0);              -- Port 1 read address
    o_rdata1 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 1 read value

    i_radr2 : in std_logic_vector(5 downto 0);              -- Port 2 read address
    o_rdata2 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 2 read value

    i_waddr : in std_logic_vector(5 downto 0);              -- Writing value
    i_wena : in std_logic;                                  -- Writing is enabled
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Writing value

    i_dport_addr : in std_logic_vector(4 downto 0);         -- Debug port address
    i_dport_ena : in std_logic;                             -- Debug port is enabled
    i_dport_write : in std_logic;                           -- Debug port write is enabled
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Debug port write value
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0)-- Debug port read value
  );
  end component; 

  --! @param[in] i_clk            CPU clock
  --! @param[in] i_nrst           Reset. Active LOW.
  --! @param[in] i_dport_valid    Debug access from DSU is valid
  --! @param[in] i_dport_write    Write command flag
  --! @param[in] i_dport_region   Registers region ID: 0=CSR; 1=IREGS; 2=Control
  --! @param[in] i_dport_addr     Register idx
  --! @param[in] i_dport_wdata    Write value
  --! @param[out] o_dport_ready   Response is ready
  --! @param[out] o_dport_rdata   Response value
  --! @param[out] o_core_addr     Address of the sub-region register
  --! @param[out] o_core_wdata    Write data
  --! @param[out] o_csr_ena       Region 0: Access to CSR bank is enabled.
  --! @param[out] o_csr_write     Region 0: CSR write enable
  --! @param[in] i_csr_rdata      Region 0: CSR read value
  --! @param[out] o_ireg_ena      Region 1: Access to integer register bank is enabled
  --! @param[out] o_ireg_write    Region 1: Integer registers bank write pulse
  --! @param[out] o_freg_ena      Region 1: Access to float register bank is enabled
  --! @param[out] o_freg_write    Region 1: Float registers bank write pulse
  --! @param[out] o_npc_write     Region 1: npc write enable
  --! @param[in] i_ireg_rdata     Region 1: Integer register read value
  --! @param[in] i_freg_rdata  Region 1: Float register read value
  --! @param[in] i_pc             Region 1: Instruction pointer
  --! @param[in] i_npc            Region 1: Next Instruction pointer
  --! @param[in] i_e_valid        Stepping control signal
  --! @param[in] i_e_call         Pseudo-instruction CALL
  --! @param[in] i_e_ret          Pseudo-instruction RET
  --! @param[in] i_m_valid        To compute number of valid executed instruction
  --! @param[out] o_clock_cnt     Number of clocks excluding halt state
  --! @param[out] o_executed_cnt  Number of executed instructions
  --! @param[out] o_halt          Halt signal is equal to hold pipeline
  --! @param[in] i_ebreak            ebreak instruction decoded
  --! @param[out] o_break_mode       Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
  --! @param[out] o_br_fetch_valid   Fetch injection address/instr are valid
  --! @param[out] o_br_address_fetch Fetch injection address to skip ebreak instruciton only once
  --! @param[out] o_br_instr_fetch   Real instruction value that was replaced by ebreak
  --! @param[out] o_flush_address    Address of instruction to remove from ICache
  --! @param[out] o_flush_valid      Remove address from ICache is valid
  --! @param[in] i_istate            ICache state machine value
  --! @param[in] i_dstate            DCache state machine value
  component DbgPort is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_dport_valid : in std_logic;
    i_dport_write : in std_logic;
    i_dport_region : in std_logic_vector(1 downto 0);
    i_dport_addr : in std_logic_vector(11 downto 0);
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_ready : out std_logic;
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_core_addr : out std_logic_vector(11 downto 0);
    o_core_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_csr_ena : out std_logic;
    o_csr_write : out std_logic;
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_ireg_ena : out std_logic;
    o_ireg_write : out std_logic;
    o_freg_ena : out std_logic;
    o_freg_write : out std_logic;
    o_npc_write : out std_logic;
    i_ireg_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_freg_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_e_valid : in std_logic;
    i_e_call : in std_logic;
    i_e_ret : in std_logic;
    i_m_valid : in std_logic;
    o_clock_cnt : out std_logic_vector(63 downto 0);
    o_executed_cnt : out std_logic_vector(63 downto 0);
    o_halt : out std_logic;
    i_ebreak : in std_logic;
    o_break_mode : out std_logic;
    o_br_fetch_valid : out std_logic;
    o_br_address_fetch : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_br_instr_fetch : out std_logic_vector(31 downto 0);
    o_flush_address : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_flush_valid : out std_logic;
    i_istate : in std_logic_vector(3 downto 0);
    i_dstate : in std_logic_vector(3 downto 0);
    i_cstate : in std_logic_vector(1 downto 0)
  );
  end component;


  --! @brief CPU 5-stages pipeline top-level
  --! @param[in] i_clk             CPU clock
  --! @param[in] i_nrst            Reset. Active LOW.
  --! @param[in] i_req_ctrl_ready  ICache is ready to accept request
  --! @param[out] o_req_ctrl_valid Request to ICache is valid
  --! @param[out] o_req_ctrl_addr  Requesting address to ICache
  --! @param[in] i_resp_ctrl_valid ICache response is valid
  --! @param[in] i_resp_ctrl_addr  Response address must be equal to the latest request address
  --! @param[in] i_resp_ctrl_data  Read value
  --! @param[out] o_resp_ctrl_ready Response from ICache is accepted
  --! @param[in] i_req_data_ready  DCache is ready to accept request
  --! @param[out] o_req_data_valid Request to DCache is valid
  --! @param[out] o_req_data_write Read/Write transaction
  --! @param[out] o_req_data_size  Size [Bytes]: 0=1B; 1=2B; 2=4B; 3=8B
  --! @param[out] o_req_data_addr  Requesting address to DCache
  --! @param[out] o_req_data_data  Writing value
  --! @param[in] i_resp_data_valid DCache response is valid
  --! @param[in] i_resp_data_addr  DCache response address must be equal to the latest request address
  --! @param[in] i_resp_data_data  Read value
  --! @param[in] i_resp_data_load_fault Bus response SLVERR or DECERR on read
  --! @param[in] i_resp_data_store_fault Bus response SLVERR or DECERR on write
  --! @param[out] o_resp_data_ready Response drom DCache is accepted
  --! @param[in] i_ext_irq         PLIC interrupt accordingly with spec
  --! @param[out] o_time           Timer in clock except halt state
  --! @param[in] i_dport_valid     Debug access from DSU is valid
  --! @param[in] i_dport_write     Write command flag
  --! @param[in] i_dport_region    Registers region ID: 0=CSR; 1=IREGS; 2=Control
  --! @param[in] i_dport_addr      Register idx
  --! @param[in] i_dport_wdata     Write value
  --! @param[out] o_dport_ready    Response is ready
  --! @param[out] o_dport_rdata    Response value
  --! @param[out] o_flush_address  Address of instruction to remove from ICache
  --! @param[out] o_flush_valid    Remove address from ICache is valid
  --! @param[in] i_istate          ICache state machine value
  --! @param[in] i_dstate          DCache state machine value
  --! @param[in] i_cstate          cachetop state machine value
  component Processor is
  generic (
    hartid : integer;
    async_reset : boolean;
    fpu_ena : boolean;
    tracer_ena : boolean
  );
  port (
    i_clk : in std_logic;                                             -- CPU clock
    i_nrst : in std_logic;                                            -- Reset. Active LOW.
    -- Control path:
    i_req_ctrl_ready : in std_logic;                                  -- ICache is ready to accept request
    o_req_ctrl_valid : out std_logic;                                 -- Request to ICache is valid
    o_req_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Requesting address to ICache
    i_resp_ctrl_valid : in std_logic;                                 -- ICache response is valid
    i_resp_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Response address must be equal to the latest request address
    i_resp_ctrl_data : in std_logic_vector(31 downto 0);              -- Read value
    i_resp_ctrl_load_fault : in std_logic;                            -- bus response with error
    i_resp_ctrl_executable : in std_logic;
    o_resp_ctrl_ready : out std_logic;
    -- Data path:
    i_req_data_ready : in std_logic;                                  -- DCache is ready to accept request
    o_req_data_valid : out std_logic;                                 -- Request to DCache is valid
    o_req_data_write : out std_logic;                                 -- Read/Write transaction
    o_req_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Requesting address to DCache
    o_req_data_wdata : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0); -- Writing value
    o_req_data_wstrb : out std_logic_vector(BUS_DATA_BYTES-1 downto 0); -- 8-bytes aligned strobs
    i_resp_data_valid : in std_logic;                                 -- DCache response is valid
    i_resp_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- DCache response address must be equal to the latest request address
    i_resp_data_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);    -- Read value
    i_resp_data_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_data_load_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on read
    i_resp_data_store_fault : in std_logic;                           -- Bus response with SLVERR or DECERR on write
    i_resp_data_er_mpu_load : in std_logic;
    i_resp_data_er_mpu_store : in std_logic;
    o_resp_data_ready : out std_logic;
    -- External interrupt pin
    i_ext_irq : in std_logic;                                         -- PLIC interrupt accordingly with spec
    o_time : out std_logic_vector(63 downto 0);                       -- Timer in clock except halt state
    o_exec_cnt : out std_logic_vector(63 downto 0);
    -- MPU interface
    o_mpu_region_we : out std_logic;
    o_mpu_region_idx : out std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    o_mpu_region_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_mpu_region_mask : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_mpu_region_flags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);  -- {ena, cachable, r, w, x}
    -- Debug interface:
    i_dport_valid : in std_logic;                                     -- Debug access from DSU is valid
    i_dport_write : in std_logic;                                     -- Write command flag
    i_dport_region : in std_logic_vector(1 downto 0);                 -- Registers region ID: 0=CSR; 1=IREGS; 2=Control
    i_dport_addr : in std_logic_vector(11 downto 0);                  -- Register idx
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);       -- Write value
    o_dport_ready : out std_logic;                                    -- Response is ready
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);      -- Response value
    o_halted : out std_logic;
    -- Debug signals:
    o_flush_address : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Address of instruction to remove from ICache
    o_flush_valid : out std_logic;                                    -- Remove address from ICache is valid
    o_data_flush_address : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Address of instruction to remove from D$
    o_data_flush_valid : out std_logic;                               -- Remove address from D$ is valid
    i_istate : in std_logic_vector(3 downto 0);                       -- ICache state machine value
    i_dstate : in std_logic_vector(3 downto 0);                       -- DCache state machine value
    i_cstate : in std_logic_vector(1 downto 0)                        -- CacheTop state machine value
  );
  end component; 

  --! @brief CPU cache top level
  --! @param[in] i_clk
  --! @param[in] i_nrst
  --! @param[in] i_req_ctrl_valid
  --! @param[in] i_req_ctrl_addr
  --! @param[out] o_req_ctrl_ready
  --! @param[out] o_resp_ctrl_valid
  --! @param[out] o_resp_ctrl_addr
  --! @param[out] o_resp_ctrl_data
  --! @param[in] i_resp_ctrl_ready
  --! @param[out] o_req_data_ready
  --! @param[in] i_req_data_valid
  --! @param[in] i_req_data_write
  --! @param[in] i_req_data_sz
  --! @param[in] i_req_data_addr
  --! @param[in] i_req_data_data
  --! @param[out] o_resp_data_valid
  --! @param[out] o_resp_data_addr
  --! @param[out] o_resp_data_data
  --! @param[out] o_resp_data_load_fault
  --! @param[out] o_resp_data_store_fault
  --! @param[in] i_resp_data_ready
  --! @param[in] i_req_mem_ready      AXI request was accepted
  --! @param[out] o_req_mem_valid
  --! @param[out] o_req_mem_write
  --! @param[out] o_req_mem_addr
  --! @param[out] o_req_mem_strob
  --! @param[out] o_req_mem_data
  --! @param[out] o_req_mem_len        burst length
  --! @param[out] o_req_mem_burst      burst type: "00" FIX; "01" INCR; "10" WRAP
  --! @param[in] i_resp_mem_data_valid
  --! @param[in] i_resp_mem_data
  --! @param[in] i_resp_mem_load_store
  --! @param[in] i_resp_mem_store_store
  --! @param[in] i_flush_address   clear ICache address from debug interface
  --! @param[in] i_flush_valid     address to clear icache is valid
  --! @param[out] o_istate        ICache state machine value
  --! @param[out] o_dstate        DCache state machine value
  --! @param[out] o_cstate        cachetop state machine value
  component CacheTop is generic (
    memtech : integer;
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;                              -- CPU clock
    i_nrst : in std_logic;                             -- Reset. Active LOW.
    -- Control path:
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    o_resp_ctrl_load_fault : out std_logic;
    o_resp_ctrl_executable : out std_logic;
    i_resp_ctrl_ready : in std_logic;
    -- Data path:
    i_req_data_valid : in std_logic;
    i_req_data_write : in std_logic;
    i_req_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_req_data_wdata : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_req_data_wstrb : in std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_data_ready : out std_logic;
    o_resp_data_valid : out std_logic;
    o_resp_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_resp_data_store_fault_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data_load_fault : out std_logic;
    o_resp_data_store_fault : out std_logic;
    o_resp_data_er_mpu_load : out std_logic;
    o_resp_data_er_mpu_store : out std_logic;
    i_resp_data_ready : in std_logic;
    -- Memory interface:
    i_req_mem_ready : in std_logic;                                    -- AXI request was accepted
    o_req_mem_path : out std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);  -- burst transaction length
    o_req_mem_len : out std_logic_vector(7 downto 0);                  -- burst length
    o_req_mem_burst : out std_logic_vector(1 downto 0);                -- burst type: "00" FIX; "01" INCR; "10" WRAP
    o_req_mem_last : out std_logic;
    i_resp_mem_valid : in std_logic;
    i_resp_mem_path : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_load_fault : in std_logic;                             -- Bus response with SLVERR or DECERR on read
    i_resp_mem_store_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on write
    i_resp_mem_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -- MPU interface:
    i_mpu_region_we : in std_logic;
    i_mpu_region_idx : in std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    i_mpu_region_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mpu_region_mask : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mpu_region_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    -- Debug signals:
    i_flush_address : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- clear ICache address from debug interface
    i_flush_valid : in std_logic;                                      -- address to clear icache is valid
    i_data_flush_address : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- clear D$ address
    i_data_flush_valid : in std_logic;                                      -- address to clear D$ is valid
    o_istate : out std_logic_vector(3 downto 0);                      -- ICache state machine value
    o_dstate : out std_logic_vector(3 downto 0);                      -- DCache state machine value
    o_cstate : out std_logic_vector(1 downto 0)                       -- cachetop state machine value
  );
  end component; 


  --! @brief "River" CPU Top level.
  --! @param[in] i_clk                 CPU clock
  --! @param[in] i_nrst                Reset. Active LOW.
  --! @param[in] i_req_mem_ready       AXI request was accepted
  --! @param[out] o_req_mem_valid      AXI memory request is valid
  --! @param[out] o_req_mem_write      AXI memory request is write type
  --! @param[out] o_req_mem_addr       AXI memory request address
  --! @param[out] o_req_mem_strob      Writing strob. 1 bit per Byte
  --! @param[out] o_req_mem_data       Writing data
  --! @param[out] o_req_mem_len        burst length
  --! @param[out] o_req_mem_burst      burst type: "00" FIX; "01" INCR; "10" WRAP
  --! @param[in] i_resp_mem_data_valid AXI response is valid
  --! @param[in] i_resp_mem_data       Read data
  --! @param[in] i_resp_mem_load_fault Bus response with SLVERR or DECERR on read
  --! @param[in] i_resp_mem_store_fault Bus response with SLVERR or DECERR on write
  --! @param[in] i_ext_irq             Interrupt line from external interrupts controller (PLIC).
  --! @param[out] o_time               Timer. Clock counter except halt state.
  --! @param[in] i_dport_valid         Debug access from DSU is valid
  --! @param[in] i_dport_write         Write command flag
  --! @param[in] i_dport_region        Registers region ID: 0=CSR; 1=IREGS; 2=Control
  --! @param[in] i_dport_addr          Register idx
  --! @param[in] i_dport_wdata         Write value
  --! @param[out] o_dport_ready        Response is ready
  --! @param[out] o_dport_rdata        Response value
  component RiverTop is
  generic (
    memtech : integer := 0;
    hartid : integer := 0;
    async_reset : boolean := false;
    fpu_ena : boolean := true;
    tracer_ena : boolean := false
  );
  port (
    i_clk : in std_logic;                                             -- CPU clock
    i_nrst : in std_logic;                                            -- Reset. Active LOW.
    -- Memory interface:
    i_req_mem_ready : in std_logic;                                   -- AXI request was accepted
    o_req_mem_path : out std_logic;                                   -- 0=ctrl; 1=data path
    o_req_mem_valid : out std_logic;                                  -- AXI memory request is valid
    o_req_mem_write : out std_logic;                                  -- AXI memory request is write type
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- AXI memory request address
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);-- Writing strob. 1 bit per Byte
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0); -- Writing data
    o_req_mem_len : out std_logic_vector(7 downto 0);                 -- burst length
    o_req_mem_burst : out std_logic_vector(1 downto 0);               -- burst type: "00" FIX; "01" INCR; "10" WRAP
    o_req_mem_last : out std_logic;
    i_resp_mem_valid : in std_logic;                                  -- AXI response is valid
    i_resp_mem_path : in std_logic;                                   -- 0=ctrl; 1=data path
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0); -- Read data
    i_resp_mem_load_fault : in std_logic;                             -- Bus response with SLVERR or DECERR on read
    i_resp_mem_store_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on write
    i_resp_mem_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    -- Interrupt line from external interrupts controller (PLIC).
    i_ext_irq : in std_logic;
    o_time : out std_logic_vector(63 downto 0);                       -- Timer. Clock counter except halt state.
    o_exec_cnt : out std_logic_vector(63 downto 0);
    -- Debug interface:
    i_dport_valid : in std_logic;                                     -- Debug access from DSU is valid
    i_dport_write : in std_logic;                                     -- Write command flag
    i_dport_region : in std_logic_vector(1 downto 0);                 -- Registers region ID: 0=CSR; 1=IREGS; 2=Control
    i_dport_addr : in std_logic_vector(11 downto 0);                  -- Register idx
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);       -- Write value
    o_dport_ready : out std_logic;                                    -- Response is ready
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);      -- Response value
    o_halted : out std_logic
  );
  end component; 

  component queue is generic (
    async_reset : boolean := false;
    szbits : integer := 2;
    dbits : integer := 32
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_re : in std_logic;
    i_we : in std_logic;
    i_wdata : in std_logic_vector(dbits-1 downto 0);
    o_rdata : out std_logic_vector(dbits-1 downto 0);
    o_full : out std_logic;
    o_nempty : out std_logic
  );
  end component;


end; -- package body
