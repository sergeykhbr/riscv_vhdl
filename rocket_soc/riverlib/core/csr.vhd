-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     CSR registers module.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;

entity CsrRegs is
  port (
    i_clk : in std_logic;                                   -- CPU clock
    i_nrst : in std_logic;                                  -- Reset. Active LOW.
    i_xret : in std_logic;                                  -- XRet instruction signals mode switching
    i_addr : in std_logic_vector(11 downto 0);              -- CSR address, if xret=1 switch mode accordingly
    i_wena : in std_logic;                                  -- Write enable
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- CSR writing value
    o_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);  -- CSR read value
    i_trap_ena : in std_logic;                              -- Trap pulse
    i_trap_code : in std_logic_vector(4 downto 0);          -- bit[4] : 1=interrupt; 0=exception; bits[3:0]=code
    i_trap_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- trap on pc

    o_ie : out std_logic;                                    -- Interrupt enable bit
    o_mode : out std_logic_vector(1 downto 0);               -- CPU mode
    o_mtvec : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0)-- Interrupt descriptors table
  );
end; 
 
architecture arch_CsrRegs of CsrRegs is

  type RegistersType is record
      mtvec : std_logic_vector(RISCV_ARCH-1 downto 0);
      mscratch : std_logic_vector(RISCV_ARCH-1 downto 0);
      mbadaddr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      mode : std_logic_vector(1 downto 0);
      uie : std_logic;                       -- User level interrupts ena for current priv. mode
      mie : std_logic;                       -- Machine level interrupts ena for current priv. mode
      mpie : std_logic;                      -- Previous MIE value
      mpp : std_logic_vector(1 downto 0);    -- Previous mode
      mepc : std_logic_vector(RISCV_ARCH-1 downto 0);

      trap_irq : std_logic;
      trap_code : std_logic_vector(3 downto 0);
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_xret, i_addr, i_wena, i_wdata, i_trap_ena,
                 i_trap_code, i_trap_pc, r)
    variable v : RegistersType;
    variable wb_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable w_ie : std_logic;
  begin

    v := r;

    wb_rdata := (others => '0');
    case i_addr is
    when CSR_misa =>
    when CSR_mvendorid =>
    when CSR_marchid =>
    when CSR_mimplementationid =>
    when CSR_mhartid =>
    when CSR_uepc =>    -- User mode program counter
    when CSR_mstatus => -- Machine mode status register
        wb_rdata(0) := r.uie;
        wb_rdata(3) := r.mie;
        wb_rdata(7) := r.mpie;
        wb_rdata(12 downto 11) := r.mpp;
        if i_wena = '1' then
            v.uie := i_wdata(0);
            v.mie := i_wdata(3);
            v.mpie := i_wdata(7);
            v.mpp := i_wdata(12 downto 11);
        end if;
    when CSR_medeleg => -- Machine exception delegation
    when CSR_mideleg => -- Machine itnerrupt delegation
    when CSR_mie =>     -- Machine interrupt enable bit
    when CSR_mtvec =>
        wb_rdata := r.mtvec;
        if i_wena = '1' then
            v.mtvec := i_wdata;
        end if;
    when CSR_mtimecmp => -- Machine wall-clock timer compare value
    when CSR_mscratch => -- Machine scratch register
        wb_rdata := r.mscratch;
        if i_wena = '1' then
            v.mscratch := i_wdata;
        end if;
    when CSR_mepc => -- Machine program counter
        wb_rdata := r.mepc;
        if i_xret = '1' then
            -- Switch to previous mode
            v.mie := r.mpie;
            v.mpie := '1';
            v.mode := r.mpp;
            v.mpp := PRV_U;
        end if;
        if i_wena = '1' then
            v.mepc := i_wdata;
        end if;
    when CSR_mcause => -- Machine trap cause
        wb_rdata := (others => '0');
        wb_rdata(63) := r.trap_irq;
        wb_rdata(3 downto 0) := r.trap_code;
    when CSR_mbadaddr => -- Machine bad address
        wb_rdata(RISCV_ARCH-1 downto BUS_ADDR_WIDTH) := (others => '0');
        wb_rdata(BUS_ADDR_WIDTH-1 downto 0) := r.mbadaddr;
    when CSR_mip =>      -- Machine interrupt pending
    when others =>
    end case;

    if i_trap_ena = '1' then
        v.mie := '0';
        v.mpp := r.mode;
        v.mepc(RISCV_ARCH-1 downto BUS_ADDR_WIDTH) := (others => '0');
        v.mepc(BUS_ADDR_WIDTH-1 downto 0) := i_trap_pc;
        v.trap_code := i_trap_code(3 downto 0);
        v.trap_irq := i_trap_code(4);
        v.mode := PRV_M;
        case r.mode is
        when PRV_U =>
            v.mpie := r.uie;
        when PRV_M =>
            v.mpie := r.mie;
        when others =>
        end case;
    end if;

    w_ie := '0';
    if (r.mode /= PRV_M) or r.mie = '1' then
        w_ie := '1';
    end if;

    if i_nrst = '0' then
        v.mtvec := (others => '0');
        v.mscratch := (others => '0');
        v.mbadaddr := (others => '0');
        v.mode := PRV_M;
        v.uie := '0';
        v.mie := '0';
        v.mpie := '0';
        v.mpp := (others => '0');
        v.mepc := (others => '0');
        v.trap_code := (others => '0');
        v.trap_irq := '0';
    end if;

    o_rdata <= wb_rdata;
    o_ie <= w_ie;
    o_mode <= r.mode;
    o_mtvec <= r.mtvec(BUS_ADDR_WIDTH-1 downto 0);
    
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
