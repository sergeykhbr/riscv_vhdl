-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Left/Right shifter arithmetic/logic 32/64 bits.
--! 
--! @details   Vivado synthesizer (2016.2) doesn't support shift
--!            from dynamic value, so implement this mux.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity Shifter is
  port (
    i_a1 : in std_logic_vector(RISCV_ARCH-1 downto 0);     -- Operand 1
    i_a2 : in std_logic_vector(5 downto 0);                -- Shift bits number
    o_sll : out std_logic_vector(RISCV_ARCH-1 downto 0);   -- Logical shift left 64-bits operand
    o_sllw : out std_logic_vector(RISCV_ARCH-1 downto 0);  -- Logical shift left 32-bits operand
    o_srl : out std_logic_vector(RISCV_ARCH-1 downto 0);   -- Logical shift 64 bits
    o_sra : out std_logic_vector(RISCV_ARCH-1 downto 0);   -- Arith. shift 64 bits
    o_srlw : out std_logic_vector(RISCV_ARCH-1 downto 0);  -- Logical shift 32 bits
    o_sraw : out std_logic_vector(RISCV_ARCH-1 downto 0)   -- Arith. shift 32 bits
  );
end; 
 
architecture arch_Shifter of Shifter is

begin

  comb : process(i_a1, i_a2)
    variable wb_sll : std_logic_vector(63 downto 0);
    variable wb_srl : std_logic_vector(63 downto 0);
    variable wb_sra : std_logic_vector(63 downto 0);
    variable wb_sllw : std_logic_vector(63 downto 0);
    variable wb_srlw : std_logic_vector(63 downto 0);
    variable wb_sraw : std_logic_vector(63 downto 0);
    variable v64 : std_logic_vector(63 downto 0);
    variable v32 : std_logic_vector(31 downto 0);
    variable msk64 : std_logic_vector(63 downto 0);
    variable msk32 : std_logic_vector(63 downto 0);
    variable shift64 : integer range 0 to 63;
    variable shift32 : integer range 0 to 31;
  begin

    v64 := i_a1;
    v32 := i_a1(31 downto 0);
    msk64 := (others => i_a1(63));
    msk32 := (others => i_a1(31));
    shift64 := conv_integer(i_a2);
    shift32 := conv_integer(i_a2(4 downto 0));

    case shift64 is
    when 0 =>
        wb_sll := v64;
        wb_srl := v64;
        wb_sra := v64;
    when 1 =>
        wb_sll := v64(62 downto 0) & "0";
        wb_srl := "0" & v64(63 downto 1);
        wb_sra := (msk64(63 downto 63) & v64(63 downto 1));
    when 2 =>
        wb_sll := v64(61 downto 0) & "00";
        wb_srl := "00" & v64(63 downto 2);
        wb_sra := (msk64(63 downto 62) & v64(63 downto 2));
    when 3 =>
        wb_sll := v64(60 downto 0) & "000";
        wb_srl := "000" & v64(63 downto 3);
        wb_sra := (msk64(63 downto 61) & v64(63 downto 3));
    when 4 =>
        wb_sll := v64(59 downto 0) & X"0";
        wb_srl := X"0" & v64(63 downto 4);
        wb_sra := (msk64(63 downto 60) & v64(63 downto 4));
    when 5 =>
        wb_sll := v64(58 downto 0) & X"0" & "0";
        wb_srl :=  X"0" & "0" & v64(63 downto 5);
        wb_sra := (msk64(63 downto 59) & v64(63 downto 5));
    when 6 =>
        wb_sll := v64(57 downto 0) & X"0" & "00";
        wb_srl := X"0" & "00" & v64(63 downto 6);
        wb_sra := (msk64(63 downto 58) & v64(63 downto 6));
    when 7 =>
        wb_sll := v64(56 downto 0) & X"0" & "000";
        wb_srl := X"0" & "000" & v64(63 downto 7);
        wb_sra := (msk64(63 downto 57) & v64(63 downto 7));
    when 8 =>
        wb_sll := v64(55 downto 0) & X"00";
        wb_srl := X"00" & v64(63 downto 8);
        wb_sra := (msk64(63 downto 56) & v64(63 downto 8));
    when 9 =>
        wb_sll := v64(54 downto 0) & X"00" & "0";
        wb_srl := X"00" & "0" & v64(63 downto 9);
        wb_sra := (msk64(63 downto 55) & v64(63 downto 9));
    when 10 =>
        wb_sll := v64(53 downto 0) & X"00" & "00";
        wb_srl := X"00" & "00" & v64(63 downto 10);
        wb_sra := (msk64(63 downto 54) & v64(63 downto 10));
    when 11 =>
        wb_sll := v64(52 downto 0) & X"00" & "000";
        wb_srl := X"00" & "000" & v64(63 downto 11);
        wb_sra := (msk64(63 downto 53) & v64(63 downto 11));
    when 12 =>
        wb_sll := v64(51 downto 0) & X"000";
        wb_srl := X"000" & v64(63 downto 12);
        wb_sra := (msk64(63 downto 52) & v64(63 downto 12));
    when 13 =>
        wb_sll := v64(50 downto 0) & X"000" & "0";
        wb_srl := X"000" & "0" & v64(63 downto 13);
        wb_sra := (msk64(63 downto 51) & v64(63 downto 13));
    when 14 =>
        wb_sll := v64(49 downto 0) & X"000" & "00";
        wb_srl := X"000" & "00" & v64(63 downto 14);
        wb_sra := (msk64(63 downto 50) & v64(63 downto 14));
    when 15 =>
        wb_sll := v64(48 downto 0) & X"000" & "000";
        wb_srl := X"000" & "000" & v64(63 downto 15);
        wb_sra := (msk64(63 downto 49) & v64(63 downto 15));
    when 16 =>
        wb_sll := v64(47 downto 0) & X"0000";
        wb_srl := X"0000" & v64(63 downto 16);
        wb_sra := (msk64(63 downto 48) & v64(63 downto 16));
    when 17 =>
        wb_sll := v64(46 downto 0) & X"0000" & "0";
        wb_srl := X"0000" & "0" & v64(63 downto 17);
        wb_sra := (msk64(63 downto 47) & v64(63 downto 17));
    when 18 =>
        wb_sll := v64(45 downto 0) & X"0000" & "00";
        wb_srl := X"0000" & "00" & v64(63 downto 18);
        wb_sra := (msk64(63 downto 46) & v64(63 downto 18));
    when 19 =>
        wb_sll := v64(44 downto 0) & X"0000" & "000";
        wb_srl := X"0000" & "000" & v64(63 downto 19);
        wb_sra := (msk64(63 downto 45) & v64(63 downto 19));
    when 20 =>
        wb_sll := v64(43 downto 0) & X"00000";
        wb_srl := X"00000" & v64(63 downto 20);
        wb_sra := (msk64(63 downto 44) & v64(63 downto 20));
    when 21 =>
        wb_sll := v64(42 downto 0) & X"00000" & "0";
        wb_srl := X"00000" & "0" & v64(63 downto 21);
        wb_sra := (msk64(63 downto 43) & v64(63 downto 21));
    when 22 =>
        wb_sll := v64(41 downto 0) & X"00000" & "00";
        wb_srl := X"00000" & "00" & v64(63 downto 22);
        wb_sra := (msk64(63 downto 42) & v64(63 downto 22));
    when 23 =>
        wb_sll := v64(40 downto 0) & X"00000" & "000";
        wb_srl := X"00000" & "000" & v64(63 downto 23);
        wb_sra := (msk64(63 downto 41) & v64(63 downto 23));
    when 24 =>
        wb_sll := v64(39 downto 0) & X"000000";
        wb_srl := X"000000" & v64(63 downto 24);
        wb_sra := (msk64(63 downto 40) & v64(63 downto 24));
    when 25 =>
        wb_sll := v64(38 downto 0) & X"000000" & "0";
        wb_srl := X"000000" & "0" & v64(63 downto 25);
        wb_sra := (msk64(63 downto 39) & v64(63 downto 25));
    when 26 =>
        wb_sll := v64(37 downto 0) & X"000000" & "00";
        wb_srl := X"000000" & "00" & v64(63 downto 26);
        wb_sra := (msk64(63 downto 38) & v64(63 downto 26));
    when 27 =>
        wb_sll := v64(36 downto 0) & X"000000" & "000";
        wb_srl := X"000000" & "000" & v64(63 downto 27);
        wb_sra := (msk64(63 downto 37) & v64(63 downto 27));
    when 28 =>
        wb_sll := v64(35 downto 0) & X"0000000";
        wb_srl := X"0000000" & v64(63 downto 28);
        wb_sra := (msk64(63 downto 36) & v64(63 downto 28));
    when 29 =>
        wb_sll := v64(34 downto 0) & X"0000000" & "0";
        wb_srl := X"0000000" & "0" & v64(63 downto 29);
        wb_sra := (msk64(63 downto 35) & v64(63 downto 29));
    when 30 =>
        wb_sll := v64(33 downto 0) & X"0000000" & "00";
        wb_srl := X"0000000" & "00" & v64(63 downto 30);
        wb_sra := (msk64(63 downto 34) & v64(63 downto 30));
    when 31 =>
        wb_sll := v64(32 downto 0) & X"0000000" & "000";
        wb_srl := X"0000000" & "000" & v64(63 downto 31);
        wb_sra := (msk64(63 downto 33) & v64(63 downto 31));
    when 32 =>
        wb_sll := v64(31 downto 0) & X"00000000";
        wb_srl := X"00000000" & v64(63 downto 32);
        wb_sra := (msk64(63 downto 32) & v64(63 downto 32));
    when 33 =>
        wb_sll := v64(30 downto 0) & X"00000000" & "0";
        wb_srl := X"00000000" & "0" & v64(63 downto 33);
        wb_sra := (msk64(63 downto 31) & v64(63 downto 33));
    when 34 =>
        wb_sll := v64(29 downto 0) & X"00000000" & "00";
        wb_srl := X"00000000" & "00" & v64(63 downto 34);
        wb_sra := (msk64(63 downto 30) & v64(63 downto 34));
    when 35 =>
        wb_sll := v64(28 downto 0) & X"00000000" & "000";
        wb_srl := X"00000000" & "000" & v64(63 downto 35);
        wb_sra := (msk64(63 downto 29) & v64(63 downto 35));
    when 36 =>
        wb_sll := v64(27 downto 0) & X"000000000";
        wb_srl := X"000000000" & v64(63 downto 36);
        wb_sra := (msk64(63 downto 28) & v64(63 downto 36));
    when 37 =>
        wb_sll := v64(26 downto 0) & X"000000000" & "0";
        wb_srl := X"000000000" & "0" & v64(63 downto 37);
        wb_sra := (msk64(63 downto 27) & v64(63 downto 37));
    when 38 =>
        wb_sll := v64(25 downto 0) & X"000000000" & "00";
        wb_srl := X"000000000" & "00" & v64(63 downto 38);
        wb_sra := (msk64(63 downto 26) & v64(63 downto 38));
    when 39 =>
        wb_sll := v64(24 downto 0) & X"000000000" & "000";
        wb_srl := X"000000000" & "000" & v64(63 downto 39);
        wb_sra := (msk64(63 downto 25) & v64(63 downto 39));
    when 40 =>
        wb_sll := v64(23 downto 0) & X"0000000000";
        wb_srl := X"0000000000" & v64(63 downto 40);
        wb_sra := (msk64(63 downto 24) & v64(63 downto 40));
    when 41 =>
        wb_sll := v64(22 downto 0) & X"0000000000" & "0";
        wb_srl := X"0000000000" & "0" & v64(63 downto 41);
        wb_sra := (msk64(63 downto 23) & v64(63 downto 41));
    when 42 =>
        wb_sll := v64(21 downto 0) & X"0000000000" & "00";
        wb_srl := X"0000000000" & "00" & v64(63 downto 42);
        wb_sra := (msk64(63 downto 22) & v64(63 downto 42));
    when 43 =>
        wb_sll := v64(20 downto 0) & X"0000000000" & "000";
        wb_srl := X"0000000000" & "000" & v64(63 downto 43);
        wb_sra := (msk64(63 downto 21) & v64(63 downto 43));
    when 44 =>
        wb_sll := v64(19 downto 0) & X"00000000000";
        wb_srl := X"00000000000" & v64(63 downto 44);
        wb_sra := (msk64(63 downto 20) & v64(63 downto 44));
    when 45 =>
        wb_sll := v64(18 downto 0) & X"00000000000" & "0";
        wb_srl := X"00000000000" & "0" & v64(63 downto 45);
        wb_sra := (msk64(63 downto 19) & v64(63 downto 45));
    when 46 =>
        wb_sll := v64(17 downto 0) & X"00000000000" & "00";
        wb_srl := X"00000000000" & "00" & v64(63 downto 46);
        wb_sra := (msk64(63 downto 18) & v64(63 downto 46));
    when 47 =>
        wb_sll := v64(16 downto 0) & X"00000000000" & "000";
        wb_srl := X"00000000000" & "000" & v64(63 downto 47);
        wb_sra := (msk64(63 downto 17) & v64(63 downto 47));
    when 48 =>
        wb_sll := v64(15 downto 0) & X"000000000000";
        wb_srl := X"000000000000" & v64(63 downto 48);
        wb_sra := (msk64(63 downto 16) & v64(63 downto 48));
    when 49 =>
        wb_sll := v64(14 downto 0) & X"000000000000" & "0";
        wb_srl := X"000000000000" & "0" & v64(63 downto 49);
        wb_sra := (msk64(63 downto 15) & v64(63 downto 49));
    when 50 =>
        wb_sll := v64(13 downto 0) & X"000000000000" & "00";
        wb_srl := X"000000000000" & "00" & v64(63 downto 50);
        wb_sra := (msk64(63 downto 14) & v64(63 downto 50));
    when 51 =>
        wb_sll := v64(12 downto 0) & X"000000000000" & "000";
        wb_srl := X"000000000000" & "000" & v64(63 downto 51);
        wb_sra := (msk64(63 downto 13) & v64(63 downto 51));
    when 52 =>
        wb_sll := v64(11 downto 0) & X"0000000000000";
        wb_srl := X"0000000000000" & v64(63 downto 52);
        wb_sra := (msk64(63 downto 12) & v64(63 downto 52));
    when 53 =>
        wb_sll := v64(10 downto 0) & X"0000000000000" & "0";
        wb_srl := X"0000000000000" & "0" & v64(63 downto 53);
        wb_sra := (msk64(63 downto 11) & v64(63 downto 53));
    when 54 =>
        wb_sll := v64(9 downto 0) & X"0000000000000" & "00";
        wb_srl := X"0000000000000" & "00" & v64(63 downto 54);
        wb_sra := (msk64(63 downto 10) & v64(63 downto 54));
    when 55 =>
        wb_sll := v64(8 downto 0) & X"0000000000000" & "000";
        wb_srl := X"0000000000000" & "000" & v64(63 downto 55);
        wb_sra := (msk64(63 downto 9) & v64(63 downto 55));
    when 56 =>
        wb_sll := v64(7 downto 0) & X"00000000000000";
        wb_srl := X"00000000000000" & v64(63 downto 56);
        wb_sra := (msk64(63 downto 8) & v64(63 downto 56));
    when 57 =>
        wb_sll := v64(6 downto 0) & X"00000000000000" & "0";
        wb_srl := X"00000000000000" & "0" & v64(63 downto 57);
        wb_sra := (msk64(63 downto 7) & v64(63 downto 57));
    when 58 =>
        wb_sll := v64(5 downto 0) & X"00000000000000" & "00";
        wb_srl := X"00000000000000" & "00" & v64(63 downto 58);
        wb_sra := (msk64(63 downto 6) & v64(63 downto 58));
    when 59 =>
        wb_sll := v64(4 downto 0) & X"00000000000000" & "000";
        wb_srl := X"00000000000000" & "000" & v64(63 downto 59);
        wb_sra := (msk64(63 downto 5) & v64(63 downto 59));
    when 60 =>
        wb_sll := v64(3 downto 0) & X"000000000000000";
        wb_srl := X"000000000000000" & v64(63 downto 60);
        wb_sra := (msk64(63 downto 4) & v64(63 downto 60));
    when 61 =>
        wb_sll := v64(2 downto 0) & X"000000000000000" & "0";
        wb_srl := X"000000000000000" & "0" & v64(63 downto 61);
        wb_sra := (msk64(63 downto 3) & v64(63 downto 61));
    when 62 =>
        wb_sll := v64(1 downto 0) & X"000000000000000" & "00";
        wb_srl := X"000000000000000" & "00" & v64(63 downto 62);
        wb_sra := (msk64(63 downto 2) & v64(63 downto 62));
    when 63 =>
        wb_sll := v64(0) & X"000000000000000" & "000";
        wb_srl := X"000000000000000" & "000" & v64(63);
        wb_sra := (msk64(63 downto 1) & v64(63));
    end case;

    case shift32 is
    when 0 =>
        wb_sllw(31 downto 0) := v32;
        wb_srlw(31 downto 0) := v32;
        wb_sraw := (msk32(63 downto 32) & v32);
    when 1 =>
        wb_sllw(31 downto 0) := v32(30 downto 0) & "0";
        wb_srlw(31 downto 0) := "0" & v32(31 downto 1);
        wb_sraw := (msk32(63 downto 31) & v32(31 downto 1));
    when 2 =>
        wb_sllw(31 downto 0) := v32(29 downto 0) & "00";
        wb_srlw(31 downto 0) := "00" & v32(31 downto 2);
        wb_sraw := (msk32(63 downto 30) & v32(31 downto 2));
    when 3 =>
        wb_sllw(31 downto 0) := v32(28 downto 0) & "000";
        wb_srlw(31 downto 0) := "000" & v32(31 downto 3);
        wb_sraw := (msk32(63 downto 29) & v32(31 downto 3));
    when 4 =>
        wb_sllw(31 downto 0) := v32(27 downto 0) & X"0";
        wb_srlw(31 downto 0) := X"0" & v32(31 downto 4);
        wb_sraw := (msk32(63 downto 28) & v32(31 downto 4));
    when 5 =>
        wb_sllw(31 downto 0) := v32(26 downto 0) & X"0" & "0";
        wb_srlw(31 downto 0) := X"0" & "0" & v32(31 downto 5);
        wb_sraw := (msk32(63 downto 27) & v32(31 downto 5));
    when 6 =>
        wb_sllw(31 downto 0) := v32(25 downto 0) & X"0" & "00";
        wb_srlw(31 downto 0) := X"0" & "00" & v32(31 downto 6);
        wb_sraw := (msk32(63 downto 26) & v32(31 downto 6));
    when 7 =>
        wb_sllw(31 downto 0) := v32(24 downto 0) & X"0" & "000";
        wb_srlw(31 downto 0) := X"0" & "000" & v32(31 downto 7);
        wb_sraw := (msk32(63 downto 25) & v32(31 downto 7));
    when 8 =>
        wb_sllw(31 downto 0) := v32(23 downto 0) & X"00";
        wb_srlw(31 downto 0) := X"00" & v32(31 downto 8);
        wb_sraw := (msk32(63 downto 24) & v32(31 downto 8));
    when 9 =>
        wb_sllw(31 downto 0) := v32(22 downto 0) & X"00" & "0";
        wb_srlw(31 downto 0) := X"00" & "0" & v32(31 downto 9);
        wb_sraw := (msk32(63 downto 23) & v32(31 downto 9));
    when 10 =>
        wb_sllw(31 downto 0) := v32(21 downto 0) & X"00" & "00";
        wb_srlw(31 downto 0) := X"00" & "00" & v32(31 downto 10);
        wb_sraw := (msk32(63 downto 22) & v32(31 downto 10));
    when 11 =>
        wb_sllw(31 downto 0) := v32(20 downto 0) & X"00" & "000";
        wb_srlw(31 downto 0) := X"00" & "000" & v32(31 downto 11);
        wb_sraw := (msk32(63 downto 21) & v32(31 downto 11));
    when 12 =>
        wb_sllw(31 downto 0) := v32(19 downto 0) & X"000";
        wb_srlw(31 downto 0) := X"000" & v32(31 downto 12);
        wb_sraw := (msk32(63 downto 20) & v32(31 downto 12));
    when 13 =>
        wb_sllw(31 downto 0) := v32(18 downto 0) & X"000" & "0";
        wb_srlw(31 downto 0) := X"000" & "0" & v32(31 downto 13);
        wb_sraw := (msk32(63 downto 19) & v32(31 downto 13));
    when 14 =>
        wb_sllw(31 downto 0) := v32(17 downto 0) & X"000" & "00";
        wb_srlw(31 downto 0) := X"000" & "00" & v32(31 downto 14);
        wb_sraw := (msk32(63 downto 18) & v32(31 downto 14));
    when 15 =>
        wb_sllw(31 downto 0) := v32(16 downto 0) & X"000" & "000";
        wb_srlw(31 downto 0) := X"000" & "000" & v32(31 downto 15);
        wb_sraw := (msk32(63 downto 17) & v32(31 downto 15));
    when 16 =>
        wb_sllw(31 downto 0) := v32(15 downto 0) & X"0000";
        wb_srlw(31 downto 0) := X"0000" & v32(31 downto 16);
        wb_sraw := (msk32(63 downto 16) & v32(31 downto 16));
    when 17 =>
        wb_sllw(31 downto 0) := v32(14 downto 0) & X"0000" & "0";
        wb_srlw(31 downto 0) := X"0000" & "0" & v32(31 downto 17);
        wb_sraw := (msk32(63 downto 15) & v32(31 downto 17));
    when 18 =>
        wb_sllw(31 downto 0) := v32(13 downto 0) & X"0000" & "00";
        wb_srlw(31 downto 0) := X"0000" & "00" & v32(31 downto 18);
        wb_sraw := (msk32(63 downto 14) & v32(31 downto 18));
    when 19 =>
        wb_sllw(31 downto 0) := v32(12 downto 0) & X"0000" & "000";
        wb_srlw(31 downto 0) := X"0000" & "000" & v32(31 downto 19);
        wb_sraw := (msk32(63 downto 13) & v32(31 downto 19));
    when 20 =>
        wb_sllw(31 downto 0) := v32(11 downto 0) & X"00000";
        wb_srlw(31 downto 0) := X"00000" & v32(31 downto 20);
        wb_sraw := (msk32(63 downto 12) & v32(31 downto 20));
    when 21 =>
        wb_sllw(31 downto 0) := v32(10 downto 0) & X"00000" & "0";
        wb_srlw(31 downto 0) := X"00000" & "0" & v32(31 downto 21);
        wb_sraw := (msk32(63 downto 11) & v32(31 downto 21));
    when 22 =>
        wb_sllw(31 downto 0) := v32(9 downto 0) & X"00000" & "00";
        wb_srlw(31 downto 0) := X"00000" & "00" & v32(31 downto 22);
        wb_sraw := (msk32(63 downto 10) & v32(31 downto 22));
    when 23 =>
        wb_sllw(31 downto 0) := v32(8 downto 0) & X"00000" & "000";
        wb_srlw(31 downto 0) := X"00000" & "000" & v32(31 downto 23);
        wb_sraw := (msk32(63 downto 9) & v32(31 downto 23));
    when 24 =>
        wb_sllw(31 downto 0) := v32(7 downto 0) & X"000000";
        wb_srlw(31 downto 0) := X"000000" & v32(31 downto 24);
        wb_sraw := (msk32(63 downto 8) & v32(31 downto 24));
    when 25 =>
        wb_sllw(31 downto 0) := v32(6 downto 0) & X"000000" & "0";
        wb_srlw(31 downto 0) := X"000000" & "0" & v32(31 downto 25);
        wb_sraw := (msk32(63 downto 7) & v32(31 downto 25));
    when 26 =>
        wb_sllw(31 downto 0) := v32(5 downto 0) & X"000000" & "00";
        wb_srlw(31 downto 0) := X"000000" & "00" & v32(31 downto 26);
        wb_sraw := (msk32(63 downto 6) & v32(31 downto 26));
    when 27 =>
        wb_sllw(31 downto 0) := v32(4 downto 0) & X"000000" & "000";
        wb_srlw(31 downto 0) := X"000000" & "000" & v32(31 downto 27);
        wb_sraw := (msk32(63 downto 5) & v32(31 downto 27));
    when 28 =>
        wb_sllw(31 downto 0) := v32(3 downto 0) & X"0000000";
        wb_srlw(31 downto 0) := X"0000000" & v32(31 downto 28);
        wb_sraw := (msk32(63 downto 4) & v32(31 downto 28));
    when 29 =>
        wb_sllw(31 downto 0) := v32(2 downto 0) & X"0000000" & "0";
        wb_srlw(31 downto 0) := X"0000000" & "0" & v32(31 downto 29);
        wb_sraw := (msk32(63 downto 3) & v32(31 downto 29));
    when 30 =>
        wb_sllw(31 downto 0) := v32(1 downto 0) & X"0000000" & "00";
        wb_srlw(31 downto 0) := X"0000000" & "00" & v32(31 downto 30);
        wb_sraw := (msk32(63 downto 2) & v32(31 downto 30));
    when 31 =>
        wb_sllw(31 downto 0) := v32(0) & X"0000000" & "000";
        wb_srlw(31 downto 0) := X"0000000" & "000" & v32(31 downto 31);
        wb_sraw := (msk32(63 downto 1) & v32(31 downto 31));
    end case;

    -- Take into account case when shift = 0 and input value a[31]=1
    wb_srlw(63 downto 32) := (others => wb_srlw(31));
    wb_sllw(63 downto 32) := (others => wb_sllw(31));

    o_sll <= wb_sll;
    o_srl <= wb_srl;
    o_sra <= wb_sra;
    o_sllw <= wb_sllw;
    o_srlw <= wb_srlw;
    o_sraw <= wb_sraw;
  end process;

end;
