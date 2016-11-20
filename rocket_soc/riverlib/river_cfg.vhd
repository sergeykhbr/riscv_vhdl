-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     "River" CPU internal configuration parameters that don't 
--!            depend of external bus.
-----------------------------------------------------------------------------

--! Standard library.
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;

--! @brief   Library global parameters.
package river_cfg is

  --! Architecture size difinition.
  constant RISCV_ARCH : integer := 64;

  --! @name System bus parameters
  --! @brief Constants specify AXI bus global settigns
  --! @{

  --! @brief   Address bus bit-size.
  constant BUS_ADDR_WIDTH : integer := 32;
  --! @brief   Data bus bit-size.
  constant BUS_DATA_WIDTH : integer := 64;
  --! @brief   Num of data bytes per transaction.
  constant BUS_DATA_BYTES : integer := BUS_DATA_WIDTH / 8;
  --! @}

  --! @name   Encoded Memory operation size values
  --! @{

  constant MEMOP_8B : std_logic_vector(1 downto 0) := "11";
  constant MEMOP_4B : std_logic_vector(1 downto 0) := "10";
  constant MEMOP_2B : std_logic_vector(1 downto 0) := "01";
  constant MEMOP_1B : std_logic_vector(1 downto 0) := "00";
  --! @}
  
  constant RESET_VECTOR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00001000";

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
  constant Instr_Total : integer := 71;
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
  --! @}

  --! @name   Exceptions
  --! @{
  -- Instruction address misaligned
  constant EXCEPTION_InstrMisalign   : std_logic_vector(3 downto 0) := X"0";
  -- Instruction access fault
  constant EXCEPTION_InstrFault      : std_logic_vector(3 downto 0) := X"1";
  -- Illegal instruction
  constant EXCEPTION_InstrIllegal    : std_logic_vector(3 downto 0) := X"2";
  -- Breakpoint
  constant EXCEPTION_Breakpoint      : std_logic_vector(3 downto 0) := X"3";
  -- Load address misaligned
  constant EXCEPTION_LoadMisalign    : std_logic_vector(3 downto 0) := X"4";
  -- Load access fault
  constant EXCEPTION_LoadFault       : std_logic_vector(3 downto 0) := X"5";
  -- Store/AMO address misaligned
  constant EXCEPTION_StoreMisalign   : std_logic_vector(3 downto 0) := X"6";
  -- Store/AMO access fault
  constant EXCEPTION_StoreFault      : std_logic_vector(3 downto 0) := X"7";
  -- Environment call from U-mode
  constant EXCEPTION_CallFromUmode   : std_logic_vector(3 downto 0) := X"8";
  -- Environment call from S-mode
  constant EXCEPTION_CallFromSmode   : std_logic_vector(3 downto 0) := X"9";
  -- Environment call from H-mode
  constant EXCEPTION_CallFromHmode   : std_logic_vector(3 downto 0) := X"A";
  -- Environment call from M-mode
  constant EXCEPTION_CallFromMmode   : std_logic_vector(3 downto 0) := X"B";
  --! @}

  --! @name   Interrupts
  --! @{
  -- External interrupt from PLIC
  constant INTERRUPT_PLIC : std_logic_vector(3 downto 0) := X"B";
  --! @}


end; -- package body
