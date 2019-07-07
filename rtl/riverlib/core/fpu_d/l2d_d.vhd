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

entity Long2Double is 
  generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_signed     : in std_logic;
    i_w32        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
end; 
 
architecture arch_Long2Double of Long2Double is
 
  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type RegistersType is record
    busy : std_logic;
    ena : std_logic_vector(2 downto 0);
    signA : std_logic;
    absA : std_logic_vector(63 downto 0);
    result : std_logic_vector(63 downto 0);
    op_signed : std_logic;
    mantAlign : std_logic_vector(63 downto 0);
    lshift : integer range 0 to 63;
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'),                      -- busy, ena
    '0', (others => '0'), (others => '0'),     -- signA, absA, result
    '0', (others => '0'), 0                    -- op_signed, mantAlign, lshift
  );

  signal r, rin : RegistersType;

begin

  -- registers:
  comb : process(i_nrst, i_ena, i_signed, i_w32, i_a, r)
    variable v : RegistersType;
    variable mantAlign : std_logic_vector(63 downto 0);
    variable lshift : integer range 0 to 63;
    variable expAlign : std_logic_vector(10 downto 0);
    variable mantEven : std_logic;
    variable mant05 : std_logic;
    variable mantOnes : std_logic;
    variable rndBit : std_logic;
    variable v_signA : std_logic;
    variable vb_A : std_logic_vector(63 downto 0);
    variable res : std_logic_vector(63 downto 0);
  begin

    v := r;

    v.ena := r.ena(1 downto 0) & (i_ena and not r.busy);
    if i_w32 = '0' then
        v_signA := i_a(63);
        vb_A := i_a;
    elsif i_signed = '1' and i_a(31) = '1' then
        v_signA := '1';
        vb_A(63 downto 32) := (others => '1');
        vb_A(31 downto 0) := i_a(31 downto 0);
    else
        v_signA := '0';
        vb_A(31 downto 0) := i_a(31 downto 0);
        vb_A(63 downto 32) := (others => '0');
    end if;

    if i_ena = '1' then
        v.busy := '1';
        if i_signed = '1' and v_signA = '1' then
            v.signA := '1';
            v.absA := not vb_A + 1;
        else
            v.signA := '0';
            v.absA := vb_A;
        end if;
        v.op_signed := i_signed;
    end if;

    -- multiplexer, probably if/elsif in rtl:
    mantAlign := (others => '0');
    lshift := 63;
    if r.absA(63) = '1' then
        mantAlign := r.absA;
    else
        for i in 1 to 63 loop
            if lshift = 63 and r.absA(63 - i) = '1' then
                mantAlign := r.absA(63-i downto 0) & zero64(i-1 downto 0);
                lshift := i;
            end if;
        end loop;
    end if;

    if r.ena(0) = '1' then
        v.mantAlign := mantAlign;
        v.lshift := lshift;
    end if;

    if r.absA = zero64 then
        expAlign := (others => '0');
    else
        expAlign := conv_std_logic_vector(1086 - r.lshift, 11);
    end if;

    mantEven := r.mantAlign(11);
    mant05 := '0';
    if r.mantAlign(10 downto 0) = "11111111111" then
        mant05 := '1';
    end if;
    rndBit := r.mantAlign(10) and not(mant05 and mantEven);
    mantOnes := '0';
    if r.mantAlign(63) = '1' and r.mantAlign(62 downto 11) = X"fffffffffffff" then
        mantOnes := '1';
    end if;

    -- Result multiplexers:
    res(63) := r.signA and r.op_signed;
    res(62 downto 52) := expAlign + ("0000000000" & (mantOnes and rndBit));
    res(51 downto 0) := r.mantAlign(62 downto 11) + rndBit;

    if r.ena(1) = '1' then
        v.result := res;
        v.busy := '0';
    end if;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
  end process;

  o_res <= r.result;
  o_valid <= r.ena(2);
  o_busy <= r.busy;
  
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
