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

entity DoubleAdd is 
  generic (
    async_reset : boolean
  );
  port (
    i_nrst       : in std_logic;
    i_clk        : in std_logic;
    i_ena        : in std_logic;
    i_add        : in std_logic;
    i_sub        : in std_logic;
    i_eq         : in std_logic;
    i_lt         : in std_logic;
    i_le         : in std_logic;
    i_max        : in std_logic;
    i_min        : in std_logic;
    i_a          : in std_logic_vector(63 downto 0);
    i_b          : in std_logic_vector(63 downto 0);
    o_res        : out std_logic_vector(63 downto 0);
    o_illegal_op : out std_logic;
    o_overflow   : out std_logic;
    o_valid      : out std_logic;
    o_busy       : out std_logic
  );
end; 
 
architecture arch_DoubleAdd of DoubleAdd is

  type RegistersType is record
    busy : std_logic;
    ena : std_logic_vector(7 downto 0);
    a : std_logic_vector(63 downto 0);
    b : std_logic_vector(63 downto 0);
    result : std_logic_vector(63 downto 0);
    illegal_op : std_logic;
    overflow : std_logic;
    add : std_logic;
    sub : std_logic;
    eq : std_logic;
    lt : std_logic;
    le : std_logic;
    max : std_logic;
    min : std_logic;
    flMore : std_logic;
    flEqual : std_logic;
    flLess : std_logic;
    preShift : integer range 0 to 4095;
    signOpMore : std_logic;
    expMore : std_logic_vector(10 downto 0);
    mantMore : std_logic_vector(52 downto 0);
    mantLess : std_logic_vector(52 downto 0);
    mantLessScale : std_logic_vector(104 downto 0);
    mantSum : std_logic_vector(105 downto 0);
    lshift : integer range 0 to 127;
    mantAlign : std_logic_vector(104 downto 0);
    expPostScale : std_logic_vector(11 downto 0);
    expPostScaleInv : integer range 0 to 4095;
    mantPostScale : std_logic_vector(104 downto 0);
  end record;

  constant R_RESET : RegistersType := (
    '0', (others => '0'),                               -- busy, ena
    (others => '0'), (others => '0'), (others => '0'),  -- a, b, result
    '0', '0', '0', '0',                                 -- illegal_op, overflow, add, sub
    '0', '0', '0', '0', '0',                            -- eq, lt, le, max, min
    '0', '0', '0',                                      -- flMore, flEqual, flLess
    0, '0', (others => '0'),                            -- preShift, signOpMore, expMore
    (others => '0'), (others => '0'), (others => '0'),  -- mantMore, mantLess, mantLessScale
    (others => '0'), 0, (others => '0'),                -- mantSum, lshift, mantAlign
    (others => '0'), 0, (others => '0')                 -- expPostScale, expPostScaleInv, mantPostScale
  );

  constant zero105 : std_logic_vector(104 downto 0) := (others => '0');

  signal r, rin : RegistersType;

