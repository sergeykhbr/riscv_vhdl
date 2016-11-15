-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Integer multiplier.
--! @details   Implemented algorithm provides 4 clocks per instruction
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity IntMul is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_ena : in std_logic;                                -- Enable bit
    i_unsigned : in std_logic;                           -- Unsigned operands
    i_high : in std_logic;                               -- High multiplied bits [127:64]
    i_rv32 : in std_logic;                               -- 32-bits operands enable
    i_a1 : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Operand 1
    i_a2 : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- Operand 1
    o_res : out std_logic_vector(RISCV_ARCH-1 downto 0); -- Result
    o_valid : out std_logic;                             -- Result is valid
    o_busy : out std_logic                               -- Multiclock instruction under processing
  );
end; 
 
architecture arch_IntMul of IntMul is

  type Level0Type is array (0 to 31) of std_logic_vector(65 downto 0);
  type Level1Type is array (0 to 15) of std_logic_vector(68 downto 0);
  type Level2Type is array (0 to 7) of std_logic_vector(73 downto 0);
  type Level3Type is array (0 to 3) of std_logic_vector(82 downto 0);
  type Level4Type is array (0 to 1) of std_logic_vector(99 downto 0);

  type RegistersType is record
      busy : std_logic;
      ena : std_logic_vector(3 downto 0);
      a1 : std_logic_vector(RISCV_ARCH-1 downto 0);
      a2 : std_logic_vector(RISCV_ARCH-1 downto 0);
      unsign : std_logic;
      high : std_logic;
      rv32 : std_logic;
      lvl1 : Level1Type;
      lvl3 : Level3Type;
      result : std_logic_vector(127 downto 0);
  end record;

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_ena, i_unsigned, i_high, i_rv32, i_a1, i_a2, r)
    variable v : RegistersType;
    variable wb_mux_lvl0 : std_logic_vector(1 downto 0);
    variable wb_lvl0 : Level0Type;
    variable wb_lvl2 : Level2Type;
    variable wb_lvl4 : Level4Type;
    variable wb_res : std_logic_vector(RISCV_ARCH-1 downto 0);
  begin

    v := r;

    v.ena := r.ena(2 downto 0) & (i_ena and not r.busy);

    if i_ena = '1' then
        v.busy := '1';
        if i_rv32 = '1' then
            v.a1(31 downto 0) := i_a1(31 downto 0);
            if (not i_unsigned and i_a1(31)) = '1' then
                v.a1(63 downto 32) := (others => '1');
            end if;
            v.a2(31 downto 0) := i_a2(31 downto 0);
            if (not i_unsigned and i_a2(31)) = '1' then
                v.a2(63 downto 32) := (others => '1');
            end if;
        else
            v.a1 := i_a1;
            v.a2 := i_a2;
        end if;
        v.rv32 := i_rv32;
        v.unsign := i_unsigned;
        v.high := i_high;
    end if;

    if r.ena(0) = '1' then
        for i in 0 to 31 loop
            wb_mux_lvl0 := r.a2(2*i + 1 downto 2*i);
            if wb_mux_lvl0 = "00" then
                wb_lvl0(i) := (others => '0');
            elsif wb_mux_lvl0 = "01" then
                wb_lvl0(i) := ("00" & r.a1);
            elsif wb_mux_lvl0 = "10" then
                wb_lvl0(i) := ("0" & r.a1 & "0");
            else
                wb_lvl0(i) := ("00" & r.a1) + ("0" & r.a1 & "0");
            end if;
        end loop;

        for i in 0 to 15 loop
            v.lvl1(i) := ("0" & wb_lvl0(2*i + 1) & "00")
                       + ("000" & wb_lvl0(2*i));
        end loop;
    end if;

    if r.ena(1) = '1' then
        for i in 0 to 7 loop
            wb_lvl2(i) := ("0" & r.lvl1(2*i + 1) & "0000")
                        + ("00000" & r.lvl1(2*i));
        end loop;

        for i in 0 to 3 loop
            v.lvl3(i) := ("0" & wb_lvl2(2*i + 1) & "00000000")
                       + ("000000000" & wb_lvl2(2*i));
        end loop;
    end if;

    if r.ena(2) = '1' then
        v.busy := '0';
        for i in 0 to 1 loop
            wb_lvl4(i) := ("0" & r.lvl3(2*i + 1) & "0000000000000000")
                        + ("00000000000000000" & r.lvl3(2*i));
        end loop;

        v.result := (wb_lvl4(1)(95 downto 0) & X"00000000")
                 + (X"0000000" & wb_lvl4(0));
    end if;

    wb_res := r.result(63 downto 0);
    if r.high = '1' then
        wb_res := r.result(127 downto 64);  --! not tested yet
    end if;

    if i_nrst = '0' then
        v.busy := '0';
        v.result := (others => '0');
        v.ena := (others => '0');
        v.a1 := (others => '0');
        v.a2 := (others => '0');
        v.rv32 := '0';
        v.unsign := '0';
        v.high := '0';
    end if;

    o_res <= wb_res;
    o_valid <= r.ena(3);
    o_busy <= r.busy;
    
    rin <= v;
  end process;

  -- registers:
  regs : process(i_clk)
  begin 
     if rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
