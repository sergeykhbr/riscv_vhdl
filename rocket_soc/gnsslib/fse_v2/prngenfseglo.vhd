-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     GLO-CA code generator
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity PrnGeneratorFseGlo is
port (
    i_nrst     : in std_logic;
    i_clk      : in std_logic;
    i_ena      : in std_logic;
    o_prn : out std_logic_vector(1024*fse_section_num(GEN_SYSTEM_GLOCA)-1 downto 0)
);
end;

architecture rtl of PrnGeneratorFseGlo is

type regtype is record
  reg_g1 : std_logic_vector(8 downto 0);
  
  accnco : integer range 0 to 513;
  bitcnt0 : std_logic_vector(9 downto 0);
  bitcnt1 : std_logic_vector(9 downto 0);
  shft : std_logic_vector(63 downto 0);
  shft_cnt : std_logic_vector(11 downto 0);
  shft_cnt_z : std_logic_vector(11 downto 0);
  prn    : std_logic_vector(1024*fse_section_num(GEN_SYSTEM_GLOCA)-1 downto 0);
  ena    : std_logic_vector(1 downto 0);
end record;


signal r, rin : regtype;

begin

  comb : process (r, i_nrst, i_ena)
  variable v : regtype;
  variable wHoldBit : std_logic;
  variable adr : integer;
  begin

    v := r;
    wHoldBit := '0';
    if r.accnco = 511 then 
        wHoldBit := '1';
    end if;
    
    if i_ena = '1' then
      v.reg_g1 := (others => '1');
      v.ena(0) := '1';
      v.bitcnt0 := (others => '0');
      v.accnco := 0;
      v.shft_cnt := (others => '0');
    elsif r.ena(0) = '0' then
      v.reg_g1 := (others => '1');
      v.bitcnt0 := (others => '0');
      v.accnco := 0;
    elsif r.bitcnt0 = conv_std_logic_vector(1022,10) then
      v.ena(0) := '0';
      v.shft_cnt := (others => '0');
    end if;
    v.ena(1) := r.ena(0);

    if r.ena(0) = '1' then
      v.accnco := r.accnco + 1;
      if r.accnco = 512 then
        v.accnco := 0;
      end if;
    end if;

    if r.ena(0) = '1' and wHoldBit= '0' then
      v.bitcnt0 := r.bitcnt0 + 1;
    end if;
    v.bitcnt1 := r.bitcnt0;
    
    if r.bitcnt0(0) = '1' and wHoldBit = '0' then
      v.reg_g1 := r.reg_g1(7 downto 0) & (r.reg_g1(4) xor r.reg_g1(8));
    end if;

    if r.ena(0) = '1' then
      v.shft := (r.reg_g1(4) xor r.reg_g1(8)) & r.shft(63 downto 1);
      v.shft_cnt := r.shft_cnt + 1;
    end if;
    v.shft_cnt_z := r.shft_cnt;
    
    if r.shft_cnt_z(5 downto 0) = "111111" then
      adr := conv_integer(r.shft_cnt_z(11 downto 6) & "000000");
      v.prn(adr+63 downto adr) := r.shft;
    end if;
    
    

    -- Reset all
    if i_nrst = '0' then
      v.reg_g1 := (others => '0');
      v.ena := (others => '0');
      v.accnco := 0;
      v.bitcnt0 := (others => '0');
      v.bitcnt1 := (others => '0');
      v.shft := (others => '0');
      v.shft_cnt := (others => '0');
      v.shft_cnt_z := (others => '0');
      v.prn := (others => '0');
    end if;

    rin <= v;
    
  end process;

  o_prn <= r.prn;

  reg : process(i_clk)
  begin 
    if rising_edge(i_clk) then 
      r <= rin;
    end if;
  end process;

end; 


