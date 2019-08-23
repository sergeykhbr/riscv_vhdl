--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--! Unless required by applicable law or agreed to in writing, software
--! distributed under the License is distributed on an "AS IS" BASIS,
--! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--! See the License for the specific language governing permissions and
--! limitations under the License.
--!

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
library misclib;
use misclib.types_misc.all;

entity tap_jtag is
  generic (
    ainst  : integer range 0 to 255 := 2;
    dinst  : integer range 0 to 255 := 3);
  port (
    nrst  : in std_logic;
    clk  : in std_logic;
    i_tck   : in std_logic;   -- in: Test Clock
    i_ntrst   : in std_logic;
    i_tms   : in std_logic;   -- in: Test Mode State
    i_tdi   : in std_logic;   -- in: Test Data Input
    o_tdo   : out std_logic;   -- out: Test Data Output
    o_jtag_vref : out std_logic;
    i_msti   : in axi4_master_in_type;
    o_msto   : out axi4_master_out_type;
    o_mstcfg : out axi4_master_config_type
    );
end;


architecture rtl of tap_jtag is

  constant ADDBITS : integer := 10;

  constant xmstconfig : axi4_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_JTAG_TAP
  );

  type dma_req_state_type is (
      DMAREQ_IDLE,
      DMAREQ_SYNC_START,
      DMAREQ_START,
      DMAREQ_WAIT_READ_RESP,
      DMAREQ_SYNC_RESP
   );

  type tckpreg_type is record
    addr       : std_logic_vector(34 downto 0);
    datashft   : std_logic_vector(32 downto 0);
    done_sync  : std_ulogic;
    prun       : std_ulogic;
    inshift    : std_ulogic;
    holdn      : std_ulogic;
  end record;

  type tcknreg_type is record
    run: std_ulogic;
    done_sync1: std_ulogic;
    qual_rdata: std_ulogic;
    addrlo    : std_logic_vector(ADDBITS-1 downto 2);
    data       : std_logic_vector(32 downto 0);
  end record;

  type axireg_type is record
    dma : dma_bank_type;
    run_sync:  std_logic_vector(1 downto 0);
    qual_dreg: std_ulogic;
    qual_areg: std_ulogic;
    areg: std_logic_vector(34 downto 0);
    dreg: std_logic_vector(31 downto 0);
    done: std_ulogic;
    dma_req_state : dma_req_state_type;
  end record;

  signal ar, arin : axireg_type;
  signal tpr, tprin: tckpreg_type;
  signal tnr, tnrin: tcknreg_type;

  signal qual_rdata, rdataq: std_logic_vector(31 downto 0);
  signal qual_dreg,  dregq: std_logic_vector(31 downto 0);
  signal qual_areg,  aregqin, aregq: std_logic_vector(34 downto 0);

  signal dma_response : dma_response_type;

  signal tapi_tdo : std_logic;
  signal tapo_rst : std_logic;
  signal tapo_tck : std_logic;
  signal tapo_tdi : std_logic;
  signal tapo_inst : std_logic_vector(7 downto 0);
  signal tapo_capt   : std_logic;
  signal tapo_shft   : std_logic;
  signal tapo_upd    : std_logic;
  signal tapo_xsel1  : std_logic;
  signal tapo_xsel2  : std_logic;

  attribute syn_keep: boolean;
  attribute syn_keep of rdataq : signal is true;
  attribute syn_keep of dregq : signal is true;
  attribute syn_keep of aregq : signal is true;

  component dcom_jtag is generic (
    irlen  : integer range 2 to 8 := 2;
    idcode : integer range 0 to 255 := 9;
    ainst  : integer range 0 to 255 := 2;  -- IR rw,address,size (35 bits)
    dinst  : integer range 0 to 255 := 3;  -- IR data (32 bits)
    id : std_logic_vector(31 downto 0) := X"01040093"
  );
  port (
    rst         : in std_ulogic;
    tck         : in std_ulogic;
    tms         : in std_ulogic;
    tdi         : in std_ulogic;
    tdo         : out std_ulogic;
    tapi_tdo    : in std_ulogic;
    tapo_tck    : out std_ulogic;
    tapo_tdi    : out std_ulogic;
    tapo_inst   : out std_logic_vector(7 downto 0);
    tapo_rst    : out std_ulogic;
    tapo_capt   : out std_ulogic;
    tapo_shft   : out std_ulogic;
    tapo_upd    : out std_ulogic;
    tapo_xsel1  : out std_ulogic;
    tapo_xsel2  : out std_ulogic
    );
  end component;


