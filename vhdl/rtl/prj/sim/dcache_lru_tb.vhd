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
library std;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;
use commonlib.types_util.all;
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_cache.all;

entity dcache_lru_tb is
end dcache_lru_tb;

architecture behavior of dcache_lru_tb is
  -- input/output signals:
  signal i_nrst : std_logic := '0';
  signal i_clk : std_logic := '0';
  signal i_req_valid : std_logic;
  signal i_req_write : std_logic;
  signal i_req_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal i_req_wdata : std_logic_vector(64-1 downto 0);
  signal i_req_wstrb : std_logic_vector(8-1 downto 0);
  signal o_req_ready : std_logic;
  signal o_resp_valid : std_logic;
  signal o_resp_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal o_resp_data : std_logic_vector(64-1 downto 0);
  signal o_resp_er_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal o_resp_er_load_fault : std_logic;
  signal o_resp_er_store_fault : std_logic;
  signal o_resp_er_mpu_load : std_logic;
  signal o_resp_er_mpu_store : std_logic;
  signal i_resp_ready : std_logic;
  signal i_req_mem_ready : std_logic;
  signal o_req_mem_valid : std_logic;
  signal o_req_mem_type : std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);
  signal o_req_mem_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal o_req_mem_strob : std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
  signal o_req_mem_data : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  signal i_mem_data_valid : std_logic;
  signal i_mem_data : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  signal i_mem_load_fault : std_logic;
  signal i_mem_store_fault : std_logic;
  signal o_mpu_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal i_mpu_flags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
  signal i_flush_address : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal i_flush_valid : std_logic;

  type bus_state_type is (Idle, Read, ReadLast);

  type BusRegisterType is record
      state : bus_state_type;
      mpu_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      burst_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  end record;

  signal r, rin : BusRegisterType;
  signal clk_cnt : integer := 0;

  constant START_POINT : integer := 10 + 1 + (2**10);
  constant START_POINT2 : integer := START_POINT + 500;
