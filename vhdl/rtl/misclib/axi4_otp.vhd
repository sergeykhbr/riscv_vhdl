--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
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

entity axi4_otp is
  generic (
    async_reset : boolean := false;
    xaddr   : integer := 0;
    xmask   : integer := 16#ffffe#
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out axi4_slave_config_type;
    i_axi  : in  axi4_slave_in_type;
    o_axi  : out axi4_slave_out_type;
    o_otp_we     : out  std_ulogic;
    o_otp_re     : out  std_ulogic;
    o_otp_addr   : out std_logic_vector(11 downto 0);
    o_otp_wdata  : out std_logic_vector(15 downto 0);
    i_otp_rdata  : in std_logic_vector(15 downto 0);
    i_cfg_rsetup : in std_logic_vector(3 downto 0);
    i_cfg_wadrsetup : in std_logic_vector(3 downto 0);
    i_cfg_wactive : in std_logic_vector(31 downto 0);
    i_cfg_whold : in std_logic_vector(3 downto 0);
    o_busy : out std_logic
  );
end; 
 
architecture arch_axi4_otp of axi4_otp is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_OTP_8KB
  );

  type state_type is (idle, rsetup, rhold, rlatch, wsetup_addr, wsetup_we, wactive, whold);

  type registers is record
     hword_cnt : integer range 0 to 4;
     hold_cnt : integer;
     state : state_type;
     data_ready : std_logic;

     otp_we : std_logic;
     otp_re : std_logic;
     otp_addr : std_logic_vector(11 downto 0);
     otp_wdata : std_logic_vector(63 downto 0);
     rdata : std_logic_vector(63 downto 0);
  end record;

  constant R_RESET : registers := (
    0, 0, idle,       -- hword_cnt, hold_cnt, state
    '0',              -- data_ready
    '0', '0',         -- otp_we, otp_re
    (others => '0'), (others => '0'), -- otp_addr, otp_wdata
    (others => '0')   -- rdata
  );

  signal r, rin : registers;

  signal wb_bus_raddr : global_addr_array_type;
  signal w_bus_re    : std_logic;
  signal wb_bus_waddr : global_addr_array_type;
  signal w_bus_we    : std_logic;
  signal wb_bus_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  signal wb_bus_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  signal w_busy : std_logic;

