-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Frequency mixer implementation
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_fse_v2.all;

entity CarrierNco is
port (
    i : in carnco_in_type;
    o : out carnco_out_type
);
end;

architecture rtl of CarrierNco is

type phase_type is array (0 to 63) of integer range 0 to 16#FF#;
constant phase_value : phase_type := (
   0 => 16#70#,
   1 => 16#71#,
   2 => 16#71#,
   3 => 16#72#,
   4 => 16#63#,
   5 => 16#63#,
   6 => 16#64#,
   7 => 16#54#,
   8 => 16#55#,
   9 => 16#45#,
  10 => 16#46#,
  11 => 16#36#,
  12 => 16#36#,
  13 => 16#27#,
  14 => 16#17#,
  15 => 16#17#,
  16 => 16#07#,
  17 => 16#F7#,
  18 => 16#F7#,
  19 => 16#E7#,
  20 => 16#D6#,
  21 => 16#D6#,
  22 => 16#C6#,
  23 => 16#C5#,
  24 => 16#B5#,
  25 => 16#B4#,
  26 => 16#A4#,
  27 => 16#A3#,
  28 => 16#A3#,
  29 => 16#92#,
  30 => 16#91#,
  31 => 16#91#,
  32 => 16#90#,
  33 => 16#9F#,
  34 => 16#9F#,
  35 => 16#9E#,
  36 => 16#AD#,
  37 => 16#AD#,
  38 => 16#AC#,
  39 => 16#BC#,
  40 => 16#BB#,
  41 => 16#CB#,
  42 => 16#CA#,
  43 => 16#DA#,
  44 => 16#DA#,
  45 => 16#E9#,
  46 => 16#F9#,
  47 => 16#F9#,
  48 => 16#09#,
  49 => 16#19#,
  50 => 16#19#,
  51 => 16#29#,
  52 => 16#3A#,
  53 => 16#3A#,
  54 => 16#4A#,
  55 => 16#4B#,
  56 => 16#5B#,
  57 => 16#5C#,
  58 => 16#6C#,
  59 => 16#6D#,
  60 => 16#6D#,
  61 => 16#7E#,
  62 => 16#7F#,
  63 => 16#7F#,
  others => 0
);

function signed_mul3x4( a3 : in  std_logic_vector(2 downto 0);
                        b4 : in  std_logic_vector(3 downto 0))
                        return std_logic_vector is
  variable absa : std_logic_vector(2 downto 0);
  variable sum : std_logic_vector(5 downto 0);
  variable ret : std_logic_vector(5 downto 0);
begin
  if a3(2) = '1' then absa := 0 - a3;
  else                absa := a3; end if;


  if absa(1 downto 0) = "00"    then sum := "000000";
  elsif absa(1 downto 0) = "01" then sum := b4(3) & b4(3) & b4;
  elsif absa(1 downto 0) = "10" then sum := b4(3) & b4 & '0';
  else                               sum := (b4(3) & b4(3) & b4) + (b4(3) & b4 & '0'); end if;

  if a3(2) = '1' then ret := 0 - sum;
  else                ret := sum;
  end if;
  return ret;
end function signed_mul3x4;


type regtype is record
  I         : std_logic_vector(2 downto 0);
  Q         : std_logic_vector(2 downto 0);
  acc_nco   : std_logic_vector(37 downto 0);
  phs_cnt   : std_logic_vector(5 downto 0);
  ena       : std_logic_vector(1 downto 0);
  mulICos   : std_logic_vector(5 downto 0);
  mulQCos   : std_logic_vector(5 downto 0);
  mulISin   : std_logic_vector(5 downto 0);
  mulQSin   : std_logic_vector(5 downto 0);
end record;

signal r, rin : regtype;
signal i_divphase : dphs_in_type;
signal o_divphase : dphs_out_type;
signal resI : std_logic_vector(6 downto 0);
signal resQ : std_logic_vector(6 downto 0);

begin

  comb : process (r, i, o_divphase.phase_cnt)
  variable v : regtype;
  variable wbNco : std_logic_vector(38 downto 0);
  variable carrNcoSum : std_logic_vector(38 downto 0);
  variable carrNcoAccPlus : std_logic_vector(38 downto 0);
  variable carrNcoAccMinus : std_logic_vector(38 downto 0);
  variable cs : std_logic_vector(7 downto 0);
  variable c : std_logic_vector(3 downto 0);
  variable s : std_logic_vector(3 downto 0);
  begin

    v := r;

    wbNco(31 downto 0) := i.nco;
    wbNco(38 downto 32) := (others => i.nco(31));
    carrNcoSum := ('0'&r.acc_nco) - wbNco;
    carrNcoAccPlus  := carrNcoSum + ('0'&i.thresh&"000000");
    carrNcoAccMinus := carrNcoSum - ('0'&i.thresh&"000000");


    if i.ena = '1' then
      if carrNcoSum(38) = '1' then
        v.acc_nco := carrNcoAccPlus(37 downto 0);
      elsif carrNcoAccMinus(38) = '0' then
        v.acc_nco := carrNcoAccMinus(37 downto 0);
      else
        v.acc_nco := carrNcoSum(37 downto 0);
      end if;
    end if;

    v.ena := r.ena(0) & i.ena;

    i_divphase.carr_th <= i.thresh;
    i_divphase.carr_acc <= r.acc_nco;

    v.phs_cnt := o_divphase.phase_cnt;
    v.I := i.I;
    v.Q := i.Q;

    cs := conv_std_logic_vector(phase_value(conv_integer(r.phs_cnt)), 8);
    c := cs(7 downto 4);
    s := cs(3 downto 0);

    -- I/Q (2+1 bits) * Phase (3+1 bits) = 5+1 bits
    v.mulICos := signed_mul3x4(r.I, c);
    v.mulQSin := signed_mul3x4(r.Q, s);
    resI <= (r.mulICos(5)&r.mulICos) - (r.mulQSin(5)&r.mulQSin);

    v.mulISin := signed_mul3x4(r.I, s);
    v.mulQCos := signed_mul3x4(r.Q, c);
    resQ <= (r.mulISin(5)&r.mulISin) + (r.mulQCos(5)&r.mulQCos);

    -- Reset all
    if i.nrst = '0' then
      v.ena := (others =>'0');
      v.acc_nco := (others =>'0');
      v.phs_cnt := (others =>'0');
      v.I :=  (others =>'0');
      v.Q :=  (others =>'0');
      v.mulICos := (others =>'0');
      v.mulQCos := (others =>'0');
      v.mulISin := (others =>'0');
      v.mulQSin := (others =>'0');
    end if;

    rin <= v;
    
  end process;

  dphase : Divphase port map
  (
    i_divphase,
    o_divphase
  );


  o.I <= resI;
  o.Q <= resQ;
  o.rdy <= r.ena(1);

  reg : process(i.clk)
  begin 
    if rising_edge(i.clk) then 
      r <= rin;
    end if;
  end process;

end; 


