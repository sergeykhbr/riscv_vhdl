-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Matrix correlator 4096 samples length
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity MatrixCorrelator is
generic (
    sys : integer := GEN_SYSTEM_GPSCA
);
port (
    i_nrst          : in std_logic;
    i_clk           : in std_logic;
    i_ena           : in std_logic;
    i_re_im         : in std_logic;
    i_prn           : in std_logic_vector(fse_section_num(sys)*1024-1 downto 0);
    i_I             : in std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
    i_Q             : in std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
    o_I             : out std_logic_vector(14 downto 0);
    o_Q             : out std_logic_vector(14 downto 0);
    o_rdy           : out std_logic
);
end;

architecture rtl of MatrixCorrelator is

type re_type is array (0 to 1) of std_logic_vector(14 downto 0);

type regtype is record
  ena      : std_logic_vector(2 downto 0);
  rdy      : std_logic;
  re_im    : std_logic_vector(2 downto 0);
  re       : re_type;
  im       : std_logic_vector(14 downto 0);
end record;

signal r, rin : regtype;
signal wbLvl10 : std_logic_vector(13*4-1 downto 0);

component Matrix1024 is
port (
    i_nrst  : in std_logic;
    i_clk   : in std_logic;
    i_ena   : in std_logic;
    i_re_im : in std_logic;
    i_prn   : in std_logic_vector(1024-1 downto 0);
    i_I     : in std_logic_vector(2*1024-1 downto 0);
    i_Q     : in std_logic_vector(2*1024-1 downto 0);
    o_sum   : out std_logic_vector(12 downto 0)
);
end component;


begin

  stx : for i in 0 to fse_section_num(sys)-1 generate
    l1 : Matrix1024 port map
    (
      i_nrst,
      i_clk,
      i_ena,
      i_re_im,
      i_prn(1024*(i+1)-1 downto 1024*i),
      i_I(2*1024*(i+1)-1 downto 2*1024*i),
      i_Q(2*1024*(i+1)-1 downto 2*1024*i),
      wbLvl10(13*(i+1)-1 downto 13*i)
    );
  end generate;



  comb : process (r, i_nrst, i_ena, i_re_im, wbLvl10)
  variable v : regtype;
  variable wbLvl11 : std_logic_vector(14*2-1 downto 0);
  
  begin
    
    --v := r;
    v.ena := r.ena(1 downto 0) & i_ena;
    v.re_im := r.re_im(1 downto 0) & (i_re_im and i_ena);
    

    if fse_section_num(sys) = 1 then
      wbLvl11(13 downto 0) := (wbLvl10(12) & wbLvl10(12 downto 0));
      wbLvl11(27 downto 14) := (others => '0');
    else 
      wbLvl11(13 downto 0) := (wbLvl10(12) & wbLvl10(12 downto 0)) + (wbLvl10(25) & wbLvl10(25 downto 13));
      wbLvl11(27 downto 14) := (wbLvl10(38) & wbLvl10(38 downto 26)) + (wbLvl10(51) & wbLvl10(51 downto 39));
    end if;


    if r.ena(2) = '1' then
      if r.re_im(2) = '0' then
        v.re(0) := (wbLvl11(13) & wbLvl11(13 downto 0)) + (wbLvl11(27) & wbLvl11(27 downto 14));
      else
        v.im := (wbLvl11(13) & wbLvl11(13 downto 0)) + (wbLvl11(27) & wbLvl11(27 downto 14));
      end if;
    end if;

    v.re(1) := r.re(0);
    v.rdy := r.re_im(2);
    
    
    if i_nrst = '0' then
      v.ena := (others => '0');
      v.rdy := '0';
      v.re_im := (others => '0');
      v.re := (others => (others => '0'));
      v.im  := (others => '0');
    end if;

    rin <= v;
  end process;
  

  o_I <= r.re(1);
  o_Q <= r.im;
  o_rdy <= r.rdy;


  regs : process(i_clk)
  begin 
    if rising_edge(i_clk) then 
      r <= rin;
    end if;
  end process;

end; 

