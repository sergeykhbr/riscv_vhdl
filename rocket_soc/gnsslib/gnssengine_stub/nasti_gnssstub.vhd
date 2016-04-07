-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Stub module of the real GNSS engine.
--! @details    This module is used for SoC sharing and it generates 1 msec
--!             interrupt. It implements AMBA AXI4 interface.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! GNSS specific library.
library gnsslib;
use gnsslib.types_gnss.all;


entity gnssengine is
  generic (
    tech   : integer range 0 to NTECH := 0;
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#
  );
  port (
    i : in gns_in_type;
    o : out gns_out_type
  );
end; 
 
architecture arch_gnssengine of gnssengine is
  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_ENGINE_STUB,
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES
  );

  type local_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1) 
       of integer;


  type bank_tmr_stub_type is record
    MsCnt          : std_logic_vector(31 downto 0); --! 
    TOW            : std_logic_vector(31 downto 0); --! 
    TOD            : std_logic_vector(31 downto 0); --! 
  end record;

  type adc_bank_type is record
    tmr  : bank_tmr_stub_type;
    clk_cnt        : integer; --! 
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank_adc : adc_bank_type;
    --! Registers clocked by system bus
    MsLength : std_logic_vector(31 downto 0); --! 
    CarrierNcoTh   : std_logic_vector(31 downto 0); --!
    CarrierNcoIF   : std_logic_vector(31 downto 0); --!
  end record;

signal r, rin : registers;

begin

  comblogic : process(i, r)
    variable v : registers;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_ALIGN_BYTES-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);

    variable rise_irq : std_logic;
  begin

    v := r;
    rise_irq := '0';

    if conv_integer(r.MsLength) /= 0 then
       if (r.bank_adc.clk_cnt + 1) = conv_integer(r.MsLength) then
         v.bank_adc.clk_cnt := 0;
         rise_irq := '1';
         v.bank_adc.tmr.MsCnt := r.bank_adc.tmr.MsCnt + 1;
       else
         v.bank_adc.clk_cnt := r.bank_adc.clk_cnt + 1;
      end if;
    end if;

    procedureAxi4(i.axi, xconfig, r.bank_axi, v.bank_axi);

    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       raddr_reg(n) := conv_integer(r.bank_axi.raddr(n)(11 downto 2));
       tmp := (others => '0');

       case raddr_reg(n) is
          --! Misc. bank (stub):
          when 0 => tmp := X"B00BCAFE";  --! hwid of the stub
          when 1 => tmp := X"00000021";  --! gnss channels configuration stub
          when 2 => tmp := r.CarrierNcoTh; --!
          when 3 => tmp := r.CarrierNcoIF; --!
          --! Global Timers bank (stub):
          when 16#10# => tmp := r.MsLength;
          when 16#11# => tmp := r.bank_adc.tmr.MsCnt;
          when 16#12# => tmp := r.bank_adc.tmr.TOW;
          when 16#13# => tmp := r.bank_adc.tmr.TOD;
          when others => 
       end case;
       rdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if i.axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(n)(11 downto 2));
         tmp := i.axi.w_data(32*(n+1)-1 downto 32*n);
         wstrb := i.axi.w_strb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n);

         if conv_integer(wstrb) /= 0 then
           case waddr_reg(n) is
             when 2 => v.CarrierNcoTh := tmp;
             when 3 => v.CarrierNcoIF := tmp;
             when 16#10# => v.MsLength := tmp;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    o.ms_pulse <= rise_irq;
    o.pps <= '0';
    o.axi <= functionAxi4Output(r.bank_axi, rdata);
    rin <= v;
  end process;

  o.cfg  <= xconfig;

  -- registers:
  regadc : process(i.clk_adc, i.nrst)
  begin 
     if i.nrst = '0' then
        r.bank_adc.tmr.MsCnt <= (others => '0');
        r.bank_adc.clk_cnt <= 15000;
     elsif rising_edge(i.clk_adc) then 
        r.bank_adc <= rin.bank_adc;
     end if; 
  end process;

  regs : process(i.clk_bus, i.nrst)
  begin 
     if i.nrst = '0' then
        r.bank_axi <= NASTI_SLAVE_BANK_RESET;
        r.MsLength <= (others => '0');
     elsif rising_edge(i.clk_bus) then 
        r.bank_axi <= rin.bank_axi;
        r.MsLength <= rin.MsLength;
     end if; 
  end process;

end;
