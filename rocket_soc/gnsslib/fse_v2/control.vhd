-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     FSE Control module
--! @details  	Common FSE control functions including group of channels
--!            registers access via CPU bus. Total number of channels 
--!            equals to 32. GPS Only.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;
library rocketlib;
use rocketlib.types_nasti.all;


entity FseControl is
generic (
    generic_sys    : integer := 0
);
port (
    i : in fsectrl_in_type;
    o : out fsectrl_out_type
);
end;

architecture rtl of FseControl is

--! Parallel reading from several registers depending system bus width:
constant RD_AT_ONCE : integer := CFG_NASTI_DATA_BITS / 32;

type ChanSettings is record
  prn          : std_logic_vector(9 downto 0);
  acc_ms       : std_logic_vector(9 downto 0);
  carr_steps   : std_logic_vector(9 downto 0);
  coh_ena      : std_logic;
  carr_nco_f0  : std_logic_vector(31 downto 0);
  carr_nco_dlt : std_logic_vector(31 downto 0);
  carr_nco_letter : std_logic_vector(31 downto 0);
end record;

type FseChannelType is record
  settings  : ChanSettings;
  max       : std_logic_vector(31 downto 0);
  ind       : std_logic_vector(11 downto 0);
  noise     : std_logic_vector(31 downto 0);
  dopler    : std_logic_vector(31 downto 0);
end record;

type FseChannel is array (0 to 31) of FseChannelType;

constant RES_ChanSettings : ChanSettings := (
  prn => (others => '0'), acc_ms => (others => '0'), carr_steps => (others => '0'),
  coh_ena => '0', carr_nco_f0 => (others => '0'), carr_nco_dlt => (others => '0'), 
  carr_nco_letter => (others => '0'));

constant RES_FseChannelType : FseChannelType := (
  settings => RES_ChanSettings, max => (others => '0'),  
  ind => (others => '0'), noise => (others => '0'), dopler => (others => '0'));


type regtype is record
  fse_ena       : std_logic;
  adc_fmt       : std_logic;
  adr           : std_logic_vector(8 downto 0);
  wdata         : std_logic_vector(31 downto 0);
  wr_ena        : std_logic;
  rd_ena        : std_logic;

  carr_nco_th   : std_logic_vector(31 downto 0);
  code_nco_th   : std_logic_vector(31 downto 0);
  carr_nco_if   : std_logic_vector(31 downto 0);
  code_nco      : std_logic_vector(31 downto 0);
  channel       : FseChannel;
  chan_sel      : std_logic_vector(4 downto 0);
  new_prn       : std_logic;

  procChanSel       : std_logic_vector(4 downto 0);
  procDopStepCnt    : std_logic_vector(9 downto 0);
  procDopStepTotal  : std_logic_vector(9 downto 0);
  procDopStepOffset : std_logic_vector(31 downto 0);
  procDopNcoOffset  : std_logic_vector(31 downto 0); -- dopStepCnt * dopStepValue
  procDopNcoZero    : std_logic_vector(31 downto 0);
  procDopNcoValue   : std_logic_vector(31 downto 0); -- final result on each cycle
  procIF            : std_logic_vector(31 downto 0);
  procPrn           : std_logic_vector(9 downto 0);
  procAccMs         : std_logic_vector(9 downto 0);
  procCohEna        : std_logic;

  MsCounter         : std_logic_vector(31 downto 0);
  MsMarker          : std_logic_vector(31 downto 0);

  stateIdle          : std_logic;
  stateWaitMsSync    : std_logic;
  stateWriting       : std_logic;
  stateSelChan       : std_logic;
  stateProcessing    : std_logic;
  stateNextDopler    : std_logic;
end record;

signal r, rin : regtype;

