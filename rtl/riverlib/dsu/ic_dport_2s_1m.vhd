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
--! @brief  DPort interconnect to provided access for 2 sources:
--!            1. Direct access from DSU to all cores
--!            2. DMI registers access
-----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;  -- or_reduce()
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_river.all;

entity ic_dport_2s_1m is
  generic (
    async_reset : boolean := false
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    -- Group <=> DMI interface
    i_sdport0i : in dport_in_vector;
    o_sdport0o : out dport_out_vector;
    -- Group <=> DSU interface
    i_sdport1i : in dport_in_vector;
    o_sdport1o : out dport_out_vector;
    -- Group connection
    o_mdporti : out dport_in_vector;
    i_mdporto : in dport_out_vector
  );
end;

architecture arch_ic_dport_2s_1m of ic_dport_2s_1m is

  type state_type is (
      idle,
      dport_request,
      dport_response,
      slave_accept
   );

  type rdata_type is array (0 to CFG_TOTAL_CPU_MAX-1) 
       of std_logic_vector(RISCV_ARCH-1 downto 0);

  type registers is record
    state : state_type;
    idx : std_logic;
    mst_req_valid : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);
    mst_resp_ready : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);
    dporti : dport_in_vector;
    rdata : rdata_type;
  end record;

  signal r, rin: registers;

begin

  comblogic : process(nrst, i_sdport0i, i_sdport1i, i_mdporto, r)
    variable v : registers;
    variable vb_req_valid : std_logic_vector(1 downto 0);
    variable vb_ic_req_ready : std_logic_vector(1 downto 0);
    variable v_dport_request : std_logic;
    variable v_dport_response : std_logic;
    variable vb_slv0_resp_valid : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);
    variable vb_slv1_resp_valid : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);
    variable vb_ic_req_valid : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);
  begin
    v := r;
    -- Slave request 0:
    vb_req_valid(0) := '0';
    for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
        vb_req_valid(0) := vb_req_valid(0) or i_sdport0i(n).req_valid;
    end loop;
    -- Slave request 1:
    vb_req_valid(1) := '0';
    for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
        vb_req_valid(1) := vb_req_valid(1) or i_sdport1i(n).req_valid;
    end loop;

    vb_ic_req_ready := (others => '0');
    v_dport_request := '0';
    v_dport_response := '0';
    vb_slv0_resp_valid := (others => '0');
    vb_slv1_resp_valid := (others => '0');
    for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
        vb_ic_req_valid(n) := r.dporti(n).req_valid;
    end loop;

    case (r.state) is
    when idle =>
        for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
            v.mst_req_valid := (others => '0');
            v.mst_resp_ready := (others => '0');
        end loop;

        -- Slave request 1 has low priority than 0:
        if vb_req_valid(0) = '1' then
            vb_ic_req_ready(0) := '1';
            v.idx := '0';
            v.state := dport_request;
            for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
                v.mst_req_valid(n) := i_sdport0i(n).req_valid;
                v.mst_resp_ready(n) := i_sdport0i(n).req_valid;
                v.dporti(n).req_valid := i_sdport0i(n).req_valid;
                v.dporti(n).write := i_sdport0i(n).write;
                v.dporti(n).addr := i_sdport0i(n).addr;
                v.dporti(n).wdata := i_sdport0i(n).wdata;
            end loop;
        elsif vb_req_valid(1) = '1' then
            vb_ic_req_ready(1) := '1';
            v.idx := '1';
            v.state := dport_request;
            for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
                v.mst_req_valid(n) := i_sdport1i(n).req_valid;
                v.mst_resp_ready(n) := i_sdport1i(n).req_valid;
                v.dporti(n).req_valid := i_sdport1i(n).req_valid;
                v.dporti(n).write := i_sdport1i(n).write;
                v.dporti(n).addr := i_sdport1i(n).addr;
                v.dporti(n).wdata := i_sdport1i(n).wdata;
            end loop;
        end if;
    when dport_request =>
        v_dport_request := '1';
        for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
            if i_mdporto(n).req_ready = '1' then
                v.mst_req_valid(n) := '0';
            end if;
        end loop;
        if or_reduce(v.mst_req_valid) = '0' then
            v.state := dport_response;
        end if;
    when dport_response =>
        v_dport_response := '1';
        for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
            if i_mdporto(n).resp_valid = '1' then
                v.mst_resp_ready(n) := '0';
                v.rdata(n) := i_mdporto(n).rdata;
            end if;
        end loop;
        if or_reduce(v.mst_resp_ready) = '0' then
            v.state := slave_accept;
        end if;

    when slave_accept =>
        if r.idx = '0' then
            vb_slv0_resp_valid := vb_ic_req_valid;
            for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
                if i_sdport0i(n).resp_ready = '1' then
                    v.dporti(n).req_valid := '0';
                end if;
            end loop;
        else
            vb_slv1_resp_valid := vb_ic_req_valid;
            for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
                if i_sdport1i(n).resp_ready = '1' then
                    v.dporti(n).req_valid := '0';
                end if;
            end loop;
        end if;
        if or_reduce(vb_ic_req_valid) = '0' then 
            v.state := idle;
        end if;
    when others =>
    end case;

    if not async_reset and nrst = '0' then 
         v.state := idle;
         v.idx := '0';
         v.mst_req_valid := (others => '0');
         v.mst_resp_ready := (others => '0');
         for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
             v.dporti(n) := dport_in_none;
             v.rdata(n) := (others => '0');
         end loop;
    end if;

    rin <= v;


    for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
        o_sdport0o(n).halted <= i_mdporto(n).halted;
        o_sdport0o(n).available <= i_mdporto(n).available;
        o_sdport0o(n).req_ready <= vb_ic_req_ready(0);
        o_sdport0o(n).resp_valid <= vb_slv0_resp_valid(n);
        o_sdport0o(n).rdata <= r.rdata(n);

        o_sdport1o(n).halted <= i_mdporto(n).halted;
        o_sdport1o(n).available <= i_mdporto(n).available;
        o_sdport1o(n).req_ready <= vb_ic_req_ready(1);
        o_sdport1o(n).resp_valid <= vb_slv1_resp_valid(n);
        o_sdport1o(n).rdata <= r.rdata(n);

        o_mdporti(n).req_valid <= r.mst_req_valid(n) and v_dport_request;
        o_mdporti(n).resp_ready <= r.mst_resp_ready(n) and v_dport_response;
        o_mdporti(n).write <= r.dporti(n).write;
        o_mdporti(n).addr <= r.dporti(n).addr;
        o_mdporti(n).wdata <= r.dporti(n).wdata;
    end loop;

  end process;

  -- registers:
  regs : process(clk, nrst)
  begin 
      if async_reset and nrst = '0' then
          r.state <= idle;
          r.idx <= '0';
          r.mst_req_valid <= (others => '0');
          r.mst_resp_ready <= (others => '0');
          for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
              r.dporti(n) <= dport_in_none;
              r.rdata(n) <= (others => '0');
          end loop;
      elsif rising_edge(clk) then 
          r <= rin;
      end if; 
  end process;

end;


