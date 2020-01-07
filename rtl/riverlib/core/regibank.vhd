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
use IEEE.std_logic_arith.all;  -- UNSIGNED function
use ieee.std_logic_misc.all;  -- or_reduce()
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity RegBank is generic (
    async_reset : boolean;
    fpu_ena : boolean
  );
  port (
    i_clk : in std_logic;                                   -- CPU clock
    i_nrst : in std_logic;                                  -- Reset. Active LOW.

    i_radr1 : in std_logic_vector(5 downto 0);              -- Port 1 read address
    o_rdata1 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 1 read value
    o_rhazard1 : out std_logic;

    i_radr2 : in std_logic_vector(5 downto 0);              -- Port 2 read address
    o_rdata2 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 2 read value
    o_rhazard2 : out std_logic;

    i_waddr : in std_logic_vector(5 downto 0);              -- Writing value
    i_wena : in std_logic;                                  -- Writing is enabled
    i_whazard : in std_logic;
    i_wtag : in std_logic_vector(3 downto 0);
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Writing value
    o_wtag : out std_logic_vector(3 downto 0);

    i_dport_addr : in std_logic_vector(5 downto 0);         -- Debug port address
    i_dport_ena : in std_logic;                             -- Debug port is enabled
    i_dport_write : in std_logic;                           -- Debug port write is enabled
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Debug port write value
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Debug port read value

    o_ra : out std_logic_vector(RISCV_ARCH-1 downto 0);     -- Return address for branch predictor
    o_sp : out std_logic_vector(RISCV_ARCH-1 downto 0)      -- Stack Pointer for the borders control
  );
end; 
 
architecture arch_RegBank of RegBank is

  constant REG_MSB : integer := 4 + conv_integer(fpu_ena);
  constant REGS_TOTAL : integer := 2**(REG_MSB + 1);

  type reg_score_type is record
    val : std_logic_vector(RISCV_ARCH-1 downto 0);
    tag : std_logic_vector(3 downto 0);
    hazard : std_logic;
  end record;

  type MemoryType is array (0 to REGS_TOTAL-1) of reg_score_type;

  type RegistersType is record
      mem : MemoryType;
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_radr1, i_radr2, i_waddr, i_wena, i_whazard, i_wtag, i_wdata,
                 i_dport_ena, i_dport_write, i_dport_addr, i_dport_wdata, r)
    variable v : RegistersType;
    variable int_waddr : integer;
    variable int_daddr : integer;
  begin

    for i in 0 to REGS_TOTAL-1 loop
        v.mem(i).hazard := r.mem(i).hazard;
        v.mem(i).val := r.mem(i).val;
        v.mem(i).tag := r.mem(i).tag;
    end loop;

    int_waddr := conv_integer(i_waddr(REG_MSB downto 0));
    int_daddr := conv_integer(i_dport_addr(REG_MSB downto 0));

    --! Debug port has higher priority. Collision must be controlled by SW
    if (i_dport_ena and i_dport_write) = '1' then
        if or_reduce(i_dport_addr) = '1' then
            v.mem(int_daddr).val := i_dport_wdata;
            v.mem(int_daddr).hazard := '0';
        end if;
    elsif i_wena = '1' and or_reduce(i_waddr(REG_MSB downto 0)) = '1' then
        if i_wtag = r.mem(int_waddr).tag then
            v.mem(int_waddr).hazard := i_whazard;
            v.mem(int_waddr).val := i_wdata;
            v.mem(int_waddr).tag := r.mem(int_waddr).tag + 1;
        end if;
    end if;

    if not async_reset and i_nrst = '0' then
        v.mem(Reg_Zero).hazard := '0';
        v.mem(Reg_Zero).val := (others => '0');
        v.mem(Reg_Zero).tag := (others => '0');
        for i in 1 to REGS_TOTAL-1 loop
            v.mem(i).hazard := '0';
            v.mem(i).val := X"00000000FEEDFACE";
            v.mem(i).tag := (others => '0');
        end loop;
    end if;

    rin <= v;
  end process;

  o_rdata1 <= r.mem(conv_integer(i_radr1(REG_MSB downto 0))).val;
  o_rhazard1 <= r.mem(conv_integer(i_radr1(REG_MSB downto 0))).hazard;
  o_rdata2 <= r.mem(conv_integer(i_radr2(REG_MSB downto 0))).val;
  o_rhazard2 <= r.mem(conv_integer(i_radr2(REG_MSB downto 0))).hazard;
  o_wtag <= r.mem(conv_integer(i_waddr(REG_MSB downto 0))).tag;
  o_dport_rdata <= r.mem(conv_integer(i_dport_addr(REG_MSB downto 0))).val;
  o_ra <= r.mem(Reg_ra).val;
  o_sp <= r.mem(Reg_sp).val;

  -- registers:
  regs : process(i_nrst, i_clk)
  begin 
     if async_reset and i_nrst = '0' then
        r.mem(Reg_Zero).hazard <= '0';
        r.mem(Reg_Zero).val <= (others => '0');
        r.mem(Reg_Zero).tag <= (others => '0');
        for i in 1 to REGS_TOTAL-1 loop
            r.mem(i).hazard <= '0';
            r.mem(i).val <= X"00000000FEEDFACE";
            r.mem(i).tag <= (others => '0');
        end loop;
     elsif rising_edge(i_clk) then 
        for i in 0 to REGS_TOTAL-1 loop
            r.mem(i).hazard <= rin.mem(i).hazard;
            r.mem(i).val <= rin.mem(i).val;
            r.mem(i).tag <= rin.mem(i).tag;
        end loop;
     end if; 
  end process;

end;
