-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     CPU Instruction Decoder stage.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity InstrDecoder is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_any_hold : in std_logic;                               -- Hold pipeline by any reason
    i_f_valid : in std_logic;                                -- Fetch input valid
    i_f_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Fetched pc
    i_f_instr : in std_logic_vector(31 downto 0);            -- Fetched instruction value

    o_valid : out std_logic;                                 -- Current output values are valid
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- Current instruction pointer value
    o_instr : out std_logic_vector(31 downto 0);             -- Current instruction value
    o_memop_store : out std_logic;                           -- Store to memory operation
    o_memop_load : out std_logic;                            -- Load from memoru operation
    o_memop_sign_ext : out std_logic;                        -- Load memory value with sign extending
    o_memop_size : out std_logic_vector(1 downto 0);         -- Memory transaction size
    o_rv32 : out std_logic;                                  -- 32-bits instruction
    o_unsigned_op : out std_logic;                           -- Unsigned operands
    o_isa_type : out std_logic_vector(ISA_Total-1 downto 0); -- Instruction format accordingly with ISA
    o_instr_vec : out std_logic_vector(Instr_Total-1 downto 0); -- One bit per decoded instruction bus
    o_exception : out std_logic                              -- Unimplemented instruction
  );
end; 
 
architecture arch_InstrDecoder of InstrDecoder is

  -- LB, LH, LW, LD, LBU, LHU, LWU
  constant OPCODE_LB     : std_logic_vector(4 downto 0) := "00000";
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
  -- ADD, AND, OR, SLT, SLTU, SLL, SRA, SRL, SUB, XOR, DIV, DIVU, MUL, REM, REMU
  constant OPCODE_ADD    : std_logic_vector(4 downto 0) := "01100";
  -- LUI
  constant OPCODE_LUI    : std_logic_vector(4 downto 0) := "01101";
  -- ADDW, SLLW, SRAW, SRLW, SUBW, DIVW, DIVUW, MULW, REMW, REMUW
  constant OPCODE_ADDW   : std_logic_vector(4 downto 0) := "01110";
  -- BEQ, BNE, BLT, BGE, BLTU, BGEU
  constant OPCODE_BEQ    : std_logic_vector(4 downto 0) := "11000";
  -- JALR
  constant OPCODE_JALR   : std_logic_vector(4 downto 0) := "11001";
  -- JAL
  constant OPCODE_JAL    : std_logic_vector(4 downto 0) := "11011";
  -- CSRRC, CSRRCI, CSRRS, CSRRSI, CSRRW, CSRRWI, URET, SRET, HRET, MRET
  constant OPCODE_CSRR   : std_logic_vector(4 downto 0) := "11100"; 

  constant INSTR_NONE : std_logic_vector(Instr_Total-1 downto 0) := (others => '0');

  type RegistersType is record
      valid : std_logic;
      pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      isa_type : std_logic_vector(ISA_Total-1 downto 0);
      instr_vec : std_logic_vector(Instr_Total-1 downto 0);
      instr : std_logic_vector(31 downto 0);
      memop_store : std_logic;
      memop_load : std_logic;
      memop_sign_ext : std_logic;
      memop_size : std_logic_vector(1 downto 0);
      unsigned_op : std_logic;
      rv32 : std_logic;
      instr_unimplemented : std_logic;
  end record;

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_any_hold, i_f_valid, i_f_pc, i_f_instr, r)
    variable v : RegistersType;
    variable w_o_valid : std_logic;
    variable w_error : std_logic;
    variable wb_instr : std_logic_vector(31 downto 0);
    variable wb_opcode1 : std_logic_vector(4 downto 0);
    variable wb_opcode2 : std_logic_vector(2 downto 0);
    variable wb_dec : std_logic_vector(Instr_Total-1 downto 0);
    variable wb_isa_type : std_logic_vector(ISA_Total-1 downto 0);
  begin

    v := r;
    w_error := '0';
    wb_instr := i_f_instr;
    wb_opcode1 := wb_instr(6 downto 2);
    wb_opcode2 := wb_instr(14 downto 12);
    wb_dec := (others => '0');
    wb_isa_type := (others => '0');

    if wb_instr(1 downto 0) /= "11" then
        w_error := '1';
    end if;


    case wb_opcode1 is
    when OPCODE_ADD =>
        wb_isa_type(ISA_R_type) := '1';
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
            wb_dec(Instr_SLL) := '1';
        when "010" =>
            wb_dec(Instr_SLT) := '1';
        when "011" =>
            wb_dec(Instr_SLTU) := '1';
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
            elsif wb_instr(31 downto 26) = "100000" then
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
    when OPCODE_BEQ =>
        wb_isa_type(ISA_SB_type) := '1';
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
    when OPCODE_JALR =>
        wb_isa_type(ISA_I_type) := '1';
        case wb_opcode2 is
        when "000" =>
            wb_dec(Instr_JALR) := '1';
        when others =>
            w_error := '1';
        end case;
    when OPCODE_LB =>
        wb_isa_type(ISA_I_type) := '1';
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
    when OPCODE_SB =>
        wb_isa_type(ISA_S_type) := '1';
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
        case wb_opcode2 is
        when "000" =>
            if wb_instr = X"00200073" then
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
        w_error := '1';
    end case;


    if i_f_valid = '1' then
        v.valid := '1';
        v.pc := i_f_pc;
        v.instr := wb_instr;

        v.isa_type := wb_isa_type;
        v.instr_vec := wb_dec;
        v.memop_store := wb_dec(Instr_SD) or wb_dec(Instr_SW) 
                      or wb_dec(Instr_SH) or wb_dec(Instr_SB);
        v.memop_load := wb_dec(Instr_LD) or wb_dec(Instr_LW)
                or wb_dec(Instr_LH) or wb_dec(Instr_LB)
                or wb_dec(Instr_LWU) or wb_dec(Instr_LHU) 
                or wb_dec(Instr_LBU);
        v.memop_sign_ext := wb_dec(Instr_LD) or wb_dec(Instr_LW)
                or wb_dec(Instr_LH) or wb_dec(Instr_LB);
        if (wb_dec(Instr_LD) or wb_dec(Instr_SD)) = '1' then
            v.memop_size := MEMOP_8B;
        elsif (wb_dec(Instr_LW) or wb_dec(Instr_LWU) or wb_dec(Instr_SW)) = '1' then
            v.memop_size := MEMOP_4B;
        elsif (wb_dec(Instr_LH) or wb_dec(Instr_LHU) or wb_dec(Instr_SH)) = '1' then
            v.memop_size := MEMOP_2B;
        else
            v.memop_size := MEMOP_1B;
        end if;
        v.unsigned_op := wb_dec(Instr_DIVU) or wb_dec(Instr_REMU) or
                         wb_dec(Instr_DIVUW) or wb_dec(Instr_REMUW);

        v.rv32 := wb_dec(Instr_ADDW) or wb_dec(Instr_ADDIW) 
            or wb_dec(Instr_SLLW) or wb_dec(Instr_SLLIW) or wb_dec(Instr_SRAW)
            or wb_dec(Instr_SRAIW)
            or wb_dec(Instr_SRLW) or wb_dec(Instr_SRLIW) or wb_dec(Instr_SUBW) 
            or wb_dec(Instr_DIVW) or wb_dec(Instr_DIVUW) or wb_dec(Instr_MULW)
            or wb_dec(Instr_REMW) or wb_dec(Instr_REMUW);
        
        v.instr_unimplemented := w_error;
    elsif i_any_hold = '0' then
        v.valid := '0';
    end if;
    w_o_valid := r.valid and not i_any_hold;

    if i_nrst = '0' then
        v.valid := '0';
        v.pc := (others => '0');
        v.isa_type := (others => '0');
        v.instr_vec := (others => '0');
        v.memop_store := '0';
        v.memop_load := '0';
        v.memop_sign_ext := '0';
        v.memop_size := MEMOP_1B;
        v.unsigned_op := '0';
        v.rv32 := '0';
        v.instr_unimplemented := '0';
        if wb_dec = INSTR_NONE then
            v.instr_unimplemented := '1';
        end if;
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
    o_isa_type <= r.isa_type;
    o_instr_vec <= r.instr_vec;
    o_exception <= r.instr_unimplemented;
    
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk)
  begin 
     if rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
