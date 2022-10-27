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

entity Double2Long is 
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
    o_overflow   : out std_logic;
    o_underflow  : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
end; 
 
architecture arch_Double2Long of Double2Long is

  type RegistersType is record
    busy : std_logic;
    ena : std_logic_vector(2 downto 0);
    signA : std_logic;
    expA : std_logic_vector(10 downto 0);
    mantA : std_logic_vector(52 downto 0);
    result : std_logic_vector(63 downto 0);
    op_signed : std_logic;
    w32 : std_logic;
    mantPostScale : std_logic_vector(63 downto 0);
    overflow : std_logic;
    underflow : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'),                      -- busy, ena
    '0', (others => '0'), (others => '0'),     -- signA, expA, mantA
    (others => '0'), '0', '0',                 -- result, op_signed, w32
    (others => '0'), '0', '0'                  -- mantPostScale, overflow, underflow
  );

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  signal r, rin : RegistersType;

begin

  -- registers:
  comb : process(i_nrst, i_ena, i_signed, i_w32, i_a, r)
    variable v : RegistersType;
    variable expDif : std_logic_vector(11 downto 0);
    variable mantPreScale : std_logic_vector(63 downto 0);
    variable mantPostScale : std_logic_vector(63 downto 0);
    variable mantA : std_logic_vector(52 downto 0);
    variable expDif_gr : std_logic;  -- greater than 1023 + 63
    variable expDif_lt : std_logic;  -- less than 1023
    variable overflow : std_logic;
    variable underflow : std_logic;
    variable expMax : std_logic_vector(10 downto 0);
    variable expShift : std_logic_vector(5 downto 0);
    variable resSign : std_logic;
    variable resMant : std_logic_vector(63 downto 0);
    variable res : std_logic_vector(63 downto 0);
  begin

    v := r;

    v.ena := r.ena(1 downto 0) & (i_ena and not r.busy);

    mantA(51 downto 0) := i_a(51 downto 0);
    mantA(52) := '0';
    if i_a(62 downto 52) /= zero64(10 downto 0) then
        mantA(52) := '1';
    end if;

    if i_ena = '1' then
        v.busy := '1';
        v.signA := i_a(63);
        v.expA := i_a(62 downto 52);
        v.mantA := mantA;
        v.op_signed := i_signed;
        v.w32 := i_w32;
        v.overflow := '0';
        v.underflow := '0';
    end if;

    -- expShift = (1086 - expA)[5:0]
    expShift := "111110" - r.expA(5 downto 0);
    if r.w32 = '1' then
        if r.op_signed = '1' then
            expMax := conv_std_logic_vector(1053, 11);
        else
            expMax := conv_std_logic_vector(1085, 11);
        end if;
    else
        if r.op_signed = '1' or r.signA = '1' then
            expMax := conv_std_logic_vector(1085, 11);
        else
            expMax := conv_std_logic_vector(1086, 11);
        end if;
     end if;

    expDif := ('0' & expMax) - ('0' & r.expA);
    expDif_gr := expDif(11);
    expDif_lt := '0';
    if r.expA /= "01111111111" and r.expA(10) = '0' then
        expDif_lt := '1';
    end if;

    mantPreScale := r.mantA & "00000000000";

    mantPostScale := (others => '0');
    if expDif_gr = '1' then
        overflow := '1';
        underflow := '0';
    elsif expDif_lt = '1' then
        overflow := '0';
        underflow := '1';
    else
        overflow := '0';
        underflow := '0';
        -- Multiplexer, probably switch case in rtl
        if expShift = "000000" then
            mantPostScale := mantPreScale;
        else
            for i in 1 to 63 loop
                if conv_integer(expShift) = i then
                    mantPostScale := zero64(i-1 downto 0) & mantPreScale(63 downto i);
                end if;
            end loop;
        end if;
    end if;

    if r.ena(0) = '1' then
        v.overflow := overflow;
        v.underflow := underflow;
        v.mantPostScale := mantPostScale;
    end if;

    -- Result multiplexers:
    resSign := (r.signA or r.overflow) and not r.underflow;
    if r.signA = '1' then
        resMant := not r.mantPostScale + 1;
    else
        resMant := r.mantPostScale;
    end if;

    res := resMant;
    if r.op_signed = '1' then
        if resSign = '1' then
            if r.w32 = '1' then
                res(63 downto 31) := (others => '1');
            else
                res(63) := '1';
            end if;
        end if;
    else
        if r.w32 = '1' then
            res(63 downto 32) := (others => '0');
        elsif r.overflow = '1' then
            res(63) := '1';
        end if;
    end if;

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
  o_overflow <= r.overflow;
  o_underflow <= r.underflow;
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
