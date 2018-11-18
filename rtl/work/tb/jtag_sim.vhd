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
    clock_rate : integer := 10;
    irlen : integer := 4
  ); 
  port (
    rst : in std_logic;
    clk : in std_logic;
    i_test_ena : in std_logic;
    i_test_burst : in std_logic_vector(7 downto 0);
    i_test_addr : in std_logic_vector(31 downto 0);
    i_test_we : in std_logic;
    i_test_wdata : in std_logic_vector(31 downto 0);
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

  constant ADDR_WIDTH : integer := 32;
  constant DATA_WIDTH : integer := 32;
  constant REG1_LEN : integer := ADDR_WIDTH + 3; -- 3 = 1 bit we field + 2 bits transaction size
  constant REG2_LEN : integer := DATA_WIDTH + 1; -- 1 bulk bit
  constant SHIFT_LEN : integer := irlen + REG1_LEN; -- maximal length

  type registers is record
      jtagstate : state_type;
      jtagstatez : state_type;
      burst_cnt : integer;
      shift_reg1 : std_logic_vector(REG1_LEN-1 downto 0);
      shift_reg2 : std_logic_vector(REG2_LEN-1 downto 0);
      shift_reg : std_logic_vector(irlen + REG1_LEN-1 downto 0);
      shift_length : integer;
      clk_rate_cnt : integer;
      is_data : std_logic;
      instr : integer range 1 to 2;
      op_cnt : integer;
      rdata : std_logic_vector(DATA_WIDTH-1 downto 0);
      edge : std_logic;
      ntrst : std_logic;
      tms : std_logic;
  end record;
  
  signal r, rin : registers;
  
begin

  comblogic : process(rst, r, i_tdi, i_test_ena, i_test_addr, i_test_we, i_test_wdata)
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

     if i_test_ena = '1' and r.jtagstate = run_idle then
        v.burst_cnt := conv_integer(i_test_burst);
        v.shift_reg1(34) := i_test_we;
        v.shift_reg1(33 downto 32) := "10"; -- size: 0=1 byte; 1=hword; 2=word; 3=dword
        v.shift_reg1(31 downto 0) := i_test_addr;
        if i_test_burst = X"00" then
            v.shift_reg2(32) := '0'; -- bulk=0
        else
            v.shift_reg2(32) := '1'; -- bulk=1
        end if;
        v.shift_reg2(31 downto 0) := i_test_wdata;
        v.is_data := '1';
     elsif w_posedge = '1' then
        v.is_data := '0';
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
                v.tms := '1';
                v.instr := 1;
                v.op_cnt := 0;
                v.jtagstate := start_ir;
            end if;
        when start_ir => -- the same as select_dr
            v.tms := '1';
            v.jtagstate := select_ir;

        when select_ir =>
            v.tms := '0';
            v.jtagstate := capture_ir;
        when capture_ir =>
            v.tms := '0'; 
            v.op_cnt := 0;
            v.jtagstate := shift_ir;
            if r.instr = 1 then
                v.shift_reg := r.shift_reg1 & conv_std_logic_vector(2, irlen);
                v.shift_length := 35-1;
            else
                v.shift_reg := "00" & r.shift_reg2 & conv_std_logic_vector(3, irlen);
                v.shift_length := 33-1;
            end if;
            v.op_cnt := 0; 
        when shift_ir =>
            v.tms := '0';
            v.op_cnt := r.op_cnt + 1;
            if r.op_cnt = irlen-1 then
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
            v.rdata := i_tdi & r.rdata(DATA_WIDTH-1 downto 1);
            v.op_cnt := r.op_cnt + 1;
            if r.op_cnt = r.shift_length then
                v.tms := '1';
                v.jtagstate := exit1_dr;
            end if;
        when exit1_dr =>
            v.tms := '1';
            v.rdata := i_tdi & r.rdata(DATA_WIDTH-1 downto 1);
            v.jtagstate := update_dr;
        when update_dr =>
            -- TODO: and size == size_bulk
            if r.instr = 2 then
                if r.burst_cnt = 0 then
                    v.tms := '0'; 
                    v.jtagstate := run_idle;
                else
                    v.tms := '1'; 
                    v.jtagstate := start_ir;
                    v.burst_cnt := r.burst_cnt - 1;
                    if r.burst_cnt = 1 then
                        v.shift_reg2(32) := '0'; -- bulk=0
                    end if;
                end if;
            else
                v.tms := '1'; 
                v.instr := 2;
                v.jtagstate := start_ir;
            end if;
        when others =>
        end case;


        if r.jtagstatez = shift_ir or r.jtagstatez = shift_dr then
            v.shift_reg := '0' & r.shift_reg(SHIFT_LEN-1 downto 1);
        end if;
     end if;

     -- Reset
     if rst = '1' then
        v.jtagstate := test_rst;
        v.jtagstatez := test_rst;
        v.clk_rate_cnt := 0;
        v.op_cnt := 0;
        v.is_data := '0';
        v.shift_reg1 := (others => '0');
        v.shift_reg2 := (others => '0');
        v.shift_reg := (others => '0');
        v.burst_cnt := 0;
        v.edge := '0';
        v.ntrst := '0';
        v.tms := '0';
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
