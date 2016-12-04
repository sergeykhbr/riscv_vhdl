-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Branch predictor.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity BranchPredictor is
  port (
    i_clk : in std_logic;                              -- CPU clock
    i_nrst : in std_logic;                             -- Reset. Active LOW.
    i_hold : in std_logic;                             -- Hold pipeline by any reason
    i_f_mem_request : in std_logic;                    -- Memory request from 'fetch' is valid, form next prediction address
    i_f_predic_miss : in std_logic;                    -- Fetch modul detects deviation between predicted and valid pc.
    i_f_instr_valid : in std_logic;                    -- Fetched instruction is valid
    i_f_instr : in std_logic_vector(31 downto 0);      -- Fetched instruction value is used for fast parse 'jump/branch' in predictor
    i_e_npc : in std_logic_vector(31 downto 0);        -- Valid instruction value awaited by 'Executor'
    i_ra : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Return address register value
    o_npc_predict : out std_logic_vector(31 downto 0)  -- Predicted next instruction address
  );
end; 
 
architecture arch_BranchPredictor of BranchPredictor is

  type RegistersType is record
      npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_hold, i_f_mem_request, i_f_predic_miss, 
                i_f_instr_valid, i_f_instr, i_e_npc, i_ra, r)
    variable v : RegistersType;
    variable wb_npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  begin

    v := r;
    if i_f_mem_request = '1' then
        if i_f_predic_miss = '1' and i_hold = '0'then
            v.npc := i_e_npc + 4;
        else
            -- todo: JAL and JALR ra (return)
            v.npc := r.npc + 4;
        end if;
    end if;

    if i_nrst = '0' then
        v.npc := RESET_VECTOR;
    end if;

    o_npc_predict <= r.npc;
    
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