begin

  -- registers:
  comb : process(i_nrst, i_ena, i_add, i_sub, i_eq, i_lt, i_le, i_max, i_min,
                 i_a, i_b, r)
    variable v : RegistersType;
    variable signOp : std_logic;
    variable signA : std_logic;
    variable signB : std_logic;
    variable signOpB : std_logic;
    variable mantA : std_logic_vector(52 downto 0);
    variable mantB : std_logic_vector(52 downto 0);
    variable mantDif : std_logic_vector(53 downto 0);
    variable expDif : std_logic_vector(11 downto 0);
    variable v_flMore : std_logic;
    variable v_flEqual : std_logic;
    variable v_flLess : std_logic;
    variable vb_preShift : std_logic_vector(11 downto 0);
    variable v_signOpMore : std_logic;
    variable vb_expMore : std_logic_vector(10 downto 0);
    variable vb_mantMore : std_logic_vector(52 downto 0);
    variable vb_mantLess : std_logic_vector(52 downto 0);
    variable mantMoreScale : std_logic_vector(104 downto 0);
    variable mantLessScale : std_logic_vector(104 downto 0);
    variable vb_mantSum : std_logic_vector(105 downto 0);
    variable vb_mantSumInv : std_logic_vector(104 downto 0);
    variable vb_lshift : integer range 0 to 127;
    variable vb_lshift_p1 : integer range 0 to 127;
    variable vb_lshift_p2 : integer range 0 to 127;
    variable vb_mantAlign : std_logic_vector(104 downto 0);
    variable vb_expPostScale : std_logic_vector(11 downto 0);
    variable vb_mantPostScale : std_logic_vector(104 downto 0);
    variable mantShort : std_logic_vector(52 downto 0);
    variable tmpMant05 : std_logic_vector(51 downto 0);
    variable mantOnes : std_logic;
    variable mantEven : std_logic;
    variable mant05 : std_logic;
    variable rndBit : std_logic;
    variable mantZeroA : std_logic;
    variable mantZeroB : std_logic;
    variable allZero : std_logic;
    variable sumZero : std_logic;
    variable nanA : std_logic;
    variable nanB : std_logic;
    variable nanAB : std_logic;
    variable overflow : std_logic;
    variable resAdd : std_logic_vector(63 downto 0);
    variable resEQ : std_logic_vector(63 downto 0);
    variable resLT : std_logic_vector(63 downto 0);
    variable resLE : std_logic_vector(63 downto 0);
    variable resMax : std_logic_vector(63 downto 0);
    variable resMin : std_logic_vector(63 downto 0);
  begin

    v := r;

    v.ena := r.ena(6 downto 0) & (i_ena and not r.busy);

    if i_ena = '1' then
        v.busy := '1';
        v.add := i_add;
        v.sub := i_sub;
        v.eq := i_eq;
        v.lt := i_lt;
        v.le := i_le;
        v.max := i_max;
        v.min := i_min;
        v.a := i_a;
        v.b := i_b;
        v.illegal_op := '0';
        v.overflow := '0';
    end if;

    signOp := r.sub or r.le or r.lt;
    signA := r.a(63);
    signB := r.b(63);
    signOpB := signB xor signOp;

    mantA(51 downto 0) := r.a(51 downto 0);
    mantA(52) := '0';
    if r.a(62 downto 52) /= zero105(10 downto 0) then
        mantA(52) := '1';
    end if;

    mantB(51 downto 0) := r.b(51 downto 0);
    mantB(52) := '0';
    if r.b(62 downto 52) /= zero105(10 downto 0) then
        mantB(52) := '1';
    end if;

    if r.a(62 downto 52) /= "00000000000" and r.b(62 downto 52) = "00000000000" then
        expDif := ('0' & r.a(62 downto 52)) - "000000000001";
    elsif r.a(62 downto 52) = "00000000000" and r.b(62 downto 52) /= "00000000000" then
        expDif := "000000000001" - ('0' & r.b(62 downto 52));
    else
        expDif := ('0' & r.a(62 downto 52)) - ('0' & r.b(62 downto 52));
    end if;

    mantDif := ('0' & mantA) - ('0' & mantB);
    if expDif = X"000" then
        vb_preShift := expDif;
        if mantDif = zero105(53 downto 0) then
            v_flMore := not signA and (signA xor signB);
            v_flEqual := not (signA xor signB);
            v_flLess := signA and (signA xor signB);

            v_signOpMore := signA;
            vb_expMore := r.a(62 downto 52);
            vb_mantMore := mantA;
            vb_mantLess := mantB;
        elsif mantDif(53) = '0' then  -- A > B
            v_flMore := not signA;
            v_flEqual := '0';
            v_flLess := signA;

            v_signOpMore := signA;
            vb_expMore := r.a(62 downto 52);
            vb_mantMore := mantA;
            vb_mantLess := mantB;
        else
            v_flMore := signB;
            v_flEqual := '0';
            v_flLess := not signB;

            v_signOpMore := signOpB;
            vb_expMore := r.b(62 downto 52);
            vb_mantMore := mantB;
            vb_mantLess := mantA;
        end if;
    elsif expDif(11) = '0' then
        v_flMore := not signA;
        v_flEqual := '0';
        v_flLess := signA;

        vb_preShift := expDif;
        v_signOpMore := signA;
        vb_expMore := r.a(62 downto 52);
        vb_mantMore := mantA;
        vb_mantLess := mantB;
    else
        v_flMore := signB;
        v_flEqual := '0';
        v_flLess := not signB;

        vb_preShift := not expDif + 1;
        v_signOpMore := signOpB;
        vb_expMore := r.b(62 downto 52);
        vb_mantMore := mantB;
        vb_mantLess := mantA;
    end if;
    if r.ena(0) = '1' then
        v.flMore := v_flMore;
        v.flEqual := v_flEqual;
        v.flLess := v_flLess;
        v.preShift := conv_integer(vb_preShift);
        v.signOpMore := v_signOpMore;
        v.expMore := vb_expMore;
        v.mantMore := vb_mantMore;
        v.mantLess := vb_mantLess;
    end if;

    -- Pre-scale 105-bits mantissa if preShift < 105:
    -- M = {mantM, 52'd0}
    mantLessScale := r.mantLess & zero105(51 downto 0);
    if r.ena(1) = '1' then
        if r.preShift = 0 then
            v.mantLessScale := mantLessScale;
        else
            v.mantLessScale := (others => '0');
            for i in 1 to 104 loop
                if i = r.preShift then
                    v.mantLessScale := zero105(i-1 downto 0) & mantLessScale(104 downto i);
                end if;
            end loop;
        end if;
    end if;

    mantMoreScale := r.mantMore & zero105(51 downto 0);

    -- 106-bits adder/subtractor
    if (signA xor signOpB) = '1' then
        vb_mantSum := ('0' & mantMoreScale) - ('0' & r.mantLessScale);
    else
        vb_mantSum := ('0' & mantMoreScale) + ('0' & r.mantLessScale);
    end if;

    if r.ena(2) = '1' then
        v.mantSum := vb_mantSum;
    end if;

    -- To avoid timing constrains violation occured in Vivado Studio
    -- try to implement parallel demuxultiplexer splitted on 2 parts
    vb_mantSumInv(0) := '0';
    for i in 0 to 103 loop
        vb_mantSumInv(i + 1) := r.mantSum(103 - i);
    end loop;
    
    vb_lshift_p1 := 0;
    for i in 0 to 63 loop
        if vb_lshift_p1 = 0 and vb_mantSumInv(i) = '1' then
            vb_lshift_p1 := i;
        end if;
    end loop;
    
    vb_lshift_p2 := 0;
    for i in 0 to 40 loop
        if vb_lshift_p2 = 0 and vb_mantSumInv(64 + i) = '1' then
            vb_lshift_p2 := 64 + i;
        end if;
    end loop;

    -- multiplexer
    if r.mantSum(105) = '1' then
        -- shift right
        vb_lshift := 127;
    elsif r.mantSum(104) = '1' then
        vb_lshift := 0;
    elsif vb_lshift_p1 /= 0 then
        vb_lshift := vb_lshift_p1;
    else
        vb_lshift := vb_lshift_p2;
    end if;
    if r.ena(3) = '1' then
        v.lshift := vb_lshift;
    end if;

    -- Prepare to mantissa post-scale
    vb_mantAlign := (others => '0');
    if r.lshift = 127 then
        vb_mantAlign := r.mantSum(105 downto 1);
    elsif r.lshift = 0 then
        vb_mantAlign := r.mantSum(104 downto 0);
    else
        for i in 1 to 104 loop
            if i = r.lshift then
                vb_mantAlign := r.mantSum(104-i downto 0) & zero105(i-1 downto 0);
            end if;
        end loop;
    end if;

    if r.lshift = 127 then
        if r.expMore = "11111111111" then
            vb_expPostScale := ('0' & r.expMore);
        else
            vb_expPostScale := ('0' & r.expMore) + 1;
        end if;
    else
        if r.expMore = "00000000000" and r.lshift = 0 then
            vb_expPostScale := X"001";
        else
            vb_expPostScale := ('0' & r.expMore) - conv_std_logic_vector(r.lshift, 12);
        end if;
    end if;
    if (signA xor signOpB) = '1' then
        -- subtractor only: result value becomes with exp=0
        if r.expMore /= "00000000000" and
           (vb_expPostScale(11) = '1' or vb_expPostScale = X"000") then
            vb_expPostScale := vb_expPostScale - 1;
        end if;
    end if;
    if r.ena(4) = '1' then
        v.mantAlign := vb_mantAlign;
        v.expPostScale := vb_expPostScale;
        v.expPostScaleInv := conv_integer((not vb_expPostScale) + 1);
    end if;

    -- Mantissa post-scale:
    --    Scaled = SumScale>>(-ExpSum) only if ExpSum < 0;
    vb_mantPostScale := r.mantAlign;
    if r.expPostScale(11) = '1' then
        for i in 1 to 104 loop
            if i = r.expPostScaleInv then
                vb_mantPostScale := zero105(i-1 downto 0) & r.mantAlign(104 downto i);
            end if;
        end loop;
    end if;
    if r.ena(5) = '1' then
        v.mantPostScale := vb_mantPostScale;
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
    mantZeroA := '0';
    if r.a(51 downto 0) = zero105(51 downto 0) then
        mantZeroA := '1';
    end if;
    mantZeroB := '0';
    if r.b(51 downto 0) = zero105(51 downto 0) then
        mantZeroB := '1';
    end if;

    -- Exceptions
    allZero := '0';
    if r.a(62 downto 0) = zero105(62 downto 0) and
       r.b(62 downto 0) = zero105(62 downto 0) then
        allZero := '1';
    end if;
    sumZero := '0';
    if r.mantPostScale = zero105 then
        sumZero := '1';
    end if;
    nanA := '0';
    if r.a(62 downto 52) = "11111111111" then
        nanA := '1';
    end if;
    nanB := '0';
    if r.b(62 downto 52) = "11111111111" then
        nanB := '1';
    end if;
    nanAB := nanA and mantZeroA and nanB and mantZeroB;
    overflow := '0';
    if r.expPostScale = X"7FF" then   -- positive
        overflow := '1';
    end if;

    -- Result multiplexers:
    if (nanAB and signOp) = '1' then
        resAdd(63) := signA xor signOpB;
    elsif nanA = '1' then
        -- when both values are NaN, value B has higher priority if sign=1
        resAdd(63) := signA or (nanB and signOpB);
    elsif nanB = '1' then
        resAdd(63) := signOpB xor (signOp and not mantZeroB);
    elsif allZero = '1' then
        resAdd(63) := signA and signOpB;
    elsif sumZero = '1' then
        resAdd(63) := '0';
    else
        resAdd(63) := r.signOpMore;
    end if;

    if (nanA or nanB) = '1' then
        resAdd(62 downto 52) := (others => '1');
    elsif r.expPostScale(11) = '1' or sumZero = '1' then
        resAdd(62 downto 52) := (others => '0');
    else
        resAdd(62 downto 52) := r.expPostScale(10 downto 0)
                       + (mantOnes and rndBit and not r.overflow);
    end if;

    if (nanA and mantZeroA and nanB and mantZeroB) = '1' then
        resAdd(51) := '1';
        resAdd(50 downto 0) := (others => '0');
    elsif nanA = '1' and (nanB and signOpB) = '0' then
        -- when both values are NaN, value B has higher priority if sign=1
        resAdd(51) := '1';
        resAdd(50 downto 0) := r.a(50 downto 0);
    elsif nanB = '1' then
        resAdd(51) := '1';
        resAdd(50 downto 0) := r.b(50 downto 0);
    elsif r.overflow = '1'  then
        resAdd(51 downto 0) := (others => '0');
    else
        resAdd(51 downto 0) := mantShort(51 downto 0) + rndBit;
    end if;

    resEQ(63 downto 1) := (others => '0');
    resEQ(0) := r.flEqual;

    resLT(63 downto 1) := (others => '0');
    resLT(0) := r.flLess;

    resLE(63 downto 1) := (others => '0');
    resLE(0) := r.flLess or r.flEqual;

    if (nanA or nanB) = '1' then
        resMax := r.b;
    elsif r.flMore = '1' then
        resMax := r.a;
    else
        resMax := r.b;
    end if;

    if (nanA or nanB) = '1' then
        resMin := r.b;
    elsif r.flLess = '1' then
        resMin := r.a;
    else
        resMin := r.b;
    end if;


    if r.ena(6) = '1' then
        if r.eq = '1' then
            v.result := resEQ;
        elsif r.lt = '1' then
            v.result := resLT;
        elsif r.le = '1' then
            v.result := resLE;
        elsif r.max = '1' then
            v.result := resMax;
        elsif r.min = '1' then
            v.result := resMin;
        else
            v.result := resAdd;
        end if;

        v.illegal_op := nanA or nanB;
        v.overflow := overflow;

        v.busy := '0';
        v.add := '0';
        v.sub := '0';
        v.eq := '0';
        v.lt := '0';
        v.le := '0';
        v.max := '0';
        v.min := '0';
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
  end process;

  o_res <= r.result;
  o_illegal_op <= r.illegal_op;
  o_overflow <= r.overflow;
  o_valid <= r.ena(7);
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
