-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Converter ADC samples into samples with 4096 MHz bitrate
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity Decimator is
port (
    i : in decim_in_type;
    o : out decim_out_type
);
end;

architecture rtl of Decimator is


type regtype is record
  code_acc   : std_logic_vector(31 downto 0);
  acc_I      : std_logic_vector(15 downto 0);
  acc_Q      : std_logic_vector(15 downto 0);
  latch_I    : std_logic_vector(15 downto 0);
  latch_Q    : std_logic_vector(15 downto 0);
  rdy        : std_logic;
end record;

signal r, rin : regtype;

begin

  comb : process (r, i)
  variable v : regtype;
  variable wbI, wbQ : std_logic_vector(15 downto 0);
  variable wOverflow : std_logic;
  variable wbSum : std_logic_vector(32 downto 0);
  variable wbDif : std_logic_vector(32 downto 0);
  begin

    v := r;

    wbI(6 downto 0) := i.I;
    wbI(15 downto 7) := (others => i.I(6));

    wbQ(6 downto 0) := i.Q;
    wbQ(15 downto 7) := (others => i.Q(6));

    wbSum := ('0'&r.code_acc) + ('0'&i.code_nco);
    wbDif := wbSum - ('0'&i.code_nco_th);
    wOverflow := not wbDif(32);

    if i.ena = '1' then
      if wOverflow = '1' then
        v.code_acc := wbDif(31 downto 0);
        v.acc_I := (others => '0');
        v.acc_Q := (others => '0');
        v.latch_I := r.acc_I + wbI;
        v.latch_Q := r.acc_Q + wbQ;
      else
        v.code_acc := wbSum(31 downto 0);
        v.acc_I := r.acc_I + wbI;
        v.acc_Q := r.acc_Q + wbQ;
      end if;
    end if;

    v.rdy := i.ena and wOverflow;


    -- Reset all
    if i.nrst = '0' then
      v.rdy := '0';
      v.acc_I := (others =>'0');
      v.acc_Q := (others =>'0');
      v.latch_I := (others =>'0');
      v.latch_Q := (others =>'0');
      v.code_acc := (others =>'0');
    end if;
    
    rin <= v;
  end process; 


  o.rdy <= r.rdy;
  o.I <= r.latch_I;
  o.Q <= r.latch_Q;

  reg : process(i.clk)
  begin 
    if rising_edge(i.clk) then 
      r <= rin;
    end if;
  end process;

end; 


