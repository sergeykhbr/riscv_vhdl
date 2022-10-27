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

entity axi4_defslv is
  generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_xslvi : in axi4_slave_in_type;
    o_xslvo : out axi4_slave_out_type
  );
end; 
 
architecture arch_axi4_defslv of axi4_defslv is

  type defslv_state_type is (DefSlave_Idle, DefSlave_R, DefSlave_W, DefSlave_B);

  type registers_type is record
    state : defslv_state_type;
    burst_cnt : std_logic_vector(7 downto 0);
  end record;

  constant R_RESET : registers_type := (DefSlave_Idle, X"00");

  signal rin, r : registers_type;

begin

  comblogic : process(i_nrst, i_xslvi, r)
    variable v : registers_type;
    variable vslvo : axi4_slave_out_type;
  begin
    v := r;
    vslvo := axi4_slave_out_none;
    vslvo.b_resp := AXI_RESP_DECERR;
    vslvo.r_resp := AXI_RESP_DECERR;

    case r.state is
    when DefSlave_Idle =>
        vslvo.ar_ready := '1';
        vslvo.aw_ready := '1';
        if i_xslvi.aw_valid = '1' then
            vslvo.ar_ready := '0';
            v.state := DefSlave_W;
            v.burst_cnt := i_xslvi.aw_bits.len;
        elsif i_xslvi.ar_valid = '1' then
            v.state := DefSlave_R;
            v.burst_cnt := i_xslvi.ar_bits.len;
        end if;
    when DefSlave_R =>
        vslvo.r_valid := '1';
        vslvo.r_data := (others => '1');
        if r.burst_cnt = X"00" then
           v.state := DefSlave_Idle;
           vslvo.r_last := '1';
        else
           v.burst_cnt := r.burst_cnt - 1;
        end if;
    when DefSlave_W =>
        vslvo.w_ready := '1';
        if r.burst_cnt = X"00" then
           v.state := DefSlave_B;
        else
           v.burst_cnt := r.burst_cnt - 1;
        end if;
    when DefSlave_B =>
        vslvo.b_valid := '1';
        v.state := DefSlave_Idle;
    when others =>
    end case;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
    o_xslvo <= vslvo;
  end process;

  -- registers
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
         r <= R_RESET;
     elsif rising_edge(i_clk) then 
         r <= rin;
     end if; 
  end process;
end;
