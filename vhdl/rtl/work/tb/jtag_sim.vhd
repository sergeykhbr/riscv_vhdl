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

library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;
use commonlib.types_util.all;

entity jtag_sim is 
  generic (
    clock_rate : integer := 10
  ); 
  port (
    rst : in std_logic;
    clk : in std_logic;
    i_dtmcs_re : in std_logic;
    i_dmi_ena : in std_logic;
    i_dmi_we : in std_logic;
    i_dmi_re : in std_logic;
    i_dmi_addr : in std_logic_vector(6 downto 0);
    i_dmi_wdata : in std_logic_vector(31 downto 0);
    i_tdi  : in std_logic;
    o_tck : out std_logic;
    o_ntrst : out std_logic;
    o_tms : out std_logic;
    o_tdo : out std_logic
  );
end;

architecture jtag_sim_rtl of jtag_sim is

  type state_type is (test_rst, run_idle, start_ir, select_dr, capture_dr, shift_dr, exit1_dr,
                      pause_dr, exit2_dr, update_dr, select_ir, capture_ir, shift_ir,
                      exit1_ir, pause_ir, exit2_ir, update_ir);

  type registers is record
      jtagstate : state_type;
      jtagstatez : state_type;
      dmi_request : std_logic_vector(40 downto 0);
      dmi_resp_addr : std_logic_vector(6 downto 0);
      dmi_resp_data : std_logic_vector(31 downto 0);
      dmi_resp_stat : std_logic_vector(1 downto 0);
      shift_reg : std_logic_vector(45 downto 0);
      shift_length : integer;
      is_data : std_logic;
      dtmcs_ena : std_logic;
      clk_rate_cnt : integer;
      op_cnt : integer;
      rdata : std_logic_vector(40 downto 0);
      edge : std_logic;
      ntrst : std_logic;
      tms : std_logic;
      dmi_dtm : std_logic;  -- last was: 0=DMI; 1=DTM
  end record;
  
  signal r, rin : registers;
  