begin

  qual_rdata <= (others => tnr.qual_rdata);
  rdataq <= not (ar.dreg(31 downto 0) and qual_rdata(31 downto 0));

  qual_dreg <= (others => ar.qual_dreg);
  dregq <= not (tnr.data(31 downto 0) and qual_dreg(31 downto 0));

  qual_areg <= (others => ar.qual_areg);
  aregqin <= tpr.addr(34 downto ADDBITS) &
             tnr.addrlo(ADDBITS-1 downto 2) &
             tpr.addr(1 downto 0);
  aregq <= not (aregqin and qual_areg(34 downto 0));


  comb : process (nrst, ar, dma_response,
                  tapo_tck, tapo_tdi, tapo_inst, tapo_rst, tapo_capt, tapo_shft, tapo_upd, tapo_xsel1, tapo_xsel2,
                  i_msti, tpr, tnr, aregq, dregq, rdataq)
    variable av : axireg_type;
    variable tpv : tckpreg_type;
    variable tnv : tcknreg_type;
    variable asel, dsel : std_ulogic;
    variable vtapi_tdo : std_logic;
    variable write, seq : std_ulogic;
    variable wb_dma_request : dma_request_type;
    variable wb_dma_response : dma_response_type;
    variable wb_msto : axi4_master_out_type;

  begin

    av := ar;
    tpv := tpr;
    tnv := tnr;

    ---------------------------------------------------------------------------
    -- TCK side logic
    ---------------------------------------------------------------------------
    asel := tapo_xsel1;
    dsel := tapo_xsel2;
    vtapi_tdo := tpr.addr(0);
    if dsel='1' then
      vtapi_tdo := tpr.datashft(0) and tpr.holdn;
    end if;
    write := tpr.addr(34);
    seq := tpr.datashft(32);

    -- Sync regs using alternating phases
    tnv.done_sync1 := ar.done;
    tpv.done_sync  := tnr.done_sync1;

    -- Data CDC
    if tnr.qual_rdata='1' then
        tpv.datashft(32 downto 0) := '1' & (not rdataq);
    end if;

    if tapo_capt = '1' then
        tpv.addr(ADDBITS-1 downto 2) := tnr.addrlo;
    end if;

    -- Track whether we're in the middle of shifting
    if tapo_shft = '1' then
        tpv.inshift:='1';
    end if;
    if tapo_upd = '1' then
        tpv.inshift:='0';
    end if;

    if tapo_shft = '1' then
        if asel = '1' and tpr.prun='0'  then
            tpv.addr(34 downto 0) := tapo_tdi & tpr.addr(34 downto 1);
        end if;
        if dsel = '1' and tpr.holdn='1' then
            tpv.datashft(32 downto 0) := tapo_tdi & tpr.datashft(32 downto 1);
        end if;
    end if;

    if tnr.run='0' then
        tpv.holdn := '1';
    end if;
    tpv.prun := tnr.run;

    if tpr.prun='0' then
        tnv.qual_rdata := '0';
        if tapo_shft = '0' and tapo_upd = '1' then
            if asel = '1' then 
                tnv.addrlo := tpr.addr(ADDBITS-1 downto 2);
            end if;
            if dsel = '1' then
                tnv.data := tpr.datashft;
            end if;
            if (asel and not write) = '1' then
                tpv.holdn := '0';
                tnv.run := '1';
            end if;
            if (dsel and (write or (not write and seq))) = '1' then
                tnv.run := '1';
                if (seq and not write) = '1' then
                    tnv.addrlo := tnr.addrlo + 1;
                    tpv.holdn := '0';
                end if;
            end if;
        end if;
    else
      if tpr.done_sync='1' and (tpv.inshift='0' or write='1') then
        tnv.run := '0';
        if write='0' then
          tnv.qual_rdata := '1';
        end if;
        if (write and tnr.data(32)) = '1' then
          tnv.addrlo := tnr.addrlo + 1;
        end if;
      end if;
    end if;

    if tapo_rst = '1' then
      tpv.inshift := '0';
      tnv.run := '0';
    end if;

    av.qual_dreg := '0';
    av.qual_areg := '0';

    wb_dma_request.valid := '0';
    wb_dma_request.ready := '0';
    wb_dma_request.write := '0';
    wb_dma_request.addr := (others => '0');
    wb_dma_request.size := (others => '0');
    wb_dma_request.bytes := (others => '0');
    wb_dma_request.wdata := (others => '0');

    --! DMA control
    case ar.dma_req_state is
    when DMAREQ_IDLE =>
        if ar.run_sync(0) = '1' then
           av.qual_dreg := '1';
           av.qual_areg := '1';
           av.dma_req_state := DMAREQ_SYNC_START;
        end if;

    when DMAREQ_SYNC_START =>
        av.dma_req_state := DMAREQ_START;

    when DMAREQ_START =>
        wb_dma_request.valid := '1';
        wb_dma_request.addr := ar.areg(31 downto 0);
        wb_dma_request.bytes := conv_std_logic_vector(4, 11);
        wb_dma_request.size := '0' & ar.areg(33 downto 32);  -- 010=4 bytes; 011=8 bytes (not impl. yet)
        if ar.areg(34) = '1' then
            wb_dma_request.write := '1';
            wb_dma_request.wdata := ar.dreg(31 downto 0) & ar.dreg(31 downto 0);
            if dma_response.ready = '1' then
                av.done := '1';
                av.dma_req_state := DMAREQ_SYNC_RESP;
            end if;
        else
            wb_dma_request.write := '0';
            wb_dma_request.wdata := (others => '0');
            av.dma_req_state := DMAREQ_WAIT_READ_RESP;
        end if;

    when DMAREQ_WAIT_READ_RESP =>
        wb_dma_request.ready := '1';
        if dma_response.valid = '1' then
            av.done := '1';
            av.dreg := dma_response.rdata(31 downto 0);
            av.dma_req_state := DMAREQ_SYNC_RESP;
        end if;

    when DMAREQ_SYNC_RESP =>
        if ar.run_sync(0) = '0' then
            av.done := '0';
            av.dma_req_state := DMAREQ_IDLE;
        end if;
    when others =>
    end case;


    procedureAxi4DMA(
        i_request => wb_dma_request,
        o_response => wb_dma_response,
        i_bank => ar.dma,
        o_bank => av.dma,
        i_msti => i_msti,
        o_msto => wb_msto
    );
    dma_response <= wb_dma_response;

    -- Sync regs and CDC transfer
    av.run_sync := tnr.run & ar.run_sync(1);

    if ar.qual_dreg='1' then
        av.dreg:=not dregq;
    end if;
    if ar.qual_areg='1' then 
        av.areg := not aregq;
    end if;


    if (nrst = '0') then
      av.dma := DMA_BANK_RESET;
      av.dma_req_state := DMAREQ_IDLE;
      av.qual_dreg := '0';
      av.qual_areg := '0';
      av.done := '0';
      av.areg := (others => '0');
      av.dreg := (others => '0');
    end if;

    tprin <= tpv;
    tnrin <= tnv;
    arin <= av;
    tapi_tdo <= vtapi_tdo;
    o_msto <= wb_msto;
  end process;


  o_jtag_vref <= '1';
  o_mstcfg <= xmstconfig;

  jtagcom0 : dcom_jtag generic map (
      irlen => 4,
      idcode => 9,
      ainst => ainst,
      dinst => dinst,
      id => X"01040093"
  ) port map (
    rst         => nrst,
    tck         => i_tck,
    tms         => i_tms,
    tdi         => i_tdi,
    tdo         => o_tdo,
    tapi_tdo => tapi_tdo,
    tapo_tck => tapo_tck,
    tapo_tdi => tapo_tdi,
    tapo_inst => tapo_inst,
    tapo_rst => tapo_rst,
    tapo_capt => tapo_capt,
    tapo_shft => tapo_shft,
    tapo_upd => tapo_upd,
    tapo_xsel1 => tapo_xsel1,
    tapo_xsel2 => tapo_xsel2
  );

  axireg : process(clk)
  begin
    if rising_edge(clk) then 
        ar <= arin;
    end if;
  end process;

  tckpreg: process(tapo_tck, tapo_rst)
  begin
    if rising_edge(tapo_tck) then
      tpr <= tprin;
    end if;
    if tapo_rst = '1' then
      tpr.addr <= (others => '0');
      tpr.datashft <= (others => '0');
      tpr.done_sync <= '0';
      tpr.prun <= '0';
      tpr.inshift <= '0';
      tpr.holdn <= '1';
    end if;
  end process;

  tcknreg: process(tapo_tck, tapo_rst)
  begin
    if falling_edge(tapo_tck) then
      tnr <= tnrin;
    end if;
    if tapo_rst = '1' then
      tnr.run <= '0';
      tnr.done_sync1 <= '0';
      tnr.qual_rdata <= '0';
      tnr.addrlo <= (others => '0');
      tnr.data <= (others => '0');
    end if;
  end process;

end;
