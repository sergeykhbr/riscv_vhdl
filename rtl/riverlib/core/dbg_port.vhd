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
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity DbgPort is generic (
    async_reset : boolean
  );
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
    o_freg_ena : out std_logic;                               -- Region 1: Access to float register bank is enabled
    o_freg_write : out std_logic;                             -- Region 1: Float registers bank write pulse
    o_npc_write : out std_logic;                              -- Region 1: npc write enable
    i_ireg_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Region 1: Integer register read value
    i_freg_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Region 1: Float register read value
    i_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Region 1: Instruction pointer
    i_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);   -- Region 1: Next Instruction pointer
    i_e_valid : in std_logic;                                 -- Stepping control signal
    i_e_call : in std_logic;                                  -- pseudo-instruction CALL
    i_e_ret : in std_logic;                                   -- pseudo-instruction RET
    i_m_valid : in std_logic;                                 -- To compute number of valid executed instruction
    o_clock_cnt : out std_logic_vector(63 downto 0);          -- Number of clocks excluding halt state
    o_executed_cnt : out std_logic_vector(63 downto 0);       -- Number of executed instructions
    o_halt : out std_logic;                                   -- Halt signal is equal to hold pipeline
    i_ebreak : in std_logic;                                  -- ebreak instruction decoded
    o_break_mode : out std_logic;                             -- Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
    o_br_fetch_valid : out std_logic;                         -- Fetch injection address/instr are valid
    o_br_address_fetch : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Fetch injection address to skip ebreak instruciton only once
    o_br_instr_fetch : out std_logic_vector(31 downto 0);     -- Real instruction value that was replaced by ebreak
    o_flush_address : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);  -- Address of instruction to remove from ICache
    o_flush_valid : out std_logic;                            -- Remove address from ICache is valid
    -- Debug signals:
    i_istate : in std_logic_vector(1 downto 0);               -- ICache state machine value
    i_dstate : in std_logic_vector(1 downto 0);               -- DCache state machine value
    i_cstate : in std_logic_vector(1 downto 0)                -- CacheTop state machine value
  );
end; 
 
architecture arch_DbgPort of DbgPort is

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');
  constant one64 : std_logic_vector(63 downto 0) := X"0000000000000001";
  constant STACKTR_ADRSZ : integer := log2(CFG_STACK_TRACE_BUF_SIZE);

  type RegistersType is record
      ready : std_logic;
      halt : std_logic;
      breakpoint : std_logic;
      stepping_mode : std_logic;
      stepping_mode_cnt : std_logic_vector(RISCV_ARCH-1 downto 0);
      trap_on_break : std_logic;
      br_address_fetch : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      br_instr_fetch : std_logic_vector(31 downto 0);
      br_fetch_valid : std_logic;
      flush_address : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
      flush_valid : std_logic;

      rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      stepping_mode_steps : std_logic_vector(RISCV_ARCH-1 downto 0); -- Number of steps before halt in stepping mode
      clock_cnt : std_logic_vector(63 downto 0);               -- Timer in clocks.
      executed_cnt : std_logic_vector(63 downto 0);            -- Number of valid executed instructions
      stack_trace_cnt : integer range 0 to CFG_STACK_TRACE_BUF_SIZE-1; -- Stack trace buffer counter
      rd_trbuf_ena : std_logic;
      rd_trbuf_addr0 : std_logic;
  end record;

  constant R_RESET : RegistersType := (
    '0', '0', '0',                          -- ready, halt, breakpoint
    '0', (others => '0'), '0',              -- stepping_mode. stepping_mode_cnt, trap_on_break
    (others => '0'), (others => '0'), '0',  -- br_address_fetch, br_instr_fetch, br_fetch_valid
    (others => '0'), '0',                   -- flush_address, flush_valid
    (others => '0'), (others => '0'),       -- rdata, stepping_mode_steps
    (others => '0'), (others => '0'),       -- clock_cnt, executed_cnt
    0, '0', '0'                             -- stack_trace_cnt, rd_trbuf_ena, rd_trbuf_addr0
  );

  signal r, rin : RegistersType;
  signal wb_stack_raddr : std_logic_vector(STACKTR_ADRSZ-1 downto 0);
  signal wb_stack_rdata : std_logic_vector(2*BUS_ADDR_WIDTH-1 downto 0);
  signal w_stack_we : std_logic;
  signal wb_stack_waddr : std_logic_vector(STACKTR_ADRSZ-1 downto 0);
  signal wb_stack_wdata : std_logic_vector(2*BUS_ADDR_WIDTH-1 downto 0);

  component StackTraceBuffer is
  generic (
    abits : integer := 5;
    dbits : integer := 64
  );
  port (
    i_clk   : in std_logic;
    i_raddr : in std_logic_vector(abits-1 downto 0);
    o_rdata : out std_logic_vector(dbits-1 downto 0);
    i_we    : in std_logic;
    i_waddr : in std_logic_vector(abits-1 downto 0);
    i_wdata : in std_logic_vector(dbits-1 downto 0)
  );
  end component;

