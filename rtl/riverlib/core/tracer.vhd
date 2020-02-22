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


entity tracer is generic (
    async_reset : boolean;
    trace_file : string := ""
  );
  port (
    i_clk : in std_logic;                                   -- CPU clock
    i_nrst : in std_logic;                                  -- Reset. Active LOW.
    i_dbg_executed_cnt : in std_logic_vector(63 downto 0);
    i_e_valid : in std_logic;
    i_e_pc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_e_instr : in std_logic_vector(31 downto 0);
    i_e_memop_store : in std_logic;
    i_e_memop_load : in std_logic;
    i_e_memop_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_e_res_data : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_e_res_addr : in std_logic_vector(5 downto 0);
    i_m_wena : in std_logic;
    i_m_waddr : in std_logic_vector(5 downto 0);
    i_m_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0)
  );
end; 
 
architecture arch_tracer of tracer is

  type regnames_type is array (0 to Reg_Total+RegFpu_Total-1) of string;
  
  constant rname : regnames_type := (
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
    "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
    "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
    "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
    "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
  );

  type RegistersType is record
      load_reg : std_logic;
      load_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  end record;

  constant R_RESET : RegistersType := ('0', (others => '0'));

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_dbg_executed_cnt, i_e_valid, i_e_pc, i_e_instr,
                 i_e_memop_store, i_e_memop_load, i_e_memop_addr, i_e_res_data,
                 i_e_res_addr, i_m_wena, i_m_waddr, i_m_wdata, r)
    variable v : RegistersType;
  begin

    v := r;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
  end process;


  -- registers:
  regs : process(i_nrst, i_clk)
  begin 
     if async_reset and i_nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
