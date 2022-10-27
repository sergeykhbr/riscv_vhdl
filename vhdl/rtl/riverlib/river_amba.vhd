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
--! @brief     "River" CPU Top level with AXI4 interface.
------------------------------------------------------------------------------

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

entity river_amba is 
  generic (
    memtech : integer;
    hartid : integer;
    async_reset : boolean;
    fpu_ena : boolean;
    coherence_ena : boolean;
    tracer_ena : boolean
  );
  port ( 
    i_nrst   : in std_logic;
    i_clk    : in std_logic;
    i_msti   : in axi4_l1_in_type;
    o_msto   : out axi4_l1_out_type;
    i_dport  : in dport_in_type;
    o_dport  : out dport_out_type;
    i_ext_irq : in std_logic
);
end;
 
architecture arch_river_amba of river_amba is

  type state_type is (
      state_idle,
      state_ar,
      state_r,
      state_aw,
      state_w,
      state_b
  );

  type snoopstate_type is (
      snoop_idle,
      snoop_ac_wait_accept,
      snoop_cr,
      snoop_cr_wait_accept,
      snoop_cd,
      snoop_cd_wait_accept
  );

  type RegistersType is record
      state : state_type;
      req_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      req_path : std_logic;
      req_cached : std_logic_vector(3 downto 0);
      req_wdata : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
      req_wstrb : std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
      req_size : std_logic_vector(2 downto 0);
      req_prot : std_logic_vector(2 downto 0);
      req_ar_snoop : std_logic_vector(3 downto 0);
      req_aw_snoop : std_logic_vector(2 downto 0);
  end record;

  constant R_RESET : RegistersType := (
      state_idle,
      (others => '0'),    -- req_addr
      '0',                -- req_path
      (others => '0'),    -- req_cached
      (others => '0'),    -- req_wdata
      (others => '0'),    -- req_wstrb
      (others => '0'),    -- req_size
      (others => '0'),    -- req_prot
      (others => '0'),    -- req_ar_snoop
      (others => '0')     -- req_aw_snoop
  );

  signal r, rin : RegistersType;

  signal req_mem_ready_i : std_logic;
  signal req_mem_path_o : std_logic;
  signal req_mem_valid_o : std_logic;
  signal req_mem_type_o : std_logic_vector(REQ_MEM_TYPE_BITS-1 downto 0);
  signal req_mem_addr_o : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal req_mem_strob_o : std_logic_vector(L1CACHE_BYTES_PER_LINE-1 downto 0);
  signal req_mem_data_o : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  signal resp_mem_valid_i : std_logic;
  signal resp_mem_load_fault_i : std_logic;
  signal resp_mem_store_fault_i : std_logic;
  -- D$ Snoop interface
  signal req_snoop_valid_i : std_logic;
  signal req_snoop_type_i : std_logic_vector(SNOOP_REQ_TYPE_BITS-1 downto 0);
  signal req_snoop_ready_o : std_logic;
  signal req_snoop_addr_i : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  signal resp_snoop_ready_i : std_logic;
  signal resp_snoop_valid_o : std_logic;
  signal resp_snoop_data_o : std_logic_vector(L1CACHE_LINE_BITS-1 downto 0);
  signal resp_snoop_flags_o : std_logic_vector(DTAG_FL_TOTAL-1 downto 0);