begin

  comb : process (r, i)
  variable v : regtype;
  variable wMsReady : std_logic;
  variable wPpsReady : std_logic;
  variable wSkipMs : std_logic;
  variable wStateNotDefined : std_logic;
  variable odata : std_logic_vector(31 downto 0);
  variable odata_full : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);

  variable rd_offset : integer range 0 to 7;
  variable rd_chn : integer range 0 to 31;

  variable wr_offset : integer range 0 to 7;
  variable wr_chn : integer range 0 to 31;
  variable chan_sel : integer range 0 to 31;

  begin

    v := r;

    v.adr := i.addr;
    v.wr_ena := i.wr_ena;
    v.rd_ena := i.rd_ena;
    v.wdata := i.wdata;

    wMsReady := i.ms_pulse and i.adc_valid;
    wPpsReady := i.pps and i.adc_valid;
    odata := (others => '0');
    odata_full := (others => '0');
    if r.rd_ena = '1' then
      for n in 0 to RD_AT_ONCE - 1 loop
          rd_offset := conv_integer(r.adr(2 downto 0)) + n;
          if conv_integer(r.adr(8 downto 3)) = 16#20# then
            -- COMMON FOR ALL CHANNELS BLOCK
            case rd_offset is
              when 0 =>  
                odata(31 downto 16) := conv_std_logic_vector(CFG_FSE_MSEC_MAX, 16);
                odata(15 downto 0) := conv_std_logic_vector(CFG_FSE_HW_ID, 16);
              when 1 =>  
                odata(31) := r.fse_ena;
                odata(30) := r.adc_fmt;
                -- status
                odata(4 downto 0) := r.procChanSel;
                odata(14 downto 5) := r.procDopStepCnt;
                odata(16) := r.stateIdle;
                odata(17) := r.stateWaitMsSync;
                odata(18) := r.stateWriting;
                odata(19) := r.stateSelChan;
                odata(20) := r.stateProcessing;
                odata(21) := r.stateNextDopler;
              when 2 =>  odata := r.MsMarker;
              when 3 =>  odata := r.carr_nco_th;
              when 4 =>  odata := r.code_nco_th;
              when 5 =>  odata := r.carr_nco_if;
              when 6 =>  odata := r.code_nco;
              when others =>
            end case;
   
          else
            -- SINGLE CHANNEL REGISTERS
            rd_chn := conv_integer(r.adr(7 downto 3));
            case rd_offset is
              when 0 =>
                odata(9 downto 0) := r.channel(rd_chn).settings.prn;
                odata(19 downto 10) := r.channel(rd_chn).settings.acc_ms;
                odata(29 downto 20) := r.channel(rd_chn).settings.carr_steps;
                odata(31) := r.channel(rd_chn).settings.coh_ena;
               when 1 => odata := r.channel(rd_chn).settings.carr_nco_f0;
               when 2 => odata := r.channel(rd_chn).settings.carr_nco_dlt;
               when 3 => odata := r.channel(rd_chn).settings.carr_nco_letter;
               when 4 => odata := r.channel(rd_chn).max;
               when 5 => odata(11 downto 0) := r.channel(rd_chn).ind;
               when 6 => odata := r.channel(rd_chn).noise;
               when 7 => odata := r.channel(rd_chn).dopler;
               when others =>
            end case;
          end if;
          odata_full(32*(n+1)-1 downto 32*n) := odata;
      end loop;
    end if;


    if r.wr_ena = '1' then
      wr_offset := conv_integer(r.adr(2 downto 0));
      if conv_integer(r.adr(8 downto 3)) = 16#20# then
        -- COMMON FOR ALL CHANNELS BLOCK
        case wr_offset is
          when 1 =>  
            v.fse_ena := r.wdata(31);
            v.adc_fmt := r.wdata(30);
          when 3 =>  v.carr_nco_th := r.wdata;
          when 4 =>  v.code_nco_th := r.wdata;
          when 5 =>  
            v.carr_nco_if := r.wdata;
            v.procIF := r.wdata;
          when 6 =>  v.code_nco := r.wdata;
          when others =>
        end case;
   
      else
        -- SINGLE CHANNEL REGISTERS
        wr_chn := conv_integer(r.adr(7 downto 3));
        case wr_offset is
          when 0 =>
            v.channel(wr_chn).settings.prn := r.wdata(9 downto 0);
            v.channel(wr_chn).settings.acc_ms := r.wdata(19 downto 10);
            v.channel(wr_chn).settings.carr_steps := r.wdata(29 downto 20);
            v.channel(wr_chn).settings.coh_ena := r.wdata(31);
            v.channel(wr_chn).max := (others => '0');
            v.channel(wr_chn).ind := (others => '0');
            v.channel(wr_chn).noise := (others => '0');
            v.channel(wr_chn).dopler := (others => '0');
           when 1 => v.channel(wr_chn).settings.carr_nco_f0 := r.wdata;
           when 2 => v.channel(wr_chn).settings.carr_nco_dlt := r.wdata;
           when 3 => v.channel(wr_chn).settings.carr_nco_letter := r.wdata;
           when others =>
        end case;
      end if;
    end if;


    if wPpsReady = '1' then 
      v.MsCounter(31 downto 2) := r.MsCounter(31 downto 2) + 1;
      v.MsCounter(1 downto 0) := "00";
    elsif wMsReady = '1' then 
      v.MsCounter := r.MsCounter + 1;
    end if;
    
    wStateNotDefined := not (r.stateIdle or r.stateWaitMsSync or r.stateWriting or
                       r.stateSelChan or r.stateProcessing or r.stateNextDopler);

    if (wStateNotDefined = '1' or r.stateIdle = '1') and r.fse_ena = '1' then
      v.stateIdle := '0';
      v.chan_sel := (others => '0');
      if generic_sys = GEN_SYSTEM_GLOCA then
        v.stateSelChan := '1';
      else
        v.stateWaitMsSync := '1';
      end if;
    end if;

    wSkipMs := '0';
    if generic_sys = GEN_SYSTEM_GALE1 and r.MsCounter(1 downto 0) /= "00" then
      wSkipMs := wMsReady;
    end if;
    
    if r.stateWaitMsSync = '1' and wMsReady = '1' and wSkipMs = '0' then
      v.stateWaitMsSync := '0';
      v.stateWriting := '1';
      v.MsMarker := r.MsCounter;
    end if;

    v.new_prn := '0';
    if r.stateWriting = '1' and i.rec_rdy = '1' then
      v.stateWriting := '0';
      if generic_sys = GEN_SYSTEM_GLOCA then
        v.stateProcessing := '1';
        v.new_prn := '1';
      else
        v.stateSelChan := '1';
      end if;
    end if;

    if r.stateSelChan = '1' then
      chan_sel := conv_integer(r.chan_sel);
      v.procChanSel       := r.chan_sel;
      v.procDopStepCnt    := (others => '0');
      v.procDopStepTotal  := r.channel(chan_sel).settings.carr_steps;
      v.procDopStepOffset := r.channel(chan_sel).settings.carr_nco_dlt;
      v.procDopNcoOffset  := r.channel(chan_sel).settings.carr_nco_dlt;  -- first dopler change
      v.procIF            := r.channel(chan_sel).settings.carr_nco_letter + r.carr_nco_if;
      v.procDopNcoZero    := r.channel(chan_sel).settings.carr_nco_f0;
      v.procDopNcoValue   := r.channel(chan_sel).settings.carr_nco_f0;
      v.procPrn           := r.channel(chan_sel).settings.prn;
      v.procAccMs         := r.channel(chan_sel).settings.acc_ms;
      v.procCohEna        := r.channel(chan_sel).settings.coh_ena;

      v.chan_sel := r.chan_sel + 1;

      if generic_sys = GEN_SYSTEM_GLOCA then
        if r.fse_ena = '0' then
          v.stateSelChan := '0';
          v.stateIdle := '1';
        elsif r.channel(chan_sel).settings.prn = "0000000000" then
          -- just wait. Do nothing
        else
          v.stateSelChan := '0';
          v.stateWaitMsSync := '1';
        end if;
      elsif conv_integer(r.procChanSel) = 31 then
        v.stateSelChan := '0';
        v.stateIdle := '1';
      elsif r.channel(chan_sel).settings.prn = "0000000000" then
        -- just wait. Do nothing
      else
        v.stateSelChan := '0';
        v.stateProcessing := '1';
        v.new_prn := '1';
      end if;
    end if;

    if r.stateProcessing = '1' and i.latch_rdy = '1' then
        chan_sel := conv_integer(r.procChanSel);
        v.channel(chan_sel).max    := i.latch_max;
        v.channel(chan_sel).ind    := i.latch_ind;
        v.channel(chan_sel).noise  := i.latch_noise;
        v.channel(chan_sel).dopler := i.latch_dopler;

        if r.procDopStepCnt = r.procDopStepTotal then
            v.stateSelChan := '1';
            v.stateProcessing := '0';
        else
            v.stateNextDopler := '1';
            v.stateProcessing := '0';
        end if;
    end if;


    if r.stateNextDopler = '1' then
        v.stateProcessing := '1';
        v.stateNextDopler := '0';
        if r.procDopStepCnt(0) = '1' then
            v.procDopNcoValue := r.procDopNcoZero + r.procDopNcoOffset;
        else
            v.procDopNcoValue := r.procDopNcoZero - r.procDopNcoOffset;
            v.procDopNcoOffset := r.procDopNcoOffset + r.procDopStepOffset;
        end if;
        v.procDopStepCnt := r.procDopStepCnt + 1;
    end if;


    -- AMBA request:
    o.rdata      <= odata_full;
    o.rdata_rdy  <= r.rd_ena;


    -- Reset all
    if i.nrst = '0' then

      v.fse_ena := '0';
      v.adc_fmt := '0';
      v.adr := (others => '0');
      v.wdata := (others => '0');
      v.wr_ena := '0';
      v.rd_ena := '0';

      v.carr_nco_th := (others => '0');
      v.code_nco_th := (others => '0');
      v.carr_nco_if := (others => '0');
      v.code_nco := (others => '0');
      v.channel := (others => RES_FseChannelType);
      v.chan_sel := (others => '0');
      v.new_prn := '0';

      v.procChanSel := (others => '0');
      v.procDopStepCnt := (others => '0');
      v.procDopStepTotal := (others => '0');
      v.procDopStepOffset := (others => '0');
      v.procDopNcoOffset := (others => '0');
      v.procDopNcoZero := (others => '0');
      v.procDopNcoValue := (others => '0');
      v.procIF := (others => '0');
      v.procPrn := (others => '0');
      v.procAccMs := (others => '0');
      v.procCohEna := '0';

      v.MsCounter := (others => '0');
      v.MsMarker := (others => '0');

      v.stateIdle := '1';
      v.stateWaitMsSync := '0';
      v.stateWriting := '0';
      v.stateSelChan := '0';
      v.stateProcessing := '0';
      v.stateNextDopler := '0';
    end if;
 
    rin <= v;
  end process;

  -- Control signals
  o.adc_fmt     <= r.adc_fmt;
  o.carrnco_th  <= r.carr_nco_th;
  o.carr_nco_if <= r.procIF;
  o.dopler_nco  <= r.procDopNcoValue;
  o.codenco_th  <= r.code_nco_th;
  o.code_nco    <= r.code_nco;
  o.prn         <= r.procPrn;
  o.ms_total    <= r.procAccMs;
  o.coh_ena     <= r.procCohEna;
  o.rec_ena     <= r.stateWriting;
  o.play_ena    <= r.stateProcessing;
  o.new_cycle   <= r.stateSelChan or r.stateNextDopler;
  o.new_prn     <= r.new_prn;
  o.rst_max     <= r.stateIdle or r.stateSelChan;

  regs : process(i.clk)
  begin 
    if rising_edge(i.clk) then 
      r <= rin;
    end if;
  end process;

end; 


