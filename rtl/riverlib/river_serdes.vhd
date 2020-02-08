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
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;
--! River top level with AMBA interface module declaration
use riverlib.types_river.all;

entity river_serdes is 
  generic (
    async_reset : boolean
  );
  port ( 
    i_nrst  : in std_logic;
    i_clk   : in std_logic;
    i_coreo : in axi4_river_out_type;
    o_corei : out axi4_river_in_type;
    i_msti  : in axi4_master_in_type;
    o_msto  : out axi4_master_out_type
);
end;
 
architecture arch_river_serdes of river_serdes is

  type state_type is (Idle, Read, Write);

  type RegistersType is record
      state : state_type;
      req_len : std_logic_vector(7 downto 0);
      b_wait : std_logic;
      cacheline : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
      wstrb : std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
      rmux : std_logic_vector(L1CACHE_BURST_LEN-1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
      idle, X"00", '0', (others => '0'), (others => '0'), (others => '0')
  );

  signal r, rin : RegistersType;

  function size2len(size: std_logic_vector) 
    return std_logic_vector is
    variable len: std_logic_vector(7 downto 0);
  begin
    case size(2 downto 0) is
    when "100" => len := X"01";
    when "101" => len := X"03";
    when "110" => len := X"07";
    when "111" => len := X"0F";
    when others => len := X"00";
    end case;
    return len;
  end function size2len;

