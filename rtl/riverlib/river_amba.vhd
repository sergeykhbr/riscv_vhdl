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
    async_reset : boolean
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

  type RegistersType is record
      reading : std_logic;
      w_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      w_valid : std_logic;
      w_last : std_logic;
      b_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      b_ready : std_logic;
  end record;

  constant R_RESET : RegistersType := (
      '0', (others => '0'), '0', '0', (others => '0'), '0'
  );

  signal r, rin : RegistersType;

  signal w_req_mem_ready : std_logic;
  signal w_req_mem_valid : std_logic;
  signal w_req_mem_write : std_logic;
  signal wb_req_mem_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
  signal wb_req_mem_strob : std_logic_vector(BUS_DATA_BYTES-1 downto 0);
  signal wb_req_mem_data : std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
  signal wb_req_mem_len : std_logic_vector(7 downto 0);
  signal wb_req_mem_burst : std_logic_vector(1 downto 0);
  signal w_resp_mem_data_valid : std_logic;
  signal w_resp_mem_load_fault : std_logic;
  signal w_resp_mem_store_fault : std_logic;

begin

  o_mstcfg <= xconfig;
  w_resp_mem_data_valid <= i_msti.r_valid or (r.w_valid and i_msti.w_ready);
  -- Slave response resp = SLVERR (2'b10)
  -- Interconnect response resp = DECERR (2'b11):
  w_resp_mem_load_fault <= r.reading and i_msti.r_valid and i_msti.r_resp(1);
  w_resp_mem_store_fault <= r.b_ready and i_msti.b_valid and i_msti.b_resp(1);
  
  river0 : RiverTop  generic map (
      memtech => memtech,
      hartid => hartid,
      async_reset => async_reset
    ) port map (
      i_clk => i_clk,
      i_nrst => i_nrst,
      i_req_mem_ready => w_req_mem_ready,
      o_req_mem_valid => w_req_mem_valid,
      o_req_mem_write => w_req_mem_write,
      o_req_mem_addr => wb_req_mem_addr,
      o_req_mem_strob => wb_req_mem_strob,
      o_req_mem_data => wb_req_mem_data,
      o_req_mem_len => wb_req_mem_len,
      o_req_mem_burst => wb_req_mem_burst,
      i_resp_mem_data_valid => w_resp_mem_data_valid,
      i_resp_mem_data => i_msti.r_data,
      i_resp_mem_load_fault => w_resp_mem_load_fault,
      i_resp_mem_store_fault => w_resp_mem_store_fault,
      i_resp_mem_store_fault_addr => r.b_addr,
      i_ext_irq => i_ext_irq,
      o_time => open,
      i_dport_valid => i_dport.valid,
      i_dport_write => i_dport.write,
      i_dport_region => i_dport.region,
      i_dport_addr => i_dport.addr,
      i_dport_wdata => i_dport.wdata,
      o_dport_ready => o_dport.ready,
      o_dport_rdata => o_dport.rdata,
      o_halted => o_dport.halted
);

  comb : process(i_nrst, w_req_mem_valid, w_req_mem_write, wb_req_mem_addr,
                 wb_req_mem_strob, wb_req_mem_data, wb_req_mem_len,
                 wb_req_mem_burst, i_msti, r)
    variable v : RegistersType;
    variable vmsto : axi4_master_out_type;
  begin

    v := r;

    vmsto := axi4_master_out_none;
    vmsto.ar_valid      := w_req_mem_valid and not w_req_mem_write;
    vmsto.ar_bits.addr  := wb_req_mem_addr;
    vmsto.ar_bits.len   := wb_req_mem_len;
    vmsto.ar_user       := '0';
    vmsto.ar_id         := conv_std_logic_vector(0, CFG_ROCKET_ID_BITS);
    vmsto.ar_bits.size  := "011"; -- 8 bytes
    vmsto.ar_bits.burst := wb_req_mem_burst;

    vmsto.aw_valid      := w_req_mem_valid and w_req_mem_write;
    vmsto.aw_bits.addr  := wb_req_mem_addr;
    vmsto.aw_bits.len   := wb_req_mem_len;
    vmsto.aw_user       := '0';
    vmsto.aw_id         := conv_std_logic_vector(0, CFG_ROCKET_ID_BITS);
    vmsto.aw_bits.size  := "011"; -- 8 bytes
    vmsto.aw_bits.burst := wb_req_mem_burst;

    vmsto.w_valid := r.w_valid;
    vmsto.w_last := r.w_last;
    vmsto.w_strb := wb_req_mem_strob;
    vmsto.w_data := wb_req_mem_data;

    vmsto.b_ready := r.b_ready;
    vmsto.r_ready := '1';

    if (w_req_mem_valid and not w_req_mem_write and i_msti.ar_ready) = '1' then
        v.reading := '1';
    elsif i_msti.r_valid = '1' and i_msti.r_last = '1' then
        v.reading := '0';
    end if;

    if (w_req_mem_valid and w_req_mem_write and i_msti.aw_ready) = '1' then
        v.w_addr := wb_req_mem_addr;
        v.w_valid := '1';
        v.w_last := '1';
    elsif i_msti.w_ready = '1' then
        v.w_valid := '0';
        v.w_last := '0';
    end if;
    
    if (r.w_valid and i_msti.w_ready) = '1' then
        v.b_addr := r.w_addr;
        v.b_ready := '1';
    elsif i_msti.b_valid = '1' then
        v.b_ready := '0';
    end if;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
    o_msto <= vmsto;
    w_req_mem_ready <= (w_req_mem_write and i_msti.aw_ready)
                    or (not w_req_mem_write and i_msti.ar_ready);
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
