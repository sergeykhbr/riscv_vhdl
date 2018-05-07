-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
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
    o_dporti : out dport_in_type;
    i_dporto : in dport_out_type;
    --! reset CPU and interrupt controller
    o_soft_rst : out std_logic;
    -- Platfrom run-time statistic
    i_miss_irq  : in std_logic;
    i_miss_addr : in std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    i_bus_util_w : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    i_bus_util_r : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0)
  );
end;

architecture arch_axi_dsu of axi_dsu is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => 0,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_DSU
  );

type state_type is (reading, writting, dport_response, ready);
  
type mst_utilization_type is array (0 to CFG_NASTI_MASTER_TOTAL-1) 
       of std_logic_vector(63 downto 0);

type mst_utilization_map_type is array (0 to 2*CFG_NASTI_MASTER_TOTAL-1) 
       of std_logic_vector(63 downto 0);

type registers is record
  bank_axi : nasti_slave_bank_type;
  --! Message multiplexer to form 32->64 request message
  state         : state_type;
  waddr : std_logic_vector(13 downto 0);
  wdata : std_logic_vector(63 downto 0);
  rdata : std_logic_vector(63 downto 0);
  soft_rst : std_logic;
  -- Platform statistic:
  clk_cnt : std_logic_vector(63 downto 0);
  miss_access_cnt : std_logic_vector(63 downto 0);
  miss_access_addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
  util_w_cnt : mst_utilization_type;
  util_r_cnt : mst_utilization_type;
end record;

signal r, rin: registers;
begin

  comblogic : process(nrst, i_axi, i_dporto, i_miss_irq, i_miss_addr,
                      i_bus_util_w, i_bus_util_r, r)
    variable v : registers;
    variable mux_rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable vdporti : dport_in_type;
    variable iraddr : integer;
    variable wb_bus_util_map : mst_utilization_map_type;
  begin
    v := r;
    v.rdata := (others => '0');
    vdporti.valid := '0';
    vdporti.write := '0';
    vdporti.region := (others => '0');
    vdporti.addr := (others => '0');
    vdporti.wdata := (others => '0');
    
    -- Update statistic:
    v.clk_cnt := r.clk_cnt + 1;
    if i_miss_irq = '1' then
      v.miss_access_addr := i_miss_addr;
      v.miss_access_cnt := r.miss_access_cnt + 1;
    end if;

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

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);
    --! redefine value 'always ready' inserting waiting states.
    v.bank_axi.rwaitready := '0';

    if r.bank_axi.wstate = wtrans then
      -- 32-bits burst transaction
      v.waddr := r.bank_axi.waddr(0)(16 downto 3);
      if r.bank_axi.wburst = NASTI_BURST_INCR and r.bank_axi.wsize = 4 then
         if r.bank_axi.waddr(0)(2) = '1' then
             v.state := writting;
             v.wdata(63 downto 32) := i_axi.w_data(31 downto 0);
         else
             v.wdata(31 downto 0) := i_axi.w_data(31 downto 0);
         end if;
      else
         -- Write data on next clock.
         if i_axi.w_strb /= X"00" then
            v.wdata := i_axi.w_data;
         end if;
         v.state := writting;
      end if;
    end if;

    case r.state is
      when reading =>
           if r.bank_axi.rstate = rtrans then
              if r.bank_axi.raddr(0)(16 downto 15) = "11" then
                --! local region
                v.bank_axi.rwaitready := '1';
                iraddr := conv_integer(r.bank_axi.raddr(0)(14 downto 3));
                case iraddr is
                when 0 =>
                  v.rdata(0) := r.soft_rst;
                when 1 =>
                  v.rdata := r.miss_access_cnt;
                when 2 =>
                  v.rdata(CFG_NASTI_ADDR_BITS-1 downto 0) := r.miss_access_addr;
                when others =>
                  if (iraddr >= 8) and (iraddr < (8 + 2*CFG_NASTI_MASTER_TOTAL)) then
                      v.rdata := wb_bus_util_map(iraddr - 8);
                  end if;
                end case;
                v.state := ready;
              else
                --! debug port regions: 0 to 2
                vdporti.valid := '1';
                vdporti.write := '0';
                vdporti.region := r.bank_axi.raddr(0)(16 downto 15);
                vdporti.addr := r.bank_axi.raddr(0)(14 downto 3);
                vdporti.wdata := (others => '0');
                v.state := dport_response;
              end if;
           end if;
      when writting =>
           v.state := reading;
           if r.waddr(13 downto 12) = "11" then
             --! local region
             case conv_integer(r.waddr(11 downto 0)) is
             when 0 =>
               v.soft_rst := r.wdata(0);
             when others =>
             end case;
           else
             --! debug port regions: 0 to 2
             vdporti.valid := '1';
             vdporti.write := '1';
             vdporti.region := r.waddr(13 downto 12);
             vdporti.addr := r.waddr(11 downto 0);
             vdporti.wdata := r.wdata;
           end if;
      when dport_response =>
             v.state := ready;
             v.bank_axi.rwaitready := '1';
             v.rdata := i_dporto.rdata;
      when ready =>
             v.state := reading;
      when others =>
    end case;

    if r.bank_axi.raddr(0)(2) = '0' then
       mux_rdata(31 downto 0) := r.rdata(31 downto 0);
    else
       -- 32-bits aligned access (can be generated by MAC)
       mux_rdata(31 downto 0) := r.rdata(63 downto 32);
    end if;
    mux_rdata(63 downto 32) := r.rdata(63 downto 32);

    o_axi <= functionAxi4Output(r.bank_axi, mux_rdata);

    if nrst = '0' then 
       v.bank_axi := NASTI_SLAVE_BANK_RESET;
       v.state := reading;
       v.waddr := (others => '0');
       v.wdata := (others => '0');
       v.rdata := (others => '0');
       v.soft_rst := '0';
       v.clk_cnt := (others => '0');
       v.miss_access_cnt := (others => '0');
       v.miss_access_addr := (others => '0');
       v.util_w_cnt := (others => (others => '0'));
       v.util_r_cnt := (others => (others => '0'));
    end if;

    rin <= v;

    o_dporti <= vdporti;
  end process;

  o_cfg  <= xconfig;
  o_soft_rst <= r.soft_rst;


  -- registers:
  regs : process(clk)
  begin 
    if rising_edge(clk) then 
       r <= rin; 
    end if; 
  end process;
end;
