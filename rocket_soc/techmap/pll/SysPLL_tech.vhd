-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Implementation of the SysPLL_tech entity
--! @details   This module file be included in all projects.
------------------------------------------------------------------------------

--! Standard library
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

--! "Virtual" components declarations library.
library techmap;
use techmap.gencomp.all;
use techmap.types_pll.all;
use techmap.types_buf.all;

--! @brief   SysPLL_tech entity declaration ("Virtual" PLL).
--! @details This module instantiates the certain PLL implementation
--!          depending generic technology argument.
entity SysPLL_tech is
  generic (
    tech    : integer range 0 to NTECH := 0 --! PLL implementation selector
  );
  port
  (
    --! Reset value. Active high.
    i_reset           : in     std_logic;
    --! @brief ADC source select
    --! @details
    --!    <table>
    --!    <tr><th> Value </th>
    --!        <th> Description </th></tr>
    --!    <tr> <td> 0 </td> 
    --!         <td>External ADC clock (Real RF front-end).</td></tr>
    --!    <tr> <td> 1 </td> 
    --!         <td>Disable external ADC/enable internal ADC simulation.</td></tr>
    --!    </table>
    i_int_clkrf       : in     std_logic;
    --! Differential clock input positive
    i_clkp            : in     std_logic;
    --! Differential clock input negative
    i_clkn            : in     std_logic;
    --! External ADC clock
    i_clk_adc         : in     std_logic;
    --! System Bus clock 100MHz/40MHz (Virtex6/Spartan6)
    o_clk_bus         : out    std_logic;
    --! ADC simulation clock = 26MHz (default).
    o_clk_adc         : out    std_logic;
    --! PLL locked status.
    o_locked          : out    std_logic
  );
end SysPLL_tech;

--! SysPLL_tech architecture declaration.
architecture rtl of SysPLL_tech is
  --! Clock bus (default 60 MHz).
  signal pll_clk_bus : std_logic;
  --! Clock bus Fsys / 4 (unbuffered).
  signal adc_clk_unbuf : std_logic;

begin

   xv6 : if tech = virtex6 generate
     pll0 : SysPLL_v6 port map (i_clkp, i_clkn, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;

   xv7 : if tech = kintex7 generate
     pll0 : SysPLL_k7 port map (i_clkp, i_clkn, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;
   
   inf : if tech = inferred generate
     pll0 : SysPLL_inferred port map (i_clkp, i_clkn, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;
   
   m180 : if tech = micron180 generate
     pll0 : SysPLL_micron180 port map (i_clkp, i_clkn, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;



  o_clk_bus <= pll_clk_bus;

  ------------------------------------
  -- Clock mux2:
  --     pass input ADC clock directly to output when i_int_clkrf=0
  --     otherwise pass sim_adc = bus/4 (for the self-test purposes without RF)
  buf1 : bufgmux_tech generic map
  (
    tech => tech
  )port map 
  (
    O  => o_clk_adc,
    I1 => i_clk_adc,
    I2 => adc_clk_unbuf,
    S  => i_int_clkrf
  );

end;
