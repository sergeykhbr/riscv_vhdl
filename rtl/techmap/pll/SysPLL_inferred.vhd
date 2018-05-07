-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @details  	PLL instance for the behaviour simulation
--!
--! "Output    Output      Phase     Duty      Pk-to-Pk        Phase"
--! "Clock    Freq (MHz) (degrees) Cycle (%) Jitter (ps)  Error (ps)"
--!
--! CLK_OUT1____70.000
-----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;

--library unisim;
--use unisim.vcomponents.all;

entity SysPLL_inferred is
port
 (-- Clock in ports
  CLK_IN            : in     std_logic;
  -- Clock out ports
  CLK_OUT1          : out    std_logic;
  -- Status and control signals
  RESET             : in     std_logic;
  LOCKED            : out    std_logic
 );
end SysPLL_inferred;

architecture rtl of SysPLL_inferred is
 
begin

  CLK_OUT1 <= CLK_IN;
  LOCKED <= not RESET;

end rtl;
