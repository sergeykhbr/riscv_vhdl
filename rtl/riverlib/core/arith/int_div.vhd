-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Integer divider.
--! @details   Algorithm spends 33 clocks per instruction
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity IntDiv is generic (
    async_reset : boolean
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

  type RegistersType is record
      rv32 : std_logic;
      resid : std_logic;                           -- Compute residual flag
      invert : std_logic;                          -- invert result value before output
      busy : std_logic;
      ena : std_logic_vector(33 downto 0);
      qr : std_logic_vector(127 downto 0);
      divider : std_logic_vector(64 downto 0);
      result : std_logic_vector(RISCV_ARCH-1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
      '0', '0', '0', '0',                -- rv32, resid, invert, busy
      (others => '0'), (others => '0'),  -- ena, qr
      (others => '0'), (others => '0')   -- divider, result
  );

  signal r, rin : RegistersType;

begin


  comb : process(i_nrst, i_ena, i_unsigned, i_residual, i_rv32, i_a1, i_a2, r)
    variable v : RegistersType;
    variable wb_diff1 : std_logic_vector(64 downto 0);
    variable wb_diff2 : std_logic_vector(64 downto 0);
    variable wb_qr1 : std_logic_vector(127 downto 0);
    variable wb_qr2 : std_logic_vector(127 downto 0);
    variable wb_a1 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_a2 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_divident : std_logic_vector(64 downto 0);
    variable wb_divider : std_logic_vector(64 downto 0);
    variable w_invert64 : std_logic;
    variable w_invert32 : std_logic;
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

    wb_divident(63 downto 0) := wb_a1;
    wb_divider(63 downto 0) := wb_a2;

    v.ena := r.ena(32 downto 0) & (i_ena and not r.busy);

    -- Level 2*i of 64:
    wb_diff1 := r.qr(127 downto 63) - r.divider;
    if wb_diff1(64) = '1' then
        wb_qr1 := r.qr(126 downto 0) & '0';
    else
        wb_qr1 := wb_diff1(63 downto 0) & r.qr(62 downto 0) & '1';
    end if;

    -- Level 2*i + 1 of 64:
    wb_diff2 := wb_qr1(127 downto 63) - r.divider;
    if wb_diff2(64) = '1' then
        wb_qr2 := wb_qr1(126 downto 0) & '0';
    else
        wb_qr2 := wb_diff2(63 downto 0) & wb_qr1(62 downto 0) & '1';
    end if;


    if i_ena = '1' then
        v.qr(127 downto 65) := (others => '0');
        v.qr(64 downto 0) := wb_divident;
        v.divider := wb_divider;
        v.busy := '1';
        v.rv32 := i_rv32;
        v.resid := i_residual;
        w_invert32 := not i_unsigned and
                ((not i_residual and (i_a1(31) xor i_a2(31)))
                or (i_residual and i_a1(31)));
        w_invert64 := not i_unsigned and
                ((not i_residual and (i_a1(63) xor i_a2(63)))
                or (i_residual and i_a1(63)));
        v.invert := (not i_rv32 and w_invert64) 
                or (i_rv32 and w_invert32);
    elsif r.ena(32) = '1' then
        v.busy := '0';
        if r.resid = '1' then
            if r.invert = '1' then
                v.result := (not r.qr(127 downto 64)) + 1;
            else
                v.result := r.qr(127 downto 64);
            end if;
        else
            if r.invert = '1' then
                v.result := (not r.qr(63 downto 0)) + 1;
            else
                v.result := r.qr(63 downto 0);
            end if;
        end if;
    elsif r.busy = '1' then
        v.qr := wb_qr2;
    end if;

    if not async_reset and i_nrst = '0' then
        v.result := (others => '0');
        v.ena := (others => '0');
        v.busy := '0';
        v.rv32 := '0';
        v.invert := '0';
        v.qr := (others => '0');
        v.resid := '0';
    end if;

    o_res <= r.result;
    o_valid <= r.ena(33);
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