begin

  o_dport.available <= '1';
  
  river0 : RiverTop  generic map (
      memtech => memtech,
      hartid => hartid,
      async_reset => async_reset,
      fpu_ena => fpu_ena,
      coherence_ena => coherence_ena,
      tracer_ena => tracer_ena
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_req_mem_ready => req_mem_ready_i,
      o_req_mem_path => req_mem_path_o,
      o_req_mem_valid => req_mem_valid_o,
      o_req_mem_type => req_mem_type_o,
      o_req_mem_addr => req_mem_addr_o,
      o_req_mem_strob => req_mem_strob_o,
      o_req_mem_data => req_mem_data_o,
      i_resp_mem_valid => resp_mem_valid_i,
      i_resp_mem_path => r.req_path,
      i_resp_mem_data => i_msti.r_data,
      i_resp_mem_load_fault => resp_mem_load_fault_i,
      i_resp_mem_store_fault => resp_mem_store_fault_i,
      i_req_snoop_valid => req_snoop_valid_i,
      i_req_snoop_type => req_snoop_type_i,
      o_req_snoop_ready => req_snoop_ready_o,
      i_req_snoop_addr => req_snoop_addr_i,
      i_resp_snoop_ready => resp_snoop_ready_i,
      o_resp_snoop_valid => resp_snoop_valid_o,
      o_resp_snoop_data => resp_snoop_data_o,
      o_resp_snoop_flags => resp_snoop_flags_o,
      i_ext_irq => i_ext_irq,
      i_dport_req_valid => i_dport.req_valid,
      i_dport_write => i_dport.write,
      i_dport_addr => i_dport.addr,
      i_dport_wdata => i_dport.wdata,
      o_dport_req_ready => o_dport.req_ready,
      i_dport_resp_ready => i_dport.resp_ready,
      o_dport_resp_valid => o_dport.resp_valid,
      o_dport_rdata => o_dport.rdata,
      o_halted => o_dport.halted
);

  comb : process(i_nrst, req_mem_path_o, req_mem_valid_o, req_mem_type_o,
                 req_mem_addr_o, req_mem_strob_o, req_mem_data_o, 
                 i_msti, r)
    variable v : RegistersType;
    variable v_resp_mem_valid : std_logic;
    variable v_mem_er_load_fault : std_logic;
    variable v_mem_er_store_fault : std_logic;
    variable v_next_ready : std_logic;
    variable vmsto : axi4_l1_out_type;

  begin

    v := r;

    v_resp_mem_valid := '0';
    v_mem_er_load_fault := '0';
    v_mem_er_store_fault := '0';
    v_next_ready := '0';

    vmsto := axi4_l1_out_none;
    vmsto.ar_bits.burst := "01";  -- INCR (possible any value)
    vmsto.aw_bits.burst := "01";  -- INCR (possible any value)

    case r.state is
    when state_idle =>
        v_next_ready := '1';
        if req_mem_valid_o = '1' then
            v.req_path := req_mem_path_o;
            v.req_addr := req_mem_addr_o;
            if req_mem_type_o(REQ_MEM_TYPE_CACHED) = '1' then
                v.req_size := "101";   -- 32 Bytes
            elsif req_mem_path_o = '1' then
                v.req_size := "100";   -- 16 Bytes: Uncached Instruction
            else
                v.req_size := "011";   -- 8 Bytes: Uncached Data
            end if;
            -- [0] 0=Unpriv/1=Priv;
            -- [1] 0=Secure/1=Non-secure;
            -- [2]  0=Data/1=Instruction
            v.req_prot := req_mem_path_o & "00";
            if req_mem_type_o(REQ_MEM_TYPE_WRITE) = '0' then
                v.state := state_ar;
                v.req_wdata := (others => '0');
                v.req_wstrb := (others => '0');
                if req_mem_type_o(REQ_MEM_TYPE_CACHED) = '1' then
                    v.req_cached := ARCACHE_WRBACK_READ_ALLOCATE;
                else
                    v.req_cached := ARCACHE_DEVICE_NON_BUFFERABLE;
                end if;
                --if coherence_ena then
                --    v.req_ar_snoop := reqtype2arsnoop(req_mem_type_o);
                --end if;
            else
                v.state := state_aw;
                v.req_wdata := req_mem_data_o;
                v.req_wstrb := req_mem_strob_o;
                if req_mem_type_o(REQ_MEM_TYPE_CACHED) = '1' then
                    v.req_cached := AWCACHE_WRBACK_WRITE_ALLOCATE;
                else
                    v.req_cached := AWCACHE_DEVICE_NON_BUFFERABLE;
                end if;
                --if coherence_ena then
                --    v.req_aw_snoop := reqtype2awsnoop(req_mem_type_o);
                --end if;
            end if;
        end if;

    when state_ar =>
        vmsto.ar_valid := '1';
        vmsto.ar_bits.addr := r.req_addr;
        vmsto.ar_bits.cache := r.req_cached;
        vmsto.ar_bits.size := r.req_size;
        vmsto.ar_bits.prot := r.req_prot;
        vmsto.ar_snoop := r.req_ar_snoop;
        if i_msti.ar_ready = '1' then
            v.state := state_r;
        end if;
    when state_r =>
        vmsto.r_ready := '1';
        v_mem_er_load_fault := i_msti.r_resp(1);
        v_resp_mem_valid := i_msti.r_valid;
        -- r_valid and r_last always should be in the same time
        if i_msti.r_valid = '1' and i_msti.r_last = '1' then
            v.state := state_idle;
        end if;

    when state_aw =>
        vmsto.aw_valid := '1';
        vmsto.aw_bits.addr := r.req_addr;
        vmsto.aw_bits.cache := r.req_cached;
        vmsto.aw_bits.size := r.req_size;
        vmsto.aw_bits.prot := r.req_prot;
        --vmsto.aw_snoop := r.req_aw_snoop;
        -- axi lite to simplify L2-cache
        vmsto.w_valid := '1';
        vmsto.w_last := '1';
        vmsto.w_data := r.req_wdata;
        vmsto.w_strb := r.req_wstrb;
        if i_msti.aw_ready = '1' then
            if i_msti.w_ready = '1' then
                v.state := state_b;
            else
                v.state := state_w;
            end if;
        end if;
    when state_w =>
        -- Shoudln't get here because of Lite interface:
        vmsto.w_valid := '1';
        vmsto.w_last := '1';
        vmsto.w_data := r.req_wdata;
        vmsto.w_strb := r.req_wstrb;
        if i_msti.w_ready = '1' then
            v.state := state_b;
        end if;
    when state_b =>
        vmsto.b_ready := '1';
        v_resp_mem_valid := i_msti.b_valid;
        v_mem_er_store_fault := i_msti.b_resp(1);
        if i_msti.b_valid = '1' then
            v.state := state_idle;
        end if;
    when others =>  
    end case;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;


    o_msto <= vmsto;

    req_mem_ready_i <= v_next_ready;  
    resp_mem_valid_i <= v_resp_mem_valid;
    resp_mem_load_fault_i <= v_mem_er_load_fault;
    resp_mem_store_fault_i <= v_mem_er_store_fault;

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
