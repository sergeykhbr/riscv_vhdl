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
library commonlib;
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;

entity axictrl is
  generic (
    async_reset : boolean
  );
  port (
    i_clk    : in std_logic;
    i_nrst   : in std_logic;
    i_slvcfg : in  nasti_slave_cfg_vector;
    i_slvo   : in  nasti_slaves_out_vector;
    i_msto   : in  nasti_master_out_vector;
    o_slvi   : out nasti_slave_in_vector;
    o_msti   : out nasti_master_in_vector;
    o_bus_util_w : out std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    o_bus_util_r : out std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0)
  );
end; 
 
architecture arch_axictrl of axictrl is

  constant MISS_ACCESS_SLAVE : nasti_slave_out_type := (
      '1', '1', '1', NASTI_RESP_DECERR,
      (others=>'0'), '0', '1', '1', NASTI_RESP_DECERR, (others=>'1'), 
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
     w_sidx : slave_selector_type;
     r_sidx : slave_selector_type;
     b_sidx : slave_selector_type;
  end record;

  signal rin, r : reg_type;

begin

  comblogic : process(i_nrst, i_slvcfg, i_msto, i_slvo, r)
    variable v : reg_type;
    variable ar_sidx : slave_selector_type;
    variable aw_sidx : slave_selector_type;
    variable ar_collision : std_logic_vector(0 to CFG_NASTI_SLAVES_TOTAL-1);
    variable aw_collision : std_logic_vector(0 to CFG_NASTI_SLAVES_TOTAL-1);
    variable vmsto : nasti_master_out_vector_miss;
    variable vmsti : nasti_master_in_vector_miss;
    variable vslvi : nasti_slave_in_vector_miss;
    variable vslvo : nasti_slave_out_vector_miss;
    variable aw_fire : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    variable ar_fire : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    variable w_fire : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    variable r_fire : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    variable b_fire : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);    
    -- Bus statistic signals
    variable wb_bus_util_w : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    variable wb_bus_util_r : std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);

  begin

    v := r;

    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
       ar_sidx(n) := CFG_NASTI_SLAVES_TOTAL;
       aw_sidx(n) := CFG_NASTI_SLAVES_TOTAL;
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

    ar_collision := (others => '0');
    aw_collision := (others => '0');

    for m in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
        for s in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
            if ar_collision(s) = '0'
                and i_slvcfg(s).xmask /= X"00000"
                and (vmsto(m).ar_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12) 
                     and i_slvcfg(s).xmask) = i_slvcfg(s).xaddr then
                ar_sidx(m) := s;
                ar_collision(s) := '1';
            end if;
            if aw_collision(s) = '0'
                and i_slvcfg(s).xmask /= X"00000"
                and (vmsto(m).aw_bits.addr(CFG_NASTI_ADDR_BITS-1 downto 12)
                     and i_slvcfg(s).xmask) = i_slvcfg(s).xaddr then
                aw_sidx(m) := s;
                aw_collision(s) := '1';
            end if;
        end loop;

        -- Read Channel:
        ar_fire(m) := vmsto(m).ar_valid and vslvo(ar_sidx(m)).ar_ready;
        r_fire(m) := vmsto(m).r_ready and vslvo(r.r_sidx(m)).r_valid and vslvo(r.r_sidx(m)).r_last;
        -- Write channel:
        aw_fire(m) := vmsto(m).aw_valid and vslvo(aw_sidx(m)).aw_ready;
        w_fire(m) := vmsto(m).w_valid and vmsto(m).w_last and vslvo(r.w_sidx(m)).w_ready;
        -- Write confirm channel
        b_fire(m) := vmsto(m).b_ready and vslvo(r.b_sidx(m)).b_valid;

        if ar_fire(m) = '1' then
            v.r_sidx(m) := ar_sidx(m);
        elsif r_fire(m) = '1' then
            v.r_sidx(m) := CFG_NASTI_SLAVES_TOTAL;
        end if;

        if aw_fire(m) = '1' then
            v.w_sidx(m) := aw_sidx(m);
        elsif w_fire(m) = '1' then
            v.w_sidx(m) := CFG_NASTI_SLAVES_TOTAL;
        end if;

        if w_fire(m) = '1' then
            v.b_sidx(m) := r.w_sidx(m);
        elsif b_fire(m) = '1' then
            v.b_sidx(m) := CFG_NASTI_SLAVES_TOTAL;
        end if;


        vmsti(m).ar_ready := vslvo(ar_sidx(m)).ar_ready;
        vslvi(ar_sidx(m)).ar_valid := vmsto(m).ar_valid;
        vslvi(ar_sidx(m)).ar_bits  := vmsto(m).ar_bits;
        vslvi(ar_sidx(m)).ar_id    := vmsto(m).ar_id;
        vslvi(ar_sidx(m)).ar_user  := vmsto(m).ar_user;

        vmsti(m).r_valid := vslvo(r.r_sidx(m)).r_valid;
        vmsti(m).r_resp  := vslvo(r.r_sidx(m)).r_resp;
        vmsti(m).r_data  := vslvo(r.r_sidx(m)).r_data;
        vmsti(m).r_last  := vslvo(r.r_sidx(m)).r_last;
        vmsti(m).r_id    := vslvo(r.r_sidx(m)).r_id;
        vmsti(m).r_user  := vslvo(r.r_sidx(m)).r_user;
        vslvi(r.r_sidx(m)).r_ready := vmsto(m).r_ready;

        vmsti(m).aw_ready := vslvo(aw_sidx(m)).aw_ready;
        vslvi(aw_sidx(m)).aw_valid := vmsto(m).aw_valid;
        vslvi(aw_sidx(m)).aw_bits  := vmsto(m).aw_bits;
        vslvi(aw_sidx(m)).aw_id    := vmsto(m).aw_id;
        vslvi(aw_sidx(m)).aw_user  := vmsto(m).aw_user;

        vmsti(m).w_ready := vslvo(r.w_sidx(m)).w_ready;
        vslvi(r.w_sidx(m)).w_valid := vmsto(m).w_valid;
        vslvi(r.w_sidx(m)).w_data := vmsto(m).w_data;
        vslvi(r.w_sidx(m)).w_last := vmsto(m).w_last;
        vslvi(r.w_sidx(m)).w_strb := vmsto(m).w_strb;
        vslvi(r.w_sidx(m)).w_user := vmsto(m).w_user;

        vmsti(m).b_valid := vslvo(r.b_sidx(m)).b_valid;
        vmsti(m).b_resp := vslvo(r.b_sidx(m)).b_resp;
        vmsti(m).b_id := vslvo(r.b_sidx(m)).b_id;
        vmsti(m).b_user := vslvo(r.b_sidx(m)).b_user;
        vslvi(r.b_sidx(m)).b_ready := vmsto(m).b_ready;

        -- Statistic
        wb_bus_util_w(m) := '0';
        if r.w_sidx(m) /= CFG_NASTI_SLAVES_TOTAL then
            wb_bus_util_w(m) :=  '1';
        end if;
        wb_bus_util_r(m) := '0';
        if r.r_sidx(m) /= CFG_NASTI_SLAVES_TOTAL then
            wb_bus_util_r(m) := '1';
        end if;
    end loop;


    if not async_reset and i_nrst = '0' then
      for m in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
         v.w_sidx(m) := CFG_NASTI_SLAVES_TOTAL;
         v.r_sidx(m) := CFG_NASTI_SLAVES_TOTAL;
         v.b_sidx(m) := CFG_NASTI_SLAVES_TOTAL;
      end loop;
    end if;
 
    rin <= v;

    for k in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
       o_msti(k) <= vmsti(k);
    end loop;
    for k in 0 to CFG_NASTI_SLAVES_TOTAL-1 loop
       o_slvi(k) <= vslvi(k);
    end loop;
    o_bus_util_w <= wb_bus_util_w;
    o_bus_util_r <= wb_bus_util_r;
  end process;

  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
         for m in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
             r.w_sidx(m) <= CFG_NASTI_SLAVES_TOTAL;
             r.r_sidx(m) <= CFG_NASTI_SLAVES_TOTAL;
             r.b_sidx(m) <= CFG_NASTI_SLAVES_TOTAL;
         end loop;
     elsif rising_edge(i_clk) then 
         r <= rin;
     end if; 
  end process;
end;
