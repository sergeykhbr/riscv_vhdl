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

entity idiv53 is 
  generic (
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
end; 
 
architecture arch_idiv53 of idiv53 is

  constant zero53 : std_logic_vector(52 downto 0) := (others => '0');

  component divstage53 is 
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
  end component; 

  type RegistersType is record
    delay : std_logic_vector(14 downto 0);
    lshift : std_logic_vector(6 downto 0);
    lshift_rdy : std_logic;
    divisor : std_logic_vector(52 downto 0);
    divident : std_logic_vector(60 downto 0);
    bits : std_logic_vector(104 downto 0);
    overflow : std_logic;
    zero_resid : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    (others => '0'), (others => '0'), '0',
    (others => '0'), (others => '0'), (others => '0'),
    '0', '0');

  signal r, rin : RegistersType;
  signal w_mux_ena_i : std_logic;
  signal wb_muxind_i : std_logic_vector(55 downto 0);
  signal wb_divident_i : std_logic_vector(60 downto 0);
  signal wb_divisor_i : std_logic_vector(52 downto 0);
  signal wb_dif_o : std_logic_vector(52 downto 0);
  signal wb_bits_o : std_logic_vector(7 downto 0);
  signal wb_muxind_o : std_logic_vector(6 downto 0);
  signal w_muxind_rdy_o : std_logic;

begin

  divstage0 : divstage53 port map (
    i_mux_ena => w_mux_ena_i,
    i_muxind => wb_muxind_i,
    i_divident => wb_divident_i,
    i_divisor => wb_divisor_i,
    o_dif => wb_dif_o,
    o_bits => wb_bits_o,
    o_muxind => wb_muxind_o,
    o_muxind_rdy => w_muxind_rdy_o
  );

  -- registers:
  comb : process(i_nrst, i_ena, i_divident, i_divisor, r,
                 wb_dif_o, wb_bits_o, wb_muxind_o, w_muxind_rdy_o)
    variable v : RegistersType;
    variable vb_muxind : std_logic_vector(55 downto 0);
    variable vb_bits : std_logic_vector(104 downto 0);
    variable v_mux_ena_i : std_logic;
  begin
    v := r;
    vb_bits := r.bits;

    v_mux_ena_i := '0';
    v.delay := r.delay(13 downto 0) & i_ena;
    vb_muxind := (others => '0');
    if i_ena = '1' then
        v.divident := X"00" & i_divident;
        v.divisor := i_divisor;
        v.lshift_rdy := '0';
        v.overflow := '0';
        v.zero_resid := '0';
    elsif r.delay(0) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_bits(104) := not wb_dif_o(52);
    elsif r.delay(1) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(1, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(2, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(3, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(4, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(5, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(6, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(7, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(8, 7);
        vb_bits(103 downto 96) := wb_bits_o;
    elsif r.delay(2) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(9, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(10, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(11, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(12, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(13, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(14, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(15, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(16, 7);
        vb_bits(95 downto 88) := wb_bits_o;
    elsif r.delay(3) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(17, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(18, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(19, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(20, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(21, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(22, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(23, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(24, 7);
        vb_bits(87 downto 80) := wb_bits_o;
    elsif r.delay(4) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(25, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(26, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(27, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(28, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(29, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(30, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(31, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(32, 7);
        vb_bits(79 downto 72) := wb_bits_o;
    elsif r.delay(5) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(33, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(34, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(35, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(36, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(37, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(38, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(39, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(40, 7);
        vb_bits(71 downto 64) := wb_bits_o;
    elsif r.delay(6) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(41, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(42, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(43, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(44, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(45, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(46, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(47, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(48, 7);
        vb_bits(63 downto 56) := wb_bits_o;
    elsif r.delay(7) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(49, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(50, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(51, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(52, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(53, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(54, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(55, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(56, 7);
        vb_bits(55 downto 48) := wb_bits_o;
    elsif r.delay(8) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(57, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(58, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(59, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(60, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(61, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(62, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(63, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(64, 7);
        vb_bits(47 downto 40) := wb_bits_o;
    elsif r.delay(9) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(65, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(66, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(67, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(68, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(69, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(70, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(71, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(72, 7);
        vb_bits(39 downto 32) := wb_bits_o;
    elsif r.delay(10) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(73, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(74, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(75, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(76, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(77, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(78, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(79, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(80, 7);
        vb_bits(31 downto 24) := wb_bits_o;
    elsif r.delay(11) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(81, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(82, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(83, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(84, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(85, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(86, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(87, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(88, 7);
        vb_bits(23 downto 16) := wb_bits_o;
    elsif r.delay(12) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(89, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(90, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(91, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(92, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(93, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(94, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(95, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(96, 7);
        vb_bits(15 downto 8) := wb_bits_o;
    elsif r.delay(13) = '1' then
        v_mux_ena_i := not r.lshift_rdy;
        v.divident := wb_dif_o & X"00";
        vb_muxind(55 downto 49) := conv_std_logic_vector(97, 7);
        vb_muxind(48 downto 42) := conv_std_logic_vector(98, 7);
        vb_muxind(41 downto 35) := conv_std_logic_vector(99, 7);
        vb_muxind(34 downto 28) := conv_std_logic_vector(100, 7);
        vb_muxind(27 downto 21) := conv_std_logic_vector(101, 7);
        vb_muxind(20 downto 14) := conv_std_logic_vector(102, 7);
        vb_muxind(13 downto 7)  := conv_std_logic_vector(103, 7);
        vb_muxind(6 downto 0)   := conv_std_logic_vector(104, 7);
        vb_bits(7 downto 0) := wb_bits_o;

        if wb_dif_o = zero53 then
            v.zero_resid := '1';
        end if;
        if r.lshift = "1111111" then
            v.overflow := '1';
        end if;
    end if;

    if r.lshift_rdy = '0' then
        if w_muxind_rdy_o = '1' then
            v.lshift_rdy := '1';
            v.lshift := wb_muxind_o;
        elsif r.delay(13) = '1' then
            v.lshift_rdy := '1';
            v.lshift := conv_std_logic_vector(104, 7);
        end if;
    end if;

    w_mux_ena_i <= v_mux_ena_i;
    wb_divident_i <= r.divident;
    wb_divisor_i <= r.divisor;
    wb_muxind_i <= vb_muxind;
    v.bits := vb_bits;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;
  end process;

  o_result <= r.bits;
  o_lshift <= r.lshift;
  o_overflow <= r.overflow;
  o_zero_resid <= r.zero_resid;
  o_rdy <= r.delay(14);
  
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
