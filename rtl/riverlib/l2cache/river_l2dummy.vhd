--!
--! Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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
--! See "Wasserfall" implementation with the real L2-cache

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;  -- or_reduce()
library commonlib;
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_river.all;

entity RiverL2Dummy is
  generic (
    async_reset : boolean := false
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    -- CPUs Workgroup
    i_l1o : in axi4_l1_out_vector;
    o_l1i : out axi4_l1_in_vector;
    -- System bus
    i_l2i : in axi4_l2_in_type;
    o_l2o : out axi4_l2_out_type;
    i_flush_valid : std_logic
  );
end;
 
architecture arch_RiverL2Dummy of RiverL2Dummy is

  type state_type is (
      Idle,
      state_ar,
      state_r,
      l1_r_resp,
      state_aw,
      state_w,
      state_b,
      l1_w_resp
  );

  type registers_type is record
      state : state_type;
      srcid : integer range 0 to CFG_SLOT_L1_TOTAL-1;
      req_addr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
      req_size : std_logic_vector(2 downto 0);
      req_prot : std_logic_vector(2 downto 0);
      req_lock : std_logic;
      req_id   : std_logic_vector(CFG_CPU_ID_BITS-1 downto 0);
      req_user : std_logic_vector(CFG_CPU_USER_BITS-1 downto 0);
      req_wdata : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
      req_wstrb : std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
      rdata : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
      resp : std_logic_vector(1 downto 0);
  end record;

  constant R_RESET : registers_type := (
      Idle,
      0,
      (others => '0'),  -- req_addr
      "000",  -- req_size
      "000",  -- req_prot
      '0',   -- req_lock
      (others => '0'),  -- req_id
      (others => '0'),  -- req_user
      (others => '0'),  -- req_wdata
      (others => '0'),  -- req_wstrb
      (others => '0'),  -- req_rdata
      AXI_RESP_OKAY  -- resp
 );

  signal rin, r : registers_type;

