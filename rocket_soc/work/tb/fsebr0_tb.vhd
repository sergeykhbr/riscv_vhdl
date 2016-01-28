------------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
------------------------------------------------------------------------------
--! @details   --  Descrption:   Bridge to access from AHB to Fse registers
--                using reclocking pipes for read and write operations
------------------------------------------------------------------------------
--! @warning   --  Warning:      'BUS clock' should be equals or greater that 'FSE clock',
--                otherwise there maybe skips of the reading data from FSE
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;
use commonlib.types_util.all;
library rocketlib;
use rocketlib.types_nasti.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity AxiFseBridge_tb is
  constant INCR_TIME : time := 3571 ps;
  constant STRING_SIZE : integer := 610; -- string size = index of the last element
end AxiFseBridge_tb;

architecture behavior of AxiFseBridge_tb is


  -- input/output signals:
  signal i : bridge_in_type;
  signal o : bridge_out_type;
  signal chk_o : bridge_out_type;
  
  signal i_ctrl : fsectrl_in_type;
  signal o_ctrl : fsectrl_out_type;

  signal S: std_logic_vector(STRING_SIZE-1 downto 0);
  signal clk_cur: std_logic;
  signal check_clk_bus : std_logic := '0';
  signal iClkCnt : integer := 0;
  signal iErrCnt : integer := 0;
  signal iErrCheckedCnt : integer := 0;
