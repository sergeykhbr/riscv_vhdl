--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity InstrDecoder is generic (
    async_reset : boolean;
    fpu_ena : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_any_hold : in std_logic;                               -- Hold pipeline by any reason
    i_f_valid : in std_logic;                                -- Fetch input valid
    i_f_pc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- Fetched pc
    i_f_instr : in std_logic_vector(31 downto 0);            -- Fetched instruction value
    i_instr_load_fault : in std_logic;                       -- Instruction fetched from fault address
    i_instr_executable : in std_logic;                       -- MPU flag

    o_radr1 : out std_logic_vector(5 downto 0);              -- register bank address 1 (rs1)
    o_radr2 : out std_logic_vector(5 downto 0);              -- register bank address 2 (rs2)
    o_waddr : out std_logic_vector(5 downto 0);              -- register bank output (rd)
    o_csr_addr : out std_logic_vector(11 downto 0);          -- CSR bank output
    o_imm : out std_logic_vector(RISCV_ARCH-1 downto 0);     -- immediate constant decoded from instruction
    i_e_ready : in std_logic;                                -- execute stage ready to accept next instruction
    i_flush_pipeline : in std_logic;                         -- reset pipeline and cache
    i_progbuf_ena : in std_logic;                            -- executing from progbuf

    o_valid : out std_logic;                                 -- Current output values are valid
    o_pc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);  -- Current instruction pointer value
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
    o_instr_executable : out std_logic;                      -- MPU flag
    o_progbuf_ena : out std_logic
  );
end; 
 
