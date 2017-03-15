-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      General Purpose Timers with the AXI4 interface.
------------------------------------------------------------------------------

--! @defgroup gptimers_page General Purpose Timers
--! @ingroup peripheries_group
--! 
--! @section gptimers_overview GPTimers overview
--!
--! This GPTimers implementation can be additionally configured using the following
--! generic parameters.
--!
--! | Name | Default   | Description
--! |:-----|:---------:|:------------
--! |irqx  | 0         | <b>Interrupt pin index</b> This value is used only as argument in output Plug'n'Play configuration.
--! |tmr_total| 2      | <b>Total Number of Timers.</b> Each timer is the 64-bits counter that can be used for interrupt generation or without.
--!
--! @section gptimers_regs GPTimers registers mapping
--! GPTimers device acts like a slave AMBA AXI4 device that is directly mapped
--! into physical memory. Default address location for our implementation 
--! is defined by 0x80005000. Memory size is 4 KB.
--!
--! @par High Precision Timer register (Least Word) (0x000).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64   | RW | 64h'0 | highcnt  | 63:0  | <b>High precision counter</b>. This counter isn't used as a source of interrupt and cannot be stopped from SW.
--!
--! @par High Precision Timer register (Most Word) (0x004).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64   | RW | 64h'0 | highcnt  | 63:0  | <b>High precision counter</b>. This counter isn't used as a source of interrupt and cannot be stopped from SW.
--!
--! @par Pending Timer IRQ register (0x008).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 32-tmr_total | RW | 0 | reserved  | 31:tmr_total  | Reserved.
--! | tmr_total  | RW | 0 | pending  | tmr_total-1:0  | <b>Pending Bit</b>. Each timer can be configured to generate interrupt. Simaltenously with interrupt is rising pending bit that has to be lowed by Software.
--!
--! @par Timer[0] Control register (0x040).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 30 | RW | 30h'0 | reserved  | 31:2  | Reserved.
--! | 1  | RW | 1b'0 | irq_ena    | 1  | <b>Interrupt Enable</b>. Enable the interrupt generation when the timer reaches zero value.
--! | 0  | RW | 1b'0 | count_ena  | 0  | <b>Count Enable</b>. Enable/Disable counter.
--!
--! @par Timer[0] Current Value register (0x048).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | value  | 63:0  | <b>Timer Value</b>. Read/Write register with counter's value. When it equals to 0 the 'init_value' will be used to re-initialize counter.
--!
--! @par Timer[0] Init Value register (0x050).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | init_value  | 63:0  | <b>Timer Init Value</b>. Read/Write register is used for cycle timer re-initializtion. If init_value = 0 and value != 0 then the timer is used as a 'single shot' timer.
--!
--! @par Timer[1] Control register (0x060 = 0x040 + Idx * 32).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 30 | RW | 30h'0 | reserved  | 31:2  | Reserved.
--! | 1  | RW | 1b'0 | irq_ena    | 1  | <b>Interrupt Enable</b>. Enable the interrupt generation when the timer reaches zero value.
--! | 0  | RW | 1b'0 | count_ena  | 0  | <b>Count Enable</b>. Enable/Disable counter.
--!
--! @par Timer[1] Current Value register (0x068 = 0x48 + Idx * 32).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | value  | 63:0  | <b>Timer Value</b>. Read/Write register with counter's value. When it equals to 0 the 'init_value' will be used to re-initialize counter.
--!
--! @par Timer[1] Init Value register (0x070 = 0x050 + Idx * 32).
--!
--! | Bits |Type| Reset |Field Name| Bits  | Description 
--! |:----:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | init_value  | 63:0  | <b>Timer Init Value</b>. Read/Write register is used for cycle timer re-initializtion. If init_value = 0 and value != 0 then the timer is used as a 'single shot' timer.
--!


library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
library rocketlib;
use rocketlib.types_rocket.all;

entity nasti_gptimers is
  generic (
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    xirq    : integer := 0;
    tmr_total  : integer := 2
  );
  port (
    clk    : in  std_logic;
    nrst   : in  std_logic;
    cfg    : out nasti_slave_config_type;
    i_axi  : in  nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_irq  : out std_logic
  );
end; 
 
