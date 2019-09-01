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

entity divstage64 is 
  port (
    i_divident   : in std_logic_vector(63 downto 0);  -- integer value
    i_divisor    : in std_logic_vector(123 downto 0); -- integer value
    o_resid      : out std_logic_vector(63 downto 0); -- residual value
    o_bits       : out std_logic_vector(3 downto 0)   -- resulting bits
  );
end; 
 
architecture arch_divstage64 of divstage64 is

  type thresh_type is array (15 downto 0) of std_logic_vector(65 downto 0);

begin

  -- registers:
  comb : process(i_divident, i_divisor)
    variable wb_thresh : thresh_type;
    variable wb_dif : std_logic_vector(63 downto 0);
    variable wb_bits : std_logic_vector(3 downto 0);
    variable wb_divident : std_logic_vector(64 downto 0);
    variable wb_divx1 : std_logic_vector(64 downto 0);
    variable wb_divx2 : std_logic_vector(64 downto 0);
    variable wb_divx3 : std_logic_vector(64 downto 0);
    variable wb_divx4 : std_logic_vector(64 downto 0);
    variable wb_divx5 : std_logic_vector(64 downto 0);
    variable wb_divx6 : std_logic_vector(64 downto 0);
    variable wb_divx7 : std_logic_vector(64 downto 0);
    variable wb_divx8 : std_logic_vector(64 downto 0);
    variable wb_divx9 : std_logic_vector(64 downto 0);
    variable wb_divx10 : std_logic_vector(64 downto 0);
    variable wb_divx11 : std_logic_vector(64 downto 0);
    variable wb_divx12 : std_logic_vector(64 downto 0);
    variable wb_divx13 : std_logic_vector(64 downto 0);
    variable wb_divx14 : std_logic_vector(64 downto 0);
    variable wb_divx15 : std_logic_vector(64 downto 0);
    variable wb_divx16 : std_logic_vector(64 downto 0);

  begin
    wb_divident := '0' & i_divident;

    wb_divx1(63 downto 0) := i_divisor(63 downto 0);
    wb_divx1(64) := or_reduce(i_divisor(123 downto 64));

    wb_divx2(63 downto 0) := i_divisor(62 downto 0) & '0';
    wb_divx2(64) := or_reduce(i_divisor(123 downto 63));

    wb_divx3(64 downto 0) := ('0' & wb_divx2(63 downto 0)) + ('0' & wb_divx1(63 downto 0));
    wb_divx3(64) := wb_divx3(64) or wb_divx2(64);

    wb_divx4(63 downto 0) := i_divisor(61 downto 0) & "00";
    wb_divx4(64) := or_reduce(i_divisor(123 downto 62));

    wb_divx5(64 downto 0) := ('0' & wb_divx4(63 downto 0)) + ('0' & wb_divx1(63 downto 0));
    wb_divx5(64) := wb_divx5(64) or wb_divx4(64);

    wb_divx6(63 downto 0) := wb_divx3(62 downto 0) & '0';
    wb_divx6(64) := wb_divx3(64) or wb_divx3(63);

    wb_divx8(63 downto 0) := wb_divx1(60 downto 0) & "000";
    wb_divx8(64) := or_reduce(i_divisor(123 downto 61));

    -- 7 = 8 - 1
    wb_divx7(64 downto 0) := wb_divx8(64 downto 0) - ('0' & wb_divx1(63 downto 0));
    wb_divx7(64) := wb_divx7(64) or or_reduce(i_divisor(123 downto 62));

    -- 9 = 8 + 1
    wb_divx9(64 downto 0) := ('0' & wb_divx8(63 downto 0)) + ('0' & wb_divx1(63 downto 0));
    wb_divx9(64) := wb_divx9(64) or or_reduce(i_divisor(123 downto 61));

    -- 10 = 8 + 2
    wb_divx10(64 downto 0) := ('0' & wb_divx8(63 downto 0)) + ('0' & wb_divx2(63 downto 0));
    wb_divx10(64) := wb_divx10(64) or or_reduce(i_divisor(123 downto 61));

    -- 11 = 8 + 3
    wb_divx11(64 downto 0) := ('0' & wb_divx8(63 downto 0)) + ('0' & wb_divx3(63 downto 0));
    wb_divx11(64) := wb_divx11(64) or or_reduce(i_divisor(123 downto 61));

    -- 12 = 3 << 2
    wb_divx12(63 downto 0) := wb_divx3(61 downto 0) & "00";
    wb_divx12(64) := wb_divx3(64) or wb_divx3(63) or wb_divx3(62);

    -- 16 = divisor << 4
    wb_divx16(63 downto 0) := wb_divx1(59 downto 0) & "0000";
    wb_divx16(64) := or_reduce(i_divisor(123 downto 60));

    -- 13 = 16 - 3
    wb_divx13(64 downto 0) := wb_divx16(64 downto 0) - ('0' & wb_divx3(63 downto 0));
    wb_divx13(64) := wb_divx13(64) or or_reduce(i_divisor(123 downto 61));

    -- 14 = 7 << 1
    wb_divx14(63 downto 0) := wb_divx7(62 downto 0) & '0';
    wb_divx14(64) := wb_divx7(64) or wb_divx7(63);

    -- 15 = 16 - 1
    wb_divx15(64 downto 0) := wb_divx16(64 downto 0) - ('0' & wb_divx1(63 downto 0));
    wb_divx15(64) := wb_divx15(64) or or_reduce(i_divisor(123 downto 61));

    wb_thresh(15) := ('0' & wb_divident) - ('0' & wb_divx15);
    wb_thresh(14) := ('0' & wb_divident) - ('0' & wb_divx14);
    wb_thresh(13) := ('0' & wb_divident) - ('0' & wb_divx13);
    wb_thresh(12) := ('0' & wb_divident) - ('0' & wb_divx12);
    wb_thresh(11) := ('0' & wb_divident) - ('0' & wb_divx11);
    wb_thresh(10) := ('0' & wb_divident) - ('0' & wb_divx10);
    wb_thresh(9) := ('0' & wb_divident) - ('0' & wb_divx9);
    wb_thresh(8) := ('0' & wb_divident) - ('0' & wb_divx8);
    wb_thresh(7) := ('0' & wb_divident) - ('0' & wb_divx7);
    wb_thresh(6) := ('0' & wb_divident) - ('0' & wb_divx6);
    wb_thresh(5) := ('0' & wb_divident) - ('0' & wb_divx5);
    wb_thresh(4) := ('0' & wb_divident) - ('0' & wb_divx4);
    wb_thresh(3) := ('0' & wb_divident) - ('0' & wb_divx3);
    wb_thresh(2) := ('0' & wb_divident) - ('0' & wb_divx2);
    wb_thresh(1) := ('0' & wb_divident) - ('0' & wb_divx1);
    wb_thresh(0) := ('0' & wb_divident);

    if wb_thresh(15)(65) = '0' then
        wb_bits := X"F";
        wb_dif := wb_thresh(15)(63 downto 0);
    elsif wb_thresh(14)(65) = '0' then
        wb_bits := X"E";
        wb_dif := wb_thresh(14)(63 downto 0);
    elsif wb_thresh(13)(65) = '0' then
        wb_bits := X"D";
        wb_dif := wb_thresh(13)(63 downto 0);
    elsif wb_thresh(12)(65) = '0' then
        wb_bits := X"C";
        wb_dif := wb_thresh(12)(63 downto 0);
    elsif wb_thresh(11)(65) = '0' then
        wb_bits := X"B";
        wb_dif := wb_thresh(11)(63 downto 0);
    elsif wb_thresh(10)(65) = '0' then
        wb_bits := X"A";
        wb_dif := wb_thresh(10)(63 downto 0);
    elsif wb_thresh(9)(65) = '0' then
        wb_bits := X"9";
        wb_dif := wb_thresh(9)(63 downto 0);
    elsif wb_thresh(8)(65) = '0' then
        wb_bits := X"8";
        wb_dif := wb_thresh(8)(63 downto 0);
    elsif wb_thresh(7)(65) = '0' then
        wb_bits := X"7";
        wb_dif := wb_thresh(7)(63 downto 0);
    elsif wb_thresh(6)(65) = '0' then
        wb_bits := X"6";
        wb_dif := wb_thresh(6)(63 downto 0);
    elsif wb_thresh(5)(65) = '0' then
        wb_bits := X"5";
        wb_dif := wb_thresh(5)(63 downto 0);
    elsif wb_thresh(4)(65) = '0' then
        wb_bits := X"4";
        wb_dif := wb_thresh(4)(63 downto 0);
    elsif wb_thresh(3)(65) = '0' then
        wb_bits := X"3";
        wb_dif := wb_thresh(3)(63 downto 0);
    elsif wb_thresh(2)(65) = '0' then
        wb_bits := X"2";
        wb_dif := wb_thresh(2)(63 downto 0);
    elsif wb_thresh(1)(65) = '0' then
        wb_bits := X"1";
        wb_dif := wb_thresh(1)(63 downto 0);
    else
        wb_bits := X"0";
        wb_dif := wb_thresh(0)(63 downto 0);
    end if;

    o_bits <= wb_bits;
    o_resid <= wb_dif;

  end process;

end;