begin

  comb : process(i_nrst, i_l1o, i_l2i, i_flush_valid, r)
    variable v : registers_type;
    variable vlxi : axi4_l1_in_vector;
    variable vl2o : axi4_l2_out_type;
    variable vb_src_aw : std_logic_vector(CFG_SLOT_L1_TOTAL-1 downto 0);
    variable vb_src_ar : std_logic_vector(CFG_SLOT_L1_TOTAL-1 downto 0);
    variable srcid : integer range 0 to CFG_SLOT_L1_TOTAL-1;
    variable selected : std_logic;
  begin

    v := r;

    for i in 0 to CFG_SLOT_L1_TOTAL-1 loop
        vlxi(i) := axi4_l1_in_none;

        vb_src_aw(i) := i_l1o(i).aw_valid;
        vb_src_ar(i) := i_l1o(i).ar_valid;
    end loop;
    vl2o := axi4_l2_out_none;

    -- select source (aw has higher priority):
    srcid := 0;
    selected := '0';
    if or_reduce(vb_src_aw) = '0' then
        for i in 0 to CFG_SLOT_L1_TOTAL-1 loop
            if (selected = '0') and (vb_src_ar(i) = '1') then
                srcid := i;
                selected := '1';
            end if;
        end loop;
    else
        for i in 0 to CFG_SLOT_L1_TOTAL-1 loop
            if (selected = '0') and (vb_src_aw(i) = '1') then
                srcid := i;
                selected := '1';
            end if;
        end loop;
    end if;

    case (r.state) is
    when Idle =>
         if or_reduce(vb_src_aw) = '1' then
            v.state := state_aw;
            vlxi(srcid).aw_ready := '1';
            vlxi(srcid).w_ready := '1';        -- AXI-Lite-interface

            v.srcid := srcid;
            v.req_addr := i_l1o(srcid).aw_bits.addr;
            v.req_size := i_l1o(srcid).aw_bits.size;
            v.req_lock := i_l1o(srcid).aw_bits.lock;
            v.req_prot := i_l1o(srcid).aw_bits.prot;
            v.req_id := i_l1o(srcid).aw_id;
            v.req_user := i_l1o(srcid).aw_user;
            -- AXI-Lite-interface
            v.req_wdata := i_l1o(srcid).w_data;
            v.req_wstrb := i_l1o(srcid).w_strb;
        elsif or_reduce(vb_src_ar) = '1' then
            v.state := state_ar;
            vlxi(srcid).ar_ready := '1';

            v.srcid := srcid;
            v.req_addr := i_l1o(srcid).ar_bits.addr;
            v.req_size := i_l1o(srcid).ar_bits.size;
            v.req_lock := i_l1o(srcid).ar_bits.lock;
            v.req_prot := i_l1o(srcid).ar_bits.prot;
            v.req_id := i_l1o(srcid).ar_id;
            v.req_user := i_l1o(srcid).ar_user;
        end if;

    when state_ar =>
        vl2o.ar_valid := '1';
        vl2o.ar_bits.addr := r.req_addr;
        vl2o.ar_bits.size := r.req_size;
        vl2o.ar_bits.lock := r.req_lock;
        vl2o.ar_bits.prot := r.req_prot;
        vl2o.ar_id := r.req_id;
        vl2o.ar_user := r.req_user;
        if i_l2i.ar_ready = '1' then
            v.state := state_r;
        end if;
    when state_r =>
        vl2o.r_ready := '1';
        if i_l2i.r_valid = '1' then
            v.rdata := i_l2i.r_data;
            v.resp := i_l2i.r_resp;
            v.state := l1_r_resp;
        end if;
    when l1_r_resp =>
        vlxi(r.srcid).r_valid := '1';
        vlxi(r.srcid).r_last := '1';
        vlxi(r.srcid).r_data := r.rdata;
        vlxi(r.srcid).r_resp := "00" & r.resp;
        vlxi(r.srcid).r_id := r.req_id;
        vlxi(r.srcid).r_user := r.req_user;
        if i_l1o(r.srcid).r_ready = '1' then
            v.state := Idle;
        end if;

    when state_aw =>
        vl2o.aw_valid := '1';
        vl2o.aw_bits.addr := r.req_addr;
        vl2o.aw_bits.size := r.req_size;
        vl2o.aw_bits.lock := r.req_lock;
        vl2o.aw_bits.prot := r.req_prot;
        vl2o.aw_id := r.req_id;
        vl2o.aw_user := r.req_user;
        vl2o.w_valid := '1';   -- AXI-Lite request
        vl2o.w_last := '1';
        vl2o.w_data := r.req_wdata;
        vl2o.w_strb := r.req_wstrb;
        vl2o.w_user := r.req_user;
        if i_l2i.aw_ready = '1' then
            if i_l2i.w_ready = '1' then
                v.state := state_b;
            else
                v.state := state_w;
            end if;
        end if;
    when state_w =>
        vl2o.w_valid := '1';
        vl2o.w_last := '1';
        vl2o.w_data := r.req_wdata;
        vl2o.w_strb := r.req_wstrb;
        vl2o.w_user := r.req_user;
        if i_l2i.w_ready = '1' then
            v.state := state_b;
        end if;
    when state_b =>
        vl2o.b_ready := '1';
        if i_l2i.b_valid = '1' then
            v.resp := i_l2i.b_resp;
            v.state := l1_w_resp;
        end if;
    when l1_w_resp =>
        vlxi(r.srcid).b_valid := '1';
        vlxi(r.srcid).b_resp := r.resp;
        vlxi(r.srcid).b_id := r.req_id;
        vlxi(r.srcid).b_user := r.req_user;
        if i_l1o(r.srcid).b_ready = '1' then
            v.state := Idle;
        end if;
    when others =>
    end case;

    rin <= v;

    o_l1i <= vlxi;
    o_l2o <= vl2o;
  end process;

  -- registers:
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
