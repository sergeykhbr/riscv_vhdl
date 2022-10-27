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
library techmap;
use techmap.gencomp.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
use ambalib.types_bus0.all;

--! @brief Hardware Configuration storage with the AMBA AXI4 interface.
entity axi4_pnp is
  generic (
    async_reset : boolean := false;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    tech    : integer := 0;
    hw_id   : std_logic_vector(31 downto 0) := X"20170101"
  );
  port (
    sys_clk : in  std_logic;
    adc_clk : in  std_logic;
    nrst   : in  std_logic;
    mstcfg : in  bus0_xmst_cfg_vector;
    slvcfg : in  bus0_xslv_cfg_vector;
    cfg    : out axi4_slave_config_type;
    i      : in  axi4_slave_in_type;
    o      : out axi4_slave_out_type;
    -- OTP Timing control
    i_otp_busy : in std_logic;
    o_otp_cfg_rsetup : out std_logic_vector(3 downto 0);
    o_otp_cfg_wadrsetup : out std_logic_vector(3 downto 0);
    o_otp_cfg_wactive : out std_logic_vector(31 downto 0);
    o_otp_cfg_whold : out std_logic_vector(3 downto 0)
  );
end; 
 
architecture axi4_nasti_pnp of axi4_pnp is

  constant xconfig : axi4_slave_config_type := (
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_SLAVE,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_PNP
  );

  type master_config_map is array (0 to 2*CFG_BUS0_XMST_TOTAL-1)
       of std_logic_vector(31 downto 0);
       
  type slave_config_map is array (0 to 4*CFG_BUS0_XSLV_TOTAL-1)
       of std_logic_vector(31 downto 0);

  type registers is record
    fw_id : std_logic_vector(31 downto 0);
    idt : std_logic_vector(63 downto 0); --! debug counter
    malloc_addr : std_logic_vector(63 downto 0); --! dynamic allocation addr
    malloc_size : std_logic_vector(63 downto 0); --! dynamic allocation size
    fwdbg1 : std_logic_vector(63 downto 0); --! FW marker for the debug porposes
    fwdbg2 : std_logic_vector(63 downto 0);
    adc_detect : std_logic_vector(7 downto 0);
    raddr : global_addr_array_type;

    otp_cfg_rsetup : std_logic_vector(3 downto 0);
    otp_cfg_wadrsetup : std_logic_vector(3 downto 0);
    otp_cfg_wactive : std_logic_vector(31 downto 0);
    otp_cfg_whold : std_logic_vector(3 downto 0);
  end record;

  constant R_RESET : registers := (
    (others => '0'), (others => '0'), (others => '0'),
    (others => '0'), (others => '0'), (others => '0'),
    (others => '0'),
    ((others => '0'), (others => '0')),
    conv_std_logic_vector(2,4),  -- otp_cfg_rsetup: read address setup > 30 ns
    conv_std_logic_vector(2,4),  -- otp_cfg_wadrsetup: write address setup before 'we' pulse > 20 ns
    conv_std_logic_vector(4000000,32),  -- otp_cfg_wactive: 'we' pulse duration:
                                        -- more 50 ms  and less 100 ms (fclk = 80 MHz)
    conv_std_logic_vector(0,4)   -- otp_cfg_whold: change addres after we=0 > 10 ns (1 clock = 0)
  );

  signal r, rin : registers;
  --! @brief   Detector of the ADC clock.
  --! @details If this register won't equal to 0xFF, then we suppose RF front-end
  --!          not connected and FW should print message to enable 'i_int_clkrf'
  --!          jumper to make possible generation of the 1 msec interrupts.
  signal r_adc_detect : std_logic_vector(7 downto 0);

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
    i_clk => sys_clk,
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

  comblogic : process(nrst, slvcfg, mstcfg, r, r_adc_detect, i_otp_busy,
                      w_bus_re, wb_bus_raddr, wb_bus_waddr,
                      w_bus_we, wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable mstmap : master_config_map;
    variable slvmap : slave_config_map;
    variable raddr : integer;
    variable waddr : integer;
    variable vrdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable rtmp : std_logic_vector(31 downto 0);
    variable wtmp : std_logic_vector(31 downto 0);
  begin

    v := r;
    v.raddr := wb_bus_raddr;
    v.adc_detect := r_adc_detect;

    for k in 0 to CFG_BUS0_XMST_TOTAL-1 loop
      mstmap(2*k) := "00" & X"00000" & mstcfg(k).descrtype & mstcfg(k).descrsize;
      mstmap(2*k+1) := mstcfg(k).vid & mstcfg(k).did;
    end loop;

    for k in 0 to CFG_BUS0_XSLV_TOTAL-1 loop
      slvmap(4*k) := X"00" & 
                     slvcfg(k).irq_idx & "000000" &
                     slvcfg(k).descrtype & slvcfg(k).descrsize;
      slvmap(4*k+1) := slvcfg(k).vid & slvcfg(k).did;
      slvmap(4*k+2)   := slvcfg(k).xmask & X"000";
      slvmap(4*k+3) := slvcfg(k).xaddr & X"000";
    end loop;


    vrdata := (others => '0');
    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       raddr := conv_integer(r.raddr(n)(11 downto 2));

       rtmp := (others => '0');
       if raddr = 0 then 
          rtmp := hw_id;
       elsif raddr = 1 then 
          rtmp := r.fw_id;
       elsif raddr = 2 then 
          rtmp := r.adc_detect 
              & conv_std_logic_vector(CFG_BUS0_XMST_TOTAL,8)
              & conv_std_logic_vector(CFG_BUS0_XSLV_TOTAL,8)
              & conv_std_logic_vector(tech,8);
       elsif raddr = 3 then 
          -- reserved
       elsif raddr = 4 then 
          rtmp := r.idt(31 downto 0);
       elsif raddr = 5 then 
          rtmp := r.idt(63 downto 32);
       elsif raddr = 6 then 
          rtmp := r.malloc_addr(31 downto 0);
       elsif raddr = 7 then 
          rtmp := r.malloc_addr(63 downto 32);
       elsif raddr = 8 then 
          rtmp := r.malloc_size(31 downto 0);
       elsif raddr = 9 then 
          rtmp := r.malloc_size(63 downto 32);
       elsif raddr = 10 then 
          rtmp := r.fwdbg1(31 downto 0);
       elsif raddr = 11 then 
          rtmp := r.fwdbg1(63 downto 32);
       elsif raddr = 12 then 
          rtmp := r.fwdbg2(31 downto 0);
       elsif raddr = 13 then 
          rtmp := r.fwdbg2(63 downto 32);
       elsif raddr = 14 then 
          rtmp(0) := i_otp_busy;
          rtmp(11 downto 8) := r.otp_cfg_rsetup;
          rtmp(15 downto 12) := r.otp_cfg_wadrsetup;
          rtmp(19 downto 16) := r.otp_cfg_whold;
       elsif raddr = 15 then 
          rtmp := r.otp_cfg_wactive;
       elsif raddr >= 16 and raddr < 16+2*CFG_BUS0_XMST_TOTAL then
          rtmp := mstmap(raddr - 16);
       elsif raddr >= 16+2*CFG_BUS0_XMST_TOTAL 
             and raddr < 16+2*CFG_BUS0_XMST_TOTAL+4*CFG_BUS0_XSLV_TOTAL then
          rtmp := slvmap(raddr - 16 - 2*CFG_BUS0_XMST_TOTAL);
       end if;
       
       vrdata(32*(n+1)-1 downto 32*n) := rtmp;
    end loop;


    if w_bus_we = '1' then
      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         if conv_integer(wb_bus_wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
           waddr := conv_integer(wb_bus_waddr(n)(11 downto 2));
           wtmp := wb_bus_wdata(32*(n+1)-1 downto 32*n);

           case waddr is
             when 1 => v.fw_id := wtmp;
             when 4 => v.idt(31 downto 0) := wtmp;
             when 5 => v.idt(63 downto 32) := wtmp;
             when 6 => v.malloc_addr(31 downto 0) := wtmp;
             when 7 => v.malloc_addr(63 downto 32) := wtmp;
             when 8 => v.malloc_size(31 downto 0) := wtmp;
             when 9 => v.malloc_size(63 downto 32) := wtmp;
             when 10 => v.fwdbg1(31 downto 0) := wtmp;
             when 11 => v.fwdbg1(63 downto 32) := wtmp;
             when 12 => v.fwdbg2(31 downto 0) := wtmp;
             when 13 => v.fwdbg2(63 downto 32) := wtmp;
             when 14 =>
                 v.otp_cfg_rsetup := wtmp(11 downto 8);
                 v.otp_cfg_wadrsetup := wtmp(15 downto 12);
                 v.otp_cfg_whold := wtmp(19 downto 16);
             when 15 => v.otp_cfg_wactive := wtmp;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    if not async_reset and nrst = '0' then
        v := R_RESET;
    end if;
    
    rin <= v;
    wb_dev_rdata <= vrdata;
  end process;

  cfg <= xconfig;

  o_otp_cfg_rsetup <= r.otp_cfg_rsetup;
  o_otp_cfg_wadrsetup <= r.otp_cfg_wadrsetup;
  o_otp_cfg_wactive <= r.otp_cfg_wactive;
  o_otp_cfg_whold <= r.otp_cfg_whold;

  -- registers:
  regs : process(sys_clk, nrst)
  begin 
     if async_reset and nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(sys_clk) then 
        r <= rin;
     end if; 
  end process;

  -- ADC clock detector:
  regsadc : process(adc_clk, nrst)
  begin 
     if nrst = '0' then
        r_adc_detect <= (others => '0');
     elsif rising_edge(adc_clk) then 
        r_adc_detect <= r_adc_detect(6 downto 0) & nrst;
     end if; 
  end process;

end;
