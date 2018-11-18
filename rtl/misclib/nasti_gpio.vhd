--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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


entity nasti_gpio is
  generic (
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
	 xirq     : integer := 0;
	 width    : integer := 12
  );
  port (
    clk  : in std_logic;
    nrst : in std_logic;
    cfg  : out nasti_slave_config_type;
    i    : in  nasti_slave_in_type;
    o    : out nasti_slave_out_type;
    i_gpio : in std_logic_vector(width-1 downto 0);
	 o_gpio : out std_logic_vector(width-1 downto 0);
    o_gpio_dir : out std_logic_vector(width-1 downto 0)
  );
end; 
 
architecture arch_nasti_gpio of nasti_gpio is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(xirq, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_GPIO
  );

  type local_addr_array_type is array (0 to CFG_WORDS_ON_BUS-1) 
       of integer;

  type bank_type is record
    direction : std_logic_vector(31 downto 0);
    iuser : std_logic_vector(31 downto 0);
    ouser : std_logic_vector(31 downto 0);
    reg32_3 : std_logic_vector(31 downto 0);
  end record;
  
  type registers is record
    bank_axi : nasti_slave_bank_type;
    bank0 : bank_type;
  end record;

  constant RESET_VALUE : registers := (
        NASTI_SLAVE_BANK_RESET,
        ((others => '1'), (others => '0'), 
         (others => '0'), (others => '0'))
  );

  signal r, rin : registers;


begin

  comblogic : process(i, i_gpio, r, nrst)
    variable v : registers;
    variable raddr_reg : local_addr_array_type;
    variable waddr_reg : local_addr_array_type;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
    variable wstrb : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
  begin

    v := r;

    procedureAxi4(i, xconfig, r.bank_axi, v.bank_axi);

    for n in 0 to CFG_WORDS_ON_BUS-1 loop
      raddr_reg(n) := conv_integer(r.bank_axi.raddr(n)(11 downto 2));
      tmp := (others => '0');

      case raddr_reg(n) is
        when 0 => tmp := r.bank0.direction;
        when 1 => tmp := r.bank0.iuser;
        when 2 => tmp := r.bank0.ouser;
        when 3 => tmp := r.bank0.reg32_3;
        when others =>
      end case;
      rdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;


    if i.w_valid = '1' and 
       r.bank_axi.wstate = wtrans and 
       r.bank_axi.wresp = NASTI_RESP_OKAY then

       wstrb := i.w_strb;
       for n in 0 to CFG_WORDS_ON_BUS-1 loop
         waddr_reg(n) := conv_integer(r.bank_axi.waddr(n)(11 downto 2));
         tmp := i.w_data(32*(n+1)-1 downto 32*n);

         if conv_integer(wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
           case waddr_reg(n) is
             when 0 => v.bank0.direction := tmp;
             --when 1 => v.bank0.iuser := tmp;  -- [RO]
             when 2 => v.bank0.ouser := tmp;
             when 3 => v.bank0.reg32_3 := tmp;
             when others =>
           end case;
         end if;
       end loop;
    end if;

    o <= functionAxi4Output(r.bank_axi, rdata);

    v.bank0.iuser(width-1 downto 0) := i_gpio;
    
    if nrst = '0' then
        v := RESET_VALUE;
    end if;
  
    rin <= v;
  end process;

  cfg  <= xconfig;
  
  o_gpio <= r.bank0.ouser(width-1 downto 0);
  o_gpio_dir <= r.bank0.direction(width-1 downto 0);

  -- registers:
  regs : process(clk)
  begin 
     if rising_edge(clk) then 
        r <= rin;
     end if; 
  end process;

end;