begin


  -- Process of reading
  procReadingFile : process
    file InputFile:TEXT is "E:\Projects\CppProjects\20150601_riscv\ProjectFiles2013\patterns\fsebr0_tb.pat";
    variable rdLine: line;
    variable strLine : string(STRING_SIZE downto 1);
    variable clk_next: std_logic;
  begin
    readline(InputFile, rdLine);
    read(rdLine, strLine);
    S <= StringToSVector(strLine);

    wait for INCR_TIME;

    while not endfile(InputFile) loop
      readline(InputFile, rdLine);
      read(rdLine, strLine);

      clk_next := SignalFromString(strLine, 0);
      if (clk_next = '1' and clk_cur = '0') then
        check_clk_bus <= '1';
      end if;

      wait for 1 ps;
      check_clk_bus <= '0';
      clk_cur <= clk_next;

      S <= StringToSVector(strLine);

      wait for INCR_TIME;
      iClkCnt <= iClkCnt + 1;

    end loop;
    report "Total clocks checked: " & tost(iErrCheckedCnt) & " Errors: " & tost(iErrCnt);
    wait for 1 sec;
  end process procReadingFile;


  -- signal parsment and assignment
  i.nrst <= S(0);
  i.clk_bus <= S(1);
  i.clk_fse <= S(2);
  i.axi.aw_valid <= S(3);
  i.axi.aw_bits.addr <= S(35 downto 4);
  i.axi.aw_bits.len <= S(43 downto 36);
  i.axi.aw_bits.size <= S(46 downto 44);
  i.axi.aw_bits.burst <= S(48 downto 47);
  i.axi.aw_bits.lock <= S(49);
  i.axi.aw_bits.cache <= S(53 downto 50);
  i.axi.aw_bits.prot <= S(56 downto 54);
  i.axi.aw_bits.qos <= S(60 downto 57);
  i.axi.aw_bits.region <= S(64 downto 61);
  i.axi.aw_id <= S(69 downto 65);
  i.axi.aw_user <= S(70);
  i.axi.w_valid <= S(71);
  i.axi.w_data <= S(199 downto 72);
  i.axi.w_last <= S(200);
  i.axi.w_strb <= S(216 downto 201);
  i.axi.w_user <= S(217);
  i.axi.b_ready <= S(218);
  i.axi.ar_valid <= S(219);
  i.axi.ar_bits.addr <= S(251 downto 220);
  i.axi.ar_bits.len <= S(259 downto 252);
  i.axi.ar_bits.size <= S(262 downto 260);
  i.axi.ar_bits.burst <= S(264 downto 263);
  i.axi.ar_bits.lock <= S(265);
  i.axi.ar_bits.cache <= S(269 downto 266);
  i.axi.ar_bits.prot <= S(272 downto 270);
  i.axi.ar_bits.qos <= S(276 downto 273);
  i.axi.ar_bits.region <= S(280 downto 277);
  i.axi.ar_id <= S(285 downto 281);
  i.axi.ar_user <= S(286);
  i.axi.r_ready <= S(287);
  i.rdata <= S(415 downto 288);
  i.rdata_rdy <= S(416);
  chk_o.axi.aw_ready <= S(417);
  chk_o.axi.w_ready <= S(418);
  chk_o.axi.b_valid <= S(419);
  chk_o.axi.b_resp <= S(421 downto 420);
  chk_o.axi.b_id <= S(426 downto 422);
  chk_o.axi.b_user <= S(427);
  chk_o.axi.ar_ready <= S(428);
  chk_o.axi.r_valid <= S(429);
  chk_o.axi.r_resp <= S(431 downto 430);
  chk_o.axi.r_data <= S(559 downto 432);
  chk_o.axi.r_last <= S(560);
  chk_o.axi.r_id <= S(565 downto 561);
  chk_o.axi.r_user <= S(566);
  chk_o.wr_ena <= S(567);
  chk_o.rd_ena <= S(568);
  chk_o.addr <= S(577 downto 569);
  chk_o.data <= S(609 downto 578);

 
  tt : AxiFseBridge generic map
  (
    xindex => 5,
    xaddr => 16#80005#,
    xmask => 16#FFFFF#,
    did => X"0079"
  ) port map
  (
    i,
    o
  );
  
  
  i_ctrl.nrst             <= i.nrst;
  i_ctrl.clk              <= i.clk_fse;
  i_ctrl.wr_ena           <= o.wr_ena;
  i_ctrl.rd_ena           <= o.rd_ena;
  i_ctrl.addr             <= o.addr;
  i_ctrl.wdata            <= o.data;
  i_ctrl.ms_pulse         <= '0';
  i_ctrl.pps              <= '0';
  i_ctrl.adc_valid        <= '0';
  i_ctrl.rec_rdy          <= '0';
  i_ctrl.latch_max        <= (others => '0');
  i_ctrl.latch_ind        <= (others => '0');
  i_ctrl.latch_noise      <= (others => '0');
  i_ctrl.latch_dopler     <= (others => '0');
  i_ctrl.latch_rdy        <= '0';

  ctrl : FseControl generic map (
    generic_sys    => 0
  ) port map (
    i => i_ctrl,
    o => o_ctrl
  );


  procCheck : process (i.nrst, check_clk_bus, chk_o)
  begin
    if rising_edge(check_clk_bus) then
      if i.nrst = '1' then
        iErrCheckedCnt <= iErrCheckedCnt + 1;
        if (chk_o.axi.aw_ready /= o.axi.aw_ready) then print("Err: o.axi.aw_ready"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.w_ready /= o.axi.w_ready) then print("Err: o.axi.w_ready"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.b_valid /= o.axi.b_valid) then print("Err: o.axi.b_valid"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.b_resp /= o.axi.b_resp) then print("Err: o.axi.b_resp"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.b_id /= o.axi.b_id) then print("Err: o.axi.b_id"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.b_user /= o.axi.b_user) then print("Err: o.axi.b_user"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.ar_ready /= o.axi.ar_ready) then print("Err: o.axi.ar_ready"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.r_valid /= o.axi.r_valid) then print("Err: o.axi.r_valid"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.r_resp /= o.axi.r_resp) then print("Err: o.axi.r_resp"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.r_data /= o.axi.r_data) then print("Err: o.axi.r_data"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.r_last /= o.axi.r_last) then print("Err: o.axi.r_last"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.r_id /= o.axi.r_id) then print("Err: o.axi.r_id"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.axi.r_user /= o.axi.r_user) then print("Err: o.axi.r_user"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.wr_ena /= o.wr_ena) then print("Err: o.wr_ena"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.rd_ena /= o.rd_ena) then print("Err: o.rd_ena"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.addr /= o.addr) then print("Err: o.addr"); iErrCnt<=iErrCnt+1; end if;
        if (chk_o.data /= o.data) then print("Err: o.data"); iErrCnt<=iErrCnt+1; end if;
      end if;
    end if;
  end process procCheck;

end;
