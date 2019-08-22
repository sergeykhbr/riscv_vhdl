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
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_dporti : out dport_in_vector;
    i_dporto : in dport_out_vector;
    --! reset CPU and interrupt controller
    o_soft_rst : out std_logic;
    -- Platfrom run-time statistic
    i_bus_util_w : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    i_bus_util_r : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0)
  );
end;

architecture arch_axi_dsu of axi_dsu is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_DSU
  );
  
  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type state_type is (idle, read_internal, read_dport, dport_response, read_msw32);
  
  type mst_utilization_type is array (0 to CFG_NASTI_MASTER_TOTAL-1) 
         of std_logic_vector(63 downto 0);

  type mst_utilization_map_type is array (0 to 2*CFG_NASTI_MASTER_TOTAL-1) 
         of std_logic_vector(63 downto 0);

  type registers is record
    state : state_type;
    r32 : std_logic;
    raddr : std_logic_vector(13 downto 0);
    waddr : std_logic_vector(13 downto 0);
    wdata : std_logic_vector(63 downto 0);
    write_valid : std_logic;
    soft_rst : std_logic;
    dev_rdata : std_logic_vector(63 downto 0);
    dev_ready : std_logic;
    -- Platform statistic:
    clk_cnt : std_logic_vector(63 downto 0);
    cpu_context : std_logic_vector(log2x(CFG_CORES_PER_DSU_MAX)-1 downto 0);
    util_w_cnt : mst_utilization_type;
    util_r_cnt : mst_utilization_type;
  end record;

  constant R_RESET : registers := (
     idle, '0',                              -- state, r32
     (others => '0'), (others => '0'),       -- raddr, waddr
     (others => '0'), '0', '0',              -- wdata, write_valid, soft_rst
     (others => '0'), '0',                   -- dev_rdata, dev_Ready
     (others => '0'), (others => '0'),       -- clk_cnt, cpu_context
     (others => zero64), (others => zero64)
  );

  signal r, rin: registers;
  signal wb_bus_raddr : global_addr_array_type;
  signal w_bus_re : std_logic;
  signal w_bus_r32 : std_logic;
  signal wb_bus_waddr : global_addr_array_type;
  signal w_bus_we : std_logic;
  signal wb_bus_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  signal wb_bus_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);

begin

  axi0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => nrst,
    i_xcfg => xconfig, 
    i_xslvi => i_axi,
    o_xslvo => o_axi,
    i_ready => r.dev_ready,
    i_rdata => r.dev_rdata,
    o_re => w_bus_re,
    o_r32 => w_bus_r32,
    o_radr => wb_bus_raddr,
    o_wadr => wb_bus_waddr,
    o_we => w_bus_we,
    o_wstrb => wb_bus_wstrb,
    o_wdata => wb_bus_wdata
  );

  comblogic : process(nrst, i_dporto, i_bus_util_w, i_bus_util_r, r, 
                      w_bus_re, w_bus_r32, wb_bus_raddr, wb_bus_waddr,
                      w_bus_we, wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable vrdata_internal : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable vdporti : dport_in_vector;
    variable iraddr : integer;
    variable wb_bus_util_map : mst_utilization_map_type;
    variable cpuidx : integer;
  begin
    v := r;
    vdporti := (others => dport_in_none);
    cpuidx := conv_integer(r.cpu_context);
    
    -- Update statistic:
    v.clk_cnt := r.clk_cnt + 1;

    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
        if i_bus_util_w(n) = '1' then
            v.util_w_cnt(n) := r.util_w_cnt(n) + 1;
        end if;
        if i_bus_util_r(n) = '1' then
            v.util_r_cnt(n) := r.util_r_cnt(n) + 1;
        end if;
    end loop;

    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
        wb_bus_util_map(2*n) := r.util_w_cnt(n);
        wb_bus_util_map(2*n+1) := r.util_r_cnt(n);
    end loop;


    iraddr := conv_integer(r.raddr(11 downto 0));
    vrdata_internal := (others => '0');
    case iraddr is
    when 0 =>
        vrdata_internal(0) := r.soft_rst;
    when 1 =>
        vrdata_internal(log2x(CFG_CORES_PER_DSU_MAX)-1 downto 0) := r.cpu_context;
    when others =>
        if (iraddr >= 8) and (iraddr < (8 + 2*CFG_NASTI_MASTER_TOTAL)) then
             vrdata_internal := wb_bus_util_map(iraddr - 8);
        end if;
    end case;

    v.write_valid := '0'; 
    if r.write_valid = '1' then
        if r.waddr(13 downto 12) = "11" then
            --! local region
            case conv_integer(r.waddr(11 downto 0)) is
            when 0 =>
                v.soft_rst := r.wdata(0);
            when 1 =>
                v.cpu_context := r.wdata(log2x(CFG_CORES_PER_DSU_MAX)-1 downto 0);
            when others =>
            end case;
        else
            --! debug port regions: 0 to 2
            vdporti(cpuidx).valid := '1';
            vdporti(cpuidx).write := '1';
            vdporti(cpuidx).region := r.waddr(13 downto 12);
            vdporti(cpuidx).addr := r.waddr(11 downto 0);
            vdporti(cpuidx).wdata := r.wdata;
        end if;
    end if;

    case r.state is
      when idle =>
          v.dev_ready := '1';
          if w_bus_we = '1' then
              if wb_bus_waddr(0)(2) = '0' and wb_bus_wstrb = X"FF" then
                  -- 64-bits access
                  v.write_valid := '1'; 
                  v.waddr := wb_bus_waddr(0)(16 downto 3);
                  v.wdata := wb_bus_wdata;
              elsif wb_bus_wstrb(3 downto 0) = X"F" then
                  -- 32-bits burst 1-st half, wait the second part
                  v.waddr := wb_bus_waddr(0)(16 downto 3);
                  v.wdata(31 downto 0) := wb_bus_wdata(31 downto 0);
              else
                  -- 32-bits burst 2-nd half
                  v.write_valid := '1'; 
                  v.wdata(63 downto 32) := wb_bus_wdata(31 downto 0);
              end if;
          elsif w_bus_re = '1' then
              v.dev_ready := '0';
              v.raddr := wb_bus_raddr(0)(16 downto 3);
              v.r32 := w_bus_r32;
              if wb_bus_raddr(0)(16 downto 15) = "11" then
                  v.state := read_internal;
              else
                  v.state := read_dport;
              end if;
          end if;

      when read_internal =>
          v.dev_ready := '1';
          v.dev_rdata := vrdata_internal;
          if r.r32 = '1' then
              v.state := read_msw32;
          else
              v.state := idle;
          end if;
      when read_dport =>
          --! debug port regions: 0 to 2
          vdporti(cpuidx).valid := '1';
          vdporti(cpuidx).write := '0';
          vdporti(cpuidx).region := r.raddr(13 downto 12);
          vdporti(cpuidx).addr := r.raddr(11 downto 0);
          vdporti(cpuidx).wdata := (others => '0');
          v.state := dport_response;
      when dport_response =>
          if r.r32 = '1' then
              v.state := read_msw32;
          else
              v.state := idle;
          end if;
          v.dev_ready := '1';
          v.dev_rdata := i_dporto(cpuidx).rdata;
      when read_msw32 =>
          v.r32 := '0';
          v.state := idle;
          v.dev_ready := '1';
      when others =>
    end case;

    if not async_reset and nrst = '0' then 
        v := R_RESET;
    end if;

    rin <= v;

    o_dporti <= vdporti;
  end process;

  o_cfg  <= xconfig;
  o_soft_rst <= r.soft_rst;


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
