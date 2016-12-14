-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Multi-port CPU Integer Registers memory.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity RegIntBank is
  port (
    i_clk : in std_logic;                                   -- CPU clock
    i_nrst : in std_logic;                                  -- Reset. Active LOW.

    i_radr1 : in std_logic_vector(4 downto 0);              -- Port 1 read address
    o_rdata1 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 1 read value

    i_radr2 : in std_logic_vector(4 downto 0);              -- Port 2 read address
    o_rdata2 : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Port 2 read value

    i_waddr : in std_logic_vector(4 downto 0);              -- Writing value
    i_wena : in std_logic;                                  -- Writing is enabled
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Writing value

    i_dport_addr : in std_logic_vector(4 downto 0);         -- Debug port address
    i_dport_ena : in std_logic;                             -- Debug port is enabled
    i_dport_write : in std_logic;                           -- Debug port write is enabled
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Debug port write value
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Debug port read value

    o_ra : out std_logic_vector(RISCV_ARCH-1 downto 0)      -- Return address for branch predictor
  );
end; 
 
architecture arch_RegIntBank of RegIntBank is

  type MemoryType is array (0 to Reg_Total-1) 
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
    elsif i_wena = '1'  then
        if i_waddr /= "00000" then
            v.mem(conv_integer(i_waddr)) := i_wdata;
        end if;
    end if;

    if i_nrst = '0' then
        v.mem(Reg_Zero) := (others => '0');
        v.mem(1 to Reg_Total-1) := (others => X"00000000FEEDFACE");
    end if;

    rin <= v;
  end process;

  o_rdata1 <= r.mem(conv_integer(i_radr1));
  o_rdata2 <= r.mem(conv_integer(i_radr2));
  o_dport_rdata <= r.mem(conv_integer(i_dport_addr));
  o_ra <= r.mem(Reg_ra);

  -- registers:
  regs : process(i_clk)
  begin 
     if rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
