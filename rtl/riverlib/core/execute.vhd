-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     CPU Instruction Execution stage.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use IEEE.std_logic_arith.all;  -- UNSIGNED function
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity InstrExecute is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;                                      -- Reset active LOW
    i_pipeline_hold : in std_logic;                             -- Hold execution by any reason
    i_d_valid : in std_logic;                                   -- Decoded instruction is valid
    i_d_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Instruction pointer on decoded instruction
    i_d_instr : in std_logic_vector(31 downto 0);               -- Decoded instruction value
    i_wb_done : in std_logic;                                   -- write back done (Used to clear hazardness)
    i_memop_store : in std_logic;                               -- Store to memory operation
    i_memop_load : in std_logic;                                -- Load from memoru operation
    i_memop_sign_ext : in std_logic;                            -- Load memory value with sign extending
    i_memop_size : in std_logic_vector(1 downto 0);             -- Memory transaction size
    i_unsigned_op : in std_logic;                               -- Unsigned operands
    i_rv32 : in std_logic;                                      -- 32-bits instruction
    i_compressed : in std_logic;                                -- C-extension (2-bytes length)
    i_isa_type : in std_logic_vector(ISA_Total-1 downto 0);     -- Type of the instruction's structure (ISA spec.)
    i_ivec : in std_logic_vector(Instr_Total-1 downto 0);       -- One pulse per supported instruction.
    i_ie : in std_logic;                                        -- Interrupt enable bit
    i_mtvec : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);   -- Interrupt descriptor table
    i_mode : in std_logic_vector(1 downto 0);                   -- Current processor mode
    i_break_mode : in std_logic;                                -- Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
    i_unsup_exception : in std_logic;                           -- Unsupported instruction exception
    i_ext_irq : in std_logic;                                   -- External interrupt from PLIC (todo: timer & software interrupts)
    i_dport_npc_write : in std_logic;                           -- Write npc value from debug port
    i_dport_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Debug port npc value to write

    o_radr1 : out std_logic_vector(4 downto 0);                 -- Integer register index 1
    i_rdata1 : in std_logic_vector(RISCV_ARCH-1 downto 0);      -- Integer register value 1
    o_radr2 : out std_logic_vector(4 downto 0);                 -- Integer register index 2
    i_rdata2 : in std_logic_vector(RISCV_ARCH-1 downto 0);      -- Integer register value 2
    o_res_addr : out std_logic_vector(4 downto 0);              -- Address to store result of the instruction (0=do not store)
    o_res_data : out std_logic_vector(RISCV_ARCH-1 downto 0);   -- Value to store
    o_pipeline_hold : out std_logic;                            -- Hold pipeline while 'writeback' not done or multi-clock instruction.
    o_xret : out std_logic;                                     -- XRET instruction: MRET, URET or other.
    o_csr_addr : out std_logic_vector(11 downto 0);             -- CSR address. 0 if not a CSR instruction with xret signals mode switching
    o_csr_wena : out std_logic;                                 -- Write new CSR value
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- CSR current value
    o_csr_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);  -- CSR new value
    i_trap_valid : in std_logic;
    i_trap_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);

    o_trap_ena : out std_logic;                                 -- Trap occurs  pulse
    o_trap_code : out std_logic_vector(4 downto 0);             -- bit[4] : 1=interrupt; 0=exception; bits[3:0]=code
    o_trap_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- trap on pc
    -- exceptions:
    o_ex_illegal_instr : out std_logic;
    o_ex_unalign_store : out std_logic;
    o_ex_unalign_load : out std_logic;
    o_ex_breakpoint : out std_logic;
    o_ex_ecall : out std_logic;

    o_memop_sign_ext : out std_logic;                           -- Load data with sign extending
    o_memop_load : out std_logic;                               -- Load data instruction
    o_memop_store : out std_logic;                              -- Store data instruction
    o_memop_size : out std_logic_vector(1 downto 0);            -- 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    o_memop_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Memory access address

    o_pre_valid : out std_logic;                                -- pre-latch of valid
    o_valid : out std_logic;                                    -- Output is valid
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);     -- Valid instruction pointer
    o_npc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    o_instr : out std_logic_vector(31 downto 0);                -- Valid instruction value
    o_breakpoint : out std_logic;                               -- ebreak instruction
    o_call : out std_logic;                                     -- CALL pseudo instruction detected
    o_ret : out std_logic                                       -- RET pseudoinstruction detected
  );
