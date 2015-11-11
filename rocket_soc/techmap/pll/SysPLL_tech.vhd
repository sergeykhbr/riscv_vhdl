-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief  	  FPGA PLL selector
--! @details  	This unit allows to form different pll for the different
--!            targets
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

library techmap;
use techmap.gencomp.all;
use techmap.syspll.all;
use techmap.bufgmux.all;


entity SysPLL_tech is
  generic (
    tech    : integer range 0 to NTECH := 0
  );
  port
  (
    i_reset           : in     std_logic;
    i_int_clkrf       : in     std_logic;-- disable external ADC/enable internal ADC simulation
    i_clkp            : in     std_logic;
    i_clkn            : in     std_logic;
    i_clk_adc         : in     std_logic;
    o_clk_bus         : out    std_logic;-- Bus interface clock = 100MHz/40MHz (Virtex6/Spartan6)
    o_clk_adc         : out    std_logic;-- ADC simulation clock = 26MHz (default)
    o_locked          : out    std_logic
  );
end SysPLL_tech;

architecture rtl of SysPLL_tech is
  signal pll_clk_bus : std_logic;
  signal clk_divider : std_logic_vector(1 downto 0);
begin

   xv6 : if tech = virtex6 generate
     pll0 : SysPLL_v6 port map (i_clkp, i_clkn, pll_clk_bus, i_reset, o_locked);
   end generate;

   xv7 : if tech = kintex7 generate
     pll0 : SysPLL_k7 port map (i_clkp, i_clkn, pll_clk_bus, i_reset, o_locked);
   end generate;
   
   inf : if tech = inferred generate
     pll0 : SysPLL_inferred port map (i_clkp, i_clkn, pll_clk_bus, i_reset, o_locked);
   end generate;
   
   m180 : if tech = micron180 generate
     pll0 : SysPLL_micron180 port map (i_clkp, i_clkn, pll_clk_bus, i_reset, o_locked);
   end generate;

  -- registers:
  regs : process(pll_clk_bus, i_reset)
  begin 
    if rising_edge(pll_clk_bus) then 
      clk_divider <= clk_divider + 1; 
    end if; 
    if i_reset = '1' then clk_divider <= (others => '0'); end if;
  end process;


  o_clk_bus <= pll_clk_bus;

  ------------------------------------
  -- Clock mux2:
  --     pass input ADC clock directly to output when i_int_clkrf=0
  --     otherwise pass sim_adc = bus/4 (for the self-test purposes without RF)
  --buf1 : bufgmux_tech generic map
  --(
  --  tech => tech
  --)port map 
  --(
  --  O  => o_clk_adc,
  --  I1 => i_clk_adc,
  --  I2 => clk_divider(1),
  --  S  => i_int_clkrf
  --);

end;
