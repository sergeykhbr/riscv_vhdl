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
  CLK_IN1_P         : in     std_logic;
  CLK_IN1_N         : in     std_logic;
  -- Clock out ports
  CLK_OUT1          : out    std_logic;
  CLK_OUT2          : out    std_logic;
  -- Status and control signals
  RESET             : in     std_logic;
  LOCKED            : out    std_logic
 );
end SysPLL_inferred;

architecture rtl of SysPLL_inferred is
 
  signal divider : std_logic_vector(1 downto 0); 
begin

  CLK_OUT1 <= CLK_IN1_P;
  CLK_OUT2 <= divider(1);
  LOCKED <= not RESET;

  regs : process(CLK_IN1_P, RESET) 
  begin 
    if RESET = '1' then
       divider <= (others => '0');
    elsif rising_edge(CLK_IN1_P) then 
       divider <= divider + 1;
    end if;
  end process;

end rtl;
