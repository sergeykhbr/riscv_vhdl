-----------------------------------------------------------------------------
-- Package:     fse_v2
-- File:        reclk.vhd
-- Author:      Sergey Khabarov - sergeykhbr@gmail.com
-- Description:	Reclocking from ADC clock domain into FSE clock domain
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_sync.all;

entity Reclk is
port (
    i : in reclk_in_type;
    o : out reclk_out_type
);
end;

architecture rtl of Reclk is

type regtype is record
  I        : std_logic_vector(1 downto 0);
  Q        : std_logic_vector(1 downto 0);
  ms_pulse : std_logic;
  pps      : std_logic;
  clk_adc  : std_logic_vector(1 downto 0);
end record;

signal r, rin : regtype;

begin

  comb : process (r, i)
  variable v : regtype;
  begin

    v := r;

    v.clk_adc := r.clk_adc(0) & i.clk_adc;

    v.I := i.I;
    v.Q := i.Q;
    v.ms_pulse := i.ms_pulse;
    v.pps := i.pps;

    -- Reset all
    if i.nrst = '0' then
      v.I := (others => '0');
      v.Q := (others => '0');
      v.ms_pulse := '0';
      v.pps := '0';
      v.clk_adc := (others => '0');
    end if;
 
    rin <= v;
  end process;

  o.I <= r.I;
  o.Q <= r.Q;
  o.ms_pulse <= r.ms_pulse;
  o.pps <= r.pps;
  o.adc_valid <= not r.clk_adc(0) and r.clk_adc(1);

  regs : process(i.clk_fse)
  begin 
    if rising_edge(i.clk_fse) then 
      r <= rin;
    end if;
  end process;

end; 


