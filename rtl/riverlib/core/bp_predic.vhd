-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Branch predictor.
--! @details   This module gives about 5% of performance improvement (CPI)
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
    i_req_mem_fire : in std_logic;                     -- Memory request was accepted
    i_resp_mem_valid : in std_logic;                   -- Memory response from ICache is valid
    i_resp_mem_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Memory response address
    i_resp_mem_data : in std_logic_vector(31 downto 0);-- Memory response value
    i_e_npc : in std_logic_vector(31 downto 0);        -- Valid instruction value awaited by 'Executor'
    i_ra : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Return address register value
    o_npc_predict : out std_logic_vector(31 downto 0); -- Predicted next instruction address
    o_predict : out std_logic;                         -- mark requested address as predicted
    o_minus2 : out std_logic;                          -- pc -= 2 flag
    o_minus4 : out std_logic                           -- pc -= 4 flag
  );
end; 
 
architecture arch_BranchPredictor of BranchPredictor is

  type HistoryType is record
      resp_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      resp_npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      ignore : std_logic;
  end record;
  constant history_none : HistoryType := (
    (others => '1'), (others => '1'), (others => '0'), '0'
  );

  type HistoryVector is array (0 to 2) of HistoryType;

  type RegistersType is record
      h : HistoryVector;
      minus2 : std_logic;
      minus4 : std_logic;
      c0 : std_logic;
      c1 : std_logic;
  end record;

  constant R_RESET : RegistersType := (
      (others => history_none), '0', '0', '0', '0'
  );

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_req_mem_fire, i_resp_mem_valid, i_resp_mem_addr,
                 i_resp_mem_data, i_e_npc, i_ra, r)
    variable v : RegistersType;
    variable vb_tmp : std_logic_vector(31 downto 0);
    variable vb_npc2 : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable vb_jal_off : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable v_predict : std_logic;
    variable v_sequence : std_logic;
    variable v_c0 : std_logic;
  begin

    v := r;

    vb_tmp := i_resp_mem_data;
 
    vb_jal_off(BUS_ADDR_WIDTH-1 downto 20) := (others => vb_tmp(31));
    vb_jal_off(19 downto 12) := vb_tmp(19 downto 12);
    vb_jal_off(11) := vb_tmp(20);
    vb_jal_off(10 downto 1) := vb_tmp(30 downto 21);
    vb_jal_off(0) := '0';

    v_predict := '0';

    if r.minus2 = '1' then
        v_c0 := r.c1;
    else
        v_c0 := not (i_resp_mem_data(1) and i_resp_mem_data(0));
    end if;

    v_sequence := '1';
    if i_e_npc = r.h(2).resp_pc then
        if r.h(2).resp_npc = r.h(1).resp_pc then
            if r.h(1).resp_npc = r.h(0).resp_pc then
                vb_npc2 := r.h(0).resp_npc;
            else
                vb_npc2 := r.h(1).resp_npc;
            end if;
        elsif r.h(2).resp_npc = r.h(0).resp_pc then
            vb_npc2 := r.h(0).resp_npc;
        else
            vb_npc2 := r.h(2).resp_npc;
        end if;
    elsif i_e_npc = r.h(1).resp_pc then
        if r.h(1).resp_npc = r.h(0).resp_pc then
            vb_npc2 := r.h(0).resp_npc;
        else
            vb_npc2 := r.h(1).resp_npc;
        end if;
    elsif i_e_npc = r.h(0).resp_pc then
        vb_npc2 := r.h(0).resp_npc;
    else
        vb_npc2 := i_e_npc;
        v_sequence := '0';
    end if;

    if i_resp_mem_valid = '1' then
        v.c0 := not (i_resp_mem_data(1) and i_resp_mem_data(0));
        v.c1 := not (i_resp_mem_data(17) and i_resp_mem_data(16));
    end if;

    if i_req_mem_fire = '1' and i_resp_mem_valid = '0' then
        v.h(0).req_addr := vb_npc2;
        v.h(0).ignore := '0';
        v.h(0).resp_pc := vb_npc2;
        v.h(0).resp_npc := vb_npc2 + 4;   -- default instruction size
        v.h(1) := r.h(0);
        v.h(2) := r.h(1);
    elsif i_req_mem_fire = '1' and i_resp_mem_valid = '1' then
        v.h(0).req_addr := vb_npc2;
        v.h(0).ignore := '0';
        v.minus2 := '0';
        v.minus4 := '0';
        v.h(1) := r.h(0);
        if v_sequence = '1' and v_c0 = '1' and r.minus2 = '1' then
            -- Two sequental C-instruction, 
            --   ignore memory response and re-use full fetched previous value
            v.h(0).ignore := '1';
            v.h(0).resp_pc := i_resp_mem_addr;
            if i_resp_mem_data(1 downto 0) /= "11" then
                v.h(0).resp_npc := i_resp_mem_addr+2;
            else
                v.h(0).resp_npc := i_resp_mem_addr+4;
            end if;
            v.h(1).resp_npc := r.h(0).resp_npc-2;
            v.minus4 := '1';
        elsif v_sequence = '1' and v_c0 = '1' and r.minus4 = '0' then
            -- 1-st of two C-instructions
            v.minus2 := not r.minus4;
            v.h(0).resp_pc := vb_npc2 - 2;
            v.h(0).resp_npc := vb_npc2 + 2;
            v.h(1).resp_pc := r.h(0).resp_pc;
            v.h(1).resp_npc := r.h(0).resp_npc-2;
        else
            v.h(0).resp_pc := vb_npc2;
            v.h(0).resp_npc := vb_npc2 + 4;
        end if;
        v.h(2) := r.h(1);

    elsif i_resp_mem_valid = '1' then
        v.h(1) := r.h(0);
        if v_sequence = '1' and v_c0 = '1' and r.minus2 = '1' then
            v.h(1).resp_npc := r.h(0).resp_npc-2;
        elsif v_sequence = '1' and v_c0 = '1' and r.minus4 = '0' then
            v.h(1).resp_npc := r.h(0).resp_npc-2;
        end if;
        v.h(2) := r.h(1);
    end if;


    if i_nrst = '0' then
        v := R_RESET;
    end if;

    o_npc_predict <= vb_npc2;
    o_predict <= v_predict;
    o_minus2 <= r.minus2;
    o_minus4 <= r.minus4;
    
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
