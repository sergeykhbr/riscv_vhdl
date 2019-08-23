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

entity axi4_gpio is
  generic (
    async_reset : boolean := false;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    xirq     : integer := 0;
    width    : integer := 12
  );
  port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out axi4_slave_config_type;
    i    : in  axi4_slave_in_type;
    o    : out axi4_slave_out_type;
    i_gpio : in std_logic_vector(width-1 downto 0);
    o_gpio : out std_logic_vector(width-1 downto 0);
    o_gpio_dir : out std_logic_vector(width-1 downto 0)
  );
end; 
 
architecture arch_axi4_gpio of axi4_gpio is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(xirq, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_GPIO
  );

  type registers is record
    direction : std_logic_vector(31 downto 0);
    iuser : std_logic_vector(31 downto 0);
    ouser : std_logic_vector(31 downto 0);
    reg32_3 : std_logic_vector(31 downto 0);
    raddr : global_addr_array_type;
  end record;

  constant R_RESET : registers := (
      (others => '1'), (others => '0'), 
      (others => '0'), (others => '0'),
      ((others => '0'), (others => '0'))
  );

  signal r, rin : registers;

  signal wb_dev_rdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
  signal wb_bus_raddr : global_addr_array_type;
  signal w_bus_re    : std_logic;
  signal wb_bus_waddr : global_addr_array_type;
  signal w_bus_we    : std_logic;
  signal wb_bus_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
  signal wb_bus_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);

begin

  axi0 :  axi4_slave generic map (
    async_reset => async_reset
  ) port map (
    i_clk => clk,
    i_nrst => nrst,
    i_xcfg => xconfig, 
    i_xslvi => i,
    o_xslvo => o,
    i_ready => '1',
    i_rdata => wb_dev_rdata,
    o_re => w_bus_re,
    o_r32 => open,
    o_radr => wb_bus_raddr,
    o_wadr => wb_bus_waddr,
    o_we => w_bus_we,
    o_wstrb => wb_bus_wstrb,
    o_wdata => wb_bus_wdata
  );

  comblogic : process(nrst, i_gpio, r, w_bus_re, wb_bus_raddr, wb_bus_waddr,
                      w_bus_we, wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable vrdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
  begin

    v := r;

    v.raddr := wb_bus_raddr;

    for n in 0 to CFG_WORDS_ON_BUS-1 loop
      tmp := (others => '0');

      case conv_integer(r.raddr(n)(11 downto 2)) is
        when 0 => tmp := r.direction;
        when 1 => tmp := r.iuser;
        when 2 => tmp := r.ouser;
        when 3 => tmp := r.reg32_3;
        when others =>
      end case;
      vrdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if w_bus_we = '1' then

       for n in 0 to CFG_WORDS_ON_BUS-1 loop
         tmp := wb_bus_wdata(32*(n+1)-1 downto 32*n);

         if conv_integer(wb_bus_wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
           case conv_integer(wb_bus_waddr(n)(11 downto 2)) is
             when 0 => v.direction := tmp;
             --when 1 => v.iuser := tmp;  -- [RO]
             when 2 => v.ouser := tmp;
             when 3 => v.reg32_3 := tmp;
             when others =>
           end case;
         end if;
       end loop;
    end if;

    v.iuser(width-1 downto 0) := i_gpio;
    
    if not async_reset and nrst = '0' then
        v := R_RESET;
    end if;
  
    rin <= v;
    wb_dev_rdata <= vrdata;
  end process;

  cfg  <= xconfig;
  
  o_gpio <= r.ouser(width-1 downto 0);
  o_gpio_dir <= r.direction(width-1 downto 0);

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
