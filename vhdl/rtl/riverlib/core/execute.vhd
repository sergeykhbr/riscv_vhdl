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
use IEEE.std_logic_arith.all;  -- UNSIGNED function
use ieee.std_logic_misc.all;  -- or_reduce()
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity InstrExecute is generic (
    async_reset : boolean;
    fpu_ena : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;                                      -- Reset active LOW
    i_d_valid : in std_logic;                                   -- Decoded instruction is valid
    i_d_radr1 : in std_logic_vector(5 downto 0);
    i_d_radr2 : in std_logic_vector(5 downto 0);
    i_d_waddr : in std_logic_vector(5 downto 0);
    i_d_imm : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_d_pc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- Instruction pointer on decoded instruction
    i_d_instr : in std_logic_vector(31 downto 0);               -- Decoded instruction value
    i_d_progbuf_ena : in std_logic;                             -- instruction from progbuf passed decoder
    i_dbg_progbuf_ena : in std_logic;                           -- progbuf mode enabled
    i_wb_waddr : in std_logic_vector(5 downto 0);               -- Write back address
    i_memop_store : in std_logic;                               -- Store to memory operation
    i_memop_load : in std_logic;                                -- Load from memoru operation
    i_memop_sign_ext : in std_logic;                            -- Load memory value with sign extending
    i_memop_size : in std_logic_vector(1 downto 0);             -- Memory transaction size
    i_unsigned_op : in std_logic;                               -- Unsigned operands
    i_rv32 : in std_logic;                                      -- 32-bits instruction
    i_compressed : in std_logic;                                -- C-extension (2-bytes length)
    i_f64 : in std_logic;                                       -- D-extension (FPU)
    i_isa_type : in std_logic_vector(ISA_Total-1 downto 0);     -- Type of the instruction's structure (ISA spec.)
    i_ivec : in std_logic_vector(Instr_Total-1 downto 0);       -- One pulse per supported instruction.
    i_unsup_exception : in std_logic;                           -- Unsupported instruction exception
    i_instr_load_fault : in std_logic;                          -- Instruction fetched from fault address
    i_instr_executable : in std_logic;                          -- MPU flag
    i_dport_npc_write : in std_logic;                           -- Write npc value from debug port
    i_dport_npc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);-- Debug port npc value to write

    i_rdata1 : in std_logic_vector(RISCV_ARCH-1 downto 0);      -- Integer/FPU registers value 1
    i_rhazard1 : in std_logic;
    i_rdata2 : in std_logic_vector(RISCV_ARCH-1 downto 0);      -- Integer/FPU registers value 2
    i_rhazard2 : in std_logic;
    i_wtag : in std_logic_vector(3 downto 0);
    o_wena : out std_logic;
    o_waddr : out std_logic_vector(5 downto 0);                 -- Address to store result of the instruction (0=do not store)
    o_whazard : out std_logic;
    o_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);      -- Value to store
    o_wtag : out std_logic_vector(3 downto 0);
    o_d_ready : out std_logic;                                  -- Hold pipeline while 'writeback' not done or multi-clock instruction.
    o_csr_wena : out std_logic;                                 -- Write new CSR value
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- CSR current value
    o_csr_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);  -- CSR new value
    i_mepc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- next instruction in a case of MRET
    i_uepc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- 
    i_trap_valid : in std_logic;
    i_trap_pc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    -- exceptions:
    o_ex_npc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_ex_instr_load_fault : out std_logic;                      -- Instruction fetched from fault address
    o_ex_instr_not_executable : out std_logic;                  -- MPU prohibit this instruction
    o_ex_illegal_instr : out std_logic;
    o_ex_unalign_store : out std_logic;
    o_ex_unalign_load : out std_logic;
    o_ex_breakpoint : out std_logic;
    o_ex_ecall : out std_logic;
    o_ex_fpu_invalidop : out std_logic;            -- FPU Exception: invalid operation
    o_ex_fpu_divbyzero : out std_logic;            -- FPU Exception: divide by zero
    o_ex_fpu_overflow : out std_logic;             -- FPU Exception: overflow
    o_ex_fpu_underflow : out std_logic;            -- FPU Exception: underflow
    o_ex_fpu_inexact : out std_logic;              -- FPU Exception: inexact
    o_fpu_valid : out std_logic;                   -- FPU output is valid

    o_memop_sign_ext : out std_logic;                           -- Load data with sign extending
    o_memop_load : out std_logic;                               -- Load data instruction
    o_memop_store : out std_logic;                              -- Store data instruction
    o_memop_size : out std_logic_vector(1 downto 0);            -- 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    o_memop_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);-- Memory access address
    o_memop_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_memop_waddr : out std_logic_vector(5 downto 0);
    o_memop_wtag : out std_logic_vector(3 downto 0);
    i_memop_ready : in std_logic;

    o_trap_ready : out std_logic;                               -- Trap branch request was accepted
    o_valid : out std_logic;                                    -- Output is valid
    o_pc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);  -- Valid instruction pointer
    o_npc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    o_instr : out std_logic_vector(31 downto 0);                -- Valid instruction value
    i_flushd_end : in std_logic;
    o_flushd : out std_logic;
    o_flushi : out std_logic;
    o_call : out std_logic;                                     -- CALL pseudo instruction detected
    o_ret : out std_logic;                                      -- RET pseudoinstruction detected
    o_mret : out std_logic;                                     -- MRET instruction
    o_uret : out std_logic;                                     -- URET instruction
    o_multi_ready : out std_logic
  );