begin


  comb : process(i_nrst, i_coreo, i_msti, r)
    variable v : RegistersType;
    variable v_req_mem_ready : std_logic;
    variable vb_line_o : std_logic_vector(DCACHE_LINE_BITS-1 downto 0);
    variable vb_r_resp : std_logic_vector(3 downto 0);
    variable v_r_valid : std_logic;
    variable v_w_valid : std_logic;
    variable v_w_last : std_logic;
    variable v_w_ready : std_logic;
    variable vb_len : std_logic_vector(7 downto 0);
    variable vb_aw_id : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
    variable vb_ar_id : std_logic_vector(CFG_ROCKET_ID_BITS-1 downto 0);
  begin

    v := r;

    v_req_mem_ready := '0';
    v_r_valid := '0';
    v_w_valid := '0';
    v_w_last := '0';
    v_w_ready := '0';
    vb_len := (others => '0');
    vb_aw_id := (others => '0');
    vb_ar_id := (others => '0');

    vb_aw_id(CFG_CPU_ID_BITS-1 downto 0) := i_coreo.aw_id;
    vb_ar_id(CFG_CPU_ID_BITS-1 downto 0) := i_coreo.ar_id;

    vb_line_o := r.cacheline;

    for i in 0 to L1CACHE_BURST_LEN-1 loop
        if r.rmux(i) = '1' then
            vb_line_o((i+1)*BUS_DATA_WIDTH-1 downto i*BUS_DATA_WIDTH) := i_msti.r_data;
        end if;
    end loop;

    if i_coreo.b_ready = '1' then
        v.b_wait := '0';
    end if;


    case r.state is
    when Idle =>
        v_req_mem_ready := '1';

    when Read =>
        if i_msti.r_valid = '1' then
            v.cacheline := vb_line_o;
            v.rmux := r.rmux(L1CACHE_BURST_LEN-2 downto 0) & '0';
            if r.req_len = X"00" then
                v_r_valid := '1';
                v_req_mem_ready := '1';
            else
                v.req_len := r.req_len - 1;
            end if;
        end if;

    when Write =>
        v_w_valid := '1';
        if r.req_len = X"00" then
            v_w_last := '1';
        end if;
        if i_msti.w_ready = '1' then
            v.cacheline(L1CACHE_LINE_BITS-1 downto L1CACHE_LINE_BITS-BUS_DATA_WIDTH) := 
                (others => '0');
            v.cacheline(L1CACHE_LINE_BITS-BUS_DATA_WIDTH-1 downto 0) := 
                r.cacheline(L1CACHE_LINE_BITS-1 downto BUS_DATA_WIDTH);
            v.wstrb(L1CACHE_BYTES_PER_LINE-1 downto L1CACHE_BYTES_PER_LINE-BUS_DATA_BYTES) := 
                (others => '0');
            v.wstrb(L1CACHE_BYTES_PER_LINE-BUS_DATA_BYTES-1 downto 0) :=
                r.wstrb(L1CACHE_BYTES_PER_LINE-1 downto BUS_DATA_BYTES);
            if r.req_len = X"00" then
                v_w_ready := '1';
                v.b_wait := '1';
                v_req_mem_ready := '1';
            else
                v.req_len := r.req_len - 1;
            end if;
        end if;

    when others =>  
    end case;

    if v_req_mem_ready = '1' then
        if (i_coreo.ar_valid and i_msti.ar_ready) = '1' then
            v.state := Read;
            v.rmux := conv_std_logic_vector(1, L1CACHE_BURST_LEN);
            vb_len := size2len(i_coreo.ar_bits.size);
        elsif (i_coreo.aw_valid and i_msti.aw_ready) = '1' then
            v.cacheline := i_coreo.w_data;                     -- Undocumented River (Axi-lite) feature
            v.wstrb := i_coreo.w_strb;
            v.state := Write;
            vb_len := size2len(i_coreo.aw_bits.size);
        else
            v.state := Idle;
        end if;
        v.req_len := vb_len;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    vb_r_resp(3 downto 2) := "00";
    vb_r_resp(1 downto 0) := i_msti.r_resp;

    o_msto.aw_valid <= i_coreo.aw_valid;
    o_msto.aw_bits.addr <= i_coreo.aw_bits.addr;
    o_msto.aw_bits.len <= vb_len;            -- burst len = len[7:0] + 1
    o_msto.aw_bits.size <= "011";            -- 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto.aw_bits.burst <= "01";            -- 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    o_msto.aw_bits.lock <= i_coreo.aw_bits.lock;
    o_msto.aw_bits.cache <= i_coreo.aw_bits.cache;
    o_msto.aw_bits.prot <= i_coreo.aw_bits.prot;
    o_msto.aw_bits.qos <= i_coreo.aw_bits.qos;
    o_msto.aw_bits.region <= i_coreo.aw_bits.region;
    o_msto.aw_id <= vb_aw_id;
    o_msto.aw_user <= i_coreo.aw_user;
    o_msto.w_valid <= v_w_valid;
    o_msto.w_last <= v_w_last;
    o_msto.w_data <= r.cacheline(BUS_DATA_WIDTH-1 downto 0);
    o_msto.w_strb <= r.wstrb(BUS_DATA_BYTES-1 downto 0);
    o_msto.w_user <= i_coreo.w_user;
    o_msto.b_ready <= i_coreo.b_ready;
    o_msto.ar_valid <= i_coreo.ar_valid;
    o_msto.ar_bits.addr <= i_coreo.ar_bits.addr;
    o_msto.ar_bits.len <= vb_len;            -- burst len = len[7:0] + 1
    o_msto.ar_bits.size <= "011";            -- 0=1B; 1=2B; 2=4B; 3=8B; ...
    o_msto.ar_bits.burst <= "01";            -- 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    o_msto.ar_bits.lock <= i_coreo.ar_bits.lock;
    o_msto.ar_bits.cache <= i_coreo.ar_bits.cache;
    o_msto.ar_bits.prot <= i_coreo.ar_bits.prot;
    o_msto.ar_bits.qos <= i_coreo.ar_bits.qos;
    o_msto.ar_bits.region <= i_coreo.ar_bits.region;
    o_msto.ar_id <= vb_ar_id;
    o_msto.ar_user <= i_coreo.ar_user;
    o_msto.r_ready <= i_coreo.r_ready;

    o_corei.aw_ready <= i_msti.aw_ready;
    o_corei.w_ready <= v_w_ready;
    o_corei.b_valid <= i_msti.b_valid and r.b_wait;
    o_corei.b_resp <= i_msti.b_resp;
    o_corei.b_id <= i_msti.b_id(CFG_CPU_ID_BITS-1 downto 0);
    o_corei.b_user <= i_msti.b_user;
    o_corei.ar_ready <= i_msti.ar_ready;
    o_corei.r_valid <= v_r_valid;
    o_corei.r_resp <= vb_r_resp;
    o_corei.r_data <= vb_line_o;
    o_corei.r_last <= v_r_valid;
    o_corei.r_id <= i_msti.r_id(CFG_CPU_ID_BITS-1 downto 0);
    o_corei.r_user <= i_msti.r_user;
    o_corei.ac_valid <= '0';
    o_corei.ac_addr <= (others => '0');
    o_corei.ac_snoop <= (others => '0');
    o_corei.ac_prot <= (others => '0');
    o_corei.cr_ready <= '0';
    o_corei.cd_ready <= '0';

    rin <= v;
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