begin

  comblogic : process(rst, r, i_tdi, i_dtmcs_re, i_dmi_ena, i_dmi_we, i_dmi_re, i_dmi_addr, i_dmi_wdata)
    variable v : registers;
    variable w_posedge : std_logic;
    variable w_negedge : std_logic;
  begin
     v := r;
     w_posedge := '0';
     w_negedge := '0';
     if r.clk_rate_cnt = (clock_rate - 1) then
         v.clk_rate_cnt := 0;
         v.edge := not r.edge;
         w_posedge := not r.edge;
         w_negedge := r.edge;
     else
         v.clk_rate_cnt := r.clk_rate_cnt + 1;
     end if;

     if (i_dtmcs_re or i_dmi_ena) = '1' and r.jtagstate = run_idle then
        v.is_data := '1';
        v.dtmcs_ena := i_dtmcs_re;
        v.dmi_request(0) := i_dmi_re;
        v.dmi_request(1) := i_dmi_we;
        v.dmi_request(33 downto 2) := i_dmi_wdata;
        v.dmi_request(40 downto 34) := i_dmi_addr;
     end if;

     if w_posedge = '1' then
        v.jtagstatez := r.jtagstate;

        case r.jtagstate is
        when test_rst =>
            v.ntrst := '0';
            v.op_cnt := r.op_cnt + 1;
            if r.op_cnt = 3 then
                v.jtagstate := run_idle;
                v.op_cnt := 0;
                v.ntrst := '1';
            end if;
        when run_idle =>
            if r.is_data = '1' then
                v.is_data := '0';
                v.tms := '1';
                v.op_cnt := 0;
                v.jtagstate := start_ir;
            end if;
        when start_ir => -- the same as select_dr
            if r.dmi_dtm /= r.dtmcs_ena then
                -- IR changed
                v.tms := '1';
                v.jtagstate := select_ir;
            else
                -- IR the same
                v.tms := '0';
                v.jtagstate := capture_dr;
                if r.dtmcs_ena = '1' then
                    v.shift_reg := (others => '0');
                    v.shift_length := 32-1;
                else
                    v.shift_reg := "00000" & r.dmi_request;
                    v.shift_length := 41-1;
                end if;
            end if;

        when select_ir =>
            v.tms := '0';
            v.jtagstate := capture_ir;
        when capture_ir =>
            v.tms := '0'; 
            v.op_cnt := 0;
            v.jtagstate := shift_ir;
            if r.dtmcs_ena = '1' then
                v.shift_reg := (others => '0');
                v.shift_reg(4 downto 0) := "10000";   -- DTMCS reg
                v.shift_length := 32-1;
            else
                v.shift_reg := r.dmi_request & "10001";
                v.shift_length := 41-1;
            end if;
            v.op_cnt := 0; 
        when shift_ir =>
            v.tms := '0';
            v.op_cnt := r.op_cnt + 1;
            if r.op_cnt = 4 then
                v.tms := '1';
                v.jtagstate := exit1_ir;
            end if;
        when exit1_ir =>
            v.tms := '1';
            v.jtagstate := update_ir;
        when update_ir =>
            v.tms := '1'; 
            v.jtagstate := select_dr;

        when select_dr =>
            v.tms := '0';
            v.jtagstate := capture_dr;
        when capture_dr => 
            v.tms := '0'; 
            v.op_cnt := 0; 
            v.jtagstate := shift_dr;
            v.rdata := (others => '0');
        when shift_dr =>
            v.tms := '0';
            if r.dtmcs_ena = '1' then
                v.rdata(31 downto 0) := i_tdi & r.rdata(31 downto 1);
            else
                v.rdata := i_tdi & r.rdata(40 downto 1);
            end if;
            v.op_cnt := r.op_cnt + 1;
            if r.op_cnt = r.shift_length then
                v.tms := '1';
                v.jtagstate := exit1_dr;
            end if;
        when exit1_dr =>
            v.tms := '1';
            if r.dtmcs_ena = '1' then
                v.rdata(31 downto 0) := i_tdi & r.rdata(31 downto 1);
            else
                v.rdata := i_tdi & r.rdata(40 downto 1);
            end if;
            v.jtagstate := update_dr;
        when update_dr =>
            v.dmi_dtm := r.dtmcs_ena;
            if r.dtmcs_ena = '0' then
                v.dmi_resp_addr := r.rdata(40 downto 34);
                v.dmi_resp_data := r.rdata(33 downto 2);
                v.dmi_resp_stat := r.rdata(1 downto 0);
            end if;
            v.tms := '0'; 
            v.jtagstate := run_idle;
        when others =>
        end case;


        if r.jtagstatez = shift_ir or r.jtagstatez = shift_dr then
            v.shift_reg := '0' & r.shift_reg(45 downto 1);
        end if;
     end if;

     -- Reset
     if rst = '1' then
        v.jtagstate := test_rst;
        v.jtagstatez := test_rst;
        v.clk_rate_cnt := 0;
        v.op_cnt := 0;
        v.is_data := '0';
        v.dtmcs_ena := '0';
        v.dmi_request := (others => '0');
        v.dmi_resp_addr := (others => '0');
        v.dmi_resp_data := (others => '0');
        v.dmi_resp_stat := (others => '0');
        v.shift_reg := (others => '0');
        v.edge := '0';
        v.ntrst := '0';
        v.tms := '0';
        v.dmi_dtm := '0';
     end if;

     rin <= v;
   end process;

   o_tdo <= r.shift_reg(0);
   o_tck <= r.edge;
   o_tms <= r.tms;
   o_ntrst <= r.ntrst;


  procCheck : process (clk)
  begin
    if rising_edge(clk) then
        r <= rin;
    end if;
  end process;
  
end;