begin

  axi0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => nrst,
    i_xcfg => xconfig, 
    i_xslvi => i_axi,
    o_xslvo => o_axi,
    i_ready => r.data_ready,
    i_rdata => r.rdata,
    o_re => w_bus_re,
    o_r32 => open,
    o_radr => wb_bus_raddr,
    o_wadr => wb_bus_waddr,
    o_we => w_bus_we,
    o_wstrb => wb_bus_wstrb,
    o_wdata => wb_bus_wdata
  );

  comblogic : process(nrst, i_otp_rdata, i_axi, r,
                      i_cfg_rsetup, i_cfg_wadrsetup, i_cfg_wactive, i_cfg_whold,
                      w_bus_re, wb_bus_raddr, wb_bus_waddr, w_bus_we,
                      wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable rdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
    variable v_busy : std_logic;

  begin

    v := r;

    v.data_ready := '0';

    v_busy := '0';
    if r.state /= idle then
        v_busy := '1';
    end if;

    case r.state is
    when idle =>
       v.otp_addr := (others => '0');
       v.otp_wdata := (others => '0');
       v.otp_we := '0';
       v.otp_re := '0';
    when rsetup =>
       if r.hword_cnt /= 0 then
           v.otp_wdata := (others => '1');
           v.otp_we := '0';
           v.otp_re := '1';
           v.hword_cnt := r.hword_cnt - 1;
           v.hold_cnt := conv_integer(i_cfg_rsetup); -- > 30 ns
           v.state := rhold;
       else
           v.state := idle;
           v.data_ready := '1';
       end if;
    when rhold =>
        -- Must be more than 30 ns:
        if r.hold_cnt /= 0 then
           v.hold_cnt := r.hold_cnt - 1;
        else
           v.state := rlatch;
        end if;

    when rlatch =>
        v.rdata := i_otp_rdata & r.rdata(63 downto 16);
        v.otp_addr := r.otp_addr + 1;
        v.state := rsetup;
        v.otp_re := '0';
        v.otp_wdata := (others => '0');

    when wsetup_addr =>
       if r.hword_cnt /= 0 then
           v.otp_we := '0';
           v.otp_re := '0';
           v.hword_cnt := r.hword_cnt - 1;
           v.hold_cnt := conv_integer(i_cfg_wadrsetup); -- > 20 ns
           v.state := wsetup_we;
       else
           v.state := idle;
       end if;
    when wsetup_we =>
        -- Must be more than 20 ns:
        --    clk = 80 MHz, 1 period = 12.5 ns => 3*Tclk = 37.5 ns
        --    warning: reading on next clock so (counter-1)
        if r.hold_cnt /= 0 then
           v.hold_cnt := r.hold_cnt - 1;
        else
           v.state := wactive;
           v.otp_we := '1';
           v.hold_cnt := conv_integer(i_cfg_wactive); -- > 50 ms && < 100 ms
        end if;
    when wactive =>
        -- hold we 50ms
        if r.hold_cnt /= 0 then
           v.hold_cnt := r.hold_cnt - 1;
        else
           v.hold_cnt := conv_integer(i_cfg_whold); -- > 10 ns
           v.state := whold;
           v.otp_we := '0';
        end if;

    when whold =>
        if r.hold_cnt /= 0 then
           v.hold_cnt := r.hold_cnt - 1;
        else
            v.state := wsetup_addr;
            v.otp_addr := r.otp_addr + 1;
            v.otp_wdata := X"0000" & r.otp_wdata(63 downto 16);
        end if;
    when others =>
    end case;


    -- Read request
    if w_bus_re = '1' then
         if r.state = idle then
             v.state := rsetup;
             v.otp_addr := wb_bus_raddr(0)(12 downto 2) & '0';
             v.hword_cnt := 4;
         else
             v.rdata := (others => '1');
             v.data_ready := '1';
         end if;
    end if;


    -- Write request
    if w_bus_we = '1'  then

        wstrb := wb_bus_wstrb;

        if r.state = idle then
            v.state := wsetup_addr;
            -- Only 4-bytes or 8-bytes access
            if wstrb = X"03" then
                v.otp_addr := wb_bus_waddr(0)(12 downto 2) & '0';
                v.otp_wdata(15 downto 0) := wb_bus_wdata(15 downto 0);
                v.otp_wdata(63 downto 16) := (others => '0');
                v.hword_cnt := 1;
            elsif wstrb = X"0C" then
                v.otp_addr := wb_bus_waddr(0)(12 downto 2) & '1';
                v.otp_wdata(15 downto 0) := wb_bus_wdata(31 downto 16);
                v.otp_wdata(63 downto 16) := (others => '0');
                v.hword_cnt := 1;
            elsif wstrb = X"30" then
                v.otp_addr := wb_bus_waddr(1)(12 downto 2) & '0';
                v.otp_wdata(15 downto 0) := wb_bus_wdata(47 downto 32);
                v.otp_wdata(63 downto 16) := (others => '0');
                v.hword_cnt := 1;
            elsif wstrb = X"C0" then
                v.otp_addr := wb_bus_waddr(1)(12 downto 2) & '1';
                v.otp_wdata(15 downto 0) := wb_bus_wdata(63 downto 48);
                v.otp_wdata(63 downto 16) := (others => '0');
                v.hword_cnt := 1;
            elsif wstrb = X"F0" then
                v.otp_addr := wb_bus_waddr(1)(12 downto 2) & '0';
                v.otp_wdata := wb_bus_wdata(63 downto 32) & wb_bus_wdata(63 downto 32);
                v.hword_cnt := 2;
            elsif wstrb = X"0F" then
                v.otp_addr := wb_bus_waddr(0)(12 downto 2) & '0';
                v.otp_wdata := wb_bus_wdata(31 downto 0) & wb_bus_wdata(31 downto 0);
                v.hword_cnt := 2;
            elsif wstrb = X"FF" then
                v.otp_addr := wb_bus_waddr(0)(12 downto 3) & "00";
                v.otp_wdata := wb_bus_wdata;
                v.hword_cnt := 4;
            else
                v.hword_cnt := 0;
            end if;
        end if;
    end if;

    if not async_reset and nrst = '0' then
        v := R_RESET;
    end if;

    w_busy <= v_busy;

    rin <= v;

  end process;

  cfg <= xconfig;

  o_otp_we <= r.otp_we;
  o_otp_re <= r.otp_re;
  o_otp_addr <= r.otp_addr;
  o_otp_wdata <= r.otp_wdata(15 downto 0);
  o_busy <= w_busy;

  -- registers:
  regs : process(nrst, clk)
  begin 
    if async_reset and nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;