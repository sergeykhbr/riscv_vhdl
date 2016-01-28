-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Milliseconds samples accumulator.
--! @details  	Coherent/Non-coherent accumulator of the input samples.
--!            Input signal should be hold at least 2 clocks for re/im
--!            parts switching.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library techmap;
use techmap.types_mem.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity Accumulator is
generic (
    generic_tech   : integer := 0;
    generic_sys    : integer := GEN_SYSTEM_GPSCA
);
port (
    i : in accm_in_type;
    o : out accm_out_type
);
end;

architecture rtl of Accumulator is

type comp_acc_type is array (0 to 2) of std_logic_vector(15 downto 0);
type cnt_type is array (0 to 5) of std_logic_vector(21 downto 0);
type rd_data_type is array (0 to 1) of std_logic_vector(31 downto 0);
type regtype is record
  ena         : std_logic_vector(4 downto 0);
  rdy         : std_logic_vector(5 downto 0);
  cnt         : cnt_type;
  I           : std_logic_vector(15 downto 0);
  Q           : std_logic_vector(15 downto 0);
  amp         : std_logic_vector(31 downto 0);
  amp_z       : std_logic_vector(31 downto 0);
  accI        : comp_acc_type;
  accQ        : comp_acc_type;
  accAmp      : std_logic_vector(31 downto 0);
  latchAmp    : std_logic_vector(31 downto 0);
  latchInd    : std_logic_vector(11 downto 0);
  latchNoise  : std_logic_vector(31 downto 0);
  latchDop    : std_logic_vector(31 downto 0);
  wr_ena      : std_logic;
  wr_adr      : std_logic_vector(11 downto 0);
  wr_data     : std_logic_vector(31 downto 0);
  rd_adr      : std_logic_vector(11 downto 0);
  rd_data     : rd_data_type;
  tail        : std_logic;
end record;

type io_ram_type is record
  i_clk       : std_logic;
  i_address   : std_logic_vector(11 downto 0);
  i_wr_ena    : std_logic;
  i_data      : std_logic_vector(31 downto 0);
  o_data      : std_logic_vector(31 downto 0);
end record;


function sqrt16( i : in  std_logic_vector(15 downto 0);
                 q : in  std_logic_vector(15 downto 0))
                 return std_logic_vector is
  variable absI : std_logic_vector(15 downto 0);
  variable absQ : std_logic_vector(15 downto 0);
  variable absDif : std_logic_vector(16 downto 0);
  variable sum : std_logic_vector(16 downto 0);
  variable ret : std_logic_vector(31 downto 0);
begin

  if i(15) = '0' then absI := i;
  else                absI := 0 - i; end if;

  if q(15) = '0' then absQ := q;
  else                absQ := 0 - q; end if;

  absDif := (absI(15) & absI) - (absQ(15) & absQ);
 
  if (absDif(16) = '0') then 
    sum := (absI(15) & absI) + (absQ(15)&absQ(15)&absQ(15 downto 1));
  else
    sum := (absQ(15) & absQ) + (absI(15)&absI(15)&absI(15 downto 1)); 
  end if;

  ret(16 downto 0) := sum;
  ret(31 downto 17) := (others => sum(16));
  return ret;
end function sqrt16;

signal r, rin : regtype;
signal io_ram : io_ram_type;

