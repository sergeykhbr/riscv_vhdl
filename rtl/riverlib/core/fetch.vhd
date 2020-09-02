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
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity InstrFetch is generic (
    async_reset : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_pipeline_hold : in std_logic;
    i_mem_req_ready : in std_logic;
    o_mem_addr_valid : out std_logic;
    o_mem_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_mem_data : in std_logic_vector(31 downto 0);
    i_mem_load_fault : in std_logic;
    i_mem_executable : in std_logic;
    o_mem_resp_ready : out std_logic;

    i_flush_pipeline : in std_logic;                   -- reset pipeline and cache
    i_progbuf_ena : in std_logic;                      -- executing from prog buffer
    i_progbuf_pc : in std_logic_vector(31 downto 0);   -- progbuf counter
    i_progbuf_data : in std_logic_vector(31 downto 0); -- progbuf instruction
    i_predict_npc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);

    o_mem_req_fire : out std_logic;                    -- used by branch predictor to form new npc value
    o_instr_load_fault : out std_logic;                -- fault instruction's address
    o_instr_executable : out std_logic;
    o_valid : out std_logic;
    o_pc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0);
    o_hold : out std_logic                                -- Hold due no response from icache yet
  );
end; 
 
architecture arch_InstrFetch of InstrFetch is

  type RegistersType is record
      wait_resp : std_logic;
      br_address : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      br_instr : std_logic_vector(31 downto 0);

      resp_address : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      resp_data : std_logic_vector(31 downto 0);
      resp_valid : std_logic;
      instr_load_fault : std_logic;
      instr_executable : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    '0',
    (others => '1'),  -- br_address
    (others =>'0'),   -- br_instr
    (others =>'0'), (others =>'0'), '0',
    '0',              -- instr_load_fault
    '0'               -- instr_executable
  );

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_pipeline_hold, i_mem_req_ready, i_mem_data_valid,
                i_mem_data_addr, i_mem_data, i_mem_load_fault, i_mem_executable,
                i_flush_pipeline, i_progbuf_ena, i_progbuf_pc, i_progbuf_data,
                i_predict_npc, r)
    variable v : RegistersType;
    variable w_o_req_valid : std_logic;
    variable w_o_req_fire : std_logic;
    variable w_o_hold : std_logic;
    variable wb_o_pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable wb_o_instr : std_logic_vector(31 downto 0);
    variable w_o_valid : std_logic;
  begin

    v := r;

    w_o_req_valid := not i_pipeline_hold
        and not (r.wait_resp and not i_mem_data_valid)
        and not i_progbuf_ena;
    w_o_req_fire := i_mem_req_ready and w_o_req_valid;

    w_o_hold := not (r.wait_resp and i_mem_data_valid)
                and not i_progbuf_ena;

    if w_o_req_fire = '1' then
        v.wait_resp := '1';
    elsif i_mem_data_valid = '1' and i_pipeline_hold = '0' then
        v.wait_resp := '0';
    end if;

    if i_mem_data_valid = '1' and r.wait_resp = '1' and i_pipeline_hold = '0' then
        v.resp_valid := '1';
        v.resp_address := i_mem_data_addr;
        v.resp_data := i_mem_data;
        v.instr_load_fault := i_mem_load_fault;
        v.instr_executable := i_mem_executable;
    end if;
    if i_flush_pipeline = '1' then
        -- Clear pipeline stage
        v.resp_address := (others => '1');
    end if;

    wb_o_pc := r.resp_address;

    if i_progbuf_ena = '1' then
        wb_o_instr := i_progbuf_data;
        wb_o_pc    := i_progbuf_pc;
    else
        wb_o_instr := r.resp_data;
        wb_o_instr := r.resp_data;
    end if;
    w_o_valid := (r.resp_valid or i_progbuf_ena)
            and not (i_pipeline_hold or w_o_hold);


    -- Breakpoint skip logic that allows to continue execution
    -- without actual breakpoint remove only once 
    if wb_o_pc = r.br_address then
        wb_o_instr := r.br_instr;
        if i_mem_data_valid = '1' and r.wait_resp = '1' and i_pipeline_hold = '0' then
            v.br_address := (others => '1');
        end if;
    end if;

    
    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_mem_addr_valid <= w_o_req_valid;
    o_mem_addr <= i_predict_npc;
    o_mem_req_fire <= w_o_req_fire;
    o_instr_load_fault <= r.instr_load_fault;
    o_instr_executable <= r.instr_executable;
    o_valid <= w_o_valid;
    o_pc <= wb_o_pc;
    o_instr <= wb_o_instr;
    o_mem_resp_ready <= r.wait_resp and not i_pipeline_hold;
    o_hold <= w_o_hold;
    
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
