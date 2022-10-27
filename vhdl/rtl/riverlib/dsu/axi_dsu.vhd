--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
--! @brief     Debug Support Unit (DSU) with AXI4 interface.
--! @details   DSU provides access to the internal CPU registers via
--!            'Debug port' bus interface available only on <b>RIVER</b> CPU.
--!            It is also implements a set of registers collecting bus
--!            utilization statistic and additional debug information.
-----------------------------------------------------------------------------

--! VHDL base library.
library ieee;
--! VHDL base types import
use ieee.std_logic_1164.all;
--! VHDL base numeric import
use ieee.numeric_std.all;
--! SoC common functionality library.
library commonlib;
--! SoC common types import
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
use ambalib.types_bus0.all; -- TODO: REMOVE ME when update dsu
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;
--! River top level with AMBA interface module declaration
use riverlib.types_river.all;

entity axi_dsu is
  generic (
    async_reset : boolean := false;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    o_cfg  : out axi4_slave_config_type;
    i_axi  : in axi4_slave_in_type;
    o_axi  : out axi4_slave_out_type;
    o_dporti : out dport_in_vector;
    i_dporto : in dport_out_vector;
    -- DMI interface
    i_dmi_hartsel : in std_logic_vector(CFG_LOG2_CPU_MAX-1 downto 0);
    o_dmi_req_valid : out std_logic;
    i_dmi_req_ready : in std_logic;
    o_dmi_write : out std_logic;
    o_dmi_addr : out std_logic_vector(6 downto 0);
    o_dmi_wdata : out std_logic_vector(31 downto 0);
    i_dmi_resp_valid : in std_logic;
    o_dmi_resp_ready : out std_logic;
    i_dmi_rdata : in std_logic_vector(31 downto 0);
    -- Platfrom run-time statistic
    i_bus_util_w : in std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0);
    i_bus_util_r : in std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0)
  );
end;

architecture arch_axi_dsu of axi_dsu is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_DSU
  );
  
  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type state_type is (
      idle,
      wait_write_msb,
      check_request,
      dmi_request,
      dmi_response,
      dport_request,
      dport_wait_resp,
      axi_response,
      axi_response_msb
   );
  
  type mst_utilization_type is array (0 to CFG_BUS0_XMST_TOTAL-1) 
         of std_logic_vector(63 downto 0);

  type mst_utilization_map_type is array (0 to 2*CFG_BUS0_XMST_TOTAL-1) 
         of std_logic_vector(63 downto 0);

  type registers is record
    state : state_type;
    r32 : std_logic;
    wdata : std_logic_vector(63 downto 0);
    -- Platform statistic:
    clk_cnt : std_logic_vector(63 downto 0);
    util_w_cnt : mst_utilization_type;
    util_r_cnt : mst_utilization_type;
    
    addr : std_logic_vector(CFG_DPORT_ADDR_BITS-1 downto 0);
    rdata : std_logic_vector(63 downto 0);
    write : std_logic;
  end record;

  constant R_RESET : registers := (
     idle, '0',                              -- state, r32
     (others => '0'),                        -- wdata,
     (others => '0'),                        -- clk_cnt
     (others => zero64), (others => zero64),
     
     (others => '0'), -- addr
     (others => '0'),  -- rdata
     '0'  -- write
  );

  signal r, rin: registers;
  signal wb_bus_raddr : global_addr_array_type;
  signal w_bus_re : std_logic;
  signal w_bus_r32 : std_logic;
  signal wb_bus_waddr : global_addr_array_type;
  signal w_bus_we : std_logic;
  signal wb_bus_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  signal wb_bus_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  signal w_axi_ready : std_logic;

