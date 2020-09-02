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
use ieee.std_logic_misc.all;  -- or_reduce()
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
    i_dport_req_valid : in std_logic;                         -- Debug access from DSU is valid
    i_dport_write : in std_logic;                             -- Write command flag
    i_dport_addr : in std_logic_vector(CFG_DPORT_ADDR_BITS-1 downto 0); -- Debug Port address
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Write value
    o_dport_req_ready : out std_logic;                        -- Ready to accept dbg request
    i_dport_resp_ready : in std_logic;                        -- Read to accept response
    o_dport_resp_valid : out std_logic;                       -- Response is valid
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Response value
    -- CPU debugging signals:
    o_csr_addr : out std_logic_vector(11 downto 0);           -- Address of the sub-region register
    o_reg_addr : out std_logic_vector(5 downto 0);
    o_core_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Write data
    o_csr_ena : out std_logic;                                -- Region 0: Access to CSR bank is enabled.
    o_csr_write : out std_logic;                              -- Region 0: CSR write enable
    i_csr_valid : in std_logic;
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0); -- Region 0: CSR read value
    o_ireg_ena : out std_logic;                               -- Region 1: Access to integer register bank is enabled
    o_ireg_write : out std_logic;                             -- Region 1: Integer registers bank write pulse
    i_ireg_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Region 1: Integer register read value
    i_pc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- Region 1: Instruction pointer
    i_npc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);-- Region 1: Next Instruction pointer
    i_e_call : in std_logic;                                  -- pseudo-instruction CALL
    i_e_ret : in std_logic                                    -- pseudo-instruction RET
  );
end; 
 
