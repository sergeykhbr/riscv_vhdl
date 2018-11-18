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
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    xirq   : integer := 0
  );
  port (
    nrst         : in std_logic;
    clk_bus      : in std_logic;
    clk_adc      : in std_logic;
    o_cfg        : out nasti_slave_config_type;
    i_axi        : in  nasti_slave_in_type;
    o_axi        : out nasti_slave_out_type;
    i_gps_I      : in std_logic_vector(1 downto 0);
    i_gps_Q      : in std_logic_vector(1 downto 0);
    i_glo_I      : in std_logic_vector(1 downto 0);
    i_glo_Q      : in std_logic_vector(1 downto 0);
    o_ms_pulse   : out std_logic;
    o_pps        : out std_logic
  );
end; 
 
architecture arch_gnssengine of gnssengine is
  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(xirq, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_ENGINE_STUB
  );

  type local_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1) 
       of integer;


  type bank_tmr_stub_type is record
    MsCnt          : std_logic_vector(31 downto 0); --! 
    TOW            : std_logic_vector(31 downto 0); --! 
    TOD            : std_logic_vector(31 downto 0); --! 
  end record;

  type adc_registers is record
    tmr  : bank_tmr_stub_type;
    clk_cnt        : integer; 
    irq : std_logic;
  end record;

  type bus_registers is record
    bank_axi : nasti_slave_bank_type;
    --! Registers clocked by system bus
    MsLength : std_logic_vector(31 downto 0); --! 
    CarrierNcoTh   : std_logic_vector(31 downto 0); --!
    CarrierNcoIF   : std_logic_vector(31 downto 0); --!
  end record;

signal ra, rain : adc_registers;
signal r, rin : bus_registers;

begin

  comblogic : process(nrst, i_axi, r, ra)
    variable v : bus_registers;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_ALIGN_BYTES-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
  begin

    v := r;

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);

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
          when 16#11# => tmp := ra.tmr.MsCnt;
          when 16#12# => tmp := ra.tmr.TOW;
          when 16#13# => tmp := ra.tmr.TOD;
          when others => 
       end case;
       rdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if i_axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(n)(11 downto 2));
         tmp := i_axi.w_data(32*(n+1)-1 downto 32*n);
         wstrb := i_axi.w_strb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n);

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

    if nrst = '0' then
        v.bank_axi := NASTI_SLAVE_BANK_RESET;
        v.MsLength := (others => '0');
        v.CarrierNcoIF := (others => '0');
        v.CarrierNcoTh := (others => '0');
    end if;

    o_axi <= functionAxi4Output(r.bank_axi, rdata);

    rin <= v;
  end process;

  o_cfg  <= xconfig;
  o_pps <= '0';
  o_ms_pulse <= ra.irq;

  -- registers:
  regadc : process(clk_adc)
  begin 
     if rising_edge(clk_adc) then
        ra.irq <= '0';
        if nrst = '0' then
            ra.tmr.TOW <= (others => '0');
            ra.tmr.TOD <= (others => '0');
            ra.tmr.MsCnt <= (others => '0');
            ra.clk_cnt <= 15000;
        elsif conv_integer(r.MsLength) /= 0 then
            if ra.clk_cnt = (conv_integer(r.MsLength) - 1) then
                ra.clk_cnt <= 0;
                ra.tmr.MsCnt <= ra.tmr.MsCnt + 1;
                ra.irq <= '1';
            else
                ra.clk_cnt <= ra.clk_cnt + 1;
            end if;
        end if;
     end if; 
  end process;

  regs : process(clk_bus) begin 
    if rising_edge(clk_bus) then 
       r <= rin; 
    end if;
  end process;

end;
