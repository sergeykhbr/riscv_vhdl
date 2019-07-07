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


entity RegFloatBank is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;                                   -- CPU clock
    i_nrst : in std_logic;                                  -- Reset. Active LOW.

    i_radr1 : in std_logic_vector(5 downto 0);              -- Port 1 read address
    o_rdata1 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 1 read value

    i_radr2 : in std_logic_vector(5 downto 0);              -- Port 2 read address
    o_rdata2 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 2 read value

    i_waddr : in std_logic_vector(5 downto 0);              -- Writing value
    i_wena : in std_logic;                                  -- Writing is enabled
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Writing value

    i_dport_addr : in std_logic_vector(4 downto 0);         -- Debug port address
    i_dport_ena : in std_logic;                             -- Debug port is enabled
    i_dport_write : in std_logic;                           -- Debug port write is enabled
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Debug port write value
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0)-- Debug port read value
  );
end; 
 
architecture arch_RegFloatBank of RegFloatBank is

  type MemoryType is array (0 to RegFpu_Total-1) 
         of std_logic_vector(RISCV_ARCH-1 downto 0);

  type RegistersType is record
      mem : MemoryType;
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_radr1, i_radr2, i_waddr, i_wena, i_wdata,
                 i_dport_ena, i_dport_write, i_dport_addr, i_dport_wdata, r)
    variable v : RegistersType;
  begin

    v := r;
    --! Debug port has higher priority. Collision must be controlled by SW
    if (i_dport_ena and i_dport_write) = '1' then
        if i_dport_addr /= "00000" then
            v.mem(conv_integer(i_dport_addr)) := i_dport_wdata;
        end if;
    elsif i_wena = '1' and i_waddr(5) = '1' then
        v.mem(conv_integer(i_waddr(4 downto 0))) := i_wdata;
    end if;

    if not async_reset and i_nrst = '0' then
        for i in 0 to RegFpu_Total-1 loop
            v.mem(i) := X"00000000FEEDFACE";
        end loop;
    end if;

    rin <= v;
  end process;

  o_rdata1 <= r.mem(conv_integer(i_radr1(4 downto 0)));
  o_rdata2 <= r.mem(conv_integer(i_radr2(4 downto 0)));
  o_dport_rdata <= r.mem(conv_integer(i_dport_addr));

  -- registers:
  regs : process(i_nrst, i_clk)
  begin 
     if async_reset and i_nrst = '0' then
        for i in 0 to RegFpu_Total-1 loop
            r.mem(i) <= X"00000000FEEDFACE";
        end loop;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
