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

  type RegistersType is record
      valid : std_logic;
  end record;

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_any_hold, i_f_valid, i_f_pc, i_f_instr, r)
    variable v : RegistersType;
    variable w_o_valid : std_logic;
    variable w_error : std_logic;
    variable wb_instr : std_logic_vector(31 downto 0);
  begin

    v := r;
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