begin

  i_clk <= not i_clk after 12.5 ns;


  comb_fecth : process (i_clk, clk_cnt)
    variable v_req_valid : std_logic;
    variable v_req_write : std_logic;
    variable vb_req_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_req_wdata : std_logic_vector(64-1 downto 0);
    variable vb_req_wstrb : std_logic_vector(8-1 downto 0);
    variable vb_mpu_flags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    variable v_flush_valid : std_logic;
    variable vb_flush_address : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  begin
    v_req_valid := '0';
    v_req_write := '0';
    vb_req_addr := (others => '0');
    vb_req_wdata := (others => '0');
    vb_req_wstrb := (others => '0');
    i_resp_ready <= '1';

    vb_mpu_flags := (others => '1');

    v_flush_valid := '0';
    vb_flush_address := (others => '0');

    if r.mpu_addr(31) = '1' then
        vb_mpu_flags(CFG_MPU_FL_CACHABLE) := '0';
    end if;

    case (clk_cnt) is
    when START_POINT =>
        -- Load line[0], way[0]
        v_req_valid := '1';
        vb_req_addr(31 downto 0) := X"00000008";
    when START_POINT + 10 =>
        -- Load line[0], way[1]
        v_req_valid := '1';
        vb_req_addr(31 downto 0) := X"00010008";
    when START_POINT + 25 =>
        -- Load line[0], way[2]
        v_req_valid := '1';
        vb_req_addr(31 downto 0) := X"00011008";
    when START_POINT + 40 =>
        -- Load line[0], way[3]
        v_req_valid := '1';
        vb_req_addr(31 downto 0) := X"00012008";
    when START_POINT + 55 =>
        -- Remove line[0], way[0] (as an older used)
        -- Load line[0], way[0] with a new data
        -- way[1] becomes and older
        v_req_valid := '1';
        vb_req_addr(31 downto 0) := X"00013008";
    when START_POINT2 =>
        -- Cached Write to loaded cache line without bus access
        -- Modify line[0], way[3]
        -- LRU should move way[3] on top (way[1] is still an older)
        v_req_valid := '1';
        v_req_write := '1';
        vb_req_addr(31 downto 0) := X"00012008";
        vb_req_wdata := conv_std_logic_vector(16#0000CC00#, 64);
        vb_req_wstrb := conv_std_logic_vector(16#02#, 8);
    when START_POINT2 + 10 =>
        -- Uncached write directly on bus 
        v_req_valid := '1';
        v_req_write := '1';
        vb_req_addr(31 downto 0) := X"80012008";
        vb_req_wdata := X"00000000_0000CCBB";
        vb_req_wstrb := conv_std_logic_vector(16#0F#, 8);
    when START_POINT2 + 20 =>
        -- Uncached read directly on bus
        v_req_valid := '1';
        v_req_write := '0';
        vb_req_addr(31 downto 0) := X"80012010";
    when START_POINT2 + 30 =>
        -- Cached Write to notloaded cache line without displacement:
        --       - load line
        --       - modify line
        --       - store to cache
        --
        v_req_valid := '1';
        v_req_write := '1';
        vb_req_addr(31 downto 0) := X"00000028";
        vb_req_wdata := X"00000000_BBAA0000";
        vb_req_wstrb := conv_std_logic_vector(16#0C#, 8);
    when others =>
    end case;

    i_req_valid <= v_req_valid;
    i_req_write <= v_req_write;
    i_req_addr <= vb_req_addr;
    i_req_wdata <= vb_req_wdata;
    i_req_wstrb <= vb_req_wstrb;

    i_mpu_flags <= vb_mpu_flags;

    i_flush_valid <= v_flush_valid;
    i_flush_address <= vb_flush_address;
  end process;


  comb_bus : process (i_nrst, r,
                      o_req_mem_valid, o_req_mem_type,
                      o_req_mem_addr, o_req_mem_strob, o_req_mem_data, 
                      o_mpu_addr)
    variable v : BusRegisterType;
  begin
    v := r;

    i_req_mem_ready <= '0';
    i_mem_data_valid <= '0';
    i_mem_data <= (others => '0');
    i_mem_load_fault <= '0';
    i_mem_store_fault <= '0';


    case r.state is
    when Idle =>
        i_req_mem_ready <= '1';
        if o_req_mem_valid = '1' then
            v.state := ReadLast;
            v.burst_addr := o_req_mem_addr;
        end if;
    when Read =>
        i_mem_data_valid <= '1';
        i_mem_data <= X"2000000010000000" + r.burst_addr;
        v.burst_addr := r.burst_addr + 8;
        v.state := ReadLast;
    when ReadLast =>
        i_req_mem_ready <= '1';
        i_mem_data_valid <= '1';
        i_mem_data <= X"2000000010000000" + r.burst_addr;
        if o_req_mem_valid = '1' then
            v.state := ReadLast;
            v.burst_addr := o_req_mem_addr;
        else
            v.state := Idle;
        end if;
    when others =>
    end case;

    v.mpu_addr := o_mpu_addr;

    if i_nrst = '0' then
        v.state := Idle;
        v.mpu_addr := (others => '0');
        v.burst_addr := (others => '0');
    end if;


    rin <= v;
  end process;

  procSignal : process (i_clk, clk_cnt)
  begin
    if rising_edge(i_clk) then
      if clk_cnt = 10 then
        i_nrst <= '1';
      end if;
    end if;
  end process procSignal;

  tt : dcache_lru generic map (
    memtech => 0,
    async_reset => false,
    coherence_ena => false
  ) port map (
    i_clk => i_clk,
    i_nrst => i_nrst,
    i_req_valid => i_req_valid,
    i_req_write => i_req_write,
    i_req_addr => i_req_addr,
    i_req_wdata => i_req_wdata,
    i_req_wstrb => i_req_wstrb,
    o_req_ready => o_req_ready,
    o_resp_valid => o_resp_valid,
    o_resp_addr => o_resp_addr,
    o_resp_data => o_resp_data,
    o_resp_er_addr => o_resp_er_addr,
    o_resp_er_load_fault => o_resp_er_load_fault,
    o_resp_er_store_fault => o_resp_er_store_fault,
    o_resp_er_mpu_load => o_resp_er_mpu_load,
    o_resp_er_mpu_store => o_resp_er_mpu_store,
    i_resp_ready => i_resp_ready,
    i_req_mem_ready => i_req_mem_ready,
    o_req_mem_valid => o_req_mem_valid,
    o_req_mem_type => o_req_mem_type,
    o_req_mem_addr => o_req_mem_addr,
    o_req_mem_strob => o_req_mem_strob,
    o_req_mem_data => o_req_mem_data,
    i_mem_data_valid => i_mem_data_valid,
    i_mem_data => i_mem_data,
    i_mem_load_fault => i_mem_load_fault,
    i_mem_store_fault => i_mem_store_fault,
    o_mpu_addr => o_mpu_addr,
    i_mpu_flags => i_mpu_flags,
    -- D$ Snoop interface
    i_req_snoop_valid => '0',
    i_req_snoop_type => "00",
    o_req_snoop_ready => open,
    i_req_snoop_addr => (others => '0'),
    i_resp_snoop_ready => '1',
    o_resp_snoop_valid => open,
    o_resp_snoop_data => open,
    o_resp_snoop_flags => open,
    i_flush_address => i_flush_address,
    i_flush_valid => i_flush_valid
  );


  procCheck : process (i_nrst, i_clk)
  begin
    if rising_edge(i_clk) then
        clk_cnt <= clk_cnt + 1;
        r <= rin;
    end if;
  end process procCheck;

end;
