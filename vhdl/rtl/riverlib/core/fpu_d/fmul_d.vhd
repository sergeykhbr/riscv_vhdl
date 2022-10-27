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

entity DoubleMul is 
  generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    i_b          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_illegal_op : out std_logic;
    o_overflow   : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
end; 
 
architecture arch_DoubleMul of DoubleMul is

  component imul53 is generic (
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
  end component;

  type RegistersType is record
    busy : std_logic;
    ena : std_logic_vector(4 downto 0);
    a : std_logic_vector(63 downto 0);
    b : std_logic_vector(63 downto 0);
    result : std_logic_vector(63 downto 0);
    zeroA : std_logic;
    zeroB : std_logic;
    mantA : std_logic_vector(52 downto 0);
    mantB : std_logic_vector(52 downto 0);
    expAB : std_logic_vector(12 downto 0);
    expAlign : std_logic_vector(11 downto 0);
    mantAlign : std_logic_vector(104 downto 0);
    postShift : std_logic_vector(11 downto 0);
    mantPostScale : std_logic_vector(104 downto 0);
    nanA : std_logic;
    nanB : std_logic;
    overflow : std_logic;
    illegal_op : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'),                               -- busy, ena
    (others => '0'), (others => '0'), (others => '0'),  -- a, b, result
    '0', '0', (others => '0'), (others => '0'),         -- zeroA, zeroB, mantA, mantB
    (others => '0'), (others => '0'), (others => '0'),  -- expAB, expAlign, mantAlign
    (others => '0'), (others => '0'),                   -- postShift, mantPostScale
    '0', '0', '0', '0'                                  -- nanA, nanB, overflow, illegal_op
  );

  constant zero11 : std_logic_vector(10 downto 0) := (others => '0');
  constant zero63 : std_logic_vector(62 downto 0) := (others => '0');
  constant zero105 : std_logic_vector(104 downto 0) := (others => '0');

  signal r, rin : RegistersType;

  signal w_imul_ena : std_logic;
  signal wb_imul_result : std_logic_vector(105 downto 0);
  signal wb_imul_shift : std_logic_vector(6 downto 0);
  signal w_imul_rdy : std_logic;
  signal w_imul_overflow : std_logic;

