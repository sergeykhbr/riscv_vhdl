-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Recorder and Player of the samples
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

entity Recorder is
generic (
    generic_tech   : integer := 0;
    generic_sys    : integer := GEN_SYSTEM_GPSCA
);
port (
    i : in rec_in_type;
    o : out rec_out_type
);
end;

architecture rtl of Recorder is

constant RECRAM_SIZE_KWORDS : integer := CFG_FSE_MSEC_MAX/2;
constant RECRAM_ADDR_WIDTH  : integer := 10+log2(RECRAM_SIZE_KWORDS);
constant REC_WORDS_TOTAL    : integer := RECRAM_SIZE_KWORDS*1024;


type play_cnt_type is array (0 to 3) of std_logic_vector(RECRAM_ADDR_WIDTH+4-1 downto 0);
type sample_cnt_wr_type is array (0 to 1) of std_logic_vector(RECRAM_ADDR_WIDTH+4-1 downto 0);

type regtype is record
  wr_word       : std_logic_vector(31 downto 0);
  rd_adr        : std_logic_vector(RECRAM_ADDR_WIDTH-1 downto 0);
  rd_word       : std_logic_vector(31 downto 0);
  rec_end       : std_logic;
  play_cnt      : play_cnt_type;
  play_ena      : std_logic_vector(2 downto 0);
  sample_cnt_wr : sample_cnt_wr_type;
  rd_sample     : std_logic_vector(3 downto 0);
end record;

type io_ram_type is record
  i_clk       : std_logic;
  i_address   : std_logic_vector(RECRAM_ADDR_WIDTH-1 downto 0);
  i_wr_ena    : std_logic;
  i_data      : std_logic_vector(31 downto 0);
  o_data      : std_logic_vector(31 downto 0);
end record;


signal r, rin : regtype;
signal io_ram : io_ram_type;

begin

  comb : process (r, i, io_ram.o_data)
  variable v : regtype;
  variable wbAdrWr : std_logic_vector(RECRAM_ADDR_WIDTH-1 downto 0);
  variable wbAdrRd : std_logic_vector(RECRAM_ADDR_WIDTH-1 downto 0);
  variable wRecEnd : std_logic;
  variable wWrEna : std_logic;
  variable iSampleInd : integer range 0 to 7;
  begin

    v := r;
    
    wbAdrWr := r.sample_cnt_wr(1)(RECRAM_ADDR_WIDTH+3-1 downto 3);
    wbAdrRd := r.play_cnt(0)(RECRAM_ADDR_WIDTH+4-1 downto 4);
    if i.test_mode = '0' and conv_integer(r.sample_cnt_wr(1)(RECRAM_ADDR_WIDTH+3 downto 3)) = REC_WORDS_TOTAL then
      wRecEnd := '1';
    elsif i.test_mode = '1' and conv_integer(r.sample_cnt_wr(1)(RECRAM_ADDR_WIDTH+3 downto 3)) = 1024 then
      wRecEnd := '1';
    else
      wRecEnd := '0';
    end if;

    if i.rec_ena = '0' then
      v.sample_cnt_wr(0) := (others => '0');
    elsif i.adc_valid='1' and i.rec_ena='1' and wRecEnd='0' then
      v.sample_cnt_wr(0) := r.sample_cnt_wr(0) + 1;
      v.wr_word := r.wr_word(27 downto 0) & i.I & i.Q;
    end if;
    v.sample_cnt_wr(1) := r.sample_cnt_wr(0);
    v.rec_end := wRecEnd;

    
    if r.sample_cnt_wr(1)(2 downto 0) = "111" 
      and r.sample_cnt_wr(0)(2 downto 0) = "000" then
        wWrEna := '1';
    else
        wWrEna := '0';
    end if;


    -- playing samples:
    if i.play_ena = '0' then
      v.play_cnt(0) := (others => '0');
    else
      if generic_sys = GEN_SYSTEM_GALE1 and r.play_cnt(0)(12 downto 0) = "1111111111111" then
        v.play_cnt(0)(RECRAM_ADDR_WIDTH+4-1 downto 13) := r.play_cnt(0)(RECRAM_ADDR_WIDTH+4-1 downto 13) + 4;
        v.play_cnt(0)(12 downto 0) := (others => '0');
      else
        v.play_cnt(0) := r.play_cnt(0) + 1;
      end if;
    end if;
    v.play_cnt(1) := r.play_cnt(0);
    v.play_cnt(2) := r.play_cnt(1);
    v.play_cnt(3) := r.play_cnt(2);

    v.play_ena := r.play_ena(1 downto 0) & i.play_ena;


    io_ram.i_clk <= i.clk;
    if i.play_ena = '1' then io_ram.i_address <= wbAdrRd;
    else                     io_ram.i_address <= wbAdrWr; end if;
    io_ram.i_data <= r.wr_word;
    io_ram.i_wr_ena <= wWrEna;


    if r.play_ena(0) = '1' then  v.rd_word := io_ram.o_data;
    else                         v.rd_word := (others => '0'); end if;

    iSampleInd := conv_integer(r.play_cnt(2)(3 downto 1));
    case iSampleInd is
      when 0 => v.rd_sample := r.rd_word(31 downto 28);
      when 1 => v.rd_sample := r.rd_word(27 downto 24);
      when 2 => v.rd_sample := r.rd_word(23 downto 20);
      when 3 => v.rd_sample := r.rd_word(19 downto 16);
      when 4 => v.rd_sample := r.rd_word(15 downto 12);
      when 5 => v.rd_sample := r.rd_word(11 downto 8);
      when 6 => v.rd_sample := r.rd_word(7 downto 4);
      when 7 => v.rd_sample := r.rd_word(3 downto 0);
    end case;


    -- Reset all
    if i.nrst = '0' then
      v.rec_end := '0';
      v.play_cnt := (others => (others => '0'));
      v.play_ena := (others =>'0');
      v.sample_cnt_wr := (others => (others => '0'));
      v.rd_adr := (others =>'0');
      v.rd_word := (others =>'0');
      v.wr_word := (others =>'0');
      v.rd_sample := (others =>'0');
    end if;

    rin <= v;
    
  end process;

  ram32 : Ram32_tech generic map
  (
    generic_tech => generic_tech,
    generic_kWords => RECRAM_SIZE_KWORDS
  ) port map
  (
    io_ram.i_clk,
    io_ram.i_address,
    io_ram.i_wr_ena,
    io_ram.i_data,
    io_ram.o_data
  );


  o.I <= r.rd_sample(3 downto 2);
  o.Q <= r.rd_sample(1 downto 0);
  o.rdy <= not r.play_cnt(3)(0) and r.play_ena(2);
  o.rec_end <= r.rec_end;

  reg : process(i.clk)
  begin 
    if rising_edge(i.clk) then 
      r <= rin;
    end if;
  end process;

end; 


