-----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov - sergeykhbr@gmail.com
--! @brief      Hardware Configuration storage  with the AMBA AXI4 interface.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;
library commonlib;
use commonlib.types_common.all;
library rocketlib;
use rocketlib.types_nasti.all;


entity nasti_pnp is
  generic (
    xindex  : integer := 0;
    xaddr   : integer := 0;
    xmask   : integer := 16#fffff#;
    tech    : integer := 0
  );
  port (
    clk  : in  std_logic;
    nrst : in  std_logic;
    cfg  : in  nasti_slave_cfg_vector;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type
  );
end; 
 
architecture arch_nasti_pnp of nasti_pnp is
  --! 4-bytes alignment so that all registers implemented as 32-bits
  --! width.
  constant ALIGNMENT_BYTES : integer := 8;

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
    idt : std_logic_vector(63 downto 0); --! Interrupt descriptor table
  end record;

  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
  end record;

signal r, rin : registers;

begin

  comblogic : process(i, cfg, r)
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

       case raddr_reg(n) is
          when 0 => val := X"00000000" & X"20151108";
          when 1 => val := X"00000000" & X"000000" & conv_std_logic_vector(tech,8);
          when 2 => val := r.bank0.idt;
          --! Slave:0
          when 8 => val := cfg(0).xaddr & X"000" &cfg(0).xmask & X"000";
          when 9 => val := X"00000000" & cfg(0).vid & cfg(0).did;
          --! Slave:1
          when 16#a# => val := cfg(1).xaddr & X"000" &cfg(1).xmask & X"000";
          when 16#b# => val := X"00000000" & cfg(1).vid & cfg(1).did;
          --! Slave:2
          when 16#c# => val := cfg(2).xaddr & X"000" &cfg(2).xmask & X"000";
          when 16#d# => val := X"00000000" & cfg(2).vid & cfg(2).did;
          --! Slave:3
          when 16#e# => val := cfg(3).xaddr & X"000" &cfg(3).xmask & X"000";
          when 16#f# => val := X"00000000" & cfg(3).vid & cfg(3).did;
          --! Slave:4
          when 16#10# => val := cfg(4).xaddr & X"000" &cfg(4).xmask & X"000";
          when 16#11# => val := X"00000000" & cfg(4).vid & cfg(4).did;
          --! Slave:5
          when 16#12# => val := cfg(5).xaddr & X"000" &cfg(5).xmask & X"000";
          when 16#13# => val := X"00000000" & cfg(5).vid & cfg(5).did;
          when others => val := X"badef00dcafecafe";
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
             when 2 => v.bank0.idt := val;
             when others =>
           end case;
         end if;
      end loop;
    end if;

    o <= functionAxi4Output(r.bank_axi, rdata);
    rin <= v;
  end process;


  -- registers:
  regs : process(clk, nrst)
  begin 
     if nrst = '0' then
        r.bank_axi <= NASTI_SLAVE_BANK_RESET;
        r.bank0.idt <= (others => '0');
     elsif rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;
