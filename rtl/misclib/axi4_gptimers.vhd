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

entity axi4_gptimers is
  generic (
    async_reset : boolean := false;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    xirq    : integer := 0;
    tmr_total  : integer := 2
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out axi4_slave_config_type;
    i_axi  : in  axi4_slave_in_type;
    o_axi  : out axi4_slave_out_type;
    o_pwm : out std_logic_vector(tmr_total-1 downto 0);
    o_irq  : out std_logic
  );
end; 
 
architecture arch_axi4_gptimers of axi4_gptimers is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(xirq, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_GPTIMERS
  );

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type timer_type is record
      count_ena : std_logic;
      irq_ena   : std_logic;
      pwm_ena   : std_logic;
      pwm_polarity : std_logic;
      value : std_logic_vector(63 downto 0);
      init_value : std_logic_vector(63 downto 0);
      pwm_threshold : std_logic_vector(63 downto 0);
  end record;
  
  constant timer_type_reset : timer_type := 
     ('0', '0', 
      '0', '0',
      (others => '0'),
      (others => '0'),
      (others => '0'));

  type vector_timer_type is array (0 to tmr_total-1) of timer_type;

  type registers is record
    tmr  : vector_timer_type;
    highcnt : std_logic_vector(63 downto 0);
    pending : std_logic_vector(tmr_total-1 downto 0);
    pwm : std_logic_vector(tmr_total-1 downto 0);
    raddr : global_addr_array_type;
  end record;

  constant R_RESET : registers := (
      (others => timer_type_reset), (others => '0'), (others => '0'),
      (others => '0'), ((others => '0'), (others => '0'))
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
    i_xslvi => i_axi,
    o_xslvo => o_axi,
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

  comblogic : process(nrst, r, w_bus_re, wb_bus_raddr, wb_bus_waddr,
                      w_bus_we, wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable raddr : integer;
    variable waddr : integer;
    variable vrdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
    variable irq_ena : std_logic;
  begin

    v := r;

    v.raddr := wb_bus_raddr;

    v.highcnt := r.highcnt + 1;

    irq_ena := '0';
    for n in 0 to tmr_total-1 loop
        if r.tmr(n).count_ena = '1' then 
           if r.tmr(n).pwm_ena = '1' and r.tmr(n).value = r.tmr(n).pwm_threshold then
               v.pwm(n) := not r.pwm(n);
           end if;

           if r.tmr(n).value = zero64 then
               irq_ena := irq_ena or r.tmr(n).irq_ena;
               v.pending(n) := r.tmr(n).irq_ena;
               v.pwm(n) := r.tmr(n).pwm_polarity;
               v.tmr(n).value := r.tmr(n).init_value;
           else
               v.tmr(n).value := r.tmr(n).value - 1;
           end if;
        else
           v.tmr(n).value := r.tmr(n).init_value;
           v.pwm(n) := r.tmr(n).pwm_polarity;
        end if;
    end loop;


    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       tmp := (others => '0');
       raddr := conv_integer(r.raddr(n)(11 downto 2));
       case raddr is
          when 0 => 
                tmp := r.highcnt(31 downto 0);
          when 1 => 
                tmp := r.highcnt(63 downto 32);
          when 2 => 
                tmp(tmr_total-1 downto 0) := r.pending;
          when 3 => 
                tmp(tmr_total-1 downto 0) := r.pwm;
          when others => 
                for k in 0 to tmr_total-1 loop
                   if raddr = (16 + 8*k) then
                      tmp(0) := r.tmr(k).count_ena;
                      tmp(1) := r.tmr(k).irq_ena;
                      tmp(4) := r.tmr(k).pwm_ena;
                      tmp(5) := r.tmr(k).pwm_polarity;
                   elsif raddr = (16 + 8*k + 2) then
                      tmp := r.tmr(k).value(31 downto 0);
                   elsif raddr = (16 + 8*k + 3) then
                      tmp := r.tmr(k).value(63 downto 32);
                   elsif raddr = (16 + 8*k + 4) then
                      tmp := r.tmr(k).init_value(31 downto 0);
                   elsif raddr = (16 + 8*k + 5) then
                      tmp := r.tmr(k).init_value(63 downto 32);
                   elsif raddr = (16 + 8*k + 6) then
                      tmp := r.tmr(k).pwm_threshold(31 downto 0);
                   elsif raddr = (16 + 8*k + 7) then
                      tmp := r.tmr(k).pwm_threshold(63 downto 32);
                   end if;
                end loop;
       end case;
       vrdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if w_bus_we = '1' then
      for n in 0 to CFG_WORDS_ON_BUS-1 loop

         if conv_integer(wb_bus_wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
           tmp := wb_bus_wdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n);
           waddr := conv_integer(wb_bus_waddr(n)(11 downto 2));
           case waddr is
             when 2 => 
                v.pending := tmp(tmr_total-1 downto 0);
             when others =>
                for k in 0 to tmr_total-1 loop
                   if waddr = (16 + 8*k) then
                      v.tmr(k).count_ena := tmp(0);
                      v.tmr(k).irq_ena := tmp(1);
                      v.tmr(k).pwm_ena := tmp(4);
                      v.tmr(k).pwm_polarity := tmp(5);
                   elsif waddr = (16 + 8*k + 2) then
                      v.tmr(k).value(31 downto 0) := tmp;
                   elsif waddr = (16 + 8*k + 3) then
                      v.tmr(k).value(63 downto 32) := tmp;
                   elsif waddr = (16 + 8*k + 4) then
                      v.tmr(k).init_value(31 downto 0) := tmp;
                   elsif waddr = (16 + 8*k + 5) then
                      v.tmr(k).init_value(63 downto 32) := tmp;
                   elsif waddr = (16 + 8*k + 6) then
                      v.tmr(k).pwm_threshold(31 downto 0) := tmp;
                   elsif waddr = (16 + 8*k + 7) then
                      v.tmr(k).pwm_threshold(63 downto 32) := tmp;
                   end if;
                end loop;
           end case;
         end if;
      end loop;
    end if;

    if not async_reset and nrst = '0' then
        v := R_RESET;
    end if;
    rin <= v;

    o_irq <= irq_ena;
    o_pwm <= r.pwm;
    wb_dev_rdata <= vrdata;
  end process;

  cfg <= xconfig;

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