architecture arch_DbgPort of DbgPort is

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');
  constant one64 : std_logic_vector(63 downto 0) := X"0000000000000001";
  
  constant idle : std_logic_vector(2 downto 0) := "000";
  constant csr_region : std_logic_vector(2 downto 0) := "001";
  constant reg_bank : std_logic_vector(2 downto 0) := "010";
  constant reg_stktr_cnt : std_logic_vector(2 downto 0) := "011";
  constant reg_stktr_buf_adr : std_logic_vector(2 downto 0) := "100";
  constant reg_stktr_buf_dat : std_logic_vector(2 downto 0) := "101";
  constant wait_to_accept : std_logic_vector(2 downto 0) := "110";

  type RegistersType is record
      dport_write : std_logic;
      dport_addr : std_logic_vector(CFG_DPORT_ADDR_BITS-1 downto 0);
      dport_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      dport_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      dstate : std_logic_vector(2 downto 0);

      rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
      stack_trace_cnt : std_logic_vector(CFG_LOG2_STACK_TRACE_ADDR-1 downto 0);              -- Stack trace buffer counter
  end record;

  constant R_RESET : RegistersType := (
    '0', -- dport_write
    (others => '0'), -- dport_addr
    (others => '0'), -- dport_wdata 
    (others => '0'), -- dport_rdata
    idle, -- dstate
    (others => '0'), -- rdata
    (others => '0')  -- stack_trace_cnt 
  );

  signal r, rin : RegistersType;
  signal wb_stack_raddr : std_logic_vector(CFG_LOG2_STACK_TRACE_ADDR-1 downto 0);
  signal wb_stack_rdata : std_logic_vector(2*CFG_CPU_ADDR_BITS-1 downto 0);
  signal w_stack_we : std_logic;
  signal wb_stack_waddr : std_logic_vector(CFG_LOG2_STACK_TRACE_ADDR-1 downto 0);
  signal wb_stack_wdata : std_logic_vector(2*CFG_CPU_ADDR_BITS-1 downto 0);

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


  stacktr_ena : if CFG_LOG2_STACK_TRACE_ADDR /= 0 generate 
    stacktr0 : StackTraceBuffer generic map (
      abits => CFG_LOG2_STACK_TRACE_ADDR,
      dbits => 2*CFG_CPU_ADDR_BITS
    ) port map (
      i_clk   => i_clk,
      i_raddr => wb_stack_raddr,
      o_rdata => wb_stack_rdata,
      i_we    => w_stack_we,
      i_waddr => wb_stack_waddr,
      i_wdata => wb_stack_wdata
    );
  end generate;

  comb : process(i_nrst, i_dport_req_valid, i_dport_write, 
                 i_dport_addr, i_dport_wdata, i_dport_resp_ready,
                 i_csr_valid, i_csr_rdata, i_ireg_rdata, i_pc, i_npc,
                 i_e_call, i_e_ret, wb_stack_rdata, r)
    variable v : RegistersType;
    variable wb_o_csr_addr : std_logic_vector(11 downto 0);
    variable wb_o_reg_addr : std_logic_vector(5 downto 0);
    variable wb_o_core_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_idx : integer range 0 to 4095;
    variable w_o_csr_ena : std_logic;
    variable w_o_csr_write : std_logic;
    variable w_o_ireg_ena : std_logic;
    variable w_o_ireg_write : std_logic;
    variable v_req_ready : std_logic;
    variable v_resp_valid : std_logic;
    variable vrdata : std_logic_vector(63 downto 0);
  begin

    v := r;

    wb_o_csr_addr := (others => '0');
    wb_o_reg_addr := (others => '0');
    wb_o_core_wdata := (others => '0');
    wb_idx := conv_integer(i_dport_addr(11 downto 0));
    w_o_csr_ena := '0';
    w_o_csr_write := '0';
    w_o_ireg_ena := '0';
    w_o_ireg_write := '0';
    wb_stack_raddr <= (others => '0');
    w_stack_we <= '0';
    wb_stack_waddr <= (others => '0');
    wb_stack_wdata <= (others => '0');
    v_req_ready := '0';
    v_resp_valid := '0';
    vrdata := r.dport_rdata;


    if CFG_LOG2_STACK_TRACE_ADDR /= 0 then
        if i_e_call = '1' and conv_integer(r.stack_trace_cnt) /= (STACK_TRACE_BUF_SIZE - 1) then
            w_stack_we <= '1';
            wb_stack_waddr <= r.stack_trace_cnt(CFG_LOG2_STACK_TRACE_ADDR-1 downto 0);
            wb_stack_wdata <= i_npc & i_pc;
            v.stack_trace_cnt := r.stack_trace_cnt + 1;
        elsif i_e_ret = '1' and or_reduce(r.stack_trace_cnt) = '1' then
            v.stack_trace_cnt := r.stack_trace_cnt - 1;
        end if;
    end if;

    case r.dstate is
    when idle =>
        v_req_ready := '1';
        vrdata := (others => '0');
        if i_dport_req_valid = '1' then
            v.dport_write := i_dport_write;
            v.dport_addr := i_dport_addr;
            v.dport_wdata := i_dport_wdata;
            if conv_integer(i_dport_addr(CFG_DPORT_ADDR_BITS-1 downto 12)) = 0 then
                v.dstate := csr_region;
            elsif conv_integer(i_dport_addr(CFG_DPORT_ADDR_BITS-1 downto 12)) = 1 then
                if wb_idx < 64 then
                    v.dstate := reg_bank;
                elsif wb_idx = 64 then
                    v.dstate := reg_stktr_cnt;
                elsif (wb_idx >= 128) and (wb_idx < (128 + 2 * STACK_TRACE_BUF_SIZE)) then
                    v.dstate := reg_stktr_buf_adr;
                else
                    vrdata := (others => '0');
                    v.dstate := wait_to_accept;
                end if;
            else
                v.dstate := wait_to_accept;
            end if;
        end if;
    when csr_region =>
        w_o_csr_ena := '1';
        wb_o_csr_addr := r.dport_addr(11 downto 0);
        if r.dport_write = '1' then
             w_o_csr_write   := '1';
             wb_o_core_wdata := r.dport_wdata;
        end if;
        if i_csr_valid = '1' then
            vrdata := i_csr_rdata;
            v.dstate := wait_to_accept;
        end if;
    when reg_bank =>
        w_o_ireg_ena := '1';
        wb_o_reg_addr := r.dport_addr(5 downto 0);
        vrdata := i_ireg_rdata;
        if r.dport_write = '1' then
            w_o_ireg_write  := '1';
            wb_o_core_wdata := r.dport_wdata;
        end if;
        v.dstate := wait_to_accept;
    when reg_stktr_cnt =>
        vrdata := (others => '0');
        vrdata(CFG_LOG2_STACK_TRACE_ADDR-1 downto 0) := r.stack_trace_cnt;
        if r.dport_write = '1' then
            v.stack_trace_cnt := r.dport_wdata(CFG_LOG2_STACK_TRACE_ADDR-1 downto 0);
        end if;
        v.dstate := wait_to_accept;
    when reg_stktr_buf_adr =>
        wb_stack_raddr <= r.dport_addr(CFG_LOG2_STACK_TRACE_ADDR downto 1);
        v.dstate := reg_stktr_buf_dat;
    when reg_stktr_buf_dat =>
        if r.dport_addr(0) = '0' then
            vrdata(CFG_CPU_ADDR_BITS-1 downto 0) :=
                     wb_stack_rdata(CFG_CPU_ADDR_BITS-1 downto 0);
        else
            vrdata(CFG_CPU_ADDR_BITS-1 downto 0) :=
                     wb_stack_rdata(2*CFG_CPU_ADDR_BITS-1 downto CFG_CPU_ADDR_BITS);
        end if;
        v.dstate := wait_to_accept;
    when wait_to_accept =>
        v_resp_valid := '1';
        if i_dport_resp_ready = '1' then
            v.dstate := idle;
        end if;
    when others =>
    end case;
    
    v.dport_rdata := vrdata;

    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    rin <= v;

    o_csr_addr <= wb_o_csr_addr;
    o_reg_addr <= wb_o_reg_addr;
    o_core_wdata <= wb_o_core_wdata;
    o_csr_ena <= w_o_csr_ena;
    o_csr_write <= w_o_csr_write;
    o_ireg_ena <= w_o_ireg_ena;
    o_ireg_write <= w_o_ireg_write;

    o_dport_req_ready <= v_req_ready;
    o_dport_resp_valid <= v_resp_valid;
    o_dport_rdata <= r.dport_rdata;
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
