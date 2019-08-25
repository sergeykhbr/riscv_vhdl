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

entity divstage64 is 
  port (
    i_divident   : in std_logic_vector(63 downto 0);  -- integer value
    i_divisor    : in std_logic_vector(123 downto 0); -- integer value
    o_resid      : out std_logic_vector(63 downto 0); -- residual value
    o_bits       : out std_logic_vector(3 downto 0)   -- resulting bits
  );
end; 
 
architecture arch_divstage64 of divstage64 is

  type thresh_type is array (7 downto 0) of std_logic_vector(128 downto 0);
  type dif_type is array (1 downto 0) of std_logic_vector(127 downto 0);

begin

  -- registers:
  comb : process(i_divident, i_divisor)
    variable wb_thresh : thresh_type;
    variable wb_dif : dif_type;
    variable wb_bits : std_logic_vector(3 downto 0);
    variable wb_divx3 : std_logic_vector(125 downto 0);
    variable wb_divx2 : std_logic_vector(125 downto 0);
  begin
    wb_divx2 := '0' & i_divisor & '0';
    wb_divx3 := wb_divx2 + ("00" & i_divisor);

    -- stage 1 of 2
    wb_thresh(7) := ('0' & X"0000000000000000" & i_divident) - ('0' & wb_divx3 & "00");
    wb_thresh(6) := ('0' & X"0000000000000000" & i_divident) - ('0' & wb_divx2 & "00");
    wb_thresh(5) := ('0' & X"0000000000000000" & i_divident) - ("000" & i_divisor & "00");
    wb_thresh(4) := ('0' & X"0000000000000000" & i_divident);

    if wb_thresh(7)(128) = '0' then
        wb_bits(3 downto 2) := "11";
        wb_dif(1) := wb_thresh(7)(127 downto 0);
    elsif wb_thresh(6)(128) = '0' then
        wb_bits(3 downto 2) := "10";
        wb_dif(1) := wb_thresh(6)(127 downto 0);
    elsif wb_thresh(5)(128) = '0' then
        wb_bits(3 downto 2) := "01";
        wb_dif(1) := wb_thresh(5)(127 downto 0);
    else
        wb_bits(3 downto 2) := "00";
        wb_dif(1) := wb_thresh(4)(127 downto 0);
    end if;

    -- stage 2 of 2
    wb_thresh(3) := ('0' & wb_dif(1)) - ("000" & wb_divx3);
    wb_thresh(2) := ('0' & wb_dif(1)) - ("000" & wb_divx2);
    wb_thresh(1) := ('0' & wb_dif(1)) - ("00000" & i_divisor);
    wb_thresh(0) := ('0' & wb_dif(1));

    if wb_thresh(3)(128) = '0' then
        wb_bits(1 downto 0) := "11";
        wb_dif(0) := wb_thresh(3)(127 downto 0);
    elsif wb_thresh(2)(128) = '0' then
        wb_bits(1 downto 0) := "10";
        wb_dif(0) := wb_thresh(2)(127 downto 0);
    elsif wb_thresh(1)(128) = '0' then
        wb_bits(1 downto 0) := "01";
        wb_dif(0) := wb_thresh(1)(127 downto 0);
    else
        wb_bits(1 downto 0) := "00";
        wb_dif(0) := wb_thresh(0)(127 downto 0);
    end if;


    o_bits <= wb_bits;
    o_resid <= wb_dif(0)(63 downto 0);

  end process;

end;
