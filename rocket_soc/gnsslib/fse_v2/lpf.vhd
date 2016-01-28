-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Low-pass filter implementation
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_fse_v2.all;

entity LpFilter is
generic (
    generic_rate    : integer := 12
);
port (
    i : in lpf_in_type;
    o : out lpf_out_type
);
end;

architecture rtl of LpFilter is

constant ACC_WIDTH : integer := 16 + 1 + generic_rate;

type regtype is record
  amp    : std_logic_vector(16 downto 0);
  acc    : std_logic_vector(ACC_WIDTH-1 downto 0);
  ena    : std_logic_vector(1 downto 0);
end record;


signal r, rin : regtype;

begin

  comb : process (r, i)
  variable v : regtype;
  variable wbAbsI, wbAbsQ : std_logic_vector(15 downto 0);
  variable wbAbsDif : std_logic_vector(16 downto 0);
  variable wbAmpDif : std_logic_vector(ACC_WIDTH-1 downto 0); 
  begin

    v := r;
    
    v.ena := r.ena(0) & i.ena;
    
    if i.I(15) = '0' then wbAbsI := i.I; 
    else                  wbAbsI := 0 - i.I; end if;
    if i.Q(15) = '0' then wbAbsQ := i.Q; 
    else                  wbAbsQ := 0 - i.Q; end if;

    wbAbsDif := ('0'&wbAbsI) - ('0'&wbAbsQ);
    if i.ena = '1' then
      if wbAbsDif(16) = '0' then v.amp := ('0'&wbAbsI)+("00"&wbAbsQ(15 downto 1));
      else                       v.amp := ('0'&wbAbsQ)+("00"&wbAbsI(15 downto 1)); end if;
    end if;
    
    wbAmpDif(16 downto 0) := r.amp - (r.acc(ACC_WIDTH-1 downto generic_rate));
    wbAmpDif(ACC_WIDTH-1 downto 17) := (others => wbAmpDif(16));
    if r.ena(0) = '1' then
      v.acc := r.acc + wbAmpDif; 
    end if;

    -- Reset all
    if i.nrst = '0' then
      v.amp := (others => '0');
      v.acc := (others => '0');
      v.ena := (others => '0');
    end if;

    rin <= v;
    
  end process;

  o.rdy <= r.ena(1); 
  -- Warning! 
  --          LPF forms 0.5*filter value, because it used in requant module only
  --          as average value.
  o.flt <= r.acc(ACC_WIDTH-1 downto generic_rate + 1);

  reg : process(i.clk)
  begin 
    if rising_edge(i.clk) then 
      r <= rin;
    end if;
  end process;

end; 