architecture arch_nasti_gptimers of nasti_gptimers is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => xirq,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_GPTIMERS
  );

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type timer_type is record
        count_ena : std_logic;
        irq_ena   : std_logic;
        value : std_logic_vector(63 downto 0);
        init_value : std_logic_vector(63 downto 0);
  end record;
  
  constant timer_type_reset : timer_type := 
      ('0', '0', (others => '0'), (others => '0'));

  type vector_timer_type is array (0 to tmr_total-1) of timer_type;

  type bank_type is record
        tmr  : vector_timer_type;
        highcnt : std_logic_vector(63 downto 0);
        pending : std_logic_vector(tmr_total-1 downto 0);
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
  end record;

signal r, rin : registers;

begin

  comblogic : process(nrst, i_axi, r)
    variable v : registers;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
    variable raddr : integer;
    variable waddr : integer;

    variable irq_ena : std_logic;
  begin

    v := r;

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);

    v.bank0.highcnt := r.bank0.highcnt + 1;

    irq_ena := '0';
    for n in 0 to tmr_total-1 loop
        if r.bank0.tmr(n).count_ena = '1' then 
           if r.bank0.tmr(n).value = zero64 then
               irq_ena := irq_ena or r.bank0.tmr(n).irq_ena;
               v.bank0.pending(n) := r.bank0.tmr(n).irq_ena;
               v.bank0.tmr(n).value := r.bank0.tmr(n).init_value;
           else
               v.bank0.tmr(n).value := r.bank0.tmr(n).value - 1;
           end if;
        else
           v.bank0.tmr(n).value := r.bank0.tmr(n).init_value;
        end if;
    end loop;


    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       tmp := (others => '0');
       raddr := conv_integer(r.bank_axi.raddr(n)(11 downto 2));
       case raddr is
          when 0 => 
                tmp := r.bank0.highcnt(31 downto 0);
          when 1 => 
                tmp := r.bank0.highcnt(63 downto 32);
          when 2 => 
                tmp(tmr_total-1 downto 0) := r.bank0.pending;
          when others => 
                for k in 0 to tmr_total-1 loop
                   if raddr = (16 + 8*k) then
                      tmp(0) := r.bank0.tmr(k).count_ena;
                      tmp(1) := r.bank0.tmr(k).irq_ena;
                   elsif raddr = (16 + 8*k + 2) then
                      tmp := r.bank0.tmr(k).value(31 downto 0);
                   elsif raddr = (16 + 8*k + 3) then
                      tmp := r.bank0.tmr(k).value(63 downto 32);
                   elsif raddr = (16 + 8*k + 4) then
                      tmp := r.bank0.tmr(k).init_value(31 downto 0);
                   elsif raddr = (16 + 8*k + 5) then
                      tmp := r.bank0.tmr(k).init_value(63 downto 32);
                   end if;
                end loop;
       end case;
       rdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if i_axi.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

      wstrb := i_axi.w_strb;
      for n in 0 to CFG_WORDS_ON_BUS-1 loop

         if conv_integer(wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
           tmp := i_axi.w_data(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n);
           waddr := conv_integer(r.bank_axi.waddr(n)(11 downto 2));
           case waddr is
             when 2 => 
                    v.bank0.pending := tmp(tmr_total-1 downto 0);
             when others =>
                for k in 0 to tmr_total-1 loop
                   if waddr = (16 + 8*k) then
                      v.bank0.tmr(k).count_ena := tmp(0);
                      v.bank0.tmr(k).irq_ena := tmp(1);
                   elsif waddr = (16 + 8*k + 2) then
                      v.bank0.tmr(k).value(31 downto 0) := tmp;
                   elsif waddr = (16 + 8*k + 3) then
                      v.bank0.tmr(k).value(63 downto 32) := tmp;
                   elsif waddr = (16 + 8*k + 4) then
                      v.bank0.tmr(k).init_value(31 downto 0) := tmp;
                   elsif waddr = (16 + 8*k + 5) then
                      v.bank0.tmr(k).init_value(63 downto 32) := tmp;
                   end if;
                end loop;
           end case;
         end if;
      end loop;
    end if;

    if nrst = '0' then
        v.bank_axi := NASTI_SLAVE_BANK_RESET;
        v.bank0.highcnt := (others => '0');
        v.bank0.pending := (others => '0');
        for k in 0 to tmr_total-1 loop
           v.bank0.tmr(k) := timer_type_reset;
        end loop;
    end if;

    o_axi <= functionAxi4Output(r.bank_axi, rdata);
    o_irq <= irq_ena;
    rin <= v;
  end process;

  cfg <= xconfig;

  -- registers:
  regs : process(clk)
  begin 
     if rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;
end;
