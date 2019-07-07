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

entity DoubleDiv is 
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
    o_divbyzero  : out std_logic;
    o_overflow   : out std_logic;
    o_underflow  : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
end; 
 
architecture arch_DoubleDiv of DoubleDiv is

  component idiv53 is generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_divident   : in std_logic_vector(52 downto 0);
    i_divisor    : in std_logic_vector(52 downto 0);
    o_result     : out std_logic_vector(104 downto 0);
    o_lshift     : out std_logic_vector(6 downto 0);
    o_rdy        : out std_logic;
    o_overflow   : out std_logic;
    o_zero_resid : out std_logic
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
    divisor : std_logic_vector(52 downto 0);
    preShift : std_logic_vector(5 downto 0);
    expAB : std_logic_vector(12 downto 0);
    expAlign : std_logic_vector(11 downto 0);
    mantAlign : std_logic_vector(104 downto 0);
    postShift : std_logic_vector(11 downto 0);
    mantPostScale : std_logic_vector(104 downto 0);
    nanRes : std_logic;
    overflow : std_logic;
    underflow : std_logic;
    illegal_op : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'),                               -- busy, ena
    (others => '0'), (others => '0'), (others => '0'),  -- a, b, result
    '0', '0', (others => '0'), (others => '0'),         -- zeroA, zeroB, divisor, preShift
    (others => '0'), (others => '0'), (others => '0'),  -- expAB, expAlign, mantAlign
    (others => '0'), (others => '0'),                   -- postShift, mantPostScale
    '0', '0', '0', '0'                                  -- nanRes, overflow, underflow, illegal_op
  );

  constant zero105 : std_logic_vector(104 downto 0) := (others => '0');

  signal r, rin : RegistersType;

  signal w_idiv_ena : std_logic;
  signal wb_divident : std_logic_vector(52 downto 0);
  signal wb_divisor : std_logic_vector(52 downto 0);
  signal wb_idiv_result : std_logic_vector(104 downto 0);
  signal wb_idiv_lshift : std_logic_vector(6 downto 0);
  signal w_idiv_rdy : std_logic;
  signal w_idiv_overflow : std_logic;
  signal w_idiv_zeroresid : std_logic;

