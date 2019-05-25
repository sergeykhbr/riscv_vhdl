-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @details   Implementation of the axictrl device. 
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;

--! @brief   AXI (NASTI) bus controller. 
--! @details Simplified version with the hardcoded priorities to bus access.
--!          Lower master index has a higher priority.
--! @todo    Round-robin algorithm for the master selection.
entity axictrl is
  generic (
    watchdog_memop : integer := 0
  );
  port (
    i_clk    : in std_logic;
    i_nrst   : in std_logic;
    i_slvcfg : in  nasti_slave_cfg_vector;
    i_slvo   : in  nasti_slaves_out_vector;
    i_msto   : in  nasti_master_out_vector;
    o_slvi   : out nasti_slave_in_vector;
    o_msti   : out nasti_master_in_vector;
    o_miss_irq  : out std_logic;
    o_miss_addr : out std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    o_bus_util_w : out std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    o_bus_util_r : out std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0)
  );
end; 
 
architecture arch_axictrl of axictrl is

  constant MISS_ACCESS_SLAVE : nasti_slave_out_type := (
      '1', '1', '1', NASTI_RESP_EXOKAY,
      (others=>'0'), '0', '1', '1', NASTI_RESP_EXOKAY, (others=>'1'), 
      '1', (others=>'0'), '0');

  type nasti_master_out_vector_miss is array (0 to CFG_NASTI_MASTER_TOTAL) 
       of nasti_master_out_type;

  type nasti_master_in_vector_miss is array (0 to CFG_NASTI_MASTER_TOTAL) 
       of nasti_master_in_type;

  type nasti_slave_out_vector_miss is array (0 to CFG_NASTI_SLAVES_TOTAL) 
       of nasti_slave_out_type;

  type nasti_slave_in_vector_miss is array (0 to CFG_NASTI_SLAVES_TOTAL) 
       of nasti_slave_in_type;
       
  type slave_selector_type is array (0 to CFG_NASTI_MASTER_TOTAL-1)
      of integer range 0 to CFG_NASTI_SLAVES_TOTAL;

  type reg_type is record
     w_busy : std_logic;
     w_mst_idx : integer range 0 to CFG_NASTI_MASTER_TOTAL;
     w_slv_idx : integer range 0 to CFG_NASTI_SLAVES_TOTAL; -- +1 miss access
     r_busy : std_logic;
     r_mst_idx : integer range 0 to CFG_NASTI_MASTER_TOTAL;
     r_slv_idx : integer range 0 to CFG_NASTI_SLAVES_TOTAL; -- +1 miss access
     b_busy : std_logic;
     b_mst_idx : integer range 0 to CFG_NASTI_MASTER_TOTAL;
     b_slv_idx : integer range 0 to CFG_NASTI_SLAVES_TOTAL; -- +1 miss access

     miss_addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
  end record;

  signal rin, r : reg_type;
