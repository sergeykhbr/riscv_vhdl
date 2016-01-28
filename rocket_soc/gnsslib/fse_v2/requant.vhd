-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Re-qunatizer to form 2-bits values.
--! @details  	Requant 16-bits value into sign/mag 2 bits value
--!            using LPF with T=(1.0/4096.0)
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_fse_v2.all;

entity Requant is
port (
    i : in rqnt_in_type;
    o : out rqnt_out_type
);
end;

architecture rtl of Requant is
       
type comp_delay_type is array (0 to 1) of std_logic_vector(15 downto 0);
type regtype is record
  I        : comp_delay_type;
  Q        : comp_delay_type;
  zeroI    : std_logic;
  zeroQ    : std_logic;
end record;

function map2MagOffset( th  : in  std_logic_vector(15 downto 0);
                        val : in  std_logic_vector(15 downto 0);
                        zero : in std_logic)
                        return std_logic_vector is
  variable difp : std_logic_vector(15 downto 0);
  variable difm : std_logic_vector(15 downto 0);
  variable ret : std_logic_vector(1 downto 0);
begin
  difp := val - th;
  difm := val + th;

  if conv_integer(val) = 0 and zero = '0' then ret := "10";
  elsif conv_integer(val) = 0             then ret := "01";
  elsif difp(15)='0' and val(15)='0' and conv_integer(difp)/= 0 then ret := "11";
  elsif val(15) = '0'                     then ret := "10";
  elsif difm(15)='1' and val(15)='1'      then ret := "00";
  else                                         ret := "01"; end if;
  return ret;
end function map2MagOffset;

signal r, rin : regtype;
signal i_lpf : lpf_in_type;
signal o_lpf : lpf_out_type;


begin

  comb : process (r, i, o_lpf)
  variable v : regtype;
  begin

    v := r;
    
    v.I(0) := i.I;
    v.I(1) := r.I(0);

    v.Q(0) := i.Q;
    v.Q(1) := r.Q(0);
    
    if o_lpf.rdy = '1' then 
      if conv_integer(r.I(1)) = 0 then v.zeroI := not r.zeroI; end if;
      if conv_integer(r.Q(1)) = 0 then v.zeroQ := not r.zeroQ; end if;
    end if;

    -- Reset all
    if i.nrst = '0' then
      v.I := (others => (others => '0'));
      v.Q := (others => (others => '0'));
      v.zeroI := '0';
      v.zeroQ := '0';
    end if;

    rin <= v;
    
  end process;

  i_lpf.nrst <= i.nrst;
  i_lpf.clk <= i.clk;
  i_lpf.ena <= i.ena;
  i_lpf.I <= i.I;
  i_lpf.Q <= i.Q;

  Lpf12 : LpFilter generic map
  (
    generic_rate => 12
  ) port map
  (
    i_lpf,
    o_lpf
  );


  o.rdy <= o_lpf.rdy;
  o.I <= map2MagOffset(o_lpf.flt, r.I(1), r.zeroI);
  o.Q <= map2MagOffset(o_lpf.flt, r.Q(1), r.zeroQ);

  reg : process(i.clk)
  begin 
    if rising_edge(i.clk) then 
      r <= rin;
    end if;
  end process;

end; 


