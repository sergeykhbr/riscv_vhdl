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
    tech    : integer range 0 to NTECH := 0; --! PLL implementation selector
    rf_frontend_ena : boolean := false
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
    --! Input clock from the external oscillator (default 200 MHz)
    i_clk_tcxo        : in     std_logic;
    --! External ADC clock
    i_clk_adc         : in     std_logic;
    --! System Bus clock 100MHz/40MHz (Virtex6/Spartan6)
    o_clk_bus         : out    std_logic;
    --! ADC simulation clock = 26MHz (default).
    o_clk_adc         : out    std_logic;
    --! PLL locked status.
    o_locked          : out    std_logic;
    -- DDR3 specific signals
    o_clk400_buf   : out std_logic;
    o_clk200_buf   : out std_logic;
    o_clk400_unbuf : out std_logic;
    -- Phase Shift interface
    i_PSEN         : in std_logic;
    i_PSINCDEC     : in std_logic;
    o_PSDONE       : out std_logic 
  );
end SysPLL_tech;

--! SysPLL_tech architecture declaration.
architecture rtl of SysPLL_tech is
  --! Clock bus (default 60 MHz).
  signal pll_clk_bus : std_logic;
  --! Clock bus Fsys / 4 (unbuffered).
  signal adc_clk_unbuf : std_logic;

  component SysPLL_inferred is
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1    : out    std_logic;
    CLK_OUT2    : out    std_logic;
    RESET       : in     std_logic;
    LOCKED      : out    std_logic );
  end component;

  component SysPLL_v6 is 
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1	: out    std_logic;
    CLK_OUT2    : out    std_logic;
    RESET	: in std_logic;
    LOCKED	: out std_logic );
  end component;

  component SysPLL_k7 is
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1    : out    std_logic;
    CLK_OUT2    : out    std_logic;
    RESET     : in     std_logic;
    LOCKED    : out    std_logic );
  end component;

  component SysPLL_micron180 is
  port (
    CLK_IN      : in     std_logic;
    CLK_OUT1    : out    std_logic;
    CLK_OUT2    : out    std_logic;
    RESET       : in     std_logic;
    LOCKED      : out    std_logic );
  end component;

begin

   xv6 : if tech = virtex6 generate
     pll0 : SysPLL_v6 port map (i_clk_tcxo, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;

   xv7 : if tech = kintex7 generate
     pll0 : SysPLL_k7 port map (i_clk_tcxo, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;
   
   inf : if tech = inferred generate
     pll0 : SysPLL_inferred port map (i_clk_tcxo, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;
   
   m180 : if tech = micron180 generate
     pll0 : SysPLL_micron180 port map (i_clk_tcxo, pll_clk_bus, adc_clk_unbuf, i_reset, o_locked);
   end generate;


  o_clk_bus <= pll_clk_bus;

  -- DDR3 specific signals (not implemented yet)
  o_clk400_buf   <= '0';
  o_clk200_buf   <= '0';
  o_clk400_unbuf <= '0';
  o_PSDONE       <= '0';

  ------------------------------------
  -- Clock mux2:
  --     pass input ADC clock directly to output when i_int_clkrf=0
  --     otherwise pass sim_adc = bus/4 (for the self-test purposes without RF)
  buf1 : bufgmux_tech generic map
  (
    tech => tech,
    rf_frontend_ena => rf_frontend_ena
  )port map 
  (
    O  => o_clk_adc,
    I1 => i_clk_adc,
    I2 => adc_clk_unbuf,
    S  => i_int_clkrf
  );

end;
