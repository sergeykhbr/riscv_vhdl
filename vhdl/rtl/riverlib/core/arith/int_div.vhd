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
use ieee.std_logic_misc.all;  -- or_reduce()
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity IntDiv is generic (
    async_reset : boolean := false
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;                               -- Reset Active LOW
    i_ena : in std_logic;                                -- Enable bit
    i_unsigned : in std_logic;                           -- Unsigned operands
    i_rv32 : in std_logic;                               -- 32-bits operands enable
    i_residual : in std_logic;                           -- Compute: 0 =division; 1=residual
    i_a1 : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Operand 1
    i_a2 : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Operand 1
    o_res : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Result
    o_valid : out std_logic;                             -- Result is valid
    o_busy : out std_logic                               -- Multiclock instruction under processing
  );
end; 
 
architecture arch_IntDiv of IntDiv is

  component divstage64 is 
  port (
    i_divident   : in std_logic_vector(63 downto 0);  -- integer value
    i_divisor    : in std_logic_vector(123 downto 0); -- integer value
    o_resid      : out std_logic_vector(63 downto 0); -- residual value
    o_bits       : out std_logic_vector(3 downto 0)   -- resulting bits
  );
  end component; 

  type RegistersType is record
      rv32 : std_logic;
      resid : std_logic;                           -- Compute residual flag
      invert : std_logic;                          -- invert result value before output
      busy : std_logic;
      div_on_zero : std_logic;
      ena : std_logic_vector(9 downto 0);
      divident_i : std_logic_vector(63 downto 0);
      divisor_i : std_logic_vector(119 downto 0);
      bits_i : std_logic_vector(63 downto 0);
      result : std_logic_vector(RISCV_ARCH-1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
      '0', '0', '0', '0',                -- rv32, resid, invert, busy
      '0',                               -- div_on_zero
      (others => '0'), (others => '0'),  -- ena, divident_i
      (others => '0'), (others => '0'),  -- divisor_i, bits_i
      (others => '0')                    -- result
  );

  signal r, rin : RegistersType;

  signal wb_divisor0_i : std_logic_vector(123 downto 0);
  signal wb_divisor1_i : std_logic_vector(123 downto 0);
  signal wb_resid0_o : std_logic_vector(63 downto 0);
  signal wb_resid1_o : std_logic_vector(63 downto 0);
  signal wb_bits0_o : std_logic_vector(3 downto 0);
  signal wb_bits1_o : std_logic_vector(3 downto 0);

begin

  stage0 : divstage64 port map (
    i_divident => r.divident_i,
    i_divisor => wb_divisor0_i,
    o_bits => wb_bits0_o,
    o_resid => wb_resid0_o
  );

  stage1 : divstage64 port map (
    i_divident => wb_resid0_o,
    i_divisor => wb_divisor1_i,
    o_bits => wb_bits1_o,
    o_resid => wb_resid1_o
  );

  comb : process(i_nrst, i_ena, i_unsigned, i_residual, i_rv32, i_a1, i_a2, r,
                 wb_resid0_o, wb_resid1_o, wb_bits0_o, wb_bits1_o)
    variable v : RegistersType;
    variable wb_a1 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_a2 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_divident : std_logic_vector(64 downto 0);
    variable wb_divider : std_logic_vector(64 downto 0);
    variable w_invert64 : std_logic;
    variable w_invert32 : std_logic;
    variable vb_rem : std_logic_vector(63 downto 0);
    variable vb_div : std_logic_vector(63 downto 0);
  begin

    v := r;

    w_invert64 := '0';
    w_invert32 := '0';
    wb_divident(64) := '0';
    wb_divider(64) := '0';

    if i_rv32 = '1' then
        wb_a1(63 downto 32) := (others => '0');
        wb_a2(63 downto 32) := (others => '0');
        if i_unsigned = '1' or i_a1(31) = '0' then
            wb_a1(31 downto 0) := i_a1(31 downto 0);
        else
            wb_a1(31 downto 0) := (not i_a1(31 downto 0)) + 1;
        end if;
        if i_unsigned = '1' or i_a2(31) = '0' then
            wb_a2(31 downto 0) := i_a2(31 downto 0);
        else
            wb_a2(31 downto 0) := (not i_a2(31 downto 0)) + 1;
        end if;
    else
        if i_unsigned = '1' or i_a1(63) = '0' then
            wb_a1 := i_a1;
        else
            wb_a1 := (not i_a1) + 1;
        end if;
        if i_unsigned = '1' or i_a2(63) = '0' then
            wb_a2 := i_a2;
        else
            wb_a2 := (not i_a2) + 1;
        end if;
    end if;

    v.ena := r.ena(8 downto 0) & (i_ena and not r.busy);


    if r.invert = '1' then
        vb_rem := (not r.divident_i) + 1;
    else
        vb_rem := r.divident_i;
    end if;

    if r.invert = '1' then
        vb_div := (not r.bits_i) + 1;
    else
        vb_div := r.bits_i;
    end if;

    -- DIVW, DIVUW, REMW and REMUW sign-extended accordingly with 
    -- User Level ISA v2.2
    if r.rv32 = '1' then
        vb_div(63 downto 32) := (others => vb_div(31));
        vb_rem(63 downto 32) := (others => vb_rem(31));
    end if;
 

    if i_ena = '1' then
        v.busy := '1';
        v.rv32 := i_rv32;
        v.resid := i_residual;

        v.divident_i := wb_a1;
        v.divisor_i := wb_a2 & X"00000000000000";

        w_invert32 := not i_unsigned and
                ((not i_residual and (i_a1(31) xor i_a2(31)))
                or (i_residual and i_a1(31)));
        w_invert64 := not i_unsigned and
                ((not i_residual and (i_a1(63) xor i_a2(63)))
                or (i_residual and i_a1(63)));
        v.invert := (not i_rv32 and w_invert64) 
                or (i_rv32 and w_invert32);

        -- Compatibility with riscv-tests but loose compatibility with x86
        if i_rv32 = '1' then
            if i_unsigned = '1' then
               v.div_on_zero := not or_reduce(i_a2(31 downto 0));
            else
               v.div_on_zero := not or_reduce(i_a2(30 downto 0));
            end if;
        else
            if i_unsigned = '1' then
               v.div_on_zero := not or_reduce(i_a2(63 downto 0));
            else
               v.div_on_zero := not or_reduce(i_a2(62 downto 0));
            end if;
        end if;
    elsif r.ena(8) = '1' then
        v.busy := '0';
        if r.resid = '1' then
            v.result := vb_rem;
        elsif r.div_on_zero = '1' then
            v.result := (others => '1');
        else
            v.result := vb_div;
        end if;
    elsif r.busy = '1' then
        v.divident_i := wb_resid1_o;
        v.divisor_i := X"00" & r.divisor_i(119 downto 8);
        v.bits_i := r.bits_i(55 downto 0) & wb_bits0_o & wb_bits1_o;
    end if;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    wb_divisor0_i <= r.divisor_i & "0000";
    wb_divisor1_i <= "0000" & r.divisor_i;

    o_res <= r.result;
    o_valid <= r.ena(9);
    o_busy <= r.busy;
    
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