end; 
 
architecture arch_InstrExecute of InstrExecute is

  constant Multi_MUL : integer := 0;
  constant Multi_DIV : integer := 1;
  constant Multi_FPU : integer := 2;
  constant Multi_Total : integer := 3;

  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type multi_arith_type is array (0 to Multi_Total-1) 
      of std_logic_vector(RISCV_ARCH-1 downto 0);

  type RegistersType is record
        pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
        npc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
        instr : std_logic_vector(31 downto 0);
        memop_waddr : std_logic_vector(5 downto 0);
        memop_wtag : std_logic_vector(3 downto 0);
        wval : std_logic_vector(RISCV_ARCH-1 downto 0);
        memop_load : std_logic;
        memop_store : std_logic;
        memop_sign_ext : std_logic;
        memop_size : std_logic_vector(1 downto 0);
        memop_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
        memop_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);

        valid : std_logic;
        call : std_logic;
        ret : std_logic;
        flushd : std_logic;
        hold_fencei : std_logic;
        progbuf_npc : std_logic_vector(31 downto 0);
  end record;

  constant R_RESET : RegistersType := (
    (others => '0'), CFG_NMI_RESET_VECTOR,             -- pc, npc
    (others => '0'), (others => '0'),                  -- instr, memop_waddr
    (others => '0'), (others => '0'),                  -- memop_wtag, wval
    '0', '0', '0', "00", (others => '0'),              -- memop_load, memop_store, memop_sign_ext, memop_size, memop_addr
    (others => '0'),                                   -- memop_wdata
    '0',                                               -- valid
    '0', '0',                                          -- call, ret
    '0', '0',                                          -- flushd, hold_fencei
    (others => '0')                                    -- progbuf_ena
  );

  signal r, rin : RegistersType;

  signal wb_arith_res : multi_arith_type;
  signal w_arith_ena : std_logic_vector(Multi_Total-1 downto 0);
  signal w_arith_valid : std_logic_vector(Multi_Total-1 downto 0);
  signal w_arith_busy : std_logic_vector(Multi_Total-1 downto 0);
  signal w_arith_residual_high: std_logic;
  signal w_mul_hsu: std_logic;
  signal w_multi_ena : std_logic;

  signal wb_rdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_rdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);

  signal wb_shifter_a1 : std_logic_vector(RISCV_ARCH-1 downto 0);  -- Shifters operand 1
  signal wb_shifter_a2 : std_logic_vector(5 downto 0);             -- Shifters operand 2
  signal wb_sll : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_sllw : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_srl : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_srlw : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_sra : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_sraw : std_logic_vector(RISCV_ARCH-1 downto 0);

  component IntMul is generic (
    async_reset : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_ena : in std_logic;
    i_unsigned : in std_logic;
    i_hsu : in std_logic;
    i_high : in std_logic;
    i_rv32 : in std_logic;
    i_a1 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_a2 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_res : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_valid : out std_logic;
    o_busy : out std_logic
  );
  end component; 

  component IntDiv is generic (
    async_reset : boolean
  );
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_ena : in std_logic;
    i_unsigned : in std_logic;
    i_rv32 : in std_logic;
    i_residual : in std_logic;
    i_a1 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_a2 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_res : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_valid : out std_logic;
    o_busy : out std_logic
  );
  end component;
  
  component Shifter is port (
    i_a1 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_a2 : in std_logic_vector(5 downto 0);
    o_sll : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_sllw : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_srl : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_sra : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_srlw : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_sraw : out std_logic_vector(RISCV_ARCH-1 downto 0)
  );
  end component;

  component FpuTop is 
  generic (
    async_reset : boolean
  );
  port (
    i_nrst         : in std_logic;
    i_clk          : in std_logic;
    i_ena          : in std_logic;
    i_ivec         : in std_logic_vector(Instr_FPU_Total-1 downto 0);
    i_a            : in std_logic_vector(63 downto 0);
    i_b            : in std_logic_vector(63 downto 0);
    o_res          : out std_logic_vector(63 downto 0);
    o_ex_invalidop : out std_logic;   -- Exception: invalid operation
    o_ex_divbyzero : out std_logic;   -- Exception: divide by zero
    o_ex_overflow  : out std_logic;   -- Exception: overflow
    o_ex_underflow : out std_logic;   -- Exception: underflow
    o_ex_inexact   : out std_logic;   -- Exception: inexact
    o_valid        : out std_logic;
    o_busy         : out std_logic
  );
  end component; 