end; 
 
architecture arch_InstrExecute of InstrExecute is

  constant Multi_MUL : integer := 0;
  constant Multi_DIV : integer := 1;
  constant Multi_Total : integer := 2;
  constant zero64 : std_logic_vector(63 downto 0) := (others => '0');

  type multi_arith_type is array (0 to Multi_Total-1) 
      of std_logic_vector(RISCV_ARCH-1 downto 0);

  type RegistersType is record
        d_valid : std_logic;                                   -- Valid decoded instruction latch
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        instr : std_logic_vector(31 downto 0);
        res_addr : std_logic_vector(4 downto 0);
        res_val : std_logic_vector(RISCV_ARCH-1 downto 0);
        memop_load : std_logic;
        memop_store : std_logic;
        memop_sign_ext : std_logic;
        memop_size : std_logic_vector(1 downto 0);
        memop_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);

        multi_res_addr : std_logic_vector(4 downto 0);         -- latched output reg. address while multi-cycle instruction
        multi_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- latched pc-value while multi-cycle instruction
        multi_npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- latched npc-value while multi-cycle instruction
        multi_instr : std_logic_vector(31 downto 0);           -- Multi-cycle instruction is under processing
        multi_ena : std_logic_vector(Multi_Total-1 downto 0);  -- Enable pulse for Operation that takes more than 1 clock
        multi_rv32 : std_logic;                                -- Long operation with 32-bits operands
        multi_unsigned : std_logic;                            -- Long operation with unsiged operands
        multi_residual_high : std_logic;                       -- Flag for Divider module: 0=divsion output; 1=residual output
                                                               -- Flag for multiplier: 0=usual; 1=get high bits
        multiclock_ena : std_logic;
        multi_a1 : std_logic_vector(RISCV_ARCH-1 downto 0);    -- Multi-cycle operand 1
        multi_a2 : std_logic_vector(RISCV_ARCH-1 downto 0);    -- Multi-cycle operand 2

        hazard_addr0 : std_logic_vector(4 downto 0);           -- Updated register address on previous step
        hazard_addr1 : std_logic_vector(4 downto 0);           -- Updated register address on pre-previous step
        hazard_depth : std_logic_vector(1 downto 0);           -- Number of modificated registers that wasn't done yet

        call : std_logic;
        ret : std_logic;
  end record;

  signal r, rin : RegistersType;

  signal wb_arith_res : multi_arith_type;
  signal w_arith_valid : std_logic_vector(Multi_Total-1 downto 0);
  signal w_arith_busy : std_logic_vector(Multi_Total-1 downto 0);
  signal w_hazard_detected : std_logic;

  signal wb_shifter_a1 : std_logic_vector(RISCV_ARCH-1 downto 0);  -- Shifters operand 1
  signal wb_shifter_a2 : std_logic_vector(5 downto 0);             -- Shifters operand 2
  signal wb_sll : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_sllw : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_srl : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_srlw : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_sra : std_logic_vector(RISCV_ARCH-1 downto 0);
  signal wb_sraw : std_logic_vector(RISCV_ARCH-1 downto 0);

  component IntMul is port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_ena : in std_logic;
    i_unsigned : in std_logic;
    i_high : in std_logic;
    i_rv32 : in std_logic;
    i_a1 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_a2 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_res : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_valid : out std_logic;
    o_busy : out std_logic
  );
  end component; 

  component IntDiv is port (
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

begin

   mul0 : IntMul port map (
      i_clk  => i_clk,
      i_nrst => i_nrst,
      i_ena => r.multi_ena(Multi_MUL),
      i_unsigned => r.multi_unsigned,
      i_high => r.multi_residual_high,
      i_rv32 => r.multi_rv32,
      i_a1 => r.multi_a1,
      i_a2 => r.multi_a2,
      o_res => wb_arith_res(Multi_MUL),
      o_valid => w_arith_valid(Multi_MUL),
      o_busy => w_arith_busy(Multi_MUL));

   div0 : IntDiv port map (
      i_clk  => i_clk,
      i_nrst => i_nrst,
      i_ena => r.multi_ena(Multi_DIV),
      i_unsigned => r.multi_unsigned,
      i_residual => r.multi_residual_high,
      i_rv32 => r.multi_rv32,
      i_a1 => r.multi_a1,
      i_a2 => r.multi_a2,
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

  comb : process(i_nrst, i_pipeline_hold, i_d_valid, i_d_pc, i_d_instr,
                 i_wb_done, i_memop_load, i_memop_store, i_memop_sign_ext,
                 i_memop_size, i_unsigned_op, i_rv32, i_compressed, i_isa_type, i_ivec,
                 i_rdata1, i_rdata2, i_csr_rdata, i_trap_valid, i_trap_pc, i_ext_irq, i_dport_npc_write,
                 i_dport_npc, i_ie, i_mtvec, i_mode, i_break_mode, i_unsup_exception,
                 wb_arith_res, w_arith_valid, w_arith_busy, w_hazard_detected,
                 wb_sll, wb_sllw, wb_srl, wb_srlw, wb_sra, wb_sraw, r)
    variable v : RegistersType;
    variable w_exception_store : std_logic;
    variable w_exception_load : std_logic;
    variable w_exception_xret : std_logic;
    variable wb_radr1 : std_logic_vector(4 downto 0);
    variable wb_rdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_radr2 : std_logic_vector(4 downto 0);
    variable wb_rdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable w_xret : std_logic;
    variable w_csr_wena : std_logic;
    variable wb_res_addr : std_logic_vector(4 downto 0);
    variable wb_csr_addr  : std_logic_vector(11 downto 0);
    variable wb_csr_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_res : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    variable wb_off : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_mask_i31 : std_logic_vector(RISCV_ARCH-1 downto 0);    -- Bits depending instr[31] bits
    variable wb_sum64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_sum32 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_sub64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_sub32 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_and64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_or64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable wb_xor64 : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable w_memop_load : std_logic;
    variable w_memop_store : std_logic;
    variable w_memop_sign_ext : std_logic;
    variable wb_memop_size : std_logic_vector(1 downto 0);
    variable wb_memop_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);

    variable w_pc_valid : std_logic;
    variable w_d_acceptable : std_logic;
    variable w_multi_valid : std_logic;
    variable w_multi_ena : std_logic;
    variable w_res_wena : std_logic;
    variable w_pc_branch : std_logic;
    variable w_hazard_lvl1 : std_logic;
    variable w_hazard_lvl2 : std_logic;
    variable w_d_valid : std_logic;
    variable w_o_valid : std_logic;
    variable w_o_pipeline_hold : std_logic;
    variable w_less : std_logic;
    variable w_gr_equal : std_logic;
    variable wv : std_logic_vector(Instr_Total-1 downto 0);
    variable opcode_len : integer;

  begin

    wb_radr1 := (others => '0');
    wb_radr2 := (others => '0');
    w_xret := '0';
    w_csr_wena := '0';
    wb_res_addr := (others => '0');
    wb_csr_addr := (others => '0');
    wb_csr_wdata := (others => '0');
    wb_res := (others => '0');
    wb_off := (others => '0');
    wb_rdata1 := (others => '0');
    wb_rdata2 := (others => '0');
    w_memop_load := '0';
    w_memop_store := '0';
    w_memop_sign_ext := '0';
    wb_memop_size := (others => '0');
    wb_memop_addr := (others => '0');
    wv := i_ivec;

    v := r;

    wb_mask_i31 := (others => i_d_instr(31));

    w_pc_valid := '0';
    if i_d_pc = r.npc then
        w_pc_valid := '1';
    end if;
    w_d_acceptable := not i_pipeline_hold and i_d_valid 
                          and w_pc_valid and not r.multiclock_ena;

    if i_isa_type(ISA_R_type) = '1' then
        wb_radr1 := i_d_instr(19 downto 15);
        wb_rdata1 := i_rdata1;
        wb_radr2 := i_d_instr(24 downto 20);
        wb_rdata2 := i_rdata2;
    elsif i_isa_type(ISA_I_type) = '1' then
        wb_radr1 := i_d_instr(19 downto 15);
        wb_rdata1 := i_rdata1;
        wb_radr2 := (others => '0');
        wb_rdata2 := wb_mask_i31(63 downto 12) & i_d_instr(31 downto 20);
    elsif i_isa_type(ISA_SB_type) = '1' then
        wb_radr1 := i_d_instr(19 downto 15);
        wb_rdata1 := i_rdata1;
        wb_radr2 := i_d_instr(24 downto 20);
        wb_rdata2 := i_rdata2;
        wb_off(RISCV_ARCH-1 downto 12) := wb_mask_i31(RISCV_ARCH-1 downto 12);
        wb_off(12) := i_d_instr(31);
        wb_off(11) := i_d_instr(7);
        wb_off(10 downto 5) := i_d_instr(30 downto 25);
        wb_off(4 downto 1) := i_d_instr(11 downto 8);
        wb_off(0) := '0';
    elsif i_isa_type(ISA_UJ_type) = '1' then
        wb_radr1 := (others => '0');
        wb_rdata1 := X"00000000" & i_d_pc;
        wb_radr2 := (others => '0');
        wb_off(RISCV_ARCH-1 downto 20) := wb_mask_i31(RISCV_ARCH-1 downto 20);
        wb_off(19 downto 12) := i_d_instr(19 downto 12);
        wb_off(11) := i_d_instr(20);
        wb_off(10 downto 1) := i_d_instr(30 downto 21);
        wb_off(0) := '0';
    elsif i_isa_type(ISA_U_type) = '1'then
        wb_radr1 := (others => '0');
        wb_rdata1 := X"00000000" & i_d_pc;
        wb_radr2 := (others => '0');
        wb_rdata2(31 downto 0) := i_d_instr(31 downto 12) & X"000";
        wb_rdata2(RISCV_ARCH-1 downto 32) := wb_mask_i31(RISCV_ARCH-1 downto 32);
    elsif i_isa_type(ISA_S_type) = '1' then
        wb_radr1 := i_d_instr(19 downto 15);
        wb_rdata1 := i_rdata1;
        wb_radr2 := i_d_instr(24 downto 20);
        wb_rdata2 := i_rdata2;
        wb_off(RISCV_ARCH-1 downto 12) := wb_mask_i31(RISCV_ARCH-1 downto 12);
        wb_off(11 downto 5) := i_d_instr(31 downto 25);
        wb_off(4 downto 0) := i_d_instr(11 downto 7);
    end if;

    -- parallel ALU:
    wb_sum64 := wb_rdata1 + wb_rdata2;
    wb_sum32(31 downto 0) := wb_rdata1(31 downto 0) + wb_rdata2(31 downto 0);
    wb_sum32(63 downto 32) := (others => wb_sum32(31));
    wb_sub64 := wb_rdata1 - wb_rdata2;
    wb_sub32(31 downto 0) := wb_rdata1(31 downto 0) - wb_rdata2(31 downto 0);
    wb_sub32(63 downto 32) := (others => wb_sub32(31));
    wb_and64 := wb_rdata1 and wb_rdata2;
    wb_or64 := wb_rdata1 or wb_rdata2;
    wb_xor64 := wb_rdata1 xor wb_rdata2;
    
    wb_shifter_a1 <= wb_rdata1;
    wb_shifter_a2 <= wb_rdata2(5 downto 0);

    w_multi_valid := w_arith_valid(Multi_MUL) or w_arith_valid(Multi_DIV);

    -- Don't modify registers on conditional jumps:
    w_res_wena := not (wv(Instr_BEQ) or wv(Instr_BGE) or wv(Instr_BGEU)
               or wv(Instr_BLT) or wv(Instr_BLTU) or wv(Instr_BNE)
               or wv(Instr_SD) or wv(Instr_SW) or wv(Instr_SH) or wv(Instr_SB)
               or wv(Instr_MRET) or wv(Instr_URET)
               or wv(Instr_ECALL) or wv(Instr_EBREAK));

    if w_multi_valid = '1' then
        wb_res_addr := r.multi_res_addr;
        v.multiclock_ena := '0';
    elsif w_res_wena = '1' then
        wb_res_addr := i_d_instr(11 downto 7);
    else
        wb_res_addr := (others => '0');
    end if;
    w_less := '0';
    w_gr_equal := '0';
    if UNSIGNED(wb_rdata1) < UNSIGNED(wb_rdata2) then
        w_less := '1';
    end if;
    if UNSIGNED(wb_rdata1) >= UNSIGNED(wb_rdata2) then
        w_gr_equal := '1';
    end if;

    -- Relative Branch on some condition:
    w_pc_branch := '0';
    if ((wv(Instr_BEQ) = '1' and (wb_sub64 = zero64))
        or (wv(Instr_BGE) = '1' and (wb_sub64(63) = '0'))
        or (wv(Instr_BGEU) = '1' and (w_gr_equal = '1'))
        or (wv(Instr_BLT) = '1' and (wb_sub64(63) = '1'))
        or (wv(Instr_BLTU) = '1' and (w_less = '1'))
        or (wv(Instr_BNE) = '1' and (wb_sub64 /= zero64))) then
        w_pc_branch := '1';
    end if;

    opcode_len := 4;
    if i_compressed = '1' then
        opcode_len := 2;
    end if;

    if w_pc_branch = '1' then
        wb_npc := i_d_pc + wb_off(BUS_ADDR_WIDTH-1 downto 0);
    elsif wv(Instr_JAL) = '1' then
        wb_res(63 downto 32) := (others => '0');
        wb_res(31 downto 0) := i_d_pc + opcode_len;
        wb_npc := wb_rdata1(BUS_ADDR_WIDTH-1 downto 0) + wb_off(BUS_ADDR_WIDTH-1 downto 0);
    elsif wv(Instr_JALR) = '1' then
        wb_res(63 downto 32) := (others => '0');
        wb_res(31 downto 0) := i_d_pc + opcode_len;
        wb_npc := wb_rdata1(BUS_ADDR_WIDTH-1 downto 0) + wb_rdata2(BUS_ADDR_WIDTH-1 downto 0);
        wb_npc(0) := '0';
    elsif wv(Instr_MRET) = '1' or wv(Instr_URET) = '1' then
        wb_res(63 downto 32) := (others => '0');
        wb_res(31 downto 0) := i_d_pc + opcode_len;
        w_xret := i_d_valid and w_pc_valid;
        w_csr_wena := '0';
        if wv(Instr_URET) = '1' then
            wb_csr_addr := CSR_uepc;
        else
            wb_csr_addr := CSR_mepc;
        end if;
        wb_npc := i_csr_rdata(BUS_ADDR_WIDTH-1 downto 0);
    else
        -- Instr_HRET, Instr_SRET, Instr_FENCE, Instr_FENCE_I:
        wb_npc := i_d_pc + opcode_len;
    end if;

    if i_memop_load = '1' then
        wb_memop_addr := wb_rdata1(BUS_ADDR_WIDTH-1 downto 0)
                      + wb_rdata2(BUS_ADDR_WIDTH-1 downto 0);
    elsif i_memop_store = '1' then
        wb_memop_addr := wb_rdata1(BUS_ADDR_WIDTH-1 downto 0)
                       + wb_off(BUS_ADDR_WIDTH-1 downto 0);
    end if;

    v.memop_addr := (others => '0');
    v.memop_load := '0';
    v.memop_store := '0';
    v.memop_sign_ext := '0';
    v.memop_size := (others => '0');
    w_exception_store := '0';
    w_exception_load := '0';
    w_exception_xret := '0';

    if ((wv(Instr_LD) = '1' and wb_memop_addr(2 downto 0) /= "000")
        or ((wv(Instr_LW) or wv(Instr_LWU)) = '1' and wb_memop_addr(1 downto 0) /= "00")
        or ((wv(Instr_LH) or wv(Instr_LHU)) = '1' and wb_memop_addr(0) /= '0'))  then
        w_exception_load := not w_hazard_detected;
    end if;
    if ((wv(Instr_SD) = '1' and wb_memop_addr(2 downto 0) /= "000")
        or (wv(Instr_SW) = '1' and wb_memop_addr(1 downto 0) /= "00")
        or (wv(Instr_SH) = '1' and wb_memop_addr(0) /= '0')) then
        w_exception_store := not w_hazard_detected;
    end if;
    if (wv(Instr_MRET) = '1' and i_mode /= PRV_M) 
        or (wv(Instr_URET) = '1' and i_mode /= PRV_U) then
        w_exception_xret := '1';
    end if;

    o_ex_illegal_instr <= (i_unsup_exception and w_pc_valid) or w_exception_xret;
    o_ex_unalign_store <= w_exception_store;
    o_ex_unalign_load <= w_exception_load;
    o_ex_breakpoint <= wv(Instr_EBREAK);
    o_ex_ecall <= wv(Instr_ECALL);

    --! Default number of cycles per instruction = 0 (1 clock per instr)
    --! If instruction is multicycle then modify this value.
    --!
    v.multi_ena := (others => '0');
    v.multi_rv32 := i_rv32;
    v.multi_unsigned := i_unsigned_op;
    v.multi_residual_high := '0';
    v.multi_a1 := i_rdata1;
    v.multi_a2 := i_rdata2;

    w_multi_ena := wv(Instr_MUL) or wv(Instr_MULW) or wv(Instr_DIV)
                    or wv(Instr_DIVU) or wv(Instr_DIVW) or wv(Instr_DIVUW)
                    or wv(Instr_REM) or wv(Instr_REMU) or wv(Instr_REMW)
                    or wv(Instr_REMUW);
    if (w_multi_ena and w_d_acceptable and (not i_trap_valid)) = '1' then
        v.multiclock_ena := '1';
        v.multi_res_addr := wb_res_addr;
        v.multi_pc := i_d_pc;
        v.multi_instr := i_d_instr;
        v.multi_npc := wb_npc;
    end if;


    -- ALU block selector:
    if w_arith_valid(Multi_MUL) = '1' then
        wb_res := wb_arith_res(Multi_MUL);
    elsif w_arith_valid(Multi_DIV) = '1' then
        wb_res := wb_arith_res(Multi_DIV);
    elsif i_memop_load = '1' then
        w_memop_load := not w_hazard_detected;
        w_memop_sign_ext := i_memop_sign_ext;
        wb_memop_size := i_memop_size;
    elsif i_memop_store = '1' then
        w_memop_store := not w_hazard_detected;
        wb_memop_size := i_memop_size;
        wb_res := wb_rdata2;
    elsif (wv(Instr_ADD) or wv(Instr_ADDI) or wv(Instr_AUIPC)) = '1' then
        wb_res := wb_sum64;
    elsif (wv(Instr_ADDW) or wv(Instr_ADDIW)) = '1' then
        wb_res := wb_sum32;
    elsif wv(Instr_SUB) = '1' then
        wb_res := wb_sub64;
    elsif wv(Instr_SUBW) = '1' then
        wb_res := wb_sub32;
    elsif (wv(Instr_SLL) or wv(Instr_SLLI)) = '1' then
        wb_res := wb_sll;
    elsif (wv(Instr_SLLW) or wv(Instr_SLLIW)) = '1' then
        wb_res := wb_sllw;
    elsif (wv(Instr_SRL) or wv(Instr_SRLI)) = '1' then
        wb_res := wb_srl;
    elsif (wv(Instr_SRLW) or wv(Instr_SRLIW)) = '1' then
        wb_res := wb_srlw;
    elsif (wv(Instr_SRA) or wv(Instr_SRAI)) = '1' then
        wb_res := wb_sra;
    elsif (wv(Instr_SRAW) or wv(Instr_SRAW) or wv(Instr_SRAIW)) = '1' then
        wb_res := wb_sraw;
    elsif (wv(Instr_AND) or wv(Instr_ANDI)) = '1' then
        wb_res := wb_and64;
    elsif (wv(Instr_OR) or wv(Instr_ORI)) = '1' then
        wb_res := wb_or64;
    elsif (wv(Instr_XOR) or wv(Instr_XORI)) = '1' then
        wb_res := wb_xor64;
    elsif (wv(Instr_SLT) or wv(Instr_SLTI)) = '1' then
        wb_res(RISCV_ARCH-1 downto 1) := (others => '0');
        wb_res(0) := wb_sub64(63);
    elsif (wv(Instr_SLTU) or wv(Instr_SLTIU)) = '1' then
        wb_res(63 downto 1) := (others => '0');
        wb_res(0) := w_less;
    elsif wv(Instr_LUI) = '1' then
        wb_res := wb_rdata2;
    elsif (wv(Instr_MUL) or wv(Instr_MULW)) = '1' then
        v.multi_ena(Multi_MUL) := w_d_acceptable and (not i_trap_valid);
    elsif (wv(Instr_DIV) or wv(Instr_DIVU)
            or wv(Instr_DIVW) or wv(Instr_DIVUW)) = '1' then
        v.multi_ena(Multi_DIV) := w_d_acceptable and (not i_trap_valid);
    elsif (wv(Instr_REM) or wv(Instr_REMU)
            or wv(Instr_REMW) or wv(Instr_REMUW)) = '1' then
        v.multi_ena(Multi_DIV) := w_d_acceptable and (not i_trap_valid);
        v.multi_residual_high := '1';
    elsif wv(Instr_CSRRC) = '1' then
        wb_res := i_csr_rdata;
        w_csr_wena := '1';
        wb_csr_addr := wb_rdata2(11 downto 0);
        wb_csr_wdata := i_csr_rdata and (not i_rdata1);
    elsif wv(Instr_CSRRCI) = '1' then
        wb_res := i_csr_rdata;
        w_csr_wena := '1';
        wb_csr_addr := wb_rdata2(11 downto 0);
        wb_csr_wdata(RISCV_ARCH-1 downto 5) := i_csr_rdata(RISCV_ARCH-1 downto 5);
        wb_csr_wdata(4 downto 0) := i_csr_rdata(4 downto 0) and not wb_radr1;  -- zero-extending 5 to 64-bits
    elsif wv(Instr_CSRRS) = '1' then
        wb_res := i_csr_rdata;
        w_csr_wena := '1';
        wb_csr_addr := wb_rdata2(11 downto 0);
        wb_csr_wdata := i_csr_rdata or i_rdata1;
    elsif wv(Instr_CSRRSI) = '1' then
        wb_res := i_csr_rdata;
        w_csr_wena := '1';
        wb_csr_addr := wb_rdata2(11 downto 0);
        wb_csr_wdata(RISCV_ARCH-1 downto 5) := i_csr_rdata(RISCV_ARCH-1 downto 5);
        wb_csr_wdata(4 downto 0) := i_csr_rdata(4 downto 0) or wb_radr1;  -- zero-extending 5 to 64-bits
    elsif wv(Instr_CSRRW) = '1' then
        wb_res := i_csr_rdata;
        w_csr_wena := '1';
        wb_csr_addr := wb_rdata2(11 downto 0);
        wb_csr_wdata := i_rdata1;
    elsif wv(Instr_CSRRWI) = '1' then
        wb_res := i_csr_rdata;
        w_csr_wena := '1';
        wb_csr_addr := wb_rdata2(11 downto 0);
        wb_csr_wdata(RISCV_ARCH-1 downto 5) := (others => '0');
        wb_csr_wdata(4 downto 0) := wb_radr1;  -- zero-extending 5 to 64-bits
    end if;


    w_d_valid := 
        (w_d_acceptable and (not w_multi_ena)) or w_multi_valid;
    o_pre_valid <= w_d_valid;

    v.call := '0';
    v.ret := '0';
    if i_dport_npc_write = '1' then
        v.npc := i_dport_npc;
    elsif w_d_valid = '1' then
        if i_trap_valid = '1' then
            v.npc := i_trap_pc;
        elsif w_multi_valid = '1' then
            v.pc := r.multi_pc;
            v.instr := r.multi_instr;
            v.npc := r.multi_npc;
            v.memop_load := '0';
            v.memop_sign_ext := '0';
            v.memop_store := '0';
            v.memop_size := (others => '0');
            v.memop_addr := (others => '0');
        else
            v.pc := i_d_pc;
            v.instr := i_d_instr;
            v.npc := wb_npc;
            v.memop_load := w_memop_load;
            v.memop_sign_ext := w_memop_sign_ext;
            v.memop_store := w_memop_store;
            v.memop_size := wb_memop_size;
            v.memop_addr := wb_memop_addr;
        end if;
        v.res_addr := wb_res_addr;
        v.res_val := wb_res;

        v.hazard_addr1 := r.hazard_addr0;
        v.hazard_addr0 := wb_res_addr;

        if wv(Instr_JAL) = '1' and conv_integer(wb_res_addr) = Reg_ra then
            v.call := '1';
        end if;
        if wv(Instr_JALR) = '1' then
            if conv_integer(wb_res_addr) = Reg_ra then
                v.call := '1';
            elsif wb_rdata2 = zero64 and conv_integer(wb_radr1) = Reg_ra then
                v.ret := '1';
            end if;
        end if;
    end if;

    v.d_valid := w_d_valid;

    if w_d_valid = '1' and i_wb_done = '0' then
        v.hazard_depth := r.hazard_depth + 1;
        v.hazard_addr0 := wb_res_addr;
    elsif w_d_valid = '0' and i_wb_done = '1' then
        v.hazard_depth := r.hazard_depth - 1;
    end if;
    w_hazard_lvl1 := '0';
    if (wb_radr1 /= "00000" and wb_radr1 = r.hazard_addr0) or
       (wb_radr2 /= "00000" and wb_radr2 = r.hazard_addr0) then
        w_hazard_lvl1 := '1';
    end if;
    w_hazard_lvl2 := '0';
    if (wb_radr1 /= "00000" and wb_radr1 = r.hazard_addr1) or
       (wb_radr2 /= "00000" and wb_radr2 = r.hazard_addr1) then
        w_hazard_lvl2 := '1';
    end if;

    if r.hazard_depth = "01" then
        w_hazard_detected <= w_hazard_lvl1;
    elsif r.hazard_depth = "10" then
        w_hazard_detected <= w_hazard_lvl1 or w_hazard_lvl2;
    else
        w_hazard_detected <= '0';
    end if;

    w_o_valid := r.d_valid;
    w_o_pipeline_hold := w_hazard_detected or r.multiclock_ena;


    if i_nrst = '0' then
        v.d_valid := '0';
        v.pc := (others => '0');
        v.npc := RESET_VECTOR;
        v.instr := (others => '0');
        v.res_addr := (others => '0');
        v.res_val := (others => '0');
        v.memop_load := '0';
        v.memop_sign_ext := '0';
        v.memop_store := '0';
        v.memop_size := (others => '0');
        v.memop_addr := (others => '0');
        v.hazard_depth := (others => '0');
        v.hazard_addr0 := (others => '0');
        v.hazard_addr1 := (others => '0');

        v.multiclock_ena := '0';
        v.multi_pc := (others => '0');
        v.multi_instr := (others => '0');
        v.multi_npc := (others => '0');
        v.multi_res_addr := (others => '0');
        v.multi_ena := (others => '0');
        v.multi_rv32 := '0';
        v.multi_unsigned := '0';
        v.multi_residual_high := '0';
        v.multi_a1 := (others => '0');
        v.multi_a2 := (others => '0');

        v.call := '0';
        v.ret := '0';
    end if;

    o_radr1 <= wb_radr1;
    o_radr2 <= wb_radr2;
    o_res_addr <= r.res_addr;
    o_res_data <= r.res_val;
    o_pipeline_hold <= w_o_pipeline_hold;

    o_xret <= w_xret;
    o_csr_wena <= w_csr_wena and w_pc_valid and not w_hazard_detected;
    o_csr_addr <= wb_csr_addr;
    o_csr_wdata <= wb_csr_wdata;
    o_trap_ena <= '0';--r.trap_ena;
    o_trap_code <= (others => '0');--r.trap_code;
    o_trap_pc <= (others => '0');--r.trap_pc;

    o_memop_sign_ext <= r.memop_sign_ext;
    o_memop_load <= r.memop_load;
    o_memop_store <= r.memop_store;
    o_memop_size <= r.memop_size;
    o_memop_addr <= r.memop_addr;

    o_valid <= w_o_valid;
    o_pc <= r.pc;
    o_npc <= r.npc;
    o_instr <= r.instr;
    o_breakpoint <= '0';
    o_call <= r.call;
    o_ret <= r.ret;
    
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