begin

  comblogic : process(i_nrst, i_slvcfg, i_msto, i_slvo, r)
    variable v : reg_type;
    variable missaccess : std_logic;
    variable ar_mst_idx : integer range 0 to CFG_NASTI_MASTER_TOTAL;
    variable aw_mst_idx : integer range 0 to CFG_NASTI_MASTER_TOTAL;
    variable ar_slv_idx : integer range 0 to CFG_NASTI_SLAVES_TOTAL; -- +1 miss access
    variable aw_slv_idx : integer range 0 to CFG_NASTI_SLAVES_TOTAL; -- +1 miss access
    variable vmsto : nasti_master_out_vector_miss;
    variable vmsti : nasti_master_in_vector_miss;
    variable vslvi : nasti_slave_in_vector_miss;
    variable vslvo : nasti_slave_out_vector_miss;
    variable aw_fire : std_logic;
    variable ar_fire : std_logic;
    variable w_fire : std_logic;
    variable w_available : std_logic;
    variable r_fire : std_logic;
    variable r_available : std_logic;
    variable b_fire : std_logic;    
    variable b_available : std_logic;
    -- Bus statistic signals
    variable wb_bus_util_w : std_logic_vector(CFG_NASTI_MASTER_TOTAL downto 0);
    variable wb_bus_util_r : std_logic_vector(CFG_NASTI_MASTER_TOTAL downto 0);

  begin

    v := r;

    missaccess := '0';
    wb_bus_util_w := (others => '0');
    wb_bus_util_r := (others => '0');
    ar_mst_idx := CFG_NASTI_MASTER_TOTAL;   -- Default master is empty master
    aw_mst_idx := CFG_NASTI_MASTER_TOTAL;   -- Default master is empty master
    ar_slv_idx := CFG_NASTI_SLAVES_TOTAL;   -- miss access index
    aw_slv_idx := CFG_NASTI_SLAVES_TOTAL;   -- miss access index

    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
       vmsto(n) := i_msto(n);
       vmsti(n) := nasti_master_in_none;
    end loop;
    vmsto(CFG_NASTI_MASTER_TOTAL) := nasti_master_out_none;
    vmsti(CFG_NASTI_MASTER_TOTAL) := nasti_master_in_none;
    for k in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
       vslvo(k) := i_slvo(k);
       vslvi(k) := nasti_slave_in_none;
    end loop;
    vslvo(CFG_NASTI_SLAVES_TOTAL) := MISS_ACCESS_SLAVE;
    vslvi(CFG_NASTI_SLAVES_TOTAL) := nasti_slave_in_none;

    -- Select master bus:
    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
       if i_msto(n).ar_valid = '1' then 
          ar_mst_idx := n;
       end if;
       if i_msto(n).aw_valid = '1' then
          aw_mst_idx := n;
       end if;
    end loop;

    -- Adress Read/Address Write channels
    for k in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
      if i_slvcfg(k).xmask /= X"00000" and
         (vmsto(ar_mst_idx).ar_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) 
          and i_slvcfg(k).xmask) = i_slvcfg(k).xaddr then
          ar_slv_idx := k;
      end if;
      if i_slvcfg(k).xmask /= X"00000" and
         (vmsto(aw_mst_idx).aw_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12)
          and i_slvcfg(k).xmask) = i_slvcfg(k).xaddr then
          aw_slv_idx := k;
      end if;
    end loop;
    
    -- Statistic
    wb_bus_util_w(r.w_mst_idx) := r.w_busy;
    wb_bus_util_r(r.r_mst_idx) := r.r_busy;

    -- Pipeline
    --! @todo It is possible to implement inpendent channels of same type for each
    --!       master slot and process transactions to separate slaves simultaneously.
    --!       I think it will be usefull feature but doesn't increase CPI of 
    --!       single core.

    -- Read Channel:
    r_fire := vmsto(r.r_mst_idx).r_ready and vslvo(r.r_slv_idx).r_valid and vslvo(r.r_slv_idx).r_last;
    r_available := not r.r_busy or (r.r_busy and r_fire);
    ar_fire := vmsto(ar_mst_idx).ar_valid and vslvo(ar_slv_idx).ar_ready and r_available;
    -- Write channel:
    w_fire := vmsto(r.w_mst_idx).w_valid and vslvo(r.w_slv_idx).w_ready and vmsto(r.w_mst_idx).w_last;
    w_available := not r.w_busy or (r.w_busy and w_fire);
    aw_fire := vmsto(aw_mst_idx).aw_valid and vslvo(aw_slv_idx).aw_ready and w_available;

    vmsti(ar_mst_idx).ar_ready := vslvo(ar_slv_idx).ar_ready and r_available;
    vslvi(ar_slv_idx).ar_valid := vmsto(ar_mst_idx).ar_valid and r_available;
    vslvi(ar_slv_idx).ar_bits  := vmsto(ar_mst_idx).ar_bits;
    vslvi(ar_slv_idx).ar_id    := vmsto(ar_mst_idx).ar_id;
    vslvi(ar_slv_idx).ar_user  := vmsto(ar_mst_idx).ar_user;

    vmsti(aw_mst_idx).aw_ready := vslvo(aw_slv_idx).aw_ready and w_available;
    vslvi(aw_slv_idx).aw_valid := vmsto(aw_mst_idx).aw_valid and w_available;
    vslvi(aw_slv_idx).aw_bits  := vmsto(aw_mst_idx).aw_bits;
    vslvi(aw_slv_idx).aw_id    := vmsto(aw_mst_idx).aw_id;
    vslvi(aw_slv_idx).aw_user  := vmsto(aw_mst_idx).aw_user;

    if (w_available and aw_fire) = '1' then
        v.w_busy := '1';
        v.w_slv_idx := aw_slv_idx;
        v.w_mst_idx := aw_mst_idx;
    end if;

    vmsti(r.w_mst_idx).w_ready := vslvo(r.w_slv_idx).w_ready;
    vslvi(r.w_slv_idx).w_valid := vmsto(r.w_mst_idx).w_valid;
    vslvi(r.w_slv_idx).w_data := vmsto(r.w_mst_idx).w_data;
    vslvi(r.w_slv_idx).w_last := vmsto(r.w_mst_idx).w_last;
    vslvi(r.w_slv_idx).w_strb := vmsto(r.w_mst_idx).w_strb;
    vslvi(r.w_slv_idx).w_user := vmsto(r.w_mst_idx).w_user;

    -- Write Handshake channel:
    b_fire := vmsto(r.b_mst_idx).b_ready and vslvo(r.b_slv_idx).b_valid;
    b_available := not r.b_busy or (r.b_busy and b_fire);
    if (b_available and w_fire) = '1' then
        v.w_busy := '0';
        v.b_busy := '1';
        v.b_slv_idx := r.w_slv_idx;
        v.b_mst_idx := r.w_mst_idx;
    end if;    
    if b_fire = '1' then
        v.b_busy := w_fire;
    end if;

    vmsti(r.b_mst_idx).b_valid := vslvo(r.b_slv_idx).b_valid;
    vmsti(r.b_mst_idx).b_resp := vslvo(r.b_slv_idx).b_resp;
    vmsti(r.b_mst_idx).b_id := vslvo(r.b_slv_idx).b_id;
    vmsti(r.b_mst_idx).b_user := vslvo(r.b_slv_idx).b_user;
    vslvi(r.b_slv_idx).b_ready := vmsto(r.b_mst_idx).b_ready;

    if (r_available and ar_fire) = '1' then
        v.r_busy := '1';
        v.r_slv_idx := ar_slv_idx;
        v.r_mst_idx := ar_mst_idx;
    end if;
    if r_fire = '1' then
        v.r_busy := ar_fire;
    end if;

    vmsti(r.r_mst_idx).r_valid := vslvo(r.r_slv_idx).r_valid;
    vmsti(r.r_mst_idx).r_resp  := vslvo(r.r_slv_idx).r_resp;
    vmsti(r.r_mst_idx).r_data  := vslvo(r.r_slv_idx).r_data;
    vmsti(r.r_mst_idx).r_last  := vslvo(r.r_slv_idx).r_last;
    vmsti(r.r_mst_idx).r_id    := vslvo(r.r_slv_idx).r_id;
    vmsti(r.r_mst_idx).r_user  := vslvo(r.r_slv_idx).r_user;
    vslvi(r.r_slv_idx).r_ready := vmsto(r.r_mst_idx).r_ready;

    if aw_fire = '1' and aw_slv_idx = CFG_NASTI_SLAVES_TOTAL then
       missaccess := '1';
       v.miss_addr := vmsto(aw_mst_idx).aw_bits.addr;
    end if;
    if ar_fire = '1' and ar_slv_idx = CFG_NASTI_SLAVES_TOTAL then
       missaccess := '1';
       v.miss_addr := vmsto(ar_mst_idx).ar_bits.addr;
    end if;

    if i_nrst = '0' then
      v.w_busy := '0';
      v.w_mst_idx := CFG_NASTI_MASTER_TOTAL;
      v.w_slv_idx := CFG_NASTI_SLAVES_TOTAL;
      v.r_busy := '0';
      v.r_mst_idx := CFG_NASTI_MASTER_TOTAL;
      v.r_slv_idx := CFG_NASTI_SLAVES_TOTAL;
      v.b_busy := '0';
      v.b_mst_idx := CFG_NASTI_MASTER_TOTAL;
      v.b_slv_idx := CFG_NASTI_SLAVES_TOTAL;
      v.miss_addr := (others => '0');
    end if;
 
    rin <= v;

    for k in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
       o_msti(k) <= vmsti(k);
    end loop;
    for k in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
       o_slvi(k) <= vslvi(k);
    end loop;
    o_miss_irq <= missaccess;
    o_miss_addr <= r.miss_addr;
    o_bus_util_w <= wb_bus_util_w(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    o_bus_util_r <= wb_bus_util_r(CFG_NASTI_MASTER_TOTAL-1 downto 0);
  end process;

  reg0 : process(i_clk) begin
     if rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;
end;
