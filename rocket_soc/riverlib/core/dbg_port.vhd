-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Debug port must be connected to DSU.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity DbgPort is
  port (
    i_clk : in std_logic;                                     -- CPU clock
    i_nrst : in std_logic;                                    -- Reset. Active LOW.
    -- "RIVER" Debug interface
    i_dport_valid : in std_logic;                             -- Debug access from DSU is valid
    i_dport_write : in std_logic;                             -- Write command flag
    i_dport_region : in std_logic_vector(1 downto 0);         -- Registers region ID: 0=CSR; 1=IREGS; 2=Control
    i_dport_addr : in std_logic_vector(11 downto 0);          -- Register idx
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Write value
    o_dport_ready : out std_logic;                            -- Response is ready
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Response value
    -- CPU debugging signals:
    o_core_addr : out std_logic_vector(11 downto 0);          -- Address of the sub-region register
    o_core_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Write data
    o_csr_ena : out std_logic;                                -- Region 0: Access to CSR bank is enabled.
    o_csr_write : out std_logic;                              -- Region 0: CSR write enable
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Region 0: CSR read value
    o_ireg_ena : out std_logic;                               -- Region 1: Access to integer register bank is enabled
    o_ireg_write : out std_logic;                             -- Region 1: Integer registers bank write pulse
    o_npc_write : out std_logic;                              -- Region 1: npc write enable
    i_ireg_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Region 1: Integer register read value
    i_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Region 1: Instruction pointer
    i_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);   -- Region 1: Next Instruction pointer
    i_e_valid : in std_logic;                                 -- Stepping control signal
    i_m_valid : in std_logic;                                 -- To compute number of valid executed instruction
    o_clock_cnt : out std_logic_vector(63 downto 0);           -- Number of clocks excluding halt state
    o_executed_cnt : out std_logic_vector(63 downto 0);        -- Number of executed instructions
    o_halt : out std_logic                                    -- Halt signal is equal to hold pipeline
  );
end; 
 
architecture arch_DbgPort of DbgPort is

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');
  constant one64 : std_logic_vector(63 downto 0) := X"0000000000000001";

  type RegistersType is record
      ready : std_logic;
      halt : std_logic;
      stepping_mode : std_logic;
      stepping_mode_cnt : std_logic_vector(RISCV_ARCH-1 downto 0);

      rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      stepping_mode_steps : std_logic_vector(RISCV_ARCH-1 downto 0); -- Number of steps before halt in stepping mode
      clock_cnt : std_logic_vector(63 downto 0);               -- Timer in clocks.
      executed_cnt : std_logic_vector(63 downto 0);            -- Number of valid executed instructions
  end record;

  signal r, rin : RegistersType;