begin

  axi0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => nrst,
    i_xcfg => xconfig, 
    i_xslvi => i_axi,
    o_xslvo => o_axi,
    i_ready => w_axi_ready,
    i_rdata => r.rdata,
    o_re => w_bus_re,
    o_r32 => w_bus_r32,
    o_radr => wb_bus_raddr,
    o_wadr => wb_bus_waddr,
    o_we => w_bus_we,
    o_wstrb => wb_bus_wstrb,
    o_wdata => wb_bus_wdata
  );

  comblogic : process(nrst, i_dporto, i_bus_util_w, i_bus_util_r, r, 
                      i_dmi_hartsel, i_dmi_req_ready, i_dmi_resp_valid, i_dmi_rdata,
                      w_bus_re, w_bus_r32, wb_bus_raddr, wb_bus_waddr,
                      w_bus_we, wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable vdporti : dport_in_vector;
    variable wb_bus_util_map : mst_utilization_map_type;
    variable cpuidx : integer;
    variable v_axi_ready : std_logic;
    variable v_dmi_req_valid : std_logic;
    variable v_dmi_resp_ready : std_logic;
  begin
    v := r;

    v_axi_ready := '0';
    v_dmi_req_valid := '0';
    v_dmi_resp_ready := '0';

    vdporti := (others => dport_in_none);
    cpuidx := conv_integer(i_dmi_hartsel);
    
    -- Update statistic:
    v.clk_cnt := r.clk_cnt + 1;

    -- TODO: move out these stuffs to bus-tracer
    for n in 0 to CFG_BUS0_XMST_TOTAL-1 loop
        if i_bus_util_w(n) = '1' then
            v.util_w_cnt(n) := r.util_w_cnt(n) + 1;
        end if;
        if i_bus_util_r(n) = '1' then
            v.util_r_cnt(n) := r.util_r_cnt(n) + 1;
        end if;
    end loop;

    for n in 0 to CFG_BUS0_XMST_TOTAL-1 loop
        wb_bus_util_map(2*n) := r.util_w_cnt(n);
        wb_bus_util_map(2*n+1) := r.util_r_cnt(n);
    end loop;
    
    case r.state is
    when idle =>
        v_axi_ready := '1';
        v.addr := (others => '0');
        v.wdata := (others => '0');
        v.rdata := (others => '0');
        if w_bus_we = '1' then
            v.write := '1';
            v.addr := wb_bus_waddr(0)(CFG_DPORT_ADDR_BITS+2 downto 3);
            v.wdata := wb_bus_wdata;
            if wb_bus_wstrb = X"FF" then
                v.state := check_request;
            elsif wb_bus_wstrb(3 downto 0) = X"F" then
                v.state := wait_write_msb;
            else
                -- shouldn't be here, it is better to generate slave error
                v.state := axi_response;
            end if;
        elsif w_bus_re = '1' then
            v.write := '0';
            v.addr := wb_bus_raddr(0)(CFG_DPORT_ADDR_BITS+2 downto 3);
            v.r32 := w_bus_r32;    -- burst 2 clocks (very bad style)
            v.state := check_request;
        end if;
    when wait_write_msb =>
        v_axi_ready := '1';
        if w_bus_we = '1' and wb_bus_wstrb(7 downto 4) = X"F" then
            v.wdata(63 downto 32) := wb_bus_wdata(63 downto 32);
            v.state := check_request;
        end if;
          
    when check_request =>
        v.rdata := (others => '0');
        if conv_integer(r.addr) < 3*4096 then  -- 0x3000 on 3 banks
            v.state := dport_request;
        else
            v.state := dmi_request;
        end if;
          
    when dmi_request =>
        v_dmi_req_valid := '1';
        if i_dmi_req_ready = '1' then
            v.state := dmi_response;
        end if;
    when dmi_response =>
        v_dmi_resp_ready := '1';
        if i_dmi_resp_valid = '1' then
            v.rdata(31 downto 0) := i_dmi_rdata;
            if r.write = '1' then
                v.state := idle;
            else
                v.state := axi_response;
            end if;
        end if;
          
    when dport_request =>  
        vdporti(cpuidx).req_valid := '1';
        vdporti(cpuidx).write := r.write;
        vdporti(cpuidx).addr := r.addr;
        vdporti(cpuidx).wdata := r.wdata;
        if i_dporto(cpuidx).req_ready = '1' then
            v.state := dport_wait_resp;
        end if;
    when dport_wait_resp =>
        vdporti(cpuidx).resp_ready := '1';
        if i_dporto(cpuidx).resp_valid = '1' then
            v.rdata := i_dporto(cpuidx).rdata;
            if r.write = '1' then
                v.state := idle;
            else
                v.state := axi_response;
            end if;
        end if;
    when axi_response =>
        v_axi_ready := '1';
        if r.write = '0' and r.r32 = '1' then
            v.state := axi_response_msb;  -- burst transaction
        else
            v.state := idle;
        end if;
    when axi_response_msb =>
        v_axi_ready := '1';
        v.state := idle;
    when others =>
    end case;

    if not async_reset and nrst = '0' then 
        v := R_RESET;
    end if;

    rin <= v;

    o_dmi_req_valid <= v_dmi_req_valid;
    o_dmi_write <= r.write;
    o_dmi_addr <= r.addr(6 downto 0);
    o_dmi_wdata <= r.wdata(31 downto 0);
    o_dmi_resp_ready <= v_dmi_resp_ready;

    o_dporti <= vdporti;
    w_axi_ready <= v_axi_ready;
  end process;

  o_cfg  <= xconfig;


  -- registers:
  regs : process(clk, nrst)
  begin 
      if async_reset and nrst = '0' then
          r <= R_RESET;
      elsif rising_edge(clk) then 
          r <= rin;
      end if; 
  end process;

end;
