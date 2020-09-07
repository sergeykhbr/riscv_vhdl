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
  port (
    nrst  : in std_logic;
    clk  : in std_logic;
    i_tck   : in std_logic;   -- in: Test Clock
    i_ntrst   : in std_logic;
    i_tms   : in std_logic;   -- in: Test Mode State
    i_tdi   : in std_logic;   -- in: Test Data Input
    o_tdo   : out std_logic;   -- out: Test Data Output
    o_jtag_vref : out std_logic;
    -- DMI interface
    o_dmi_req_valid : out std_logic;
    i_dmi_req_ready : in std_logic;
    o_dmi_write : out std_logic;
    o_dmi_addr : out std_logic_vector(6 downto 0);
    o_dmi_wdata : out std_logic_vector(31 downto 0);
    i_dmi_resp_valid : in std_logic;
    o_dmi_resp_ready : out std_logic;
    i_dmi_rdata : in std_logic_vector(31 downto 0)
    );
end;


architecture rtl of tap_jtag is

  constant ADDBITS : integer := 10;

  type dmi_req_state_type is (
      DMIREQ_IDLE,
      DMIREQ_SYNC_START,
      DMIREQ_START,
      DMIREQ_WAIT_READ_RESP,
      DMIREQ_SYNC_RESP
   );

  type tckpreg_type is record
    dmishft    : std_logic_vector(40 downto 0);
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
    run_sync:  std_logic_vector(1 downto 0);
    qual_dreg: std_ulogic;
    qual_dmireg: std_ulogic;
    dmireg: std_logic_vector(40 downto 0);
    dreg: std_logic_vector(31 downto 0);
    done: std_ulogic;
    dmi_req_state : dmi_req_state_type;
  end record;

  signal ar, arin : axireg_type;
  signal tpr, tprin: tckpreg_type;
  signal tnr, tnrin: tcknreg_type;

  signal qual_rdata, rdataq: std_logic_vector(31 downto 0);
  signal qual_dreg,  dregq: std_logic_vector(31 downto 0);
  signal qual_dmireg,  dmiregq: std_logic_vector(40 downto 0);

  signal dma_response : dma_response_type;

  signal tapi_tdo : std_logic;
  signal tapo_rst : std_logic;
  signal tapo_tck : std_logic;
  signal tapo_tdi : std_logic;
  signal tapo_inst : std_logic_vector(4 downto 0);
  signal tapo_capt   : std_logic;
  signal tapo_shft   : std_logic;
  signal tapo_upd    : std_logic;
  signal tapo_xsel1  : std_logic;
  signal tapo_xsel2  : std_logic;

  attribute syn_keep: boolean;
  attribute syn_keep of rdataq : signal is true;
  attribute syn_keep of dregq : signal is true;
  attribute syn_keep of dmiregq : signal is true;

  component dcom_jtag is generic (
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
    tapo_inst   : out std_logic_vector(4 downto 0);
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

  qual_dmireg <= (others => ar.qual_dmireg);
  dmiregq <= not (tpr.dmishft and qual_dmireg);


  comb : process (nrst, ar, dma_response,
                  tapo_tck, tapo_tdi, tapo_inst, tapo_rst, tapo_capt, tapo_shft, tapo_upd, tapo_xsel1, tapo_xsel2,
                  i_dmi_req_ready, i_dmi_resp_valid, i_dmi_rdata,
                  tpr, tnr, dmiregq, dregq, rdataq)
    variable av : axireg_type;
    variable tpv : tckpreg_type;
    variable tnv : tcknreg_type;
    variable dsel : std_ulogic;
    variable vtapi_tdo : std_logic;
    variable write, seq : std_ulogic;
    variable v_dmi_req_valid : std_logic;
    variable v_dmi_write : std_logic;
    variable vb_dmi_addr : std_logic_vector(6 downto 0);
    variable vb_dmi_wdata : std_logic_vector(31 downto 0);
    variable vb_dmi_resp_ready : std_logic;
    variable wb_dma_response : dma_response_type;

  begin

    av := ar;
    tpv := tpr;
    tnv := tnr;

    ---------------------------------------------------------------------------
    -- TCK side logic
    ---------------------------------------------------------------------------
    dsel := tapo_xsel2;
    vtapi_tdo := tpr.dmishft(0);
    if dsel='1' then
      vtapi_tdo := tpr.datashft(0) and tpr.holdn;
    end if;
    write := tpr.dmishft(34);
    seq := tpr.datashft(32);

    -- Sync regs using alternating phases
    tnv.done_sync1 := ar.done;
    tpv.done_sync  := tnr.done_sync1;

    -- Data CDC
    if tnr.qual_rdata='1' then
--        tpv.datashft(32 downto 0) := '1' & (not rdataq);
        tpv.dmishft(33 downto 0) :=  (not rdataq) & "00";  -- 00=status OK
    end if;

    -- Track whether we're in the middle of shifting
    if tapo_shft = '1' then
        tpv.inshift:='1';
    end if;
    if tapo_upd = '1' then
        tpv.inshift:='0';
    end if;

    if tapo_shft = '1' then
        if tapo_xsel1 = '1' and tpr.prun='0'  then
            tpv.dmishft(40 downto 0) := tapo_tdi & tpr.dmishft(40 downto 1);
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
            if dsel = '1' then
                tnv.data := tpr.datashft;
            end if;
            if tapo_xsel1 = '1' then
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
      if tpr.done_sync='1' and tpv.inshift='0' then
        tnv.run := '0';
        tnv.qual_rdata := '1';
      end if;
    end if;

    if tapo_rst = '1' then
      tpv.inshift := '0';
      tnv.run := '0';
    end if;

    av.qual_dreg := '0';
    av.qual_dmireg := '0';

    v_dmi_req_valid := '0';
    v_dmi_write := '0';
    vb_dmi_addr := (others => '0');
    vb_dmi_wdata := (others => '0');
    vb_dmi_resp_ready := '0';

    --! DMA control
    case ar.dmi_req_state is
    when DMIREQ_IDLE =>
        if ar.run_sync(0) = '1' then
           av.qual_dreg := '1';
           av.qual_dmireg := '1';
           av.dmi_req_state := DMIREQ_SYNC_START;
        end if;

    when DMIREQ_SYNC_START =>
        av.dmi_req_state := DMIREQ_START;

    when DMIREQ_START =>
        v_dmi_req_valid := ar.dmireg(1) or ar.dmireg(0);
        v_dmi_write := ar.dmireg(1);
        vb_dmi_addr := ar.dmireg(40 downto 34);
        vb_dmi_wdata := ar.dmireg(33 downto 2);
        if v_dmi_req_valid = '0' then
            -- empty request 'nop'
            av.done := '1';
            av.dmi_req_state := DMIREQ_SYNC_RESP;
        elsif i_dmi_req_ready = '1' then
            av.dmi_req_state := DMIREQ_WAIT_READ_RESP;
        end if;

    when DMIREQ_WAIT_READ_RESP =>
        vb_dmi_resp_ready := '1';
        if i_dmi_resp_valid = '1' then
            av.done := '1';
            av.dreg := i_dmi_rdata(31 downto 0);
            av.dmi_req_state := DMIREQ_SYNC_RESP;
        end if;

    when DMIREQ_SYNC_RESP =>
        if ar.run_sync(0) = '0' then
            av.done := '0';
            av.dmi_req_state := DMIREQ_IDLE;
        end if;
    when others =>
    end case;


    -- Sync regs and CDC transfer
    av.run_sync := tnr.run & ar.run_sync(1);

    if ar.qual_dreg='1' then
        av.dreg := not dregq;
    end if;
    if ar.qual_dmireg='1' then 
        av.dmireg := not dmiregq;
    end if;


    if (nrst = '0') then
      av.dmi_req_state := DMIREQ_IDLE;
      av.qual_dreg := '0';
      av.qual_dmireg := '0';
      av.done := '0';
      av.dmireg := (others => '0');
      av.dreg := (others => '0');
    end if;

    tprin <= tpv;
    tnrin <= tnv;
    arin <= av;
    tapi_tdo <= vtapi_tdo;
    o_dmi_req_valid <= v_dmi_req_valid;
    o_dmi_write <= v_dmi_write;
    o_dmi_addr <= vb_dmi_addr;
    o_dmi_wdata <= vb_dmi_wdata;
    o_dmi_resp_ready <= vb_dmi_resp_ready;
  end process;


  o_jtag_vref <= '1';

  jtagcom0 : dcom_jtag generic map (
      id => X"00000001"
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
      tpr.dmishft <= (others => '0');
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