begin

  u_idiv53 : idiv53 generic map (
    async_reset => async_reset
  ) port map (
    i_nrst => i_nrst,
    i_clk => i_clk,
    i_ena => w_idiv_ena,
    i_divident => wb_divident,
    i_divisor => wb_divisor,
    o_result => wb_idiv_result,
    o_lshift => wb_idiv_lshift,
    o_rdy => w_idiv_rdy,
    o_overflow => w_idiv_overflow,
    o_zero_resid => w_idiv_zeroresid
  );

  -- registers:
  comb : process(i_nrst, i_ena, i_a, i_b, r,
                 wb_idiv_result, wb_idiv_lshift, w_idiv_rdy, w_idiv_overflow,
                 w_idiv_zeroresid)
    variable v : RegistersType;
    variable signA : std_logic;
    variable signB : std_logic;
    variable mantA : std_logic_vector(52 downto 0);
    variable mantB : std_logic_vector(52 downto 0);
    variable zeroA : std_logic;
    variable zeroB : std_logic;
    variable divisor : std_logic_vector(52 downto 0);
    variable preShift : integer range 0 to 52;
    variable expAB_t : std_logic_vector(11 downto 0);
    variable expAB : std_logic_vector(12 downto 0);
    variable mantAlign : std_logic_vector(104 downto 0);
    variable expShift : std_logic_vector(11 downto 0);
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
    v.ena(4 downto 2) := r.ena(3 downto 2) & w_idiv_rdy;

    if i_ena = '1' then
        v.busy := '1';
        v.overflow := '0';
        v.underflow := '0';
        v.illegal_op := '0';
        v.a := i_a;
        v.b := i_b;
    end if;

    signA := r.a(63);
    signB := r.b(63);

    zeroA := '0';
    if r.a(62 downto 0) = zero105(62 downto 0) then
        zeroA := '1';
    end if;

    zeroB := '0';
    if r.b(62 downto 0) = zero105(62 downto 0) then
        zeroB := '1';
    end if;

    mantA(51 downto 0) := r.a(51 downto 0);
    mantA(52) := '0';
    if r.a(62 downto 52) /= zero105(10 downto 0) then
        mantA(52) := '1';
    end if;

    mantB(51 downto 0) := r.b(51 downto 0);
    mantB(52) := '0';
    if r.b(62 downto 52) /= zero105(10 downto 0) then
        mantB(52) := '1';
        divisor := mantB;
        preShift := 0;
    else
        divisor := mantB;
        preShift := 0;
        for i in 1 to 52 loop
            if preShift = 0 and mantB(52 - i) = '1' then
                divisor := mantB(52-i downto 0) & zero105(i-1 downto 0);
                preShift := i;
            end if;
        end loop;
    end if;

    -- expA - expB + 1023
    expAB_t := ('0' & r.a(62 downto 52)) + 1023;
    expAB := ('0' & expAB_t) - ("00" & r.b(62 downto 52));

    if r.ena(0) = '1' then
        v.divisor := divisor;
        v.preShift := conv_std_logic_vector(preShift, 6);
        v.expAB := expAB;
        v.zeroA := zeroA;
        v.zeroB := zeroB;
    end if;

    w_idiv_ena <= r.ena(1);
    wb_divident <= mantA;
    wb_divisor <= r.divisor;

    -- idiv53 module:
    mantAlign := (others => '0');
    if wb_idiv_lshift = zero105(6 downto 0) then
        mantAlign := wb_idiv_result;
    else
        for i in 1 to 104 loop
            if i = conv_integer(wb_idiv_lshift) then
                mantAlign := wb_idiv_result(104-i downto 0) & zero105(i-1 downto 0);
            end if;
        end loop;
    end if;

    expShift := ("000000" & r.preShift) - ("00000" & wb_idiv_lshift);
    if r.b(62 downto 52) = "00000000000" and r.a(62 downto 52) /= "00000000000" then
        expShift := expShift - 1;
    elsif r.b(62 downto 52) /= "00000000000" and r.a(62 downto 52) = "00000000000" then
        expShift := expShift + 1;
    end if;

    expAlign := r.expAB + (expShift(11) & expShift);
    if expAlign(12) = '1' then
        postShift := not expAlign(11 downto 0) + 2;
    else
        postShift := (others => '0');
    end if;

    if w_idiv_rdy = '1' then
        v.expAlign := expAlign(11 downto 0);
        v.mantAlign := mantAlign;
        v.postShift := postShift;

        -- Exceptions:
        v.nanRes := '0';
        if expAlign = "0011111111111" then
            v.nanRes := '1';
        end if;
        v.overflow := not expAlign(12) and expAlign(11);
        v.underflow := expAlign(12) and expAlign(11);
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
    if r.a(51 downto 0) = zero105(51 downto 0) then
        mantZeroA := '1';
    end if;
    mantZeroB := '0';
    if r.b(51 downto 0) = zero105(51 downto 0) then
        mantZeroB := '1';
    end if;

    -- Result multiplexers:
    if (nanA and mantZeroA and nanB and mantZeroB) = '1' then
        res(63) := '1';
    elsif (nanA and not mantZeroA) = '1' then
        res(63) := signA;
    elsif (nanB and not mantZeroB) = '1' then
        res(63) := signB;
    elsif (r.zeroA and r.zeroB) = '1' then
        res(63) := '1';
    else
        res(63) := r.a(63) xor r.b(63);
    end if;

    if nanB = '1' and mantZeroB = '0' then
        res(62 downto 52) := r.b(62 downto 52);
    elsif (r.underflow or r.zeroA) = '1' and r.zeroB = '0' then
        res(62 downto 52) := (others => '0');
    elsif (r.overflow or r.zeroB) = '1' then
        res(62 downto 52) := (others => '1');
    elsif nanA = '1' then
        res(62 downto 52) := r.a(62 downto 52);
    elsif ((nanB and mantZeroB) or r.expAlign(11)) = '1' then
        res(62 downto 52) := (others => '0');
    else
        res(62 downto 52) := r.expAlign(10 downto 0)
                       + (mantOnes and rndBit and not r.overflow);
    end if;

    if (r.zeroA and r.zeroB) = '1'
        or (nanA and mantZeroA and nanB and mantZeroB) = '1' then
        res(51) := '1';
        res(50 downto 0) := (others => '0');
    elsif nanA = '1' and mantZeroA = '0' then
        res(51) := '1';
        res(50 downto 0) := r.a(50 downto 0);
    elsif nanB = '1' and mantZeroB = '0'then
        res(51) := '1';
        res(50 downto 0) := r.b(50 downto 0);
    elsif r.overflow = '1' or r.nanRes = '1' or (nanA and mantZeroA) = '1'
          or (nanB and mantZeroB) = '1' then
        res(51 downto 0) := (others => '0');
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
  o_divbyzero <= r.zeroB;
  o_overflow <= r.overflow;
  o_underflow <= r.underflow;
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
