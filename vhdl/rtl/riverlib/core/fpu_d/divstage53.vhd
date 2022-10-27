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

entity divstage53 is 
  port (
    i_mux_ena    : in std_logic;  -- find first non-zero bit
    i_muxind     : in std_logic_vector(55 downto 0);  -- bits indexes 8x7 bits bus
    i_divident   : in std_logic_vector(60 downto 0);  -- integer value
    i_divisor    : in std_logic_vector(52 downto 0);  -- integer value
    o_dif        : out std_logic_vector(52 downto 0); -- residual value
    o_bits       : out std_logic_vector(7 downto 0);  -- resulting bits
    o_muxind     : out std_logic_vector(6 downto 0);  -- first found non-zero bit
    o_muxind_rdy : out std_logic                      -- seeking was successfull
  );
end; 
 
architecture arch_divstage53 of divstage53 is

  type thresh_type is array (15 downto 0) of std_logic_vector(61 downto 0);
  type dif_type is array (3 downto 0) of std_logic_vector(60 downto 0);

begin

  -- registers:
  comb : process(i_mux_ena, i_muxind, i_divident, i_divisor)
    variable wb_thresh : thresh_type;
    variable wb_dif : dif_type;
    variable wb_bits : std_logic_vector(7 downto 0);
    variable wb_divx3 : std_logic_vector(54 downto 0);
    variable wb_divx2 : std_logic_vector(54 downto 0);
    variable wb_muxind : std_logic_vector(6 downto 0);
    variable w_muxind_rdy : std_logic;
  begin
    wb_divx2 := '0' & i_divisor & '0';
    wb_divx3 := wb_divx2 + ("00" & i_divisor);

    -- stage 1 of 4
    wb_thresh(15) := ('0' & i_divident) - ('0' & wb_divx3 & "000000");
    wb_thresh(14) := ('0' & i_divident) - ('0' & wb_divx2 & "000000");
    wb_thresh(13) := ('0' & i_divident) - ("000" & i_divisor & "000000");
    wb_thresh(12) := ('0' & i_divident);

    if wb_thresh(15)(61) = '0' then
        wb_bits(7 downto 6) := "11";
        wb_dif(0) := wb_thresh(15)(60 downto 0);
    elsif wb_thresh(14)(61) = '0' then
        wb_bits(7 downto 6) := "10";
        wb_dif(0) := wb_thresh(14)(60 downto 0);
    elsif wb_thresh(13)(61) = '0' then
        wb_bits(7 downto 6) := "01";
        wb_dif(0) := wb_thresh(13)(60 downto 0);
    else
        wb_bits(7 downto 6) := "00";
        wb_dif(0) := wb_thresh(12)(60 downto 0);
    end if;

    -- stage 2 of 4
    wb_thresh(11) := ('0' & wb_dif(0)) - ("000" & wb_divx3 & "0000");
    wb_thresh(10) := ('0' & wb_dif(0)) - ("000" & wb_divx2 & "0000");
    wb_thresh(9) := ('0' & wb_dif(0)) - ("00000" & i_divisor & "0000");
    wb_thresh(8) := ('0' & wb_dif(0));

    if wb_thresh(11)(61) = '0' then
        wb_bits(5 downto 4) := "11";
        wb_dif(1) := wb_thresh(11)(60 downto 0);
    elsif wb_thresh(10)(61) = '0' then
        wb_bits(5 downto 4) := "10";
        wb_dif(1) := wb_thresh(10)(60 downto 0);
    elsif wb_thresh(9)(61) = '0' then
        wb_bits(5 downto 4) := "01";
        wb_dif(1) := wb_thresh(9)(60 downto 0);
    else
        wb_bits(5 downto 4) := "00";
        wb_dif(1) := wb_thresh(8)(60 downto 0);
    end if;

    -- stage 3 of 4
    wb_thresh(7) := ('0' & wb_dif(1)) - ("00000" & wb_divx3 & "00");
    wb_thresh(6) := ('0' & wb_dif(1)) - ("00000" & wb_divx2 & "00");
    wb_thresh(5) := ('0' & wb_dif(1)) - ("0000000" & i_divisor & "00");
    wb_thresh(4) := ('0' & wb_dif(1));

    if wb_thresh(7)(61) = '0' then
        wb_bits(3 downto 2) := "11";
        wb_dif(2) := wb_thresh(7)(60 downto 0);
    elsif wb_thresh(6)(61) = '0' then
        wb_bits(3 downto 2) := "10";
        wb_dif(2) := wb_thresh(6)(60 downto 0);
    elsif wb_thresh(5)(61) = '0' then
        wb_bits(3 downto 2) := "01";
        wb_dif(2) := wb_thresh(5)(60 downto 0);
    else
        wb_bits(3 downto 2) := "00";
        wb_dif(2) := wb_thresh(4)(60 downto 0);
    end if;

    -- stage 4 of 4
    wb_thresh(3) := ('0' & wb_dif(2)) - ("0000000" & wb_divx3);
    wb_thresh(2) := ('0' & wb_dif(2)) - ("0000000" & wb_divx2);
    wb_thresh(1) := ('0' & wb_dif(2)) - ("000000000" & i_divisor);
    wb_thresh(0) := ('0' & wb_dif(2));

    if wb_thresh(3)(61) = '0' then
        wb_bits(1 downto 0) := "11";
        wb_dif(3) := wb_thresh(3)(60 downto 0);
    elsif wb_thresh(2)(61) = '0' then
        wb_bits(1 downto 0) := "10";
        wb_dif(3) := wb_thresh(2)(60 downto 0);
    elsif wb_thresh(1)(61) = '0' then
        wb_bits(1 downto 0) := "01";
        wb_dif(3) := wb_thresh(1)(60 downto 0);
    else
        wb_bits(1 downto 0) := "00";
        wb_dif(3) := wb_thresh(0)(60 downto 0);
    end if;

    -- Number multiplexor
    wb_muxind := (others => '0');
    if i_mux_ena = '1' then
        if wb_thresh(15)(61) = '0' then
            wb_muxind := i_muxind(55 downto 49);
        elsif wb_thresh(14)(61) = '0' then
            wb_muxind := i_muxind(55 downto 49);
        elsif wb_thresh(13)(61) = '0' then
            wb_muxind := i_muxind(48 downto 42);
        elsif wb_thresh(11)(61) = '0' then
            wb_muxind := i_muxind(41 downto 35);
        elsif wb_thresh(10)(61) = '0' then
            wb_muxind := i_muxind(41 downto 35);
        elsif wb_thresh(9)(61) = '0' then
            wb_muxind := i_muxind(34 downto 28);
        elsif wb_thresh(7)(61) = '0' then
            wb_muxind := i_muxind(27 downto 21);
        elsif wb_thresh(6)(61) = '0' then
            wb_muxind := i_muxind(27 downto 21);
        elsif wb_thresh(5)(61) = '0' then
            wb_muxind := i_muxind(20 downto 14);
        elsif wb_thresh(3)(61) = '0' then
            wb_muxind := i_muxind(13 downto 7);
        elsif wb_thresh(2)(61) = '0' then
            wb_muxind := i_muxind(13 downto 7);
        elsif wb_thresh(1)(61) = '0' then
            wb_muxind := i_muxind(6 downto 0);
        else
            wb_muxind := i_muxind(6 downto 0);
        end if;
    end if;

    w_muxind_rdy := '0';
    if i_mux_ena = '1' and wb_bits /= X"00" then
        w_muxind_rdy := '1';
    end if;

    o_bits <= wb_bits;
    o_dif <= wb_dif(3)(52 downto 0);
    o_muxind <= wb_muxind;
    o_muxind_rdy <= w_muxind_rdy;

  end process;

end;
