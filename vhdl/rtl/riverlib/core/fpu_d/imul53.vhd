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

entity imul53 is 
  generic (
    async_reset : boolean
  );
  port (
    i_nrst     : in std_logic;
    i_clk      : in std_logic;
    i_ena      : in std_logic;
    i_a        : in std_logic_vector(52 downto 0);
    i_b        : in std_logic_vector(52 downto 0);
    o_result   : out std_logic_vector(105 downto 0);
    o_shift    : out std_logic_vector(6 downto 0);
    o_rdy      : out std_logic;
    o_overflow : out std_logic
  );
end; 
 
architecture arch_imul53 of imul53 is

  type mux_type is array (16 downto 0) of std_logic_vector(56 downto 0);

  type RegistersType is record
    delay : std_logic_vector(15 downto 0);
    shift : std_logic_vector(6 downto 0);
    accum_ena : std_logic;
    b : std_logic_vector(55 downto 0);
    sum : std_logic_vector(105 downto 0);
    overflow : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    (others => '0'), (others => '0'), '0',
    (others => '0'), (others => '0'), '0');

  signal r, rin : RegistersType;

begin

  -- registers:
  comb : process(i_nrst, i_ena, i_a, i_b, r)
    variable v : RegistersType;
    variable vb_mux : mux_type;
    variable vb_sel : std_logic_vector(56 downto 0);
    variable vb_shift : std_logic_vector(6 downto 0);
    variable vb_sumInv : std_logic_vector(104 downto 0);
    variable vb_lshift_p1 : integer range 0 to 104;
    variable vb_lshift_p2 : integer range 0 to 104;
  begin

    v := r;

    vb_mux(0) := (others => '0');
    vb_mux(1) := "0000" & i_a;                   -- 1*a
    vb_mux(2) := "000" & i_a & '0';              -- 2*a
    vb_mux(3) := vb_mux(2) + vb_mux(1);          -- 2*a + 1*a
    vb_mux(4) := "00" & i_a & "00";              -- 4*a
    vb_mux(5) := vb_mux(4) + vb_mux(1);          -- 4*a + 1*a
    vb_mux(6) := vb_mux(4) + vb_mux(2);          -- 4*a + 2*a
    vb_mux(8) := '0' & i_a & "000";              -- 8*a
    vb_mux(7) := vb_mux(8) - vb_mux(1);          -- 8*a - 1*a
    vb_mux(9) := vb_mux(8) + vb_mux(1);          -- 8*a + 1*a
    vb_mux(10) := vb_mux(8) + vb_mux(2);         -- 8*a + 2*a
    vb_mux(11) := vb_mux(10) + vb_mux(1);        -- (8*a + 2*a) + 1*a
    vb_mux(12) := vb_mux(8) + vb_mux(4);         -- 8*a + 4*a
    vb_mux(16) := i_a & "0000";                  -- unused
    vb_mux(13) := vb_mux(16) - vb_mux(3);        -- 16*a - (2*a + 1*a)
    vb_mux(14) := vb_mux(16) - vb_mux(2);        -- 16*a - 2*a
    vb_mux(15) := vb_mux(16) - vb_mux(1);        -- 16*a - 1*a

    v.delay := r.delay(14 downto 0) & i_ena;
    if i_ena = '1' then
        v.b := "000" & i_b;
        v.overflow := '0';
        v.accum_ena := '1';
        v.sum := (others => '0');
        v.shift := (others => '0');
    elsif r.delay(13) = '1' then
        v.accum_ena := '0';
    end if;

    case r.b(55 downto 52) is
    when X"1" => vb_sel := vb_mux(1);
    when X"2" => vb_sel := vb_mux(2);
    when X"3" => vb_sel := vb_mux(3);
    when X"4" => vb_sel := vb_mux(4);
    when X"5" => vb_sel := vb_mux(5);
    when X"6" => vb_sel := vb_mux(6);
    when X"7" => vb_sel := vb_mux(7);
    when X"8" => vb_sel := vb_mux(8);
    when X"9" => vb_sel := vb_mux(9);
    when X"A" => vb_sel := vb_mux(10);
    when X"B" => vb_sel := vb_mux(11);
    when X"C" => vb_sel := vb_mux(12);
    when X"D" => vb_sel := vb_mux(13);
    when X"E" => vb_sel := vb_mux(14);
    when X"F" => vb_sel := vb_mux(15);
    when others =>
        vb_sel := (others => '0');
    end case;
    if r.accum_ena = '1' then
        v.sum := (r.sum(101 downto 0) & "0000") + vb_sel;
        v.b := r.b(51 downto 0) & "0000";
    end if;

    -- To avoid timing constrains violation occured in Vivado Studio
    -- try to implement parallel demuxultiplexer splitted on 2 parts
    vb_sumInv(0) := '0';
    for i in 0 to 103 loop
        vb_sumInv(i + 1) := r.sum(103 - i);
    end loop;
    
    vb_lshift_p1 := 0;
    for i in 0 to 63 loop
        if vb_lshift_p1 = 0 and vb_sumInv(i) = '1' then
            vb_lshift_p1 := i;
        end if;
    end loop;
    
    vb_lshift_p2 := 0;
    for i in 0 to 40 loop
        if vb_lshift_p2 = 0 and vb_sumInv(64 + i) = '1' then
            vb_lshift_p2 := 64 + i;
        end if;
    end loop;

    if r.sum(105) = '1' then
        vb_shift := "1111111";
        v.overflow := '1';
    elsif r.sum(104) = '1' then
        vb_shift := (others => '0');
    elsif vb_lshift_p1 /= 0 then
        vb_shift := conv_std_logic_vector(vb_lshift_p1, 7);
    else
        vb_shift := conv_std_logic_vector(vb_lshift_p2, 7);
    end if;

    if r.delay(14) = '1' then
        v.shift := vb_shift;
        v.overflow := '0';
        if vb_shift = "1111111" then
            v.overflow := '1';
        end if;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
  end process;

  o_result <= r.sum;
  o_shift <= r.shift;
  o_overflow <= r.overflow;
  o_rdy <= r.delay(15);
  
  -- registers:
  regs : process(i_nrst, i_clk)
  begin 
     if async_reset and i_nrst = '0' then
        r <= R_RESET;
     elsif rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
