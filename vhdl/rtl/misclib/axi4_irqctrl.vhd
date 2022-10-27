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
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
library misclib;
use misclib.types_misc.all;

entity axi4_irqctrl is
  generic (
    async_reset : boolean := false;
    xaddr : integer := 0;
    xmask : integer := 16#fffff#
  );
  port 
 (
    clk    : in std_logic;
    nrst   : in std_logic;
    i_irqs : in std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);
    o_cfg  : out axi4_slave_config_type;
    i_axi  : in axi4_slave_in_type;
    o_axi  : out axi4_slave_out_type;
    o_irq_meip : out std_logic
  );
end;

architecture axi4_irqctrl_rtl of axi4_irqctrl is

  constant xconfig : axi4_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => conv_std_logic_vector(0, 8),
     xaddr => conv_std_logic_vector(xaddr, CFG_SYSBUS_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_SYSBUS_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_IRQCTRL
  );

  constant IRQ_ZERO : std_logic_vector(CFG_IRQ_TOTAL-1 downto 1) := (others => '0');

  type registers is record
    --! interrupt signal delay signal to detect interrupt positive edge
    irqs_z        : std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);  
    irqs_zz       : std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);  

    --! mask irq disabled: 1=disabled; 0=enabled
    irqs_mask     : std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);
    --! irq pending bit mask
    irqs_pending  : std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);
    --! interrupt handler address initialized by FW:
    isr_table     : std_logic_vector(63 downto 0);
    --! hold-on generation of interrupt.
    irq_lock      : std_logic;
    --! delayed interrupt
    irq_wait_unlock : std_logic_vector(CFG_IRQ_TOTAL-1 downto 1);
    irq_cause_idx : std_logic_vector(31 downto 0);
    --! Function trap_entry copies the values of CSRs into these two regs:
    dbg_cause    : std_logic_vector(63 downto 0);
    dbg_epc      : std_logic_vector(63 downto 0);
    raddr : global_addr_array_type;
  end record;

  constant R_RESET : registers := (
    (others => '0'), (others => '0'),  -- irqs_z, irqs_zz
    (others => '1'), (others => '0'),  -- irqs_mask, irqs_pending
    (others => '0'), '0',              -- isr_table, isr_lock
    (others => '0'), (others => '0'),  -- irq_wait_unlock, irq_cause_idx
    (others => '0'), (others => '0'),  -- dbg_cause, dbg_epc
    ((others => '0'), (others => '0'))
  );

  signal r, rin: registers;

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

  comblogic : process(nrst, i_irqs, r, w_bus_re, wb_bus_raddr, wb_bus_waddr,
                      w_bus_we, wb_bus_wstrb, wb_bus_wdata)
    variable v : registers;
    variable raddr : integer;
    variable waddr : integer;
    variable vrdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable tmp : std_logic_vector(31 downto 0);
    variable wstrb : std_logic_vector(CFG_ALIGN_BYTES-1 downto 0);
    variable w_generate_ipi : std_logic;
  begin
    v := r;
    v.raddr := wb_bus_raddr;
    w_generate_ipi := '0';

    vrdata := (others => '0');
    for n in 0 to CFG_WORDS_ON_BUS-1 loop
       raddr := conv_integer(r.raddr(n)(11 downto 2));
       tmp := (others => '0');
       case raddr is
         when 0      => tmp(CFG_IRQ_TOTAL-1 downto 1) := r.irqs_mask;     --! [RW]: 1=irq disable; 0=enable
         when 1      => tmp(CFG_IRQ_TOTAL-1 downto 1) := r.irqs_pending;  --! [RO]: Rised interrupts.
         when 2      => tmp := (others => '0');                           --! [WO]: Clear interrupts mask.
         when 3      => tmp := (others => '0');                           --! [WO]: Rise interrupts mask.
         when 4      => tmp := r.isr_table(31 downto 0);                  --! [RW]: LSB of the function address
         when 5      => tmp := r.isr_table(63 downto 32);                 --! [RW]: MSB of the function address
         when 6      => tmp := r.dbg_cause(31 downto 0);                  --! [RW]: Cause of the interrupt
         when 7      => tmp := r.dbg_cause(63 downto 32);                 --! [RW]: 
         when 8      => tmp := r.dbg_epc(31 downto 0);                    --! [RW]: Instruction pointer
         when 9      => tmp := r.dbg_epc(63 downto 32);                   --! [RW]: 
         when 10     => tmp(0) := r.irq_lock;
         when 11     => tmp := r.irq_cause_idx;
         when others =>
       end case;
       vrdata(8*CFG_ALIGN_BYTES*(n+1)-1 downto 8*CFG_ALIGN_BYTES*n) := tmp;
    end loop;

    if w_bus_we = '1' then
      for n in 0 to CFG_WORDS_ON_BUS-1 loop
         if conv_integer(wb_bus_wstrb(CFG_ALIGN_BYTES*(n+1)-1 downto CFG_ALIGN_BYTES*n)) /= 0 then
             waddr := conv_integer(wb_bus_waddr(n)(11 downto 2));
             tmp := wb_bus_wdata(32*(n+1)-1 downto 32*n);

             case waddr is
             when 0 => v.irqs_mask := tmp(CFG_IRQ_TOTAL-1 downto 1);
             when 1 =>     --! Read only
             when 2 => 
                v.irqs_pending := r.irqs_pending and (not tmp(CFG_IRQ_TOTAL-1 downto 1));
             when 3 => 
                w_generate_ipi := '1';
                v.irqs_pending := (not r.irqs_mask) and tmp(CFG_IRQ_TOTAL-1 downto 1);
             when 4 => v.isr_table(31 downto 0) := tmp;
             when 5 => v.isr_table(63 downto 32) := tmp;
             when 6 => v.dbg_cause(31 downto 0) := tmp;
             when 7 => v.dbg_cause(63 downto 32) := tmp;
             when 8 => v.dbg_epc(31 downto 0) := tmp;
             when 9 => v.dbg_epc(63 downto 32) := tmp;
             when 10 => v.irq_lock := tmp(0);
             when 11 => v.irq_cause_idx := tmp;
             when others =>
             end case;
         end if;
      end loop;
    end if;

    v.irqs_z := i_irqs;
    v.irqs_zz := r.irqs_z;
    for n in 1 to CFG_IRQ_TOTAL-1 loop
      if (r.irqs_z(n) = '1' and r.irqs_zz(n) = '0') or r.irq_wait_unlock(n) = '1' then
         if r.irq_lock = '0' then
             v.irq_wait_unlock(n) := '0';
             v.irqs_pending(n) := not r.irqs_mask(n);
             w_generate_ipi := w_generate_ipi or (not r.irqs_mask(n));
         else
             v.irq_wait_unlock(n) := '1';
         end if;
      end if;
    end loop;


    if r.irqs_pending = IRQ_ZERO or r.irq_lock = '1' then
      o_irq_meip <= '0';
    else
      o_irq_meip <= '1';
    end if;

    if not async_reset and nrst = '0' then 
       v := R_RESET;
    end if;

    rin <= v;
    wb_dev_rdata <= vrdata;
  end process;

  o_cfg  <= xconfig;


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
