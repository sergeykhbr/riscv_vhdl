-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      System reset former.
-----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

--! @brief NoC global reset former.
--! @details This module produces output reset signal in a case if
--!          button 'Reset' was pushed or PLL isn't a 'lock' state.
--! param[in]  inSysReset Button generated signal
--! param[in]  inSysClk Clock from the PLL. Bus clock.
--! param[in]  inPllLock PLL status.
--! param[out] outReset Output reset signal with active 'High' (1 = reset).
entity reset_global is
  port (
    inSysReset  : in std_ulogic;
    inSysClk    : in std_ulogic;
    inPllLock   : in std_ulogic;
    outReset    : out std_ulogic
    );
end;

architecture arch_reset_global of reset_global is

  type reg_type is record
    PllLock : std_logic_vector(4 downto 0);
    Reset   : std_ulogic;
  end record;


  signal r : reg_type;
  signal wSysReset : std_ulogic;

begin

  wSysReset <= inSysReset or not inPllLock;


  proc_rst : process (inSysClk, wSysReset) begin
    if wSysReset = '1' then
      r.PllLock <= (others => '0');
      r.Reset <= '0';
    elsif rising_edge(inSysClk) then 
      r.PllLock <= r.PllLock(3 downto 0) & '1';
      r.Reset <= r.PllLock(4) and r.PllLock(3) and r.PllLock(2);
    end if;
  end process;

  outReset <= not r.Reset;
  
end;  