begin

  comb : process (r, i, io_ram.o_data)
  variable v : regtype;
  variable wRdy : std_logic;
  variable wMsEnd : std_logic;
  variable wbMsCnt : std_logic_vector(9 downto 0);
  variable iMsCnt_z : integer range 0 to 1023;
  variable wbRdData : std_logic_vector(31 downto 0);
  variable wbRamAddress : std_logic_vector(11 downto 0);
  variable wbAmpDif : std_logic_vector(31 downto 0);
  begin

    v := r;
    wRdy := '0';
    wbMsCnt := r.cnt(0)(21 downto 12);
    iMsCnt_z := conv_integer(r.cnt(1)(21 downto 12));
    
    wMsEnd := '0';
    if fse_section_num(generic_sys) = 1 then 
      if r.cnt(0)(9 downto 0) = "1111111111" then
        wMsEnd := '1';
      end if;
    else
      if r.cnt(0)(11 downto 0) = X"FFF" then
        wMsEnd := '1';
      end if;
    end if;

    if i.new_cycle = '1' then v.ena := (others => '0');
    else  v.ena := r.ena(3 downto 0) & (i.ena and not r.tail) ; end if;
    
    if i.new_cycle = '1' then 
      v.cnt(0) := (others => '0');
    elsif i.ena = '1' then 
      if wMsEnd = '1' then
        v.cnt(0)(21 downto 12) := r.cnt(0)(21 downto 12) + 1; 
        v.cnt(0)(11 downto 0) := (others => '0');
      else
        v.cnt(0) := r.cnt(0) + 1; 
      end if;
    end if;

    if i.ena = '1' then 
      if wbMsCnt = i.ms_total and wMsEnd = '1' then 
        wRdy := '1';
      end if;

      v.I := i.I;
      v.Q := i.Q;
    end if;

    if i.new_cycle = '1' then v.tail := '0';
    elsif wRdy = '1'     then v.tail := '1'; end if;

    if iMsCnt_z = 0 or iMsCnt_z = 1 then wbRdData := (others => '0');
    else                                 wbRdData := io_ram.o_data; end if;

    v.rdy := r.rdy(4 downto 0) & wRdy;
    v.cnt(1) := r.cnt(0);
    v.cnt(2) := r.cnt(1);
    v.cnt(3) := r.cnt(2);
    v.cnt(4) := r.cnt(3);
    v.cnt(5) := r.cnt(4);

    v.rd_data(1) := wbRdData;

    if r.ena(0) = '1' then
      if i.coh_ena = '1' then
        v.accI(0) := wbRdData(31 downto 16) + r.I;
        v.accQ(0) := wbRdData(15 downto 0) + r.Q;
      else
        v.accI(0) := r.I;
        v.accQ(0) := r.Q;
      end if;
    end if;

    v.accI(1) := r.accI(0);
    v.accI(2) := r.accI(1);

    v.accQ(1) := r.accQ(0);
    v.accQ(2) := r.accQ(1);


    if r.ena(1) = '1' then
      v.amp := sqrt16(r.accI(0), r.accQ(0));
      if i.coh_ena = '0' then 
        v.amp_z := r.rd_data(1);
      else
        v.amp_z := (others => '0');
      end if;
    end if;

    if r.ena(2) = '1' then
      v.accAmp := r.amp_z + r.amp;
    end if;

    if r.ena(3) = '1' then
      if i.coh_ena = '1' then 
        v.wr_data := r.accI(2) & r.accQ(2);
      else
        v.wr_data := r.accAmp;
      end if;
    end if;

    if i.ena = '1' then wbRamAddress := r.cnt(0)(11 downto 0);
    else                wbRamAddress := r.cnt(5)(11 downto 0); end if;

    io_ram.i_clk <= i.clk;
    io_ram.i_address <= wbRamAddress;
    io_ram.i_data <= r.wr_data;
    io_ram.i_wr_ena <= r.ena(4);

    wbAmpDif := r.latchAmp - r.accAmp;
    if i.rst_max = '1' then 
      v.latchAmp := (others => '0');
      v.latchInd := (others => '0');
      v.latchDop := (others => '0');
      v.latchNoise := (others => '0');
    elsif r.ena(3)='1' and conv_integer(r.cnt(4)(21 downto 12)) /= 0 then
      if wbAmpDif(31) = '1' then
        v.latchAmp := r.accAmp;
        v.latchInd := r.cnt(4)(11 downto 0);
        v.latchDop := i.dopler;
        v.latchNoise := (others => '0');
      end if;
      if conv_integer(r.cnt(4)(11 downto 0)) = 0 then
        v.latchNoise := r.accAmp;
      else
        v.latchNoise := r.latchNoise + r.accAmp;
      end if;
    end if;

    -- Reset all
    if i.nrst = '0' then
      v.ena := (others =>'0');
      v.rdy := (others =>'0');
      v.cnt := (others => (others => '0'));
      v.I := (others =>'0');
      v.Q := (others =>'0');
      v.amp := (others =>'0');
      v.amp_z := (others =>'0');
      v.accI := (others => (others => '0'));
      v.accQ := (others => (others => '0'));
      v.accAmp := (others =>'0');
      v.latchAmp := (others =>'0');
      v.latchInd := (others =>'0');
      v.latchNoise := (others =>'0');
      v.latchDop := (others =>'0');
      v.wr_ena := '0';
      v.wr_adr := (others =>'0');
      v.wr_data := (others =>'0');
      v.rd_adr := (others =>'0');
      v.rd_data := (others => (others => '0'));
      v.tail := '0';
    end if;

    rin <= v;
    
  end process;

  ram32 : Ram32_tech generic map
  (
    generic_tech => generic_tech,
    generic_kWords => 4
  ) port map
  (
    io_ram.i_clk,
    io_ram.i_address,
    io_ram.i_wr_ena,
    io_ram.i_data,
    io_ram.o_data
  );


  o.rdy <= r.rdy(5);
  o.max <= r.latchAmp;
  o.maxind <= r.latchInd;
  o.noise <= r.latchNoise;
  o.dopler <= r.latchDop;

  reg : process(i.clk)
  begin 
    if rising_edge(i.clk) then 
      r <= rin;
    end if;
  end process;

end; 