architecture arch_InstrDecoder of InstrDecoder is

  -- LB, LH, LW, LD, LBU, LHU, LWU
  constant OPCODE_LB     : std_logic_vector(4 downto 0) := "00000";
  -- FLD
  constant OPCODE_FPU_LD : std_logic_vector(4 downto 0) := "00001";
  -- FENCE, FENCE_I
  constant OPCODE_FENCE  : std_logic_vector(4 downto 0) := "00011";
  --  ADDI, ANDI, ORI, SLLI, SLTI, SLTIU, SRAI, SRLI, XORI
  constant OPCODE_ADDI   : std_logic_vector(4 downto 0) := "00100";
  -- AUIPC
  constant OPCODE_AUIPC  : std_logic_vector(4 downto 0) := "00101";
  -- ADDIW, SLLIW, SRAIW, SRLIW
  constant OPCODE_ADDIW  : std_logic_vector(4 downto 0) := "00110";
  -- SB, SH, SW, SD
  constant OPCODE_SB     : std_logic_vector(4 downto 0) := "01000";
  -- FSD
  constant OPCODE_FPU_SD : std_logic_vector(4 downto 0) := "01001";
  -- ADD, AND, OR, SLT, SLTU, SLL, SRA, SRL, SUB, XOR, DIV, DIVU, MUL, REM, REMU
  constant OPCODE_ADD    : std_logic_vector(4 downto 0) := "01100";
  -- LUI
  constant OPCODE_LUI    : std_logic_vector(4 downto 0) := "01101";
  -- ADDW, SLLW, SRAW, SRLW, SUBW, DIVW, DIVUW, MULW, REMW, REMUW
  constant OPCODE_ADDW   : std_logic_vector(4 downto 0) := "01110";
  -- FPU operations
  constant OPCODE_FPU_OP : std_logic_vector(4 downto 0) := "10100";
  -- BEQ, BNE, BLT, BGE, BLTU, BGEU
  constant OPCODE_BEQ    : std_logic_vector(4 downto 0) := "11000";
  -- JALR
  constant OPCODE_JALR   : std_logic_vector(4 downto 0) := "11001";
  -- JAL
  constant OPCODE_JAL    : std_logic_vector(4 downto 0) := "11011";
  -- CSRRC, CSRRCI, CSRRS, CSRRSI, CSRRW, CSRRWI, URET, SRET, HRET, MRET
  constant OPCODE_CSRR   : std_logic_vector(4 downto 0) := "11100"; 
  -- Compressed instruction set
  constant OPCODE_C_ADDI4SPN : std_logic_vector(4 downto 0) := "00000";
  constant OPCODE_C_NOP_ADDI : std_logic_vector(4 downto 0) := "00001";
  constant OPCODE_C_SLLI     : std_logic_vector(4 downto 0) := "00010";
  constant OPCODE_C_JAL_ADDIW : std_logic_vector(4 downto 0) := "00101";
  constant OPCODE_C_LW       : std_logic_vector(4 downto 0) := "01000";
  constant OPCODE_C_LI       : std_logic_vector(4 downto 0) := "01001";
  constant OPCODE_C_LWSP     : std_logic_vector(4 downto 0) := "01010";
  constant OPCODE_C_LD       : std_logic_vector(4 downto 0) := "01100";
  constant OPCODE_C_ADDI16SP_LUI : std_logic_vector(4 downto 0) := "01101";
  constant OPCODE_C_LDSP     : std_logic_vector(4 downto 0) := "01110";
  constant OPCODE_C_MATH     : std_logic_vector(4 downto 0) := "10001";
  constant OPCODE_C_JR_MV_EBREAK_JALR_ADD : std_logic_vector(4 downto 0) := "10010";
  constant OPCODE_C_J        : std_logic_vector(4 downto 0) := "10101";
  constant OPCODE_C_SW       : std_logic_vector(4 downto 0) := "11000";
  constant OPCODE_C_BEQZ     : std_logic_vector(4 downto 0) := "11001";
  constant OPCODE_C_SWSP     : std_logic_vector(4 downto 0) := "11010";
  constant OPCODE_C_SD       : std_logic_vector(4 downto 0) := "11100";
  constant OPCODE_C_BNEZ     : std_logic_vector(4 downto 0) := "11101";
  constant OPCODE_C_SDSP     : std_logic_vector(4 downto 0) := "11110";


  constant INSTR_NONE : std_logic_vector(Instr_Total-1 downto 0) := (others => '0');

  type RegistersType is record
      valid : std_logic;
      pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      isa_type : std_logic_vector(ISA_Total-1 downto 0);
      instr_vec : std_logic_vector(Instr_Total-1 downto 0);
      instr : std_logic_vector(31 downto 0);
      memop_store : std_logic;
      memop_load : std_logic;
      memop_sign_ext : std_logic;
      memop_size : std_logic_vector(1 downto 0);
      unsigned_op : std_logic;
      rv32 : std_logic;
      f64 : std_logic;
      compressed : std_logic;
      instr_load_fault : std_logic;
      instr_executable : std_logic;

      instr_unimplemented : std_logic;
      radr1 : std_logic_vector(5 downto 0);
      radr2 : std_logic_vector(5 downto 0);
      waddr : std_logic_vector(5 downto 0);
      csr_addr : std_logic_vector(11 downto 0);
      imm : std_logic_vector(RISCV_ARCH-1 downto 0);
      progbuf_ena : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'), (others => '0'),   -- valid, pc, isa_type
    (others => '0'), (others => '0'), '0',   -- instr_vec, instr, memop_store
    '0', '0', "00",                          -- memop_load, memop_sign_ext, memop_size
    '0', '0', '0',                           -- unsigned_op, rv32, f64
    '0', '0', '0',                           -- compressed, instr_load_fault, instr_executable
    '0',                                     -- instr_unimpl
     (others => '0'),                        -- radr1
     (others => '0'),                        -- radr2
     (others => '0'),                        -- waddr
     (others => '0'),                        -- csr_addr
     (others => '0'),                        -- imm
     '0'                                     -- progbuf_ena
  );

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_any_hold, i_f_valid, i_f_pc, i_f_instr, i_instr_load_fault,
                i_instr_executable, i_e_ready, i_flush_pipeline, i_progbuf_ena, r)
    variable v : RegistersType;
    variable w_o_valid : std_logic;
    variable w_error : std_logic;
    variable w_compressed : std_logic;
    variable wb_instr : std_logic_vector(31 downto 0);
    variable wb_instr_out : std_logic_vector(31 downto 0);
    variable wb_opcode1 : std_logic_vector(4 downto 0);
    variable wb_opcode2 : std_logic_vector(2 downto 0);
    variable wb_dec : std_logic_vector(Instr_Total-1 downto 0);
    variable wb_isa_type : std_logic_vector(ISA_Total-1 downto 0);
    variable vb_radr1 : std_logic_vector(5 downto 0);
    variable vb_radr2 : std_logic_vector(5 downto 0);
    variable vb_waddr : std_logic_vector(5 downto 0);
    variable vb_csr_addr : std_logic_vector(11 downto 0);
    variable vb_imm : std_logic_vector(RISCV_ARCH-1 downto 0);
  begin

    v := r;
    w_error := '0';
    w_compressed := '0';
    wb_instr := i_f_instr;
    wb_opcode1 := wb_instr(6 downto 2);
    wb_opcode2 := wb_instr(14 downto 12);
    wb_dec := (others => '0');
    wb_isa_type := (others => '0');
    vb_radr1 := (others => '0');
    vb_radr2 := (others => '0');
    vb_waddr := (others => '0');
    vb_csr_addr := (others => '0');
    vb_imm := (others => '0');

    if wb_instr(1 downto 0) /= "11" then
        w_compressed := '1';
        wb_opcode1 := wb_instr(15 downto 13) & wb_instr(1 downto 0);
        wb_instr_out := X"00000003";
        case wb_opcode1 is
        when OPCODE_C_ADDI4SPN =>
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_ADDI) := '1';
            wb_instr_out(11 downto 7) := "01" & wb_instr(4 downto 2);   -- rd
            wb_instr_out(19 downto 15) := "00010";                     -- rs1 = sp
            wb_instr_out(29 downto 22) :=
                wb_instr(10 downto 7) & wb_instr(12 downto 11) & wb_instr(5) & wb_instr(6);
            vb_radr1 := "000010";                       -- rs1 = sp
            vb_waddr := "001" & wb_instr(4 downto 2);   -- rd
            vb_imm(9 downto 2) := wb_instr(10 downto 7) & wb_instr(12 downto 11)
                                & wb_instr(5) & wb_instr(6);
        when OPCODE_C_NOP_ADDI =>
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_ADDI) := '1';
            wb_instr_out(11 downto 7) := wb_instr(11 downto 7);   -- rd
            wb_instr_out(19 downto 15) := wb_instr(11 downto 7);  -- rs1
            wb_instr_out(24 downto 20) := wb_instr(6 downto 2);   -- imm
            if wb_instr(12) = '1' then
                wb_instr_out(31 downto 25) := (others => '1');
            end if;
            vb_radr1 := '0' & wb_instr(11 downto 7);      -- rs1
            vb_waddr := '0' & wb_instr(11 downto 7);      -- rd
            vb_imm(4 downto 0) := wb_instr(6 downto 2);
            vb_imm(RISCV_ARCH-1 downto 5) := (others => wb_instr(12));
        when OPCODE_C_SLLI =>
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_SLLI) := '1';
            wb_instr_out(11 downto 7) := wb_instr(11 downto 7);      -- rd
            wb_instr_out(19 downto 15) := wb_instr(11 downto 7);     -- rs1
            wb_instr_out(25 downto 20) := wb_instr(12) & wb_instr(6 downto 2);  -- shamt
            vb_radr1 := '0' & wb_instr(11 downto 7);                 -- rs1
            vb_waddr := '0' & wb_instr(11 downto 7);                 -- rd
            vb_imm(5 downto 0) := wb_instr(12) & wb_instr(6 downto 2);
        when OPCODE_C_JAL_ADDIW =>
            -- JAL is the RV32C only instruction
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_ADDIW) := '1';
            wb_instr_out(11 downto 7) := wb_instr(11 downto 7);      -- rd
            wb_instr_out(19 downto 15) := wb_instr(11 downto 7);     -- rs1
            wb_instr_out(24 downto 20) := wb_instr(6 downto 2);      -- imm
            if wb_instr(12) = '1' then
                wb_instr_out(31 downto 25) := (others => '1');
            end if;
            vb_radr1 := '0' & wb_instr(11 downto 7);                 -- rs1
            vb_waddr := '0' & wb_instr(11 downto 7);                 -- rd
            vb_imm(4 downto 0) := wb_instr(6 downto 2);
            vb_imm(RISCV_ARCH-1 downto 5) := (others => wb_instr(12));
        when OPCODE_C_LW =>
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_LW) := '1';
            wb_instr_out(11 downto 7) := "01" & wb_instr(4 downto 2);   -- rd
            wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
            wb_instr_out(26 downto 22) :=
                wb_instr(5) & wb_instr(12 downto 10) & wb_instr(6);
            vb_radr1 := "001" & wb_instr(9 downto 7);   -- rs1
            vb_waddr := "001" & wb_instr(4 downto 2);   -- rd
            vb_imm(6 downto 2) := wb_instr(5) & wb_instr(12 downto 10) & wb_instr(6);
        when OPCODE_C_LI =>  -- ADDI rd = r0 + imm
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_ADDI) := '1';
            wb_instr_out(11 downto 7) := wb_instr(11 downto 7);      -- rd
            wb_instr_out(24 downto 20) := wb_instr(6 downto 2);      -- imm
            if wb_instr(12) = '1' then
                wb_instr_out(31 downto 25) := (others => '1');
            end if;
            vb_waddr := '0' & wb_instr(11 downto 7);      -- rd
            vb_imm(4 downto 0) := wb_instr(6 downto 2);
            vb_imm(RISCV_ARCH-1 downto 5) := (others => wb_instr(12));
        when OPCODE_C_LWSP =>
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_LW) := '1';
            wb_instr_out(11 downto 7) := wb_instr(11 downto 7);    -- rd
            wb_instr_out(19 downto 15) := "00010";                 -- rs1 = sp
            wb_instr_out(27 downto 22) :=
                wb_instr(3 downto 2) & wb_instr(12) & wb_instr(6 downto 4);
            vb_radr1 := "000010";                        -- rs1 = sp
            vb_waddr := '0' & wb_instr(11 downto 7);     -- rd
            vb_imm(7 downto 2) := wb_instr(3 downto 2) & wb_instr(12) & wb_instr(6 downto 4);
        when OPCODE_C_LD =>
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_LD) := '1';
            wb_instr_out(11 downto 7) := "01" & wb_instr(4 downto 2);   -- rd
            wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
            wb_instr_out(27 downto 23) :=
                wb_instr(6) & wb_instr(5) & wb_instr(12 downto 10);
            vb_radr1 := "001" & wb_instr(9 downto 7);   -- rs1
            vb_waddr := "001" & wb_instr(4 downto 2);   -- rd
            vb_imm(7 downto 3) := wb_instr(6) & wb_instr(5) & wb_instr(12 downto 10);
        when OPCODE_C_ADDI16SP_LUI =>
            if wb_instr(11 downto 7) = "00010" then
                wb_isa_type(ISA_I_type) := '1';
                wb_dec(Instr_ADDI) := '1';
                wb_instr_out(11 downto 7) := "00010";     -- rd = sp
                wb_instr_out(19 downto 15) := "00010";    -- rs1 = sp
                wb_instr_out(28 downto 24) :=
                    wb_instr(4 downto 3) & wb_instr(5) & wb_instr(2) & wb_instr(6);
                if wb_instr(12) = '1' then
                    wb_instr_out(31 downto 29) := (others => '1');
                end if;
                vb_radr1 := "000010";    -- rs1 = sp
                vb_waddr := "000010";    -- rd = sp
                vb_imm(8 downto 4) := wb_instr(4 downto 3) & wb_instr(5) & wb_instr(2) & wb_instr(6);
                vb_imm(RISCV_ARCH-1 downto 9) := (others => wb_instr(12));
            else
                wb_isa_type(ISA_U_type) := '1';
                wb_dec(Instr_LUI) := '1';
                wb_instr_out(11 downto 7) := wb_instr(11 downto 7);  -- rd
                wb_instr_out(16 downto 12) := wb_instr(6 downto 2);
                if wb_instr(12) = '1' then
                    wb_instr_out(31 downto 17) := (others => '1');
                end if;
                vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
                vb_imm(16 downto 12) := wb_instr(6 downto 2);
                vb_imm(RISCV_ARCH-1 downto 17) := (others => wb_instr(12));
            end if;
        when OPCODE_C_LDSP =>
            wb_isa_type(ISA_I_type) := '1';
            wb_dec(Instr_LD) := '1';
            wb_instr_out(11 downto 7) := wb_instr(11 downto 7);  -- rd
            wb_instr_out(19 downto 15) := "00010";               -- rs1 = sp
            wb_instr_out(28 downto 23) :=
                wb_instr(4 downto 2) & wb_instr(12) & wb_instr(6 downto 5);
            vb_radr1 := "000010";                                -- rs1 = sp
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(8 downto 3) := wb_instr(4 downto 2) & wb_instr(12) & wb_instr(6 downto 5);
        when OPCODE_C_MATH =>
            if wb_instr(11 downto 10) = "00" then
                wb_isa_type(ISA_I_type) := '1';
                wb_dec(Instr_SRLI) := '1';
                wb_instr_out(11 downto 7) := "01" & wb_instr(9 downto 7);   -- rd
                wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
                wb_instr_out(25 downto 20) := wb_instr(12) & wb_instr(6 downto 2);  -- shamt
                vb_radr1 := "001" & wb_instr(9 downto 7);            -- rs1
                vb_waddr := "001" & wb_instr(9 downto 7);            -- rd
                vb_imm(5 downto 0) := wb_instr(12) & wb_instr(6 downto 2);  -- shamt
            elsif wb_instr(11 downto 10) = "01" then
                wb_isa_type(ISA_I_type) := '1';
                wb_dec(Instr_SRAI) := '1';
                wb_instr_out(11 downto 7) := "01" & wb_instr(9 downto 7);   -- rd
                wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
                wb_instr_out(25 downto 20) := wb_instr(12) & wb_instr(6 downto 2);  -- shamt
                vb_radr1 := "001" & wb_instr(9 downto 7);            -- rs1
                vb_waddr := "001" & wb_instr(9 downto 7);            -- rd
                vb_imm(5 downto 0) := wb_instr(12) & wb_instr(6 downto 2);  -- shamt
            elsif wb_instr(11 downto 10) = "10" then
                wb_isa_type(ISA_I_type) := '1';
                wb_dec(Instr_ANDI) := '1';
                wb_instr_out(11 downto 7) := "01" & wb_instr(9 downto 7);   -- rd
                wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
                wb_instr_out(24 downto 20) := wb_instr(6 downto 2);        -- imm
                if wb_instr(12) = '1' then
                    wb_instr_out(31 downto 25) := (others => '1');
                end if;
                vb_radr1 := "001" & wb_instr(9 downto 7);            -- rs1
                vb_waddr := "001" & wb_instr(9 downto 7);            -- rd
                vb_imm(4 downto 0) :=  wb_instr(6 downto 2);
                vb_imm(RISCV_ARCH-1 downto 5) := (others => wb_instr(12));
            elsif wb_instr(12) = '0' then
                wb_isa_type(ISA_R_type) := '1';
                wb_instr_out(11 downto 7) := "01" & wb_instr(9 downto 7);   -- rd
                wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
                wb_instr_out(24 downto 20) := "01" & wb_instr(4 downto 2);  -- rs2
                vb_radr1 := "001" & wb_instr(9 downto 7);            -- rs1
                vb_radr2 := "001" & wb_instr(4 downto 2);            -- rs2
                vb_waddr := "001" & wb_instr(9 downto 7);            -- rd
                case wb_instr(6 downto 5) is
                when "00" =>
                    wb_dec(Instr_SUB) := '1';
                when "01" =>
                    wb_dec(Instr_XOR) := '1';
                when "10" =>
                    wb_dec(Instr_OR) := '1';
                when others =>
                    wb_dec(Instr_AND) := '1';
                end case;
            else
                wb_isa_type(ISA_R_type) := '1';
                wb_instr_out(11 downto 7) := "01" & wb_instr(9 downto 7);   -- rd
                wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
                wb_instr_out(24 downto 20) := "01" & wb_instr(4 downto 2);  -- rs2
                vb_radr1 := "001" & wb_instr(9 downto 7);            -- rs1
                vb_radr2 := "001" & wb_instr(4 downto 2);            -- rs2
                vb_waddr := "001" & wb_instr(9 downto 7);            -- rd
                case wb_instr(6 downto 5) is
                when "00" =>
                    wb_dec(Instr_SUBW) := '1';
                when "01" =>
                    wb_dec(Instr_ADDW) := '1';
                when others =>
                    w_error := '1';
                end case;
            end if;
        when OPCODE_C_JR_MV_EBREAK_JALR_ADD =>
            wb_isa_type(ISA_I_type) := '1';
            if wb_instr(12) = '0' then
                if wb_instr(6 downto 2) = "00000" then
                    wb_dec(Instr_JALR) := '1';
                    wb_instr_out(19 downto 15) := wb_instr(11 downto 7);  -- rs1
                    vb_radr1 := '0' & wb_instr(11 downto 7);            -- rs1
                else
                    wb_dec(Instr_ADDI) := '1';
                    wb_instr_out(11 downto 7) := wb_instr(11 downto 7);   -- rd
                    wb_instr_out(19 downto 15) := wb_instr(6 downto 2);   -- rs1
                    vb_radr1 := '0' & wb_instr(6 downto 2);               -- rs1
                    vb_waddr := '0' & wb_instr(11 downto 7);              -- rd
                end if;
            else
                if wb_instr(11 downto 7) = "00000" and wb_instr(6 downto 2) = "00000" then
                    wb_dec(Instr_EBREAK) := '1';
                elsif wb_instr(6 downto 2) = "00000" then
                    wb_dec(Instr_JALR) := '1';
                    wb_instr_out(11 downto 7) := "00001";                 -- rd = ra
                    wb_instr_out(19 downto 15) := wb_instr(11 downto 7);  -- rs1
                    vb_radr1 := '0' & wb_instr(11 downto 7);  -- rs1;
                    vb_waddr := "000001";
                else
                    wb_dec(Instr_ADD) := '1';
                    wb_isa_type(ISA_R_type) := '1';
                    wb_instr_out(11 downto 7) := wb_instr(11 downto 7);   -- rd
                    wb_instr_out(19 downto 15) := wb_instr(11 downto 7);  -- rs1
                    wb_instr_out(24 downto 20) := wb_instr(6 downto 2);   -- rs2
                    vb_radr1 := '0' & wb_instr(11 downto 7);              -- rs1
                    vb_radr2 := '0' & wb_instr(6 downto 2);               -- rs2
                    vb_waddr := '0' & wb_instr(11 downto 7);              -- rd
                end if;
            end if;
        when OPCODE_C_J =>   -- JAL with rd = 0
            wb_isa_type(ISA_UJ_type) := '1';
            wb_dec(Instr_JAL) := '1';
            wb_instr_out(20) := wb_instr(12);            -- imm11
            wb_instr_out(23 downto 21) := wb_instr(5 downto 3);      -- imm10_1(3:1)
            wb_instr_out(24) := wb_instr(11);            -- imm10_1(4)
            wb_instr_out(25) := wb_instr(2);             -- imm10_1(5)
            wb_instr_out(26) := wb_instr(7);             -- imm10_1(6)
            wb_instr_out(27) := wb_instr(6);             -- imm10_1(7)
            wb_instr_out(29 downto 28) := wb_instr(10 downto 9);     -- imm10_1(9:8)
            wb_instr_out(30) := wb_instr(8);             -- imm10_1(10)
            if wb_instr(12) = '1' then
                wb_instr_out(19 downto 12) := (others => '1'); -- imm19_12
                wb_instr_out(31) := '1';                 -- imm20
            end if;
            vb_imm(10 downto 1) := wb_instr(8) & wb_instr(10 downto 9) & wb_instr(6) & wb_instr(7)
                                 & wb_instr(2) & wb_instr(11) & wb_instr(5 downto 3);
            vb_imm(RISCV_ARCH-1 downto 11) := (others => wb_instr(12));
        when OPCODE_C_SW =>
            wb_isa_type(ISA_S_type) := '1';
            wb_dec(Instr_SW) := '1';
            wb_instr_out(24 downto 20) := "01" & wb_instr(4 downto 2);    -- rs2
            wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);    -- rs1
            wb_instr_out(11 downto 9) := wb_instr(11 downto 10) & wb_instr(6);
            wb_instr_out(26 downto 25) := wb_instr(5) & wb_instr(12);
            vb_radr1 := "001" & wb_instr(9 downto 7);    -- rs1
            vb_radr2 := "001" & wb_instr(4 downto 2);    -- rs2
            vb_imm(6 downto 2) := wb_instr(5) & wb_instr(12) & wb_instr(11 downto 10) & wb_instr(6);
        when OPCODE_C_BEQZ =>
            wb_isa_type(ISA_SB_type) := '1';
            wb_dec(Instr_BEQ) := '1';
            wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);    -- rs1
            wb_instr_out(11 downto 8) := wb_instr(11 downto 10) & wb_instr(4 downto 3);
            wb_instr_out(27 downto 25) := wb_instr(6 downto 5) & wb_instr(2);
            if wb_instr(12) = '1' then
                wb_instr_out(30 downto 28) := (others => '1');
                wb_instr_out(7) := '1';
                wb_instr_out(31) := '1';
            end if;
            vb_radr1 := "001" & wb_instr(9 downto 7);    -- rs1
            vb_imm(7 downto 1) := wb_instr(6 downto 5) & wb_instr(2)
                                & wb_instr(11 downto 10) & wb_instr(4 downto 3);
            vb_imm(RISCV_ARCH-1 downto 8) := (others => wb_instr(12));
        when OPCODE_C_SWSP =>
            wb_isa_type(ISA_S_type) := '1';
            wb_dec(Instr_SW) := '1';
            wb_instr_out(24 downto 20) := wb_instr(6 downto 2);  -- rs2
            wb_instr_out(19 downto 15) := "00010";             -- rs1 = sp
            wb_instr_out(11 downto 9) := wb_instr(11 downto 9);
            wb_instr_out(27 downto 25) := wb_instr(8 downto 7) & wb_instr(12);
            vb_radr1 := "000010";                    -- rs1 = sp
            vb_radr2 := '0' & wb_instr(6 downto 2);  -- rs2
            vb_imm(7 downto 2) := wb_instr(8 downto 7) & wb_instr(12) & wb_instr(11 downto 9);
        when OPCODE_C_SD =>
            wb_isa_type(ISA_S_type) := '1';
            wb_dec(Instr_SD) := '1';
            wb_instr_out(24 downto 20) := "01" & wb_instr(4 downto 2);  -- rs2
            wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);  -- rs1
            wb_instr_out(11 downto 10) := wb_instr(11 downto 10);
            wb_instr_out(27 downto 25) := wb_instr(6 downto 5) & wb_instr(12);
            vb_radr1 := "001" & wb_instr(9 downto 7);  -- rs1
            vb_radr2 := "001" & wb_instr(4 downto 2);  -- rs2
            vb_imm(7 downto 3) := wb_instr(6 downto 5) & wb_instr(12) & wb_instr(11 downto 10);
        when OPCODE_C_BNEZ =>
            wb_isa_type(ISA_SB_type) := '1';
            wb_dec(Instr_BNE) := '1';
            wb_instr_out(19 downto 15) := "01" & wb_instr(9 downto 7);    -- rs1
            wb_instr_out(11 downto 8) := wb_instr(11 downto 10) & wb_instr(4 downto 3);
            wb_instr_out(27 downto 25) := wb_instr(6 downto 5) & wb_instr(2);
            if wb_instr(12) = '1' then
                wb_instr_out(30 downto 28) := (others => '1');
                wb_instr_out(7) := '1';
                wb_instr_out(31) := '1';
            end if;
            vb_radr1 := "001" & wb_instr(9 downto 7);    -- rs1
            vb_imm(7 downto 1) := wb_instr(6 downto 5) & wb_instr(2)
                               & wb_instr(11 downto 10) & wb_instr(4 downto 3);
            vb_imm(RISCV_ARCH-1 downto 8) := (others => wb_instr(12));
        when OPCODE_C_SDSP =>
            wb_isa_type(ISA_S_type) := '1';
            wb_dec(Instr_SD) := '1';
            wb_instr_out(24 downto 20) := wb_instr(6 downto 2);  -- rs2
            wb_instr_out(19 downto 15) := "00010";               -- rs1 = sp
            wb_instr_out(11 downto 10) := wb_instr(11 downto 10);
            wb_instr_out(28 downto 25) := wb_instr(9 downto 7) & wb_instr(12);
            vb_radr1 := "000010";                          -- rs1 = sp
            vb_radr2 := '0' & wb_instr(6 downto 2);        -- rs2
            vb_imm(8 downto 3) := wb_instr(9 downto 7) & wb_instr(12) & wb_instr(11 downto 10);
        when others =>
            w_error := '1';
        end case;
    else -- compressed/!not compressed
        case wb_opcode1 is
        when OPCODE_ADD =>
            wb_isa_type(ISA_R_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_radr2 := '0' & wb_instr(24 downto 20);
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            case wb_opcode2 is
            when "000" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_ADD) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_MUL) := '1';
                elsif wb_instr(31 downto 25) = "0100000" then
                    wb_dec(Instr_SUB) := '1';
                else
                    w_error := '1';
                end if;
            when "001" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_SLL) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_MULH) := '1';
                else 
                    w_error := '1';
                end if;
            when "010" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_SLT) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_MULHSU) := '1';
                else 
                    w_error := '1';
                end if;
        when "011" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_SLTU) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_MULHU) := '1';
                else 
                    w_error := '1';
                end if;
            when "100" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_XOR) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_DIV) := '1';
                else 
                    w_error := '1';
                end if;
            when "101" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_SRL) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_DIVU) := '1';
                elsif wb_instr(31 downto 25) = "0100000" then
                    wb_dec(Instr_SRA) := '1';
                else
                    w_error := '1';
                end if;
            when "110" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_OR) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_REM) := '1';
                else
                    w_error := '1';
                end if;
            when "111" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_AND) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_REMU) := '1';
                else
                    w_error := '1';
                end if;
            when others =>
                w_error := '1';
            end case;
        when OPCODE_ADDI =>
            wb_isa_type(ISA_I_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(11 downto 0) := wb_instr(31 downto 20);
            vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
            case wb_opcode2 is
            when "000" =>
                wb_dec(Instr_ADDI) := '1';
            when "001" =>
                wb_dec(Instr_SLLI) := '1';
            when "010" =>
                wb_dec(Instr_SLTI) := '1';
            when "011" =>
                wb_dec(Instr_SLTIU) := '1';
            when "100" =>
                wb_dec(Instr_XORI) := '1';
            when "101" =>
                if wb_instr(31 downto 26) = "000000" then
                    wb_dec(Instr_SRLI) := '1';
                elsif wb_instr(31 downto 26) = "010000" then
                    wb_dec(Instr_SRAI) := '1';
                else
                    w_error := '1';
                end if;
            when "110" =>
                wb_dec(Instr_ORI) := '1';
            when "111" =>
                wb_dec(Instr_ANDI) := '1';
            when others =>
                w_error := '1';
            end case;
        when OPCODE_ADDIW =>
            wb_isa_type(ISA_I_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(11 downto 0) := wb_instr(31 downto 20);
            vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
            case wb_opcode2 is
            when "000" =>
                wb_dec(Instr_ADDIW) := '1';
            when "001" =>
                wb_dec(Instr_SLLIW) := '1';
            when "101" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_SRLIW) := '1';
                elsif wb_instr(31 downto 25) = "0100000" then
                    wb_dec(Instr_SRAIW) := '1';
                else
                    w_error := '1';
                end if;
            when others =>
                w_error := '1';
            end case;
        when OPCODE_ADDW =>
            wb_isa_type(ISA_R_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_radr2 := '0' & wb_instr(24 downto 20);
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            case wb_opcode2 is
            when "000" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_ADDW) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_MULW) := '1';
                elsif wb_instr(31 downto 25) = "0100000" then
                    wb_dec(Instr_SUBW) := '1';
                else
                    w_error := '1';
                end if;
            when "001" =>
                wb_dec(Instr_SLLW) := '1';
            when "100" =>
                if wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_DIVW) := '1';
                else
                    w_error := '1';
                end if;
            when "101" =>
                if wb_instr(31 downto 25) = "0000000" then
                    wb_dec(Instr_SRLW) := '1';
                elsif wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_DIVUW) := '1';
                elsif wb_instr(31 downto 25) = "0100000" then
                    wb_dec(Instr_SRAW) := '1';
                else
                    w_error := '1';
                end if;
            when "110" =>
                if wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_REMW) := '1';
                else
                    w_error := '1';
                end if;
            when "111" =>
                if wb_instr(31 downto 25) = "0000001" then
                    wb_dec(Instr_REMUW) := '1';
                else
                    w_error := '1';
                end if;
            when others =>
                w_error := '1';
            end case;
        when OPCODE_AUIPC =>
            wb_isa_type(ISA_U_type) := '1';
            wb_dec(Instr_AUIPC) := '1';
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(31 downto 12) := wb_instr(31 downto 12);
            vb_imm(RISCV_ARCH-1 downto 32) := (others => wb_instr(31));
        when OPCODE_BEQ =>
            wb_isa_type(ISA_SB_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_radr2 := '0' & wb_instr(24 downto 20);
            vb_imm(11 downto 1) := wb_instr(7) & wb_instr(30 downto 25) & wb_instr(11 downto 8);
            vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
            case wb_opcode2 is
            when "000" =>
                wb_dec(Instr_BEQ) := '1';
            when "001" =>
                wb_dec(Instr_BNE) := '1';
            when "100" =>
                wb_dec(Instr_BLT) := '1';
            when "101" =>
                wb_dec(Instr_BGE) := '1';
            when "110" =>
                wb_dec(Instr_BLTU) := '1';
            when "111" =>
                wb_dec(Instr_BGEU) := '1';
            when others =>
                w_error := '1';
            end case;
        when OPCODE_JAL =>
            wb_isa_type(ISA_UJ_type) := '1';
            wb_dec(Instr_JAL) := '1';
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(19 downto 1) := wb_instr(19 downto 12) & wb_instr(20) & wb_instr(30 downto 21);
            vb_imm(RISCV_ARCH-1 downto 20) := (others => wb_instr(31));
        when OPCODE_JALR =>
            wb_isa_type(ISA_I_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(11 downto 0) := wb_instr(31 downto 20);
            vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
            case wb_opcode2 is
            when "000" =>
                wb_dec(Instr_JALR) := '1';
            when others =>
                w_error := '1';
            end case;
        when OPCODE_LB =>
            wb_isa_type(ISA_I_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(11 downto 0) := wb_instr(31 downto 20);
            vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
            case wb_opcode2 is
            when "000" =>
                wb_dec(Instr_LB) := '1';
            when "001" =>
                wb_dec(Instr_LH) := '1';
            when "010" =>
                wb_dec(Instr_LW) := '1';
            when "011" =>
                wb_dec(Instr_LD) := '1';
            when "100" =>
                wb_dec(Instr_LBU) := '1';
            when "101" =>
                wb_dec(Instr_LHU) := '1';
            when "110" =>
                wb_dec(Instr_LWU) := '1';
            when others =>
                w_error := '1';
            end case;
        when OPCODE_LUI =>
            wb_isa_type(ISA_U_type) := '1';
            wb_dec(Instr_LUI) := '1';
            vb_waddr := '0' & wb_instr(11 downto 7);             -- rd
            vb_imm(31 downto 12) := wb_instr(31 downto 12);
            vb_imm(RISCV_ARCH-1 downto 32) := (others => wb_instr(31));
        when OPCODE_SB =>
            wb_isa_type(ISA_S_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_radr2 := '0' & wb_instr(24 downto 20);
            vb_imm(11 downto 0) := wb_instr(31 downto 25) & wb_instr(11 downto 7);
            vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
            case wb_opcode2 is
            when "000" =>
                wb_dec(Instr_SB) := '1';
            when "001" =>
                wb_dec(Instr_SH) := '1';
            when "010" =>
                wb_dec(Instr_SW) := '1';
            when "011" =>
                wb_dec(Instr_SD) := '1';
            when others =>
                w_error := '1';
            end case;
        when OPCODE_CSRR =>
            wb_isa_type(ISA_I_type) := '1';
            vb_radr1 := '0' & wb_instr(19 downto 15);
            vb_waddr := '0' & wb_instr(11 downto 7);
            vb_csr_addr(11 downto 0) := wb_instr(31 downto 20);
            vb_imm(11 downto 0) := wb_instr(31 downto 20);
            vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
            case wb_opcode2 is
            when "000" =>
                if wb_instr = X"00000073" then
                    wb_dec(Instr_ECALL) := '1';
                elsif wb_instr = X"00100073" then
                    wb_dec(Instr_EBREAK) := '1';
                elsif wb_instr = X"00200073" then
                    wb_dec(Instr_URET) := '1';
                elsif wb_instr = X"10200073" then
                    wb_dec(Instr_SRET) := '1';
                elsif wb_instr = X"20200073" then
                    wb_dec(Instr_HRET) := '1';
                elsif wb_instr = X"30200073" then
                    wb_dec(Instr_MRET) := '1';
                else
                    w_error := '1';
                end if;
            when "001" =>
                wb_dec(Instr_CSRRW) := '1';
            when "010" =>
                wb_dec(Instr_CSRRS) := '1';
            when "011" =>
                wb_dec(Instr_CSRRC) := '1';
            when "101" =>
                wb_dec(Instr_CSRRWI) := '1';
            when "110" =>
                wb_dec(Instr_CSRRSI) := '1';
            when "111" =>
                wb_dec(Instr_CSRRCI) := '1';
            when others =>
                w_error := '1';
            end case;
        when OPCODE_FENCE =>
            case wb_opcode2 is
            when "000" =>
                wb_dec(Instr_FENCE) := '1';
            when "001" =>
                wb_dec(Instr_FENCE_I) := '1';
            when others =>
                w_error := '1';
            end case;

        when others =>
            if fpu_ena then
                case wb_opcode1 is
                when OPCODE_FPU_LD =>
                    wb_isa_type(ISA_I_type) := '1';
                    vb_radr1 := '0' & wb_instr(19 downto 15);
                    vb_waddr := '1' & wb_instr(11 downto 7);
                    vb_imm(11 downto 0) := wb_instr(31 downto 20);
                    vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
                    if wb_opcode2 = "011" then
                        wb_dec(Instr_FLD) := '1';
                    else
                        w_error := '1';
                    end if;
                when OPCODE_FPU_SD =>
                    wb_isa_type(ISA_S_type) := '1';
                    vb_radr1 := '0' & wb_instr(19 downto 15);
                    vb_radr2 := '1' & wb_instr(24 downto 20);
                    vb_imm(11 downto 0) := wb_instr(31 downto 25) & wb_instr(11 downto 7);
                    vb_imm(RISCV_ARCH-1 downto 12) := (others => wb_instr(31));
                    if wb_opcode2 = "011" then
                        wb_dec(Instr_FSD) := '1';
                    else
                        w_error := '1';
                    end if;
                when OPCODE_FPU_OP =>
                    wb_isa_type(ISA_R_type) := '1';
                    vb_radr1 := '1' & wb_instr(19 downto 15);
                    vb_radr2 := '1' & wb_instr(24 downto 20);
                    vb_waddr := '1' & wb_instr(11 downto 7);
                    case wb_instr(31 downto 25) is
                    when "0000001" =>
                        wb_dec(Instr_FADD_D) := '1';
                    when "0000101" =>
                        wb_dec(Instr_FSUB_D) := '1';
                    when "0001001" =>
                        wb_dec(Instr_FMUL_D) := '1';
                    when "0001101" =>
                        wb_dec(Instr_FDIV_D) := '1';
                    when "0010101" =>
                        if wb_opcode2 = "000" then
                            wb_dec(Instr_FMIN_D) := '1';
                        elsif wb_opcode2 = "001" then
                            wb_dec(Instr_FMAX_D) := '1';
                        else
                            w_error := '1';
                        end if;
                    when "1010001" =>
                        vb_waddr(5) := '0';
                        if wb_opcode2 = "000" then
                            wb_dec(Instr_FLE_D) := '1';
                        elsif wb_opcode2 = "001" then
                            wb_dec(Instr_FLT_D) := '1';
                        elsif wb_opcode2 = "010" then
                            wb_dec(Instr_FEQ_D) := '1';
                        else
                            w_error := '1';
                        end if;
                    when "1100001" =>
                        vb_waddr(5) := '0';
                        if wb_instr(24 downto 20) = "00000" then
                            wb_dec(Instr_FCVT_W_D) := '1';
                        elsif wb_instr(24 downto 20) = "00001" then
                            wb_dec(Instr_FCVT_WU_D) := '1';
                        elsif wb_instr(24 downto 20) = "00010" then
                            wb_dec(Instr_FCVT_L_D) := '1';
                        elsif wb_instr(24 downto 20) = "00011" then
                            wb_dec(Instr_FCVT_LU_D) := '1';
                        else
                            w_error := '1';
                        end if;
                    when "1101001" =>
                        vb_radr1(5) := '0';
                        if wb_instr(24 downto 20) = "00000" then
                            wb_dec(Instr_FCVT_D_W) := '1';
                        elsif wb_instr(24 downto 20) = "00001" then
                            wb_dec(Instr_FCVT_D_WU) := '1';
                        elsif wb_instr(24 downto 20) = "00010" then
                            wb_dec(Instr_FCVT_D_L) := '1';
                        elsif wb_instr(24 downto 20) = "00011" then
                            wb_dec(Instr_FCVT_D_LU) := '1';
                        else
                            w_error := '1';
                        end if;
                    when "1110001" =>
                        vb_waddr(5) := '0';
                        if wb_instr(24 downto 20) = "00000" and wb_opcode2 = "000" then
                            wb_dec(Instr_FMOV_X_D) := '1';
                        else
                            w_error := '1';
                        end if;
                    when "1111001" =>
                        vb_radr1(5) := '0';
                        if wb_instr(24 downto 20) = "00000" and wb_opcode2 = "000" then
                            wb_dec(Instr_FMOV_D_X) := '1';
                        else
                            w_error := '1';
                        end if;
                    when others =>
                        w_error := '1';
                    end case;
                when others =>
                    w_error := '1';
                end case;
            else
                w_error := '1';
            end if;
        end case;
        wb_instr_out := wb_instr;
    end if;


    if i_flush_pipeline = '1' and i_progbuf_ena = '0' then
        v.pc := (others => '1');
        v.valid := '0';
    elsif i_e_ready = '1' and i_f_valid = '1' then
        v.valid := '1';
        v.pc := i_f_pc;
        v.instr := i_f_instr;
        v.compressed := w_compressed;
        v.instr_load_fault := i_instr_load_fault;
        v.instr_executable := i_instr_executable;
        v.progbuf_ena := i_progbuf_ena;

        v.isa_type := wb_isa_type;
        v.instr_vec := wb_dec;
        v.memop_store := wb_dec(Instr_SD) or wb_dec(Instr_SW) 
                      or wb_dec(Instr_SH) or wb_dec(Instr_SB)
                      or wb_dec(Instr_FSD);
        v.memop_load := wb_dec(Instr_LD) or wb_dec(Instr_LW)
                or wb_dec(Instr_LH) or wb_dec(Instr_LB)
                or wb_dec(Instr_LWU) or wb_dec(Instr_LHU) 
                or wb_dec(Instr_LBU) or wb_dec(Instr_FLD);
        v.memop_sign_ext := wb_dec(Instr_LD) or wb_dec(Instr_LW)
                or wb_dec(Instr_LH) or wb_dec(Instr_LB);
        if (wb_dec(Instr_LD) or wb_dec(Instr_SD) or
            wb_dec(Instr_FLD) or wb_dec(Instr_FSD)) = '1' then
            v.memop_size := MEMOP_8B;
        elsif (wb_dec(Instr_LW) or wb_dec(Instr_LWU) or wb_dec(Instr_SW)) = '1' then
            v.memop_size := MEMOP_4B;
        elsif (wb_dec(Instr_LH) or wb_dec(Instr_LHU) or wb_dec(Instr_SH)) = '1' then
            v.memop_size := MEMOP_2B;
        else
            v.memop_size := MEMOP_1B;
        end if;
        v.unsigned_op := wb_dec(Instr_DIVU) or wb_dec(Instr_REMU) or
                         wb_dec(Instr_DIVUW) or wb_dec(Instr_REMUW) or
                         wb_dec(Instr_MULHU) or
                         wb_dec(Instr_FCVT_WU_D) or
                         wb_dec(Instr_FCVT_LU_D);

        v.rv32 := wb_dec(Instr_ADDW) or wb_dec(Instr_ADDIW) 
            or wb_dec(Instr_SLLW) or wb_dec(Instr_SLLIW) or wb_dec(Instr_SRAW)
            or wb_dec(Instr_SRAIW)
            or wb_dec(Instr_SRLW) or wb_dec(Instr_SRLIW) or wb_dec(Instr_SUBW) 
            or wb_dec(Instr_DIVW) or wb_dec(Instr_DIVUW) or wb_dec(Instr_MULW)
            or wb_dec(Instr_REMW) or wb_dec(Instr_REMUW);

        v.f64 := wb_dec(Instr_FADD_D) or wb_dec(Instr_FSUB_D)
            or wb_dec(Instr_FMUL_D) or wb_dec(Instr_FDIV_D)
            or wb_dec(Instr_FMIN_D) or wb_dec(Instr_FMAX_D)
            or wb_dec(Instr_FLE_D) or wb_dec(Instr_FLT_D)
            or wb_dec(Instr_FEQ_D) or wb_dec(Instr_FCVT_W_D)
            or wb_dec(Instr_FCVT_WU_D) or wb_dec(Instr_FCVT_L_D)
            or wb_dec(Instr_FCVT_LU_D) or wb_dec(Instr_FMOV_X_D)
            or wb_dec(Instr_FCVT_D_W) or wb_dec(Instr_FCVT_D_WU)
            or wb_dec(Instr_FCVT_D_L) or wb_dec(Instr_FCVT_D_LU)
            or wb_dec(Instr_FMOV_D_X) or wb_dec(Instr_FLD)
            or wb_dec(Instr_FSD);
        
        v.instr_unimplemented := w_error;
        v.radr1 := vb_radr1;
        v.radr2 := vb_radr2;
        v.waddr := vb_waddr;
        v.csr_addr := vb_csr_addr;
        v.imm := vb_imm;
    elsif i_any_hold = '0' then
        v.valid := '0';
    end if;
    w_o_valid := r.valid;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_valid <= w_o_valid;
    o_pc <= r.pc;
    o_instr <= r.instr;
    o_memop_load <= r.memop_load;
    o_memop_store <= r.memop_store;
    o_memop_sign_ext <= r.memop_sign_ext;
    o_memop_size <= r.memop_size;
    o_unsigned_op <= r.unsigned_op;
    o_rv32 <= r.rv32;
    o_f64 <= r.f64;
    o_compressed <= r.compressed;
    o_isa_type <= r.isa_type;
    o_instr_vec <= r.instr_vec;
    o_exception <= r.instr_unimplemented;
    o_instr_load_fault <= r.instr_load_fault;
    o_instr_executable <= r.instr_executable;

    o_radr1 <= r.radr1;
    o_radr2 <= r.radr2;
    o_waddr <= r.waddr;
    o_csr_addr <= r.csr_addr;
    o_imm <= r.imm;
    o_progbuf_ena <= r.progbuf_ena;
    
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