begin


  stacktr_ena : if CFG_STACK_TRACE_BUF_SIZE /= 0 generate 
    stacktr0 : StackTraceBuffer generic map (
      abits => STACKTR_ADRSZ,
      dbits => 2*BUS_ADDR_WIDTH
    ) port map (
      i_clk   => i_clk,
      i_raddr => wb_stack_raddr,
      o_rdata => wb_stack_rdata,
      i_we    => w_stack_we,
      i_waddr => wb_stack_waddr,
      i_wdata => wb_stack_wdata
    );
  end generate;

  comb : process(i_nrst, i_dport_valid, i_dport_write, i_dport_region, 
                 i_dport_addr, i_dport_wdata, i_ireg_rdata, i_freg_rdata,
                 i_csr_rdata, i_pc, i_npc, i_e_valid, i_m_valid, i_ebreak, r,
                 wb_stack_rdata, i_e_call, i_e_ret, i_istate, i_dstate,
                 i_cstate)
    variable v : RegistersType;
    variable wb_o_core_addr : std_logic_vector(11 downto 0);
    variable wb_o_core_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_rdata : std_logic_vector(63 downto 0);
    variable wb_o_rdata : std_logic_vector(63 downto 0);
    variable wb_idx : integer range 0 to 4095;
    variable w_o_csr_ena : std_logic;
    variable w_o_csr_write : std_logic;
    variable w_o_ireg_ena : std_logic;
    variable w_o_ireg_write : std_logic;
    variable w_o_freg_ena : std_logic;
    variable w_o_freg_write : std_logic;
    variable w_o_npc_write : std_logic;
    variable w_cur_halt : std_logic;
  begin

    v := r;

    wb_o_core_addr := (others => '0');
    wb_o_core_wdata := (others => '0');
    wb_rdata := (others => '0');
    wb_o_rdata := (others => '0');
    wb_idx := conv_integer(i_dport_addr);
    w_o_csr_ena := '0';
    w_o_csr_write := '0';
    w_o_ireg_ena := '0';
    w_o_ireg_write := '0';
    w_o_freg_ena := '0';
    w_o_freg_write := '0';
    w_o_npc_write := '0';
    v.br_fetch_valid := '0';
    v.flush_valid := '0';
    v.rd_trbuf_ena := '0';
    wb_stack_raddr <= (others => '0');
    w_stack_we <= '0';
    wb_stack_waddr <= (others => '0');
    wb_stack_wdata <= (others => '0');

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
    if i_e_valid = '1' then
        v.executed_cnt := r.executed_cnt + 1;
    end if;
    if i_ebreak = '1' then
        v.breakpoint := '1';
        if r.trap_on_break = '0' then
            v.halt := '1';
        end if;
    end if;

    if CFG_STACK_TRACE_BUF_SIZE /= 0 then
        if i_e_call = '1' and r.stack_trace_cnt /= (CFG_STACK_TRACE_BUF_SIZE - 1) then
            w_stack_we <= '1';
            wb_stack_waddr <= conv_std_logic_vector(r.stack_trace_cnt, STACKTR_ADRSZ);
            wb_stack_wdata <= i_npc & i_pc;
            v.stack_trace_cnt := r.stack_trace_cnt + 1;
        elsif i_e_ret = '1' and r.stack_trace_cnt /= 0 then
            v.stack_trace_cnt := r.stack_trace_cnt - 1;
        end if;
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
            elsif wb_idx = 34 then
                wb_rdata(STACKTR_ADRSZ-1 downto 0) := 
							conv_std_logic_vector(r.stack_trace_cnt, STACKTR_ADRSZ);
                if i_dport_write = '1' then
                    v.stack_trace_cnt := conv_integer(i_dport_wdata);
                end if;
            elsif (wb_idx >= 64) and (wb_idx < 96) then
                w_o_freg_ena := '1';
                wb_o_core_addr := i_dport_addr;
                wb_rdata := i_freg_rdata;
                if i_dport_write = '1' then
                    w_o_freg_write := '1';
                    wb_o_core_wdata := i_dport_wdata;
                end if;
            elsif (wb_idx >= 128) and (wb_idx < (128 + 2 * CFG_STACK_TRACE_BUF_SIZE)) then
                    v.rd_trbuf_ena := '1';
                    v.rd_trbuf_addr0 := conv_std_logic_vector(wb_idx, 1)(0);
                    wb_stack_raddr <= conv_std_logic_vector((wb_idx - 128) / 2, STACKTR_ADRSZ);
            end if;
        when "10" =>
            case wb_idx is
            when 0 =>
                wb_rdata(0) := r.halt;
                wb_rdata(2) := r.breakpoint;
                wb_rdata(33 downto 32) := i_istate;
                wb_rdata(37 downto 36) := i_dstate;
                wb_rdata(41 downto 40) := i_cstate;
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
                --! Trap on instruction:
                --!      0 = Halt pipeline on ECALL instruction
                --!      1 = Generate trap on ECALL instruction
                wb_rdata(0) := r.trap_on_break;
                if i_dport_write = '1' then
                    v.trap_on_break := i_dport_wdata(0);
                end if;
            when 5 =>
                -- todo: add hardware breakpoint
            when 6 =>
                -- todo: remove hardware breakpoint
            when 7 =>
                wb_rdata(BUS_ADDR_WIDTH-1 downto 0) := r.br_address_fetch;
                if i_dport_write = '1' then
                    v.br_address_fetch := i_dport_wdata(BUS_ADDR_WIDTH-1 downto 0);
                end if;
            when 8 =>
                wb_rdata(31 downto 0) := r.br_instr_fetch;
                if i_dport_write = '1' then
                    v.br_fetch_valid := '1';
                    v.breakpoint := '0';
                    v.br_instr_fetch := i_dport_wdata(31 downto 0);
                end if;
            when 9 =>
                wb_rdata(BUS_ADDR_WIDTH-1 downto 0) := r.flush_address;
                if i_dport_write = '1' then
                    v.flush_valid := '1';
                    v.flush_address := i_dport_wdata(BUS_ADDR_WIDTH-1 downto 0);
                end if;
            when others =>
            end case;
        when others =>
        end case;
    end if;
    v.rdata := wb_rdata;
    if r.rd_trbuf_ena = '1' then
        if r.rd_trbuf_addr0 = '0' then
            wb_o_rdata(BUS_ADDR_WIDTH-1 downto 0) :=
					wb_stack_rdata(BUS_ADDR_WIDTH-1 downto 0);
        else
            wb_o_rdata(BUS_ADDR_WIDTH-1 downto 0) :=
					wb_stack_rdata(2*BUS_ADDR_WIDTH-1 downto BUS_ADDR_WIDTH);
        end if;
    else
        wb_o_rdata := r.rdata;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;

    o_core_addr <= wb_o_core_addr;
    o_core_wdata <= wb_o_core_wdata;
    o_csr_ena <= w_o_csr_ena;
    o_csr_write <= w_o_csr_write;
    o_ireg_ena <= w_o_ireg_ena;
    o_ireg_write <= w_o_ireg_write;
    o_freg_ena <= w_o_freg_ena;
    o_freg_write <= w_o_freg_write;
    o_npc_write <= w_o_npc_write;
    o_clock_cnt <= r.clock_cnt;
    o_executed_cnt <= r.executed_cnt;
    o_halt <= r.halt or w_cur_halt;
    o_break_mode <= r.trap_on_break;
    o_br_fetch_valid <= r.br_fetch_valid;
    o_br_address_fetch <= r.br_address_fetch;
    o_br_instr_fetch <= r.br_instr_fetch;
    o_flush_address <= r.flush_address;
    o_flush_valid <= r.flush_valid;

    o_dport_ready <= r.ready;
    o_dport_rdata <= wb_o_rdata;
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
