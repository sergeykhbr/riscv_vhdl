-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     "River" CPU internal configuration parameters that are not 
--!            depend from external bus.
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


end; -- package body
