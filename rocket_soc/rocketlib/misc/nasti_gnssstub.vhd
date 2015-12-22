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
library rocketlib;
use rocketlib.types_nasti.all;


entity nasti_gnssstub is
  generic (
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#
  );
  port (
    clk  : in  std_logic;
    nrst : in  std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type;
    irq  : out std_logic
  );
end; 
 
architecture arch_nasti_gnssstub of nasti_gnssstub is
  --! 4-bytes alignment so that all registers implemented as 32-bits
  --! width.
  constant ALIGNMENT_BYTES : integer := 4;

  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_ENGINE
  );

  type local_addr_array_type is array (0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1) 
       of integer;

  type bank_misc_stub_type is record
    CarrierNcoTh   : std_logic_vector(31 downto 0); --!
    CarrierNcoIF   : std_logic_vector(31 downto 0); --!
  end record;

  type bank_tmr_stub_type is record
    MsLength       : std_logic_vector(31 downto 0); --! 
    MsCnt          : std_logic_vector(31 downto 0); --! 
    TOW            : std_logic_vector(31 downto 0); --! 
    TOD            : std_logic_vector(31 downto 0); --! 
  end record;

  type bank_type is record
    misc : bank_misc_stub_type;
    tmr  : bank_tmr_stub_type;
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
    clk_cnt : integer; --! 
  end record;

signal r, rin : registers;

begin

  comblogic : process(i, r)
    variable v : registers;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    variable val : std_logic_vector(8*ALIGNMENT_BYTES-1 downto 0);

    variable rise_irq : std_logic;
  begin

    v := r;
    rise_irq := '0';

    if conv_integer(r.bank0.tmr.MsLength) /= 0 then
       if (r.clk_cnt + 1) = conv_integer(r.bank0.tmr.MsLength) then
         v.clk_cnt := 0;
         rise_irq := '1';
         v.bank0.tmr.MsCnt := r.bank0.tmr.MsCnt + 1;
       else
         v.clk_cnt := r.clk_cnt + 1;
      end if;
    end if;

    procedureAxi4(i, xconfig, r.bank_axi, v.bank_axi);

    for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
       raddr_reg(n) := conv_integer(r.bank_axi.raddr(ALIGNMENT_BYTES*n)(11 downto log2(ALIGNMENT_BYTES)));

       case raddr_reg(n) is
          --! Misc. bank (stub):
          when 0 => val := X"B00BCAFE";  --! hwid of the stub
          when 1 => val := X"00000021";  --! gnss channels configuration stub
          when 2 => val := r.bank0.misc.CarrierNcoTh; --!
          when 3 => val := r.bank0.misc.CarrierNcoIF; --!
          --! Global Timers bank (stub):
          when 16#10# => val := r.bank0.tmr.MsLength;
          when 16#11# => val := r.bank0.tmr.MsCnt;
          when 16#12# => val := r.bank0.tmr.TOW;
          when 16#13# => val := r.bank0.tmr.TOD;
          when others => val := X"cafef00d";
       end case;
       rdata(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n) := val;
    end loop;


    if i.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      wdata := i.w_data;
      wstrb := i.w_strb;
      for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(ALIGNMENT_BYTES*n)(11 downto log2(ALIGNMENT_BYTES)));

         if conv_integer(wstrb(ALIGNMENT_BYTES*(n+1)-1 downto ALIGNMENT_BYTES*n)) /= 0 then
           val := wdata(8*ALIGNMENT_BYTES*(n+1)-1 downto 8*ALIGNMENT_BYTES*n);
           case waddr_reg(n) is
             when 2 => v.bank0.misc.CarrierNcoTh := val;
             when 3 => v.bank0.misc.CarrierNcoIF := val;
             when 16#10# => v.bank0.tmr.MsLength := val;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    irq <= rise_irq;
    o <= functionAxi4Output(r.bank_axi, rdata);
    rin <= v;
  end process;

  cfg  <= xconfig;

  -- registers:
  regs : process(clk, nrst)
  begin 
     if nrst = '0' then
        r.bank_axi <= NASTI_SLAVE_BANK_RESET;
        r.bank0.tmr.MsLength <= (others => '0');
        r.bank0.tmr.MsCnt <= (others => '0');
        r.clk_cnt <= 69000;
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;