begin

  comb : process(i_nrst, i_dport_valid, i_dport_write, i_dport_region, 
                 i_dport_addr, i_dport_wdata, i_ireg_rdata, i_csr_rdata,
                 i_pc, i_npc, i_e_valid, i_m_valid, r)
    variable v : RegistersType;
    variable wb_o_core_addr : std_logic_vector(11 downto 0);
    variable wb_o_core_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_rdata : std_logic_vector(63 downto 0);
    variable wb_idx : integer range 0 to 4095;
    variable w_o_csr_ena : std_logic;
    variable w_o_csr_write : std_logic;
    variable w_o_ireg_ena : std_logic;
    variable w_o_ireg_write : std_logic;
    variable w_o_npc_write : std_logic;
    variable w_cur_halt : std_logic;
  begin

    v := r;

    wb_o_core_addr := (others => '0');
    wb_o_core_wdata := (others => '0');
    wb_rdata := (others => '0');
    wb_idx := conv_integer(i_dport_addr);
    w_o_csr_ena := '0';
    w_o_csr_write := '0';
    w_o_ireg_ena := '0';
    w_o_ireg_write := '0';
    w_o_npc_write := '0';

    v.ready := i_dport_valid;

    w_cur_halt := '0';
    if i_e_valid = '1' then
        if r.stepping_mode_cnt /= zero64(RISCV_ARCH-1 downto 0) then
            v.stepping_mode_cnt := r.stepping_mode_cnt - 1;
            if r.stepping_mode_cnt = one64 then
                v.halt := '1';
                w_cur_halt := '1';
                v.stepping_mode := '0';
            end if;
        end if;
    end if;

    if r.halt = '0' then
        v.clock_cnt := r.clock_cnt + 1;
    end if;
    if i_m_valid = '1' then
        v.executed_cnt := r.executed_cnt + 1;
    end if;

    if i_dport_valid = '1' then
        case i_dport_region is
        when "00" =>
            w_o_csr_ena := '1';
            wb_o_core_addr := i_dport_addr;
            wb_rdata := i_csr_rdata;
            if i_dport_write = '1' then
                w_o_csr_write := '1';
                wb_o_core_wdata := i_dport_wdata;
            end if;
        when "01" =>
            if wb_idx < 32 then
                w_o_ireg_ena := '1';
                wb_o_core_addr := i_dport_addr;
                wb_rdata := i_ireg_rdata;
                if i_dport_write = '1' then
                    w_o_ireg_write := '1';
                    wb_o_core_wdata := i_dport_wdata;
                end if;
            elsif wb_idx = 32 then
                --! Read only register
                wb_rdata(BUS_ADDR_WIDTH-1 downto 0) := i_pc;
            elsif wb_idx = 33 then
                wb_rdata(BUS_ADDR_WIDTH-1 downto 0) := i_npc;
                if i_dport_write = '1' then
                    w_o_npc_write := '1';
                    wb_o_core_wdata := i_dport_wdata;
                end if;
            end if;
        when "10" =>
            case wb_idx is
            when 0 =>
                wb_rdata(0) := r.halt;
                if i_dport_write = '1' then
                    v.halt := i_dport_wdata(0);
                    v.stepping_mode := i_dport_wdata(1);
                    if i_dport_wdata(1) = '1' then
                        v.stepping_mode_cnt := r.stepping_mode_steps;
                    end if;
                end if;
            when 1 =>
                wb_rdata := r.stepping_mode_steps;
                if i_dport_write = '1' then
                    v.stepping_mode_steps := i_dport_wdata;
                end if;
            when 2 =>
                wb_rdata := r.clock_cnt;
            when 3 =>
                wb_rdata := r.executed_cnt;
            when 4 =>
                -- todo: add hardware breakpoint
            when 5 =>
                -- todo: remove hardware breakpoint
            when others =>
            end case;
        when others =>
        end case;
    end if;
    v.rdata := wb_rdata;

    if i_nrst = '0' then
        v.ready := '0';
        v.halt := '0';
        v.stepping_mode := '0';
        v.rdata := (others => '0');
        v.stepping_mode_cnt := (others => '0');
        v.stepping_mode_steps := (others => '0');
        v.clock_cnt := (others => '0');
        v.executed_cnt := (others => '0');
    end if;

    rin <= v;

    o_core_addr <= wb_o_core_addr;
    o_core_wdata <= wb_o_core_wdata;
    o_csr_ena <= w_o_csr_ena;
    o_csr_write <= w_o_csr_write;
    o_ireg_ena <= w_o_ireg_ena;
    o_ireg_write <= w_o_ireg_write;
    o_npc_write <= w_o_npc_write;
    o_clock_cnt <= r.clock_cnt;
    o_executed_cnt <= r.executed_cnt;
    o_halt <= r.halt or w_cur_halt;

    o_dport_ready <= r.ready;
    o_dport_rdata <= r.rdata;
  end process;


  -- registers:
  regs : process(i_clk)
  begin 
     if rising_edge(i_clk) then 
        r <= rin;
     end if; 
  end process;

end;
