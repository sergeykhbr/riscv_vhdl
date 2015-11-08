--
------------------------------------------------------------------------------
-- "Output    Output      Phase     Duty      Pk-to-Pk        Phase"
-- "Clock    Freq (MHz) (degrees) Cycle (%) Jitter (ps)  Error (ps)"
------------------------------------------------------------------------------
-- CLK_OUT1____70.000
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

--library unisim;
--use unisim.vcomponents.all;

entity SysPLL_inferred is
port
 (-- Clock in ports
  CLK_IN1_P         : in     std_logic;
  CLK_IN1_N         : in     std_logic;
  -- Clock out ports
  CLK_OUT1          : out    std_logic;
  -- Status and control signals
  RESET             : in     std_logic;
  LOCKED            : out    std_logic
 );
end SysPLL_inferred;

architecture rtl of SysPLL_inferred is

  constant DELTA_TIME : time := 1250 ps;-- 0.25 periof of 200 MHz
  constant TH200MHz : integer := 4*200000000/2;
  
begin

  CLK_OUT1 <= CLK_IN1_P;
  LOCKED <= not RESET;

end rtl;
