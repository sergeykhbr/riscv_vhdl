-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Entity nasti_pnp implementation for the plug'n'play support.
------------------------------------------------------------------------------

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


--! @brief Hardware Configuration storage with the AMBA AXI4 interface.
entity nasti_pnp is
  generic (
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    tech    : integer := 0
  );
  port (
    sys_clk : in  std_logic;
    adc_clk : in  std_logic;
    nrst   : in  std_logic;
    cfgvec : in  nasti_slave_cfg_vector;
    cfg    : out  nasti_slave_config_type;
    i      : in  nasti_slave_in_type;
    o      : out nasti_slave_out_type
  );
end; 
 
architecture arch_nasti_pnp of nasti_pnp is
  --! 4-bytes alignment so that all registers implemented as 32-bits
  --! width.
  constant ALIGNMENT_BYTES : integer := 8;
  --! Total bytes for each configuration structure.
  --! Firmware uses this value instead of sizeof(nasti_slave_config_type).
  constant PNP_CONFIG_DEFAULT_BYTES : std_logic_vector(7 downto 0) := X"10";

  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_PNP
  );

  type local_addr_array_type is array (0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1) 
       of integer;

  type bank_type is record
    idt : std_logic_vector(63 downto 0); --! debug counter
    malloc_addr : std_logic_vector(63 downto 0); --! dynamic allocation addr
    malloc_size : std_logic_vector(63 downto 0); --! dynamic allocation size
    fwdbg1 : std_logic_vector(63 downto 0); --! FW marker for the debug porposes
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
  end record;

signal r, rin : registers;
--! @brief   Detector of the ADC clock.
--! @details If this register won't equal to 0xFF, then we suppose RF front-end
--!          not connected and FW should print message to enable 'i_int_clkrf'
--!          jumper to make possible generation of the 1 msec interrupts.
signal r_adc_detect : std_logic_vector(7 downto 0);

begin

  comblogic : process(i, cfgvec, r, r_adc_detect)
    variable v : registers;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    variable val : std_logic_vector(8*ALIGNMENT_BYTES-1 downto 0);
  begin

    v := r;

    procedureAxi4(i, xconfig, r.bank_axi, v.bank_axi);

    for n in 0 to CFG_NASTI_DATA_BYTES/ALIGNMENT_BYTES-1 loop
       raddr_reg(n) := conv_integer(r.bank_axi.raddr(ALIGNMENT_BYTES*n)(11 downto log2(ALIGNMENT_BYTES)));

       val := X"badef00dcafecafe";
       if raddr_reg(n) = 0 then val := X"00000000" & X"20160115";
       elsif raddr_reg(n) = 1 then 
          val := X"00000000" & X"00" & r_adc_detect
              & conv_std_logic_vector(CFG_NASTI_SLAVES_TOTAL,8)
              & conv_std_logic_vector(tech,8);
       elsif raddr_reg(n) = 2 then val := r.bank0.idt;
       elsif raddr_reg(n) = 3 then val := r.bank0.malloc_addr;
       elsif raddr_reg(n) = 4 then val := r.bank0.malloc_size;
       elsif raddr_reg(n) = 5 then val := r.bank0.fwdbg1;
       else
         for k in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
             if raddr_reg(n) = 8+2*k then 
               val := cfgvec(k).xaddr & X"000" & cfgvec(k).xmask & X"000";
             elsif raddr_reg(n) = 8+2*k+1 then 
               val := X"000000" & PNP_CONFIG_DEFAULT_BYTES & cfgvec(k).vid & cfgvec(k).did;
             end if;
         end loop;
       end if;
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
             when 2 => v.bank0.idt := val;
             when 3 => v.bank0.malloc_addr := val;
             when 4 => v.bank0.malloc_size := val;
             when 5 => v.bank0.fwdbg1 := val;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    o <= functionAxi4Output(r.bank_axi, rdata);
    rin <= v;
  end process;

  cfg <= xconfig;

  -- registers:
  regs : process(sys_clk, nrst)
  begin 
     if nrst = '0' then
        r.bank_axi <= NASTI_SLAVE_BANK_RESET;
        r.bank0.idt <= (others => '0');
        r.bank0.malloc_addr <= (others => '0');
        r.bank0.malloc_size <= (others => '0');
        r.bank0.fwdbg1 <= (others => '0');
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
        r_adc_detect <= r_adc_detect(6 downto 0) & '1';
     end if; 
  end process;

end;
