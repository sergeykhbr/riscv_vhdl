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
    tracer_ena : boolean
  );
  port ( 
    i_nrst   : in std_logic;
    i_clk    : in std_logic;
    i_msti   : in axi4_master_in_type;
    o_msto   : out axi4_master_out_type;
    o_mstcfg : out axi4_master_config_type;
    i_dport  : in dport_in_type;
    o_dport  : out dport_out_type;
    i_ext_irq : in std_logic
);
end;
 
architecture arch_river_amba of river_amba is

  constant xconfig : axi4_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => RISCV_RIVER_CPU
  );

  type state_type is (idle, reading, writing);

  type RegistersType is record
      state : state_type;
      resp_path : std_logic;
      w_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      b_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
      idle, '0', (others => '0'), (others => '0')
  );

  signal r, rin : RegistersType;

  signal req_mem_ready_i : std_logic;
  signal req_mem_path_o : std_logic;
  signal req_mem_valid_o : std_logic;
  signal req_mem_write_o : std_logic;
  signal req_mem_addr_o : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal req_mem_strob_o : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
  signal req_mem_data_o : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  signal req_mem_len_o : std_logic_vector(7 downto 0);
  signal req_mem_burst_o : std_logic_vector(1 downto 0);
  signal req_mem_last_o : std_logic;
  signal resp_mem_valid_i : std_logic;
  signal resp_mem_load_fault_i : std_logic;
  signal resp_mem_store_fault_i : std_logic;

begin

  o_mstcfg <= xconfig;
  
  river0 : RiverTop  generic map (
      memtech => memtech,
      hartid => hartid,
      async_reset => async_reset,
      fpu_ena => fpu_ena,
      tracer_ena => tracer_ena
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_req_mem_ready => req_mem_ready_i,
      o_req_mem_path => req_mem_path_o,
      o_req_mem_valid => req_mem_valid_o,
      o_req_mem_write => req_mem_write_o,
      o_req_mem_addr => req_mem_addr_o,
      o_req_mem_strob => req_mem_strob_o,
      o_req_mem_data => req_mem_data_o,
      o_req_mem_len => req_mem_len_o,
      o_req_mem_burst => req_mem_burst_o,
      o_req_mem_last => req_mem_last_o,
      i_resp_mem_valid => resp_mem_valid_i,
      i_resp_mem_path => r.resp_path,
      i_resp_mem_data => i_msti.r_data,
      i_resp_mem_load_fault => resp_mem_load_fault_i,
      i_resp_mem_store_fault => resp_mem_store_fault_i,
      i_resp_mem_store_fault_addr => r.b_addr,
      i_ext_irq => i_ext_irq,
      o_time => open,
      o_exec_cnt => open,
      i_dport_valid => i_dport.valid,
      i_dport_write => i_dport.write,
      i_dport_region => i_dport.region,
      i_dport_addr => i_dport.addr,
      i_dport_wdata => i_dport.wdata,
      o_dport_ready => o_dport.ready,
      o_dport_rdata => o_dport.rdata,
      o_halted => o_dport.halted
);

  comb : process(i_nrst, req_mem_path_o, req_mem_valid_o, req_mem_write_o,
                 req_mem_addr_o, req_mem_strob_o, req_mem_data_o, req_mem_len_o,
                 req_mem_burst_o, req_mem_last_o, i_msti, r)
    variable v : RegistersType;
    variable vmsto : axi4_master_out_type;
    variable v_req_mem_ready : std_logic;
    variable v_resp_mem_valid : std_logic;
    variable v_mem_er_load_fault : std_logic;
    variable v_mem_er_store_fault : std_logic;
    variable v_next_ready : std_logic;
  begin

    v := r;

    v_req_mem_ready := '0';
    v_resp_mem_valid := '0';
    v_mem_er_load_fault := '0';
    v_mem_er_store_fault := i_msti.b_valid and i_msti.b_resp(1);
    v_next_ready := '0';

    vmsto := axi4_master_out_none;

    vmsto.ar_bits.addr  := req_mem_addr_o;
    vmsto.ar_bits.len   := req_mem_len_o;
    vmsto.ar_user       := '0';
    vmsto.ar_id         := (others => '0');
    vmsto.ar_bits.size  := "011"; -- 8 bytes
    vmsto.ar_bits.burst := req_mem_burst_o;

    vmsto.aw_bits.addr  := req_mem_addr_o;
    vmsto.aw_bits.len   := req_mem_len_o;
    vmsto.aw_user       := '0';
    vmsto.aw_id         := (others => '0');
    vmsto.aw_bits.size  := "011"; -- 8 bytes
    vmsto.aw_bits.burst := req_mem_burst_o;

    vmsto.w_strb := req_mem_strob_o;
    vmsto.w_data := req_mem_data_o;

    vmsto.b_ready := '1';
    vmsto.r_ready := '1';

    case r.state is
    when idle =>
        v_next_ready := '1';

    when reading =>
        vmsto.r_ready := '1';
        v_mem_er_load_fault := i_msti.r_valid and i_msti.r_resp(1);
        v_resp_mem_valid := i_msti.r_valid;
        if i_msti.r_valid = '1' and i_msti.r_last = '1' then
            v_next_ready := '1';
            v.state := idle;
        end if;

    when writing =>
        vmsto.w_valid := '1';
        vmsto.w_last := req_mem_last_o;
        v_resp_mem_valid := i_msti.w_ready;
        if i_msti.w_ready = '1' and req_mem_last_o = '1' then
            v_next_ready := '1';
            v.state := idle;
            v.b_addr := r.w_addr;
        end if;

    when others =>  
    end case;

    if v_next_ready = '1' then
        if req_mem_valid_o = '1' then
            v.resp_path := req_mem_path_o;
            if req_mem_write_o = '0' then
                vmsto.ar_valid := '1';
                if i_msti.ar_ready = '1' then
                    v_req_mem_ready := '1';
                    v.state := reading;
                end if;
            else
                vmsto.aw_valid := '1';
                if i_msti.aw_ready = '1' then
                    v_req_mem_ready := '1';
                    v.state := writing;
                    v.w_addr := req_mem_addr_o;
                end if;
            end if;
        end if;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_msto <= vmsto;
    req_mem_ready_i <= v_req_mem_ready;  
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