begin

   mul0 : IntMul generic map (
      async_reset => async_reset
   ) port map (
      i_clk  => i_clk,
      i_nrst => i_nrst,
      i_ena => w_arith_ena(Multi_MUL),
      i_unsigned => i_unsigned_op,
      i_hsu => w_mul_hsu,
      i_high => w_arith_residual_high,
      i_rv32 => i_rv32,
      i_a1 => wb_rdata1,
      i_a2 => wb_rdata2,
      o_res => wb_arith_res(Multi_MUL),
      o_valid => w_arith_valid(Multi_MUL),
      o_busy => w_arith_busy(Multi_MUL));

   div0 : IntDiv generic map (
      async_reset => async_reset
   ) port map (
      i_clk  => i_clk,
      i_nrst => i_nrst,
      i_ena => w_arith_ena(Multi_DIV),
      i_unsigned => i_unsigned_op,
      i_residual => w_arith_residual_high,
      i_rv32 => i_rv32,
      i_a1 => wb_rdata1,
      i_a2 => wb_rdata2,
      o_res => wb_arith_res(Multi_DIV),
      o_valid => w_arith_valid(Multi_DIV),
      o_busy => w_arith_busy(Multi_DIV));
      
  sh0 : Shifter port map (
      i_a1 => wb_shifter_a1,
      i_a2 => wb_shifter_a2,
      o_sll => wb_sll,
      o_sllw => wb_sllw,
      o_srl => wb_srl,
      o_sra => wb_sra,
      o_srlw => wb_srlw,
      o_sraw => wb_sraw);

  fpuena : if fpu_ena generate
     fpu0 : FpuTop generic map (
        async_reset => async_reset
     ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_ena => w_arith_ena(Multi_FPU),
        i_ivec => i_ivec(Instr_FSUB_D downto Instr_FADD_D),
        i_a => wb_rdata1,
        i_b => wb_rdata2,
        o_res => wb_arith_res(Multi_FPU),
        o_ex_invalidop => o_ex_fpu_invalidop,
        o_ex_divbyzero => o_ex_fpu_divbyzero,
        o_ex_overflow => o_ex_fpu_overflow,
        o_ex_underflow => o_ex_fpu_underflow,
        o_ex_inexact => o_ex_fpu_inexact,
        o_valid => w_arith_valid(Multi_FPU),
        o_busy => w_arith_busy(Multi_FPU)
     );
  end generate;

  fpudis : if not fpu_ena generate
        wb_arith_res(Multi_FPU) <= (others => '0');
        w_arith_valid(Multi_FPU) <= '0';
        w_arith_busy(Multi_FPU) <= '0';
        o_fpu_valid <= '0';
        o_ex_fpu_invalidop <= '0';
        o_ex_fpu_divbyzero <= '0';
        o_ex_fpu_overflow <= '0';
        o_ex_fpu_underflow <= '0';
        o_ex_fpu_inexact <= '0';
  end generate;

  comb : process(i_nrst, i_d_valid, i_d_radr1, i_d_radr2, i_d_waddr, i_d_imm,
                 i_d_pc, i_d_instr, i_d_progbuf_ena, i_dbg_progbuf_ena, i_wb_waddr,
                 i_memop_load, i_memop_store, i_memop_sign_ext,
                 i_memop_size, i_unsigned_op, i_rv32, i_compressed, i_f64, i_isa_type, i_ivec,
                 i_unsup_exception, i_instr_load_fault, i_instr_executable,
                 i_dport_npc_write, i_dport_npc, 
                 i_rdata1, i_rhazard1, i_rdata2, i_rhazard2, i_wtag, i_csr_rdata, 
                 i_mepc, i_uepc,
                 i_trap_valid, i_trap_pc, i_memop_ready, i_flushd_end,
                 wb_arith_res, w_arith_valid, w_arith_busy,
                 wb_sll, wb_sllw, wb_srl, wb_srlw, wb_sra, wb_sraw, r)
    variable v : RegistersType;

    variable w_exception_store : std_logic;
    variable w_exception_load : std_logic;
    variable v_multi_ena : std_logic;
    variable w_multi_ready : std_logic;
    variable w_multi_busy : std_logic;
    variable w_next_ready : std_logic;
    variable w_hold_multi : std_logic;
    variable w_hold_hazard : std_logic;
    variable w_hold_memop : std_logic;
    variable v_fence : std_logic;
    variable v_fencei : std_logic;
    variable v_mret : std_logic;
    variable v_uret : std_logic;
    variable v_csr_wena : std_logic;
    variable vb_csr_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_res : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_prog_npc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_npc_incr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_npc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable vb_off : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_sum64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_sum32 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_sub64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_sub32 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_and64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_or64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_xor64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_memop_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable wv : std_logic_vector(Instr_Total-1 downto 0);
    variable opcode_len : integer;
    variable v_call : std_logic;
    variable v_ret : std_logic;
    variable v_pc_branch : std_logic;
    variable v_less : std_logic;
    variable v_gr_equal : std_logic;
    variable vb_i_rdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_i_rdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_rdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_rdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_rfdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable vb_rfdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable v_o_valid : std_logic;
    variable vb_o_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable v_hold_exec : std_logic;
    variable int_radr1 : integer;
    variable int_radr2 : integer;
    variable int_waddr : integer;
    variable v_next_mul_ready : std_logic;
    variable v_next_div_ready : std_logic;
    variable v_next_fpu_ready : std_logic;
    variable v_wena : std_logic;
    variable v_whazard : std_logic;
    variable vb_waddr : std_logic_vector(5 downto 0);
    variable v_next_normal : std_logic;
    variable v_next_progbuf : std_logic;

  begin

    v := r;

    v_csr_wena := '0';
    vb_csr_wdata := (others => '0');
    vb_res := (others => '0');
    vb_off := (others => '0');
    vb_memop_addr := (others => '0');
    wv := i_ivec;
    v_call := '0';
    v_ret := '0';
    v.valid := '0';
    v.call := '0';
    v.ret := '0';
    vb_rdata1 := (others => '0');
    vb_rdata2 := (others => '0');
    int_radr1 := conv_integer(i_d_radr1);
    int_radr2 := conv_integer(i_d_radr2);
    int_waddr := conv_integer(i_d_waddr);

    vb_i_rdata1 := i_rdata1;
    vb_i_rdata2 := i_rdata2;

    if i_isa_type(ISA_R_type) = '1' then
        vb_rdata1 := vb_i_rdata1;
        vb_rdata2 := vb_i_rdata2;
    elsif i_isa_type(ISA_I_type) = '1' then
        vb_rdata1 := vb_i_rdata1;
        vb_rdata2 := i_d_imm;
    elsif i_isa_type(ISA_SB_type) = '1' then
        vb_rdata1 := vb_i_rdata1;
        vb_rdata2 := vb_i_rdata2;
        vb_off := i_d_imm;
    elsif i_isa_type(ISA_UJ_type) = '1' then
        vb_rdata1(CFG_CPU_ADDR_BITS-1 downto 0) := i_d_pc;
        vb_off := i_d_imm;
    elsif i_isa_type(ISA_U_type) = '1' then
        vb_rdata1(CFG_CPU_ADDR_BITS-1 downto 0) := i_d_pc;
        vb_rdata2 := i_d_imm;
    elsif i_isa_type(ISA_S_type) = '1' then
        vb_rdata1 := vb_i_rdata1;
        vb_rdata2 := vb_i_rdata2;
        vb_off := i_d_imm;
    end if;

    w_multi_busy := w_arith_busy(Multi_MUL) or w_arith_busy(Multi_DIV)
                  or w_arith_busy(Multi_FPU);

    w_multi_ready := w_arith_valid(Multi_MUL) or w_arith_valid(Multi_DIV)
                  or w_arith_valid(Multi_FPU);


    -- Hold signals:
    --      1. hazard
    --      2. memaccess not ready to accept next memop operation (or flush request)
    --      3. multi instruction
    --      4. Flushing $D on fence.i instruction
    --
    w_hold_hazard := i_rhazard1 or i_rhazard2;

    w_hold_memop := (i_memop_load or i_memop_store or
                    wv(Instr_FENCE) or wv(Instr_FENCE_I))
                    and not i_memop_ready;

    w_hold_multi := w_multi_busy or w_multi_ready;

    v_hold_exec := w_hold_hazard or w_hold_memop or w_hold_multi
                   or r.hold_fencei;

    v_next_normal := '0';
    if i_d_pc = r.npc and i_dbg_progbuf_ena = '0'
        and i_d_progbuf_ena = '0' then
        v_next_normal := '1';
    end if;

    v_next_progbuf := '0';
    if i_d_pc = r.progbuf_npc and i_dbg_progbuf_ena = '1'
        and i_d_progbuf_ena = '1' then
        v_next_progbuf := '1';
    end if;

    w_next_ready := '0';
    if i_d_valid = '1' and (v_next_normal or v_next_progbuf) = '1'
       and v_hold_exec = '0' then
        w_next_ready := '1';
    end if;

    v_fence := wv(Instr_FENCE) and w_next_ready;
    v_fencei := wv(Instr_FENCE_I) and w_next_ready;
    v_mret := wv(Instr_MRET) and w_next_ready;
    v_uret := wv(Instr_URET) and w_next_ready;

    v_next_mul_ready := (wv(Instr_MUL) or wv(Instr_MULW)
                            or wv(Instr_MULH) or wv(Instr_MULHSU)
                            or wv(Instr_MULHU)) and w_next_ready;
    v_next_div_ready := (wv(Instr_DIV) or wv(Instr_DIVU)
                            or wv(Instr_DIVW) or wv(Instr_DIVUW)
                            or wv(Instr_REM) or wv(Instr_REMU)
                            or wv(Instr_REMW) or wv(Instr_REMUW)) and w_next_ready;
    v_next_fpu_ready := '0';
    if fpu_ena then
        if i_f64 = '1' and (wv(Instr_FSD) or wv(Instr_FLD)) = '0' then
            v_next_fpu_ready := w_next_ready;
        end if;
    end if;

    w_arith_residual_high <= (wv(Instr_REM) or wv(Instr_REMU)
                          or wv(Instr_REMW) or wv(Instr_REMUW)
                          or wv(Instr_MULH) or wv(Instr_MULHSU) or wv(Instr_MULHU));

    w_mul_hsu <= wv(Instr_MULHSU);

    v_multi_ena := v_next_mul_ready or v_next_div_ready or v_next_fpu_ready;

    w_arith_ena(Multi_MUL) <= v_next_mul_ready;
    w_arith_ena(Multi_DIV) <= v_next_div_ready;
    w_arith_ena(Multi_FPU) <= v_next_fpu_ready;

    if i_memop_load = '1' then
        vb_memop_addr :=
            vb_rdata1(CFG_CPU_ADDR_BITS-1 downto 0) + vb_rdata2(CFG_CPU_ADDR_BITS-1 downto 0);
    elsif i_memop_store = '1' then
        vb_memop_addr := 
            vb_rdata1(CFG_CPU_ADDR_BITS-1 downto 0) + vb_off(CFG_CPU_ADDR_BITS-1 downto 0);
    elsif (wv(Instr_FENCE) or wv(Instr_FENCE_I)) = '1' then
        vb_memop_addr := (others => '1');
    elsif wv(Instr_EBREAK) = '1' then
        vb_memop_addr := i_d_pc;
    end if;

    w_exception_store := '0';
    w_exception_load := '0';

    if (wv(Instr_LD) = '1' and vb_memop_addr(2 downto 0) /= "000")
        or ((wv(Instr_LW) or wv(Instr_LWU)) = '1' and vb_memop_addr(1 downto 0) /= "00")
        or ((wv(Instr_LH) or wv(Instr_LHU)) = '1' and vb_memop_addr(0) = '1') then
        w_exception_load := '1';
    end if;
    if (wv(Instr_SD) = '1' and vb_memop_addr(2 downto 0) /= "000")
        or (wv(Instr_SW) = '1' and vb_memop_addr(1 downto 0) /= "00")
        or (wv(Instr_SH) = '1' and vb_memop_addr(0) = '1') then
        w_exception_store := '1';
    end if;


    -- parallel ALU:
    vb_sum64 := vb_rdata1 + vb_rdata2;
    vb_sum32(31 downto 0) := vb_rdata1(31 downto 0) + vb_rdata2(31 downto 0);
    vb_sum32(63 downto 32) := (others => vb_sum32(31));
    vb_sub64 := vb_rdata1 - vb_rdata2;
    vb_sub32(31 downto 0) := vb_rdata1(31 downto 0) - vb_rdata2(31 downto 0);
    vb_sub32(63 downto 32) := (others => vb_sub32(31));
    vb_and64 := vb_rdata1 and vb_rdata2;
    vb_or64 := vb_rdata1 or vb_rdata2;
    vb_xor64 := vb_rdata1 xor vb_rdata2;
    
    wb_shifter_a1 <= vb_rdata1;
    wb_shifter_a2 <= vb_rdata2(5 downto 0);

    v_less := '0';
    v_gr_equal := '0';
    if UNSIGNED(vb_rdata1) < UNSIGNED(vb_rdata2) then
        v_less := '1';
    end if;
    if UNSIGNED(vb_rdata1) >= UNSIGNED(vb_rdata2) then
        v_gr_equal := '1';
    end if;

    -- Relative Branch on some condition:
    v_pc_branch := '0';
    if ((wv(Instr_BEQ) = '1' and (vb_sub64 = zero64))
        or (wv(Instr_BGE) = '1' and (vb_sub64(63) = '0'))
        or (wv(Instr_BGEU) = '1' and (v_gr_equal = '1'))
        or (wv(Instr_BLT) = '1' and (vb_sub64(63) = '1'))
        or (wv(Instr_BLTU) = '1' and (v_less = '1'))
        or (wv(Instr_BNE) = '1' and (vb_sub64 /= zero64))) then
        v_pc_branch := '1';
    end if;

    opcode_len := 4;
    if i_compressed = '1' then
        opcode_len := 2;
    end if;
    vb_npc_incr := i_d_pc + opcode_len;


    if v_pc_branch = '1' then
        vb_prog_npc := i_d_pc + vb_off(CFG_CPU_ADDR_BITS-1 downto 0);
    elsif wv(Instr_JAL) = '1' then
        vb_prog_npc := vb_rdata1(CFG_CPU_ADDR_BITS-1 downto 0) + vb_off(CFG_CPU_ADDR_BITS-1 downto 0);
    elsif wv(Instr_JALR) = '1' then
        vb_prog_npc := vb_rdata1(CFG_CPU_ADDR_BITS-1 downto 0) + vb_rdata2(CFG_CPU_ADDR_BITS-1 downto 0);
        vb_prog_npc(0) := '0';
    elsif wv(Instr_MRET) = '1' then
        vb_prog_npc := i_mepc;
    elsif wv(Instr_URET) = '1' then
        vb_prog_npc := i_uepc;
    else
        vb_prog_npc := vb_npc_incr;
    end if;

    if i_trap_valid = '1' then
        vb_npc := i_trap_pc;
    else
        vb_npc := vb_prog_npc;
    end if;

    -- ALU block selector:
    if w_arith_valid(Multi_MUL) = '1' then
        vb_res := wb_arith_res(Multi_MUL);
    elsif w_arith_valid(Multi_DIV) = '1' then
        vb_res := wb_arith_res(Multi_DIV);
    elsif w_arith_valid(Multi_FPU) = '1' then
        vb_res := wb_arith_res(Multi_FPU);
    elsif i_memop_load = '1' then
        vb_res := (others => '0');
    elsif i_memop_store = '1' then
        vb_res := vb_rdata2;
    elsif wv(Instr_JAL) = '1' then
        vb_res(CFG_CPU_ADDR_BITS-1 downto 0) := vb_npc_incr;
        if int_waddr = Reg_ra then
            v_call := '1';
        end if;
    elsif wv(Instr_JALR) = '1' then
        vb_res(CFG_CPU_ADDR_BITS-1 downto 0) := vb_npc_incr;
        if int_waddr = Reg_ra then
            v_call := '1';
        elsif or_reduce(vb_rdata2) = '0' and int_radr1 = Reg_ra then
            v_ret := '1';
        end if;
    elsif (wv(Instr_ADD) or wv(Instr_ADDI) or wv(Instr_AUIPC)) = '1' then
        vb_res := vb_sum64;
    elsif (wv(Instr_ADDW) or wv(Instr_ADDIW)) = '1' then
        vb_res := vb_sum32;
    elsif wv(Instr_SUB) = '1' then
        vb_res := vb_sub64;
    elsif wv(Instr_SUBW) = '1' then
        vb_res := vb_sub32;
    elsif (wv(Instr_SLL) or wv(Instr_SLLI)) = '1' then
        vb_res := wb_sll;
    elsif (wv(Instr_SLLW) or wv(Instr_SLLIW)) = '1' then
        vb_res := wb_sllw;
    elsif (wv(Instr_SRL) or wv(Instr_SRLI)) = '1' then
        vb_res := wb_srl;
    elsif (wv(Instr_SRLW) or wv(Instr_SRLIW)) = '1' then
        vb_res := wb_srlw;
    elsif (wv(Instr_SRA) or wv(Instr_SRAI)) = '1' then
        vb_res := wb_sra;
    elsif (wv(Instr_SRAW) or wv(Instr_SRAW) or wv(Instr_SRAIW)) = '1' then
        vb_res := wb_sraw;
    elsif (wv(Instr_AND) or wv(Instr_ANDI)) = '1' then
        vb_res := vb_and64;
    elsif (wv(Instr_OR) or wv(Instr_ORI)) = '1' then
        vb_res := vb_or64;
    elsif (wv(Instr_XOR) or wv(Instr_XORI)) = '1' then
        vb_res := vb_xor64;
    elsif (wv(Instr_SLT) or wv(Instr_SLTI)) = '1' then
        vb_res(RISCV_ARCH-1 downto 1) := (others => '0');
        vb_res(0) := vb_sub64(63);
    elsif (wv(Instr_SLTU) or wv(Instr_SLTIU)) = '1' then
        vb_res(63 downto 1) := (others => '0');
        vb_res(0) := v_less;
    elsif wv(Instr_LUI) = '1' then
        vb_res := vb_rdata2;
    elsif wv(Instr_CSRRC) = '1' then
        vb_res := i_csr_rdata;
        v_csr_wena := '1';
        vb_csr_wdata := i_csr_rdata and (not vb_rdata1);
    elsif wv(Instr_CSRRCI) = '1' then
        vb_res := i_csr_rdata;
        v_csr_wena := '1';
        vb_csr_wdata(RISCV_ARCH-1 downto 5) := i_csr_rdata(RISCV_ARCH-1 downto 5);
        vb_csr_wdata(4 downto 0) := i_csr_rdata(4 downto 0) and not i_d_radr1(4 downto 0);  -- zero-extending 5 to 64-bits
    elsif wv(Instr_CSRRS) = '1' then
        vb_res := i_csr_rdata;
        v_csr_wena := '1';
        vb_csr_wdata := i_csr_rdata or vb_rdata1;
    elsif wv(Instr_CSRRSI) = '1' then
        vb_res := i_csr_rdata;
        v_csr_wena := '1';
        vb_csr_wdata(RISCV_ARCH-1 downto 5) := i_csr_rdata(RISCV_ARCH-1 downto 5);
        vb_csr_wdata(4 downto 0) := i_csr_rdata(4 downto 0) or i_d_radr1(4 downto 0);  -- zero-extending 5 to 64-bits
    elsif wv(Instr_CSRRW) = '1' then
        vb_res := i_csr_rdata;
        v_csr_wena := '1';
        vb_csr_wdata := vb_rdata1;
    elsif wv(Instr_CSRRWI) = '1' then
        vb_res := i_csr_rdata;
        v_csr_wena := '1';
        vb_csr_wdata(RISCV_ARCH-1 downto 5) := (others => '0');
        vb_csr_wdata(4 downto 0) := i_d_radr1(4 downto 0);  -- zero-extending 5 to 64-bits
    end if;


    -- Latch ready result
    v_wena := '0';
    v_whazard := '0';
    vb_waddr := i_d_waddr;
    vb_o_wdata := vb_res;
    if w_next_ready = '1' then
        v.valid := '1';

        if i_dbg_progbuf_ena = '0' then
            v.pc := i_d_pc;
            v.npc := vb_npc;
        else
            v.progbuf_npc := vb_npc_incr;
        end if;
        v.instr := i_d_instr;
        v.memop_load := i_memop_load;
        v.memop_sign_ext := i_memop_sign_ext;
        v.memop_store := i_memop_store;
        v.memop_size := i_memop_size;
        v.memop_addr := vb_memop_addr;
        v.memop_wdata := vb_res;

        v.memop_waddr := i_d_waddr;
        v.memop_wtag := i_wtag + 1;
        v_whazard := i_memop_load;
        v_wena := or_reduce(i_d_waddr) and not v_multi_ena;

        v.wval := vb_res;
        v.call := v_call;
        v.ret := v_ret;
        v.flushd := v_fencei or v_fence;
        v.hold_fencei := v_fencei;
    end if;

    if i_dbg_progbuf_ena = '0' then
        v.progbuf_npc := (others => '0');
    end if;

    if i_flushd_end = '1' then
        v.hold_fencei := '0';
    end if;

    if w_multi_ready = '1' then
        v_wena := or_reduce(r.memop_waddr);
        vb_waddr := r.memop_waddr;
    end if;

    v_o_valid := (r.valid and not w_multi_busy) or w_multi_ready;

    if i_dport_npc_write = '1' then
        v.npc := i_dport_npc;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    wb_rdata1 <= vb_rdata1;
    wb_rdata2 <= vb_rdata2;

    w_multi_ena <= v_multi_ena;

    o_trap_ready <= w_next_ready;

    o_ex_instr_load_fault <= i_instr_load_fault and w_next_ready;
    o_ex_instr_not_executable <= not i_instr_executable and w_next_ready;
    o_ex_illegal_instr <= i_unsup_exception and w_next_ready;
    o_ex_unalign_store <= w_exception_store and w_next_ready;
    o_ex_unalign_load <= w_exception_load and w_next_ready;
    o_ex_breakpoint <= wv(Instr_EBREAK) and w_next_ready;
    o_ex_ecall <= wv(Instr_ECALL) and w_next_ready;

    o_wena <= v_wena;
    o_whazard <= v_whazard;
    o_waddr <= vb_waddr;
    o_wdata <= vb_o_wdata;
    o_wtag <= i_wtag;
    o_d_ready <= not v_hold_exec;

    o_csr_wena <= v_csr_wena and w_next_ready;
    o_csr_wdata <= vb_csr_wdata;
    o_ex_npc <= vb_prog_npc;

    o_memop_sign_ext <= r.memop_sign_ext;
    o_memop_load <= r.memop_load;
    o_memop_store <= r.memop_store;
    o_memop_size <= r.memop_size;
    o_memop_addr <= r.memop_addr;
    o_memop_wdata <= r.memop_wdata;
    o_memop_waddr <= r.memop_waddr;
    o_memop_wtag <= r.memop_wtag;

    o_valid <= v_o_valid;
    o_pc <= r.pc;
    o_npc <= r.npc;
    o_instr <= r.instr;
    o_flushd <= r.flushd;   -- must be post in memory queue to avoid too early flushing
    o_flushi <= v_fencei or v_fence;
    o_call <= r.call;
    o_ret <= r.ret;
    o_mret <= v_mret;
    o_uret <= v_uret;
    o_fpu_valid <= w_arith_valid(Multi_FPU);
    -- Tracer only:
    o_multi_ready <= w_multi_ready;
    
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
