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
    o_mtvec : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Interrupt descriptors table

    i_dport_ena : in std_logic;                              -- Debug port request is enabled
    i_dport_write : in std_logic;                            -- Debug port Write enable
    i_dport_addr : in std_logic_vector(11 downto 0);         -- Debug port CSR address
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Debug port CSR writing value
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0)-- Debug port CSR read value
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
  
  procedure procedure_RegAccess(
     iaddr  : in std_logic_vector(11 downto 0);
     iwena  : in std_logic;
     iwdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
     ir : in RegistersType;
     ov : out RegistersType;
     ordata : out std_logic_vector(RISCV_ARCH-1 downto 0)) is
  begin
    ordata := (others => '0');
    case iaddr is
    when CSR_misa =>
        --! Base[XLEN-1:XLEN-2]
        --!     1 = 32
        --!     2 = 64
        --!     3 = 128
        --!
        ordata(RISCV_ARCH-1 downto RISCV_ARCH-2) := "10";
        --! BitCharacterDescription
        --! 0  A Atomic extension
        --! 1  B Tentatively reserved for Bit operations extension
        --! 2  C Compressed extension3DDouble-precision Foating-point extension
        --! 4  E RV32E base ISA
        --! 5  F Single-precision Foating-point extension
        --! 6  G Additional standard extensions present
        --! 7  H Hypervisor mode implemented
        --! 8  I RV32I/64I/128I base ISA
        --! 9  J Reserved
        --! 10 K Reserved
        --! 11 L Tentatively reserved for Decimal Floating-Point extension
        --! 12 M Integer Multiply/Divide extension
        --! 13 N User-level interrupts supported
        --! 14 O Reserved
        --! 15 P Tentatively reserved for Packed-SIMD extension
        --! 16 Q Quad-precision Foating-point extension
        --! 17 R Reserved
        --! 18 S Supervisor mode implemented
        --! 19 T Tentatively reserved for Transactional Memory extension
        --! 20 U User mode implemented
        --! 21 V Tentatively reserved for Vector extension
        --! 22 W Reserved
        --! 23 X Non-standard extensions present
        --! 24 Y Reserved
        --! 25 Z Reserve
        --!
        ordata(8) := '1';
        ordata(12) := '1';
        ordata(20) := '1';
    when CSR_mvendorid =>
    when CSR_marchid =>
    when CSR_mimplementationid =>
    when CSR_mhartid =>
    when CSR_uepc =>    -- User mode program counter
    when CSR_mstatus => -- Machine mode status register
        ordata(0) := ir.uie;
        ordata(3) := ir.mie;
        ordata(7) := ir.mpie;
        ordata(12 downto 11) := ir.mpp;
        if iwena = '1' then
            ov.uie := iwdata(0);
            ov.mie := iwdata(3);
            ov.mpie := iwdata(7);
            ov.mpp := iwdata(12 downto 11);
        end if;
    when CSR_medeleg => -- Machine exception delegation
    when CSR_mideleg => -- Machine interrupt delegation
    when CSR_mie =>     -- Machine interrupt enable bit
    when CSR_mtvec =>
        ordata := ir.mtvec;
        if iwena = '1' then
            ov.mtvec := iwdata;
        end if;
    when CSR_mtimecmp => -- Machine wall-clock timer compare value
    when CSR_mscratch => -- Machine scratch register
        ordata := ir.mscratch;
        if iwena = '1' then
            ov.mscratch := iwdata;
        end if;
    when CSR_mepc => -- Machine program counter
        ordata := ir.mepc;
        if iwena = '1' then
            ov.mepc := iwdata;
        end if;
    when CSR_mcause => -- Machine trap cause
        ordata(63) := ir.trap_irq;
        ordata(3 downto 0) := ir.trap_code;
    when CSR_mbadaddr => -- Machine bad address
        ordata(BUS_ADDR_WIDTH-1 downto 0) := ir.mbadaddr;
    when CSR_mip =>      -- Machine interrupt pending
    when others =>
    end case;
  end;

begin

  comb : process(i_nrst, i_xret, i_addr, i_wena, i_wdata, i_trap_ena,
                 i_dport_ena, i_dport_write, i_dport_addr, i_dport_wdata,
                 i_trap_code, i_trap_pc, r)
    variable v : RegistersType;
    variable wb_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_dport_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable w_ie : std_logic;
    variable w_dport_wena : std_logic;
  begin

    v := r;

    w_dport_wena := i_dport_ena and i_dport_write;

    procedure_RegAccess(i_addr, i_wena, i_wdata,
                        r, v, wb_rdata);

    procedure_RegAccess(i_dport_addr, w_dport_wena,
                        i_dport_wdata, r, v, wb_dport_rdata);

    if i_addr = CSR_mepc and i_xret = '1' then
        -- Switch to previous mode
        v.mie := r.mpie;
        v.mpie := '1';
        v.mode := r.mpp;
        v.mpp := PRV_U;
    end if;

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
    o_dport_rdata <= wb_dport_rdata;
    
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
