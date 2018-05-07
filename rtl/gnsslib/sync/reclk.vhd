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

entity Reclk is
port (
  nrst       : in std_logic;
  clk_bus    : in std_logic;
  clk_adc    : in std_logic;
  i_I        : in std_logic_vector(1 downto 0);
  i_Q        : in std_logic_vector(1 downto 0);
  i_ms_pulse : in std_logic;
  i_pps      : in std_logic;
  o_I        : out std_logic_vector(1 downto 0);
  o_Q        : out std_logic_vector(1 downto 0);
  o_ms_pulse : out std_logic;
  o_pps      : out std_logic;
  o_valid    : out std_logic
);
end;

architecture rtl of Reclk is

  type regadctype is record
      flag : std_logic;
      I        : std_logic_vector(1 downto 0);
      Q        : std_logic_vector(1 downto 0);
      ms_pulse : std_logic;
      pps      : std_logic;
  end record;


  type regtype is record
      flag     : std_logic;
      I        : std_logic_vector(1 downto 0);
      Q        : std_logic_vector(1 downto 0);
      ms_pulse : std_logic;
      pps      : std_logic;
      valid    : std_logic;
  end record;

  signal ra : regadctype;
  signal r, rin : regtype;

begin

  adcproc : process (clk_adc)
  begin
     if rising_edge(clk_adc) then
        if nrst = '0' then
           ra.flag <= '0';
           ra.I <= (others => '0');
           ra.Q <= (others => '0');
           ra.ms_pulse <= '0';
           ra.pps <= '0';
        else
           ra.flag <= not ra.flag;
           ra.I <= i_I;
           ra.Q <= i_Q;
           ra.ms_pulse <= i_ms_pulse;
           ra.pps <= i_pps;
        end if;
     end if;
  end process;

  comb : process (nrst, ra, r)
    variable v : regtype;
  begin

    v := r;
    v.valid := '0';
    v.ms_pulse := '0';
    v.pps := '0';

    if r.flag /= ra.flag then
       v.flag := ra.flag;
       v.valid := '1';
       v.I := ra.I;
       v.Q := ra.Q;
       v.ms_pulse := ra.ms_pulse;
       v.pps := ra.pps;
    end if;

    -- Reset all
    if nrst = '0' then
      v.flag := '0';
      v.I := (others => '0');
      v.Q := (others => '0');
      v.ms_pulse := '0';
      v.pps := '0';
    end if;
 
    rin <= v;
  end process;

  o_I <= r.I;
  o_Q <= r.Q;
  o_ms_pulse <= r.ms_pulse;
  o_pps <= r.pps;
  o_valid <= r.valid;

  regs : process(clk_bus)
  begin 
    if rising_edge(clk_bus) then 
      r <= rin;
    end if;
  end process;

end; 