begin

  u_imul53 : imul53 generic map (
    async_reset => async_reset
  ) port map (
    i_nrst => i_nrst,
    i_clk => i_clk,
    i_ena => w_imul_ena,
    i_a => r.mantA,
    i_b => r.mantB,
    o_result => wb_imul_result,
    o_shift => wb_imul_shift,
    o_rdy => w_imul_rdy,
    o_overflow => w_imul_overflow
  );

  -- registers:
  comb : process(i_nrst, i_ena, i_a, i_b, r,
                 wb_imul_result, wb_imul_shift, w_imul_rdy, w_imul_overflow)
    variable v : RegistersType;
    variable signA : std_logic;
    variable signB : std_logic;
    variable mantA : std_logic_vector(52 downto 0);
    variable mantB : std_logic_vector(52 downto 0);
    variable zeroA : std_logic;
    variable zeroB : std_logic;
    variable expAB_t : std_logic_vector(11 downto 0);
    variable expAB : std_logic_vector(12 downto 0);
    variable mantAlign : std_logic_vector(104 downto 0);
    variable expAlign_t : std_logic_vector(12 downto 0);
    variable expAlign : std_logic_vector(12 downto 0);
    variable postShift : std_logic_vector(11 downto 0);
    variable mantPostScale : std_logic_vector(104 downto 0);
    variable mantShort : std_logic_vector(52 downto 0);
    variable tmpMant05 : std_logic_vector(51 downto 0);
    variable mantOnes : std_logic;
    variable mantEven : std_logic;
    variable mant05 : std_logic;
    variable rndBit : std_logic;
    variable nanA : std_logic;
    variable nanB : std_logic;
    variable mantZeroA : std_logic;
    variable mantZeroB : std_logic;
    variable res : std_logic_vector(63 downto 0);
  begin

    v := r;

    v.ena(0) := i_ena and not r.busy;
    v.ena(1) := r.ena(0);
    v.ena(4 downto 2) := r.ena(3 downto 2) & w_imul_rdy;

    if i_ena = '1' then
        v.busy := '1';
        v.overflow := '0';
        v.a := i_a;
        v.b := i_b;
    end if;

    signA := r.a(63);
    signB := r.b(63);

    zeroA := '0';
    if r.a(62 downto 0) = zero63 then
        zeroA := '1';
    end if;

    zeroB := '0';
    if r.b(62 downto 0) = zero63 then
        zeroB := '1';
    end if;

    mantA(51 downto 0) := r.a(51 downto 0);
    mantA(52) := '0';
    if r.a(62 downto 52) /= zero11 then
        mantA(52) := '1';
    end if;

    mantB(51 downto 0) := r.b(51 downto 0);
    mantB(52) := '0';
    if r.b(62 downto 52) /= zero11 then
        mantB(52) := '1';
    end if;

    -- expA - expB + 1023
    expAB_t := ('0' & r.a(62 downto 52)) + ('0' & r.b(62 downto 52));
    expAB := ('0' & expAB_t) - 1023;

    if r.ena(0) = '1' then
        v.expAB := expAB;
        v.zeroA := zeroA;
        v.zeroB := zeroB;
        v.mantA := mantA;
        v.mantB := mantB;
    end if;

    w_imul_ena <= r.ena(1);

    -- imul53 module:
    mantAlign := (others => '0');
    if wb_imul_result(105) = '1' then
        mantAlign := wb_imul_result(105 downto 1);
    elsif wb_imul_result(104) = '1' then
        mantAlign := wb_imul_result(104 downto 0);
    else
        for i in 1 to 104 loop
            if i = conv_integer(wb_imul_shift) then
                mantAlign := wb_imul_result(104-i downto 0) & zero105(i-1 downto 0);
            end if;
        end loop;
    end if;

    expAlign_t := r.expAB + 1;
    if wb_imul_result(105) = '1' then
        expAlign := expAlign_t;
    elsif r.a(62 downto 52) = zero11 or r.b(62 downto 52) = zero11 then
        expAlign := expAlign_t - ("000000" & wb_imul_shift);
    else
        expAlign := r.expAB - ("000000" & wb_imul_shift);
    end if;

    -- IMPORTANT exception! new ZERO value
    if expAlign(12) = '1' or expAlign = zero63(12 downto 0) then
        if wb_imul_shift = "0000000" or wb_imul_result(105) = '1'
            or r.a(62 downto 52) = zero11 or r.b(62 downto 52) = zero11 then
            postShift := not expAlign(11 downto 0) + 2;
        else
            postShift := not expAlign(11 downto 0) + 1;
        end if;
    else
        postShift := (others => '0');
    end if;

    if w_imul_rdy = '1' then
        v.expAlign := expAlign(11 downto 0);
        v.mantAlign := mantAlign;
        v.postShift := postShift;

        -- Exceptions:
        v.nanA := '0';
        if r.a(62 downto 52) = "11111111111" then
            v.nanA := '1';
        end if;
        v.nanB := '0';
        if r.b(62 downto 52) = "11111111111" then
            v.nanB := '1';
        end if;
        v.overflow := '0';
        if expAlign(12) = '0' and expAlign >= "0011111111111" then
            v.overflow := '1';
        end if;
    end if;

    -- Prepare to mantissa post-scale
    mantPostScale := (others => '0');
    if r.postShift = X"000" then
        mantPostScale := r.mantAlign;
    elsif r.postShift < conv_std_logic_vector(105, 12) then
        for i in 1 to 104 loop
            if conv_std_logic_vector(i, 7) = r.postShift(6 downto 0) then
                mantPostScale := zero105(i-1 downto 0) & r.mantAlign(104 downto i);
            end if;
        end loop;
    end if;
    if r.ena(2) = '1' then
        v.mantPostScale := mantPostScale;
    end if;

    -- Rounding bit
    mantShort := r.mantPostScale(104 downto 52);
    tmpMant05 := r.mantPostScale(51 downto 0);
    mantOnes := '0';
    if mantShort(52) = '1' and mantShort(51 downto 0) = X"fffffffffffff" then
        mantOnes := '1';
    end if;
    mantEven := r.mantPostScale(52);
    mant05 := '0';
    if tmpMant05 = X"8000000000000" then
        mant05 := '1';
    end if;
    rndBit := r.mantPostScale(51) and not(mant05 and not mantEven);

    -- Check Borders
    nanA := '0';
    if r.a(62 downto 52) = "11111111111" then
        nanA := '1';
    end if;
    nanB := '0';
    if r.b(62 downto 52) = "11111111111" then
        nanB := '1';
    end if;
    mantZeroA := '0';
    if r.a(51 downto 0) = zero63(51 downto 0) then
        mantZeroA := '1';
    end if;
    mantZeroB := '0';
    if r.b(51 downto 0) = zero63(51 downto 0) then
        mantZeroB := '1';
    end if;

    -- Result multiplexers:
    if (nanA and mantZeroA and r.zeroB) = '1' or (nanB and mantZeroB and r.zeroA) = '1' then
        res(63) := '1';
    elsif (nanA and not mantZeroA) = '1' then
        -- when both values are NaN, value B has higher priority if sign=1
        res(63) := signA or (nanA and signB);
    elsif (nanB and not mantZeroB) = '1' then
        res(63) := signB;
    else
        res(63) := r.a(63) xor r.b(63);
    end if;

    if nanA = '1' then
        res(62 downto 52) := r.a(62 downto 52);
    elsif nanB = '1' then
        res(62 downto 52) := r.b(62 downto 52);
    elsif (r.expAlign(11) or r.zeroA or r.zeroB) = '1' then
        res(62 downto 52) := (others => '0');
    elsif r.overflow = '1' then
        res(62 downto 52) := (others => '1');
    else
        res(62 downto 52) := r.expAlign(10 downto 0)
                       + (mantOnes and rndBit and not r.overflow);
    end if;

    if (nanA and mantZeroA and not mantZeroB) = '1'
        or (nanB and mantZeroB and not mantZeroA) = '1'
        or (not nanA and not nanB and r.overflow) = '1' then
        res(51 downto 0) := (others => '0');
    elsif (nanA and not (nanB and signB)) = '1' then
        -- when both values are NaN, value B has higher priority if sign=1
        res(51) := '1';
        res(50 downto 0) := r.a(50 downto 0);
    elsif nanB = '1' then
        res(51) := '1';
        res(50 downto 0) := r.b(50 downto 0);
    else
        res(51 downto 0) := mantShort(51 downto 0) + rndBit;
    end if;

    if r.ena(3) = '1' then
        v.result := res;
        v.illegal_op := nanA or nanB;
        v.busy := '0';
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
  end process;

  o_res <= r.result;
  o_illegal_op <= r.illegal_op;
  o_overflow <= r.overflow;
  o_valid <= r.ena(4);
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
