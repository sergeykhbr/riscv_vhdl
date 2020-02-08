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


entity BranchPredictor is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;                              -- CPU clock
    i_nrst : in std_logic;                             -- Reset. Active LOW.
    i_req_mem_fire : in std_logic;                     -- Memory request was accepted
    i_resp_mem_valid : in std_logic;                   -- Memory response from ICache is valid
    i_resp_mem_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);-- Memory response address
    i_resp_mem_data : in std_logic_vector(31 downto 0);-- Memory response value
    i_e_npc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);  -- Valid instruction value awaited by 'Executor'
    i_ra : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Return address register value
    o_npc_predict : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0)  -- Predicted next instruction address
  );
end; 
 
architecture arch_BranchPredictor of BranchPredictor is

  type HistoryType is record
      resp_pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      resp_npc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  end record;
  constant history_none : HistoryType := (
    (others => '1'), (others => '1')
  );

  type HistoryVector is array (0 to 2) of HistoryType;

  type RegistersType is record
      h : HistoryVector;
      wait_resp : std_logic;
  end record;

  constant R_RESET : RegistersType := (
      (others => history_none), '0'
  );

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_req_mem_fire, i_resp_mem_valid, i_resp_mem_addr,
                 i_resp_mem_data, i_e_npc, i_ra, r)
    variable v : RegistersType;
    variable vb_tmp : std_logic_vector(31 downto 0);
    variable vb_npc_predicted : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_npc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable v_jal : std_logic;
    variable vb_jal_off : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_jal_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable v_branch : std_logic;
    variable vb_branch_off : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_branch_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable v_c_j : std_logic;
    variable vb_c_j_off : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_c_j_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable v_c_ret : std_logic;
  begin

    v := r;

    vb_pc := r.h(0).resp_pc;
    vb_tmp := i_resp_mem_data;
 
    -- Unconditional jump "J"
    vb_jal_off(CFG_CPU_ADDR_BITS-1 downto 20) := (others => vb_tmp(31));
    vb_jal_off(19 downto 12) := vb_tmp(19 downto 12);
    vb_jal_off(11) := vb_tmp(20);
    vb_jal_off(10 downto 1) := vb_tmp(30 downto 21);
    vb_jal_off(0) := '0';
    vb_jal_addr := vb_pc + vb_jal_off;

    v_jal := '0';
    if vb_tmp(6 downto 0) = "1101111" then
        if (vb_jal_addr /= r.h(1).resp_pc) and (vb_jal_addr /= r.h(2).resp_pc) then
            v_jal := '1';
        end if;
    end if;

    -- Conditional branches "BEQ", "BNE", "BLT", "BGE", BLTU", "BGEU"
    -- Only negative offset leads to predicted jumps
    if vb_tmp(31) = '1' then
        vb_branch_off(CFG_CPU_ADDR_BITS-1 downto 12) := (others => '1');
    else
        vb_branch_off(CFG_CPU_ADDR_BITS-1 downto 12) := (others => '0');
    end if;
    vb_branch_off(11) := vb_tmp(7);
    vb_branch_off(10 downto 5) := vb_tmp(30 downto 25);
    vb_branch_off(4 downto 1) := vb_tmp(11 downto 8);
    vb_branch_off(0) := '0';
    vb_branch_addr := vb_pc + vb_branch_off;

    v_branch := '0';
    if (vb_tmp(6 downto 0) = "1100011") and (vb_tmp(31) = '1') then
        if (vb_branch_addr /= r.h(1).resp_pc)
            and (vb_branch_addr /= r.h(2).resp_pc) then
            v_branch := '1';
        end if;
    end if;

    -- Check Compressed "C_J" unconditional jump
    if vb_tmp(12) = '1' then
        vb_c_j_off(CFG_CPU_ADDR_BITS-1 downto 11) := (others => '1');
    else
        vb_c_j_off(CFG_CPU_ADDR_BITS-1 downto 11) := (others => '0');
    end if;
    vb_c_j_off(10) := vb_tmp(8);
    vb_c_j_off(9 downto 8) := vb_tmp(10 downto 9);
    vb_c_j_off(7) := vb_tmp(6);
    vb_c_j_off(6) := vb_tmp(7);
    vb_c_j_off(5) := vb_tmp(2);
    vb_c_j_off(4) := vb_tmp(11);
    vb_c_j_off(3 downto 1) := vb_tmp(5 downto 3);
    vb_c_j_off(0) := '0';
    vb_c_j_addr := vb_pc + vb_c_j_off;

    v_c_j := '0';
    if (vb_tmp(15 downto 13) = "101") and (vb_tmp(1 downto 0) = "01") then
        if (vb_c_j_addr /= r.h(1).resp_pc) and (vb_c_j_addr /= r.h(2).resp_pc) then
            v_c_j := '1';
        end if;
    end if;

    -- Compressed RET pseudo-instruction
    v_c_ret := '0';
    if vb_tmp(15 downto 0) = X"8082" then
        v_c_ret := '1';
    end if;

    if v_jal = '1' then
        vb_npc_predicted := vb_jal_addr;
    elsif v_branch = '1' then
        vb_npc_predicted := vb_branch_addr;
    elsif v_c_j = '1' then
        vb_npc_predicted := vb_c_j_addr;
    elsif v_c_ret = '1' then
        vb_npc_predicted := i_ra(CFG_CPU_ADDR_BITS-1 downto 0);
    elsif vb_tmp(1 downto 0) = "11" then
        vb_npc_predicted := r.h(0).resp_pc + 4;
    else
        vb_npc_predicted := r.h(0).resp_pc + 2;
    end if;

    if i_e_npc = r.h(2).resp_pc then
        if r.h(2).resp_npc = r.h(1).resp_pc then
            if r.h(1).resp_npc = r.h(0).resp_pc then
                vb_npc := vb_npc_predicted;
            else
                vb_npc := r.h(1).resp_npc;
            end if;
        elsif r.h(2).resp_npc = r.h(0).resp_pc then
            vb_npc := vb_npc_predicted;
        else
            vb_npc := r.h(2).resp_npc;
        end if;
    elsif i_e_npc = r.h(1).resp_pc then
        if r.h(1).resp_npc = r.h(0).resp_pc then
            vb_npc := vb_npc_predicted;
        else
            vb_npc := r.h(1).resp_npc;
        end if;
    elsif i_e_npc = r.h(0).resp_pc then
        vb_npc := vb_npc_predicted;
    else
        vb_npc := i_e_npc;
    end if;

    if i_req_mem_fire = '1' and r.wait_resp = '0' then
        v.wait_resp := '1';
        v.h(0).resp_pc := vb_npc;
        v.h(0).resp_npc := (others => '1');
        v.h(1) := r.h(0);
        v.h(2) := r.h(1);
    elsif i_req_mem_fire = '1' and i_resp_mem_valid = '1' then
        v.wait_resp := '1';
        v.h(0).resp_pc := vb_npc;
        v.h(0).resp_npc := (others => '1');
        v.h(1) := r.h(0);
        v.h(1).resp_npc := vb_npc;
        v.h(2) := r.h(1);
    elsif i_resp_mem_valid = '1' and r.wait_resp = '1' then
        v.wait_resp := '0';
        v.h(0).resp_npc := vb_npc;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_npc_predict <= vb_npc;
    
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
