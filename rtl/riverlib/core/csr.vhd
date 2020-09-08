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

entity CsrRegs is 
  generic (
    hartid : integer;
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;                                   -- CPU clock
    i_nrst : in std_logic;                                  -- Reset. Active LOW.
    i_mret : in std_logic;                                  -- mret instruction signals mode switching
    i_uret : in std_logic;                                  -- uret instruction signals mode switching
    i_sp : in std_logic_vector(RISCV_ARCH-1 downto 0);      -- Stack Pointer for the borders control
    i_addr : in std_logic_vector(11 downto 0);              -- CSR address, if xret=1 switch mode accordingly
    i_wena : in std_logic;                                  -- Write enable
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);   -- CSR writing value
    o_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);  -- CSR read value
    o_mepc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_uepc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_trap_ready : in std_logic;                            -- Trap branch request was accepted
    i_e_pc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_e_npc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_ex_npc : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_ex_data_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);-- Data path: address must be equal to the latest request address
    i_ex_data_load_fault : in std_logic;                    -- Data path: Bus response with SLVERR or DECERR on read
    i_ex_data_store_fault : in std_logic;                   -- Data path: Bus response with SLVERR or DECERR on write
    i_ex_data_store_fault_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_ex_instr_load_fault : in std_logic;
    i_ex_illegal_instr : in std_logic;
    i_ex_unalign_store : in std_logic;
    i_ex_unalign_load : in std_logic;
    i_ex_mpu_store : in std_logic;
    i_ex_mpu_load : in std_logic;
    i_ex_breakpoint : in std_logic;
    i_ex_ecall : in std_logic;
    i_ex_fpu_invalidop : in std_logic;         -- FPU Exception: invalid operation
    i_ex_fpu_divbyzero : in std_logic;         -- FPU Exception: divide by zero
    i_ex_fpu_overflow : in std_logic;          -- FPU Exception: overflow
    i_ex_fpu_underflow : in std_logic;         -- FPU Exception: underflow
    i_ex_fpu_inexact : in std_logic;           -- FPU Exception: inexact
    i_fpu_valid : in std_logic;                -- FPU output is valid
    i_irq_external : in std_logic;
    i_e_next_ready: in std_logic;
    i_e_valid : in std_logic;
    o_executed_cnt : out std_logic_vector(63 downto 0);        -- Number of executed instructions
    o_trap_valid : out std_logic;                              -- Trap pulse
    o_trap_pc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- trap on pc
    o_dbg_pc_write : out std_logic;                                 -- Modify pc via debug interface
    o_dbg_pc : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);  -- Writing value into pc register

    o_progbuf_ena : out std_logic;                            -- Execution from prog buffer
    o_progbuf_pc : out std_logic_vector(31 downto 0);         -- prog buffer instruction counter
    o_progbuf_data : out std_logic_vector(31 downto 0);       -- prog buffer instruction opcode
    o_flushi_ena : out std_logic;                             -- clear specified addr in ICache without execution of fence.i
    o_flushi_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0); -- ICache address to flush

    o_mpu_region_we : out std_logic;
    o_mpu_region_idx : out std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    o_mpu_region_addr : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_mpu_region_mask : out std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    o_mpu_region_flags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);  -- {ena, cachable, r, w, x}

    i_dport_ena : in std_logic;                              -- Debug port request is enabled
    i_dport_write : in std_logic;                            -- Debug port Write enable
    i_dport_addr : in std_logic_vector(11 downto 0);         -- Debug port CSR address
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);-- Debug port CSR writing value
    o_dport_valid : out std_logic;                              -- Debug read data is valid
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);-- Debug port CSR read value
    o_halt : out std_logic
  );
end; 
 
architecture arch_CsrRegs of CsrRegs is

  type RegistersType is record
      mtvec : std_logic_vector(RISCV_ARCH-1 downto 0);
      mscratch : std_logic_vector(RISCV_ARCH-1 downto 0);
      mstackovr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      mstackund : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      mbadaddr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      mode : std_logic_vector(1 downto 0);
      uie : std_logic;                       -- User level interrupts ena for current priv. mode
      mie : std_logic;                       -- Machine level interrupts ena for current priv. mode
      mpie : std_logic;                      -- Previous MIE value
      mstackovr_ena : std_logic;             -- Stack Overflow control enabled
      mstackund_ena : std_logic;             -- Stack Underflow control enabled
      mpp : std_logic_vector(1 downto 0);    -- Previous mode
      mepc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      uepc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      ext_irq : std_logic;

      mpu_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      mpu_mask : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      mpu_idx : std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
      mpu_flags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
      mpu_we : std_logic;

      ex_fpu_invalidop : std_logic;          -- FPU Exception: invalid operation
      ex_fpu_divbyzero : std_logic;          -- FPU Exception: divide by zero
      ex_fpu_overflow : std_logic;           -- FPU Exception: overflow
      ex_fpu_underflow : std_logic;          -- FPU Exception: underflow
      ex_fpu_inexact : std_logic;            -- FPU Exception: inexact
      trap_irq : std_logic;
      trap_code : std_logic_vector(4 downto 0);
      trap_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      break_event : std_logic;
      hold_data_store_fault : std_logic;
      hold_data_load_fault : std_logic;
      hold_mbadaddr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);

      timer : std_logic_vector(63 downto 0);                   -- Timer in clocks.
      cycle_cnt : std_logic_vector(63 downto 0);               -- Cycle in clocks.
      executed_cnt : std_logic_vector(63 downto 0);            -- Number of valid executed instructions

      break_mode : std_logic;               -- Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
      halt : std_logic;
      halt_cause : std_logic_vector(2 downto 0);      -- 1=ebreak instruction; 2=breakpoint exception; 3=haltreq; 4=step
      progbuf_ena : std_logic;
      progbuf_data : std_logic_vector(CFG_PROGBUF_REG_TOTAL*32-1 downto 0);
      progbuf_data_out : std_logic_vector(31 downto 0);
      progbuf_data_pc : std_logic_vector(4 downto 0);
      progbuf_data_npc : std_logic_vector(4 downto 0);
      progbuf_err : std_logic_vector(2 downto 0);         -- 1=busy;2=cmd not supported;3=exception;4=halt/resume;5=bus error
      stepping_mode : std_logic;
      stepping_mode_cnt : std_logic_vector(RISCV_ARCH-1 downto 0);
      ins_per_step : std_logic_vector(RISCV_ARCH-1 downto 0); -- Number of steps before halt in stepping mode
      flushi_ena : std_logic;
      flushi_addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
  end record;

  constant R_RESET : RegistersType := (
        (others => '0'), -- mtvec
        (others => '0'), -- mscratch
        (others => '0'), -- mstackovr
        (others => '0'), -- mstackund
        (others => '0'), -- mbadaddr
        PRV_M,           -- mode
        '0', '0', '0',
        '0',             -- mstackovr_ena
        '0',             -- mstackund_ena
        (others => '0'),  --mpp
        (others => '0'), -- mepc
        (others => '0'), -- uepc
         '0',            -- ext_irq
        (others => '0'), -- mpu_addr
        (others => '0'), -- mpu_mask
        (others => '0'), -- mpu_idx
        (others => '0'), -- mpu_flags
        '0',             -- mpu_we
        '0', '0', '0', '0', '0', 
        '0', (others => '0'), (others => '0'), '0',
        '0', '0', (others => '0'),
        (others => '0'), --timer
        (others => '0'), --cycle_cnt
        (others => '0'), -- executed_cnt

        '0',             -- break_mode
        '0',             -- halt
        (others => '0'), -- halt_cause
        '0',             -- progbuf_ena
        (others => '0'), -- progbuf_data
        (others => '0'), -- progbuf_data_out
        (others => '0'), -- progbuf_data_pc
        (others => '0'), -- progbuf_data_npc
        PROGBUF_ERR_NONE,-- progbuf_err
        '0',             -- stepping_mode
        (others => '0'), -- stepping_mode_cnt
        conv_std_logic_vector(1, RISCV_ARCH), -- ins_per_step
        '0',             -- flushi_ena
        (others => '0')  -- flushi_addr
  );

  signal r, rin : RegistersType;
  
begin

  comb : process(i_nrst, i_mret, i_uret, i_sp, i_addr, i_wena, i_wdata, i_trap_ready,
                 i_e_pc, i_e_npc, i_ex_npc, i_ex_data_addr, i_ex_data_load_fault, i_ex_data_store_fault,
                 i_ex_data_store_fault_addr,
                 i_ex_instr_load_fault, i_ex_illegal_instr, i_ex_unalign_load, i_ex_unalign_store,
                 i_ex_mpu_store, i_ex_mpu_load, i_ex_breakpoint, i_ex_ecall, 
                 i_ex_fpu_invalidop, i_ex_fpu_divbyzero, i_ex_fpu_overflow,
                 i_ex_fpu_underflow, i_ex_fpu_inexact, i_fpu_valid, i_irq_external,
                 i_e_next_ready, i_e_valid,
                 i_dport_ena, i_dport_write, i_dport_addr, i_dport_wdata,
                 r)
    variable v : RegistersType;
    variable w_ie : std_logic;
    variable w_ext_irq : std_logic;
    variable w_trap_valid : std_logic;
    variable wb_trap_pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable v_dbg_pc_write : std_logic;
    variable vb_dbg_pc : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable w_trap_irq : std_logic;
    variable w_exception_xret : std_logic;
    variable wb_trap_code : std_logic_vector(4 downto 0);
    variable wb_mbadaddr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    variable w_mstackovr : std_logic;
    variable w_mstackund : std_logic;
    variable vb_csr_addr : std_logic_vector(11 downto 0);
    variable vb_csr_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable v_csr_wena : std_logic;
    variable v_dport_valid : std_logic;
    variable vb_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    variable v_cur_halt : std_logic;
    variable v_req_halt : std_logic;
    variable v_req_resume : std_logic;
    variable v_req_progbuf : std_logic;
    variable v_clear_progbuferr : std_logic;
    variable tidx : integer range 0 to 15;
    variable tnpc : integer;
  begin

    v := r;

    vb_rdata := (others => '0');
    v_dbg_pc_write := '0';
    vb_dbg_pc := (others => '0');
    v_cur_halt := '0';
    v_req_halt := '0';
    v_req_resume := '0';
    v_req_progbuf := '0';
    v_clear_progbuferr := '0';
    v.flushi_ena := '0';
    v.flushi_addr := (others => '0');
    tnpc := 16*conv_integer(r.progbuf_data_npc);

    if i_wena = '1' then
        vb_csr_addr := i_addr;
        vb_csr_wdata := i_wdata;
        v_csr_wena := '1';
        v_dport_valid := '0';
    else
        vb_csr_addr := i_dport_addr;
        v_csr_wena := i_dport_ena and i_dport_write;
        vb_csr_wdata := i_dport_wdata;
        v_dport_valid := '1';
    end if;

    tidx := conv_integer(vb_csr_wdata(35 downto 32));

    case vb_csr_addr is
    when CSR_fflags =>
        vb_rdata(0) := r.ex_fpu_inexact;
        vb_rdata(1) := r.ex_fpu_underflow;
        vb_rdata(2) := r.ex_fpu_overflow;
        vb_rdata(3) := r.ex_fpu_divbyzero;
        vb_rdata(4) := r.ex_fpu_invalidop;
        if CFG_HW_FPU_ENABLE then
            if v_csr_wena = '1' then
                v.ex_fpu_inexact := vb_csr_wdata(0);
                v.ex_fpu_underflow := vb_csr_wdata(1);
                v.ex_fpu_overflow := vb_csr_wdata(2);
                v.ex_fpu_divbyzero := vb_csr_wdata(3);
                v.ex_fpu_invalidop := vb_csr_wdata(4);
            end if;
        end if;
    when CSR_frm =>
        if CFG_HW_FPU_ENABLE then
            vb_rdata(2 downto 0) := "100";  -- Round mode: round to Nearest (RMM)
        end if;
    when CSR_fcsr =>
        vb_rdata(0) := r.ex_fpu_inexact;
        vb_rdata(1) := r.ex_fpu_underflow;
        vb_rdata(2) := r.ex_fpu_overflow;
        vb_rdata(3) := r.ex_fpu_divbyzero;
        vb_rdata(4) := r.ex_fpu_invalidop;
        if CFG_HW_FPU_ENABLE then
            vb_rdata(7 downto 5) := "100";  -- Round mode: round to Nearest (RMM)
            if v_csr_wena = '1' then
                v.ex_fpu_inexact := vb_csr_wdata(0);
                v.ex_fpu_underflow := vb_csr_wdata(1);
                v.ex_fpu_overflow := vb_csr_wdata(2);
                v.ex_fpu_divbyzero := vb_csr_wdata(3);
                v.ex_fpu_invalidop := vb_csr_wdata(4);
            end if;
        end if;
    when CSR_misa =>
        --! Base[XLEN-1:XLEN-2]
        --!     1 = 32
        --!     2 = 64
        --!     3 = 128
        --!
        vb_rdata(RISCV_ARCH-1 downto RISCV_ARCH-2) := "10";
        --! BitCharacterDescription
        --! 0  A Atomic extension
        --! 1  B Tentatively reserved for Bit operations extension
        --! 2  C Compressed extension
        --! 3  D Double-precision Foating-point extension
        --! 4  E RV32E base ISA (embedded)
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
        vb_rdata(8) := '1';
        vb_rdata(12) := '1';
        vb_rdata(20) := '1';
        vb_rdata(2) := '1';
        if CFG_HW_FPU_ENABLE then
            vb_rdata(3) := '1';
        end if;
    when CSR_mvendorid =>
        vb_rdata(31 downto 0) := CFG_VENDOR_ID;
    when CSR_marchid =>
    when CSR_mimplementationid =>
        vb_rdata(31 downto 0) := CFG_IMPLEMENTATION_ID;
    when CSR_mhartid =>
        vb_rdata(31 downto 0) := conv_std_logic_vector(hartid, 32);
    when CSR_uepc =>    -- User mode program counter
        vb_rdata(CFG_CPU_ADDR_BITS-1 downto 0) := r.uepc;
        if v_csr_wena = '1' then
            v.uepc := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
        end if;
    when CSR_mstatus => -- Machine mode status register
        vb_rdata(0) := r.uie;
        vb_rdata(3) := r.mie;
        vb_rdata(7) := r.mpie;
        vb_rdata(12 downto 11) := r.mpp;
        if CFG_HW_FPU_ENABLE then
            vb_rdata(14 downto 13) := "01";  -- FS field: Initial state
        end if;
        vb_rdata(33 downto 32) := "10";  -- UXL: User mode supported 64-bits
        if v_csr_wena = '1' then
            v.uie := vb_csr_wdata(0);
            v.mie := vb_csr_wdata(3);
            v.mpie := vb_csr_wdata(7);
            v.mpp := vb_csr_wdata(12 downto 11);
        end if;
    when CSR_medeleg => -- Machine exception delegation
    when CSR_mideleg => -- Machine interrupt delegation
    when CSR_mie =>     -- Machine interrupt enable bit
    when CSR_mtvec =>
        vb_rdata := r.mtvec;
        if v_csr_wena = '1' then
            v.mtvec := vb_csr_wdata;
        end if;
    when CSR_mscratch => -- Machine scratch register
        vb_rdata := r.mscratch;
        if v_csr_wena = '1' then
            v.mscratch := vb_csr_wdata;
        end if;
    when CSR_mepc => -- Machine program counter
        vb_rdata(CFG_CPU_ADDR_BITS-1 downto 0) := r.mepc;
        if v_csr_wena = '1' then
            v.mepc := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
        end if;
    when CSR_mcause => -- Machine trap cause
        vb_rdata(63) := r.trap_irq;
        vb_rdata(4 downto 0) := r.trap_code;
    when CSR_mbadaddr =>   -- Machine bad address
        vb_rdata(CFG_CPU_ADDR_BITS-1 downto 0) := r.mbadaddr;
    when CSR_mip =>        -- Machine interrupt pending
    when CSR_cycle =>
        vb_rdata := r.cycle_cnt;
    when CSR_time =>
        vb_rdata := r.timer;
    when CSR_insret =>
        vb_rdata := r.executed_cnt;
    when CSR_mstackovr =>  -- Machine stack overflow
        vb_rdata(CFG_CPU_ADDR_BITS-1 downto 0) := r.mstackovr;
        if v_csr_wena = '1' then
            v.mstackovr := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
            v.mstackovr_ena := or_reduce(vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0));
        end if;
    when CSR_mstackund =>  -- Machine stack underflow
        vb_rdata(CFG_CPU_ADDR_BITS-1 downto 0) := r.mstackund;
        if v_csr_wena = '1' then
            v.mstackund := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
            v.mstackund_ena := or_reduce(vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0));
        end if;
    when CSR_mpu_addr =>
        if v_csr_wena = '1' then
            v.mpu_addr := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
        end if;
    when CSR_mpu_mask =>
        if v_csr_wena = '1' then
            v.mpu_mask := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
        end if;
    when CSR_mpu_ctrl =>
        vb_rdata(15 downto 8) := conv_std_logic_vector(CFG_MPU_TBL_SIZE, 8);
        if v_csr_wena = '1' then
            v.mpu_idx := vb_csr_wdata(8+CFG_MPU_TBL_WIDTH-1 downto 8);
            v.mpu_flags := vb_csr_wdata(CFG_MPU_FL_TOTAL-1 downto 0);
            v.mpu_we := '1';
        end if;
    when CSR_runcontrol =>
        if v_csr_wena = '1' then
            v_req_halt := vb_csr_wdata(31);
            v_req_resume := vb_csr_wdata(30);
            if vb_csr_wdata(27) = '1' then
                if r.halt = '1' then
                    v_req_progbuf := '1';
                else
                    v.progbuf_err := PROGBUF_ERR_HALT_RESUME;
                end if;
            end if;
        end if;
    when CSR_insperstep =>
        vb_rdata := r.ins_per_step;
        if v_csr_wena = '1' then
            v.ins_per_step := vb_csr_wdata;
            if or_reduce(vb_csr_wdata) = '0' then
                v.ins_per_step := conv_std_logic_vector(1, RISCV_ARCH);  -- cannot be zero
            end if;
            if r.halt = '1' then
                v.stepping_mode_cnt := vb_csr_wdata;
            end if;
        end if;
    when CSR_progbuf =>
        if v_csr_wena = '1' then
            v.progbuf_data(32*tidx+31 downto 32*tidx) := vb_csr_wdata(31 downto 0);
        end if;
    when CSR_abstractcs =>
        vb_rdata(28 downto 24) := conv_std_logic_vector(CFG_PROGBUF_REG_TOTAL, 5);
        vb_rdata(12) := r.progbuf_ena;       -- busy
        vb_rdata(10 downto 8) := r.progbuf_err;
        vb_rdata(3 downto 0) := conv_std_logic_vector(CFG_DATA_REG_TOTAL, 4);
        if v_csr_wena = '1' then
            v_clear_progbuferr := vb_csr_wdata(8);   -- W1C err=1
        end if;
    when CSR_flushi =>
        if v_csr_wena = '1' then
            v.flushi_ena := '1';
            v.flushi_addr := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
        end if;
    when CSR_dcsr =>
        vb_rdata(31 downto 28) := "0100";        -- xdebugver: 4=External debug supported
        vb_rdata(8 downto 6) := r.halt_cause;    -- cause:
        vb_rdata(2) := r.stepping_mode;          -- step: before resumereq
        vb_rdata(1 downto 0) := "11";            -- prv: privilege in debug mode: 3=machine
        if v_csr_wena = '1' then
            v.stepping_mode := vb_csr_wdata(2);
            if vb_csr_wdata(2) = '1' then
                v.stepping_mode_cnt := r.ins_per_step;  -- default =1
            end if;
        end if;
    when CSR_dpc =>
        -- Upon entry into debug mode DPC must contains:
        --       cause        |   Address
        -- -------------------|----------------
        -- ebreak             |  Address of ebreak instruction
        -- single step        |  Address of next instruction to be executed
        -- trigger (HW BREAK) |  if timing=0, cause isntruction, if timing=1 enxt instruction
        -- halt request       |  next instruction
        --
        if r.halt_cause = HALT_CAUSE_EBREAK then
            vb_rdata(CFG_CPU_ADDR_BITS-1 downto 0) := i_e_pc;
        else
            vb_rdata(CFG_CPU_ADDR_BITS-1 downto 0) := i_e_npc;
        end if;
        if v_csr_wena = '1' then
            v_dbg_pc_write := '1';
            vb_dbg_pc := vb_csr_wdata(CFG_CPU_ADDR_BITS-1 downto 0);
        end if;
    when others =>
    end case;

    if r.mpu_we = '1' then
        v.mpu_we := '0';
    end if;

    w_ie := '0';
    if (r.mode /= PRV_M) or r.mie = '1' then
        w_ie := '1';
    end if;
    w_ext_irq := i_irq_external and w_ie;
    if i_trap_ready = '1' then
        v.ext_irq := w_ext_irq;
    end if;

    w_exception_xret := '0';
    if (i_mret = '1' and r.mode /= PRV_M) or
        (i_uret = '1' and r.mode /= PRV_U) then
        w_exception_xret := '1';
    end if;

    w_mstackovr := '0';
    if i_sp(CFG_CPU_ADDR_BITS-1 downto 0) < r.mstackovr then
        w_mstackovr := '1';
    end if;

    w_mstackund := '0';
    if i_sp(CFG_CPU_ADDR_BITS-1 downto 0) > r.mstackund then
        w_mstackund := '1';
    end if;

    if i_fpu_valid = '1' then
        v.ex_fpu_invalidop := i_ex_fpu_invalidop;
        v.ex_fpu_divbyzero := i_ex_fpu_divbyzero;
        v.ex_fpu_overflow := i_ex_fpu_overflow;
        v.ex_fpu_underflow := i_ex_fpu_underflow;
        v.ex_fpu_inexact := i_ex_fpu_inexact;
    end if;

    w_trap_valid := '0';
    w_trap_irq := '0';
    wb_trap_code := (others => '0');
    v.break_event := '0';
    wb_trap_pc := r.mtvec(CFG_CPU_ADDR_BITS-1 downto 0);
    wb_mbadaddr := i_e_npc;

    if i_ex_instr_load_fault = '1' then
        w_trap_valid := '1';
        wb_trap_pc := CFG_NMI_INSTR_FAULT_ADDR;
        wb_trap_code := EXCEPTION_InstrFault;
        -- illegal address instruction can generate any other exceptions
        v.hold_data_load_fault := '0';
        v.hold_data_store_fault := '0';
    elsif i_ex_illegal_instr = '1' or w_exception_xret = '1' then
        w_trap_valid := '1';
        wb_trap_pc := CFG_NMI_INSTR_ILLEGAL_ADDR;
        wb_trap_code := EXCEPTION_InstrIllegal;
        -- illegal instruction can generate any other exceptions
        v.hold_data_load_fault := '0';
        v.hold_data_store_fault := '0';
    elsif i_ex_breakpoint = '1' then
        v.break_event := '1';
        w_trap_valid := '1';
        wb_trap_code := EXCEPTION_Breakpoint;
        if r.break_mode = '0' then
            wb_trap_pc := i_e_npc;
        else
            wb_trap_pc := CFG_NMI_BREAKPOINT_ADDR;
        end if;
    elsif i_ex_unalign_load = '1' then
        w_trap_valid := '1';
        wb_trap_pc := CFG_NMI_LOAD_UNALIGNED_ADDR;
        wb_trap_code := EXCEPTION_LoadMisalign;
    elsif i_ex_data_load_fault = '1' or r.hold_data_load_fault = '1' then
        w_trap_valid := '1';
        v.hold_data_load_fault := '0';
        if i_trap_ready = '0' then
            v.hold_data_load_fault := '1';
        end if;
        wb_trap_pc := CFG_NMI_LOAD_FAULT_ADDR;
        if i_ex_data_load_fault = '1'  then
            wb_mbadaddr := i_ex_data_addr;     -- miss-access read data address
            v.hold_mbadaddr := i_ex_data_addr;
        else
            wb_mbadaddr := r.hold_mbadaddr;
        end if;
        wb_trap_code := EXCEPTION_LoadFault;
    elsif i_ex_unalign_store = '1' then
        w_trap_valid := '1';
        wb_trap_pc := CFG_NMI_STORE_UNALIGNED_ADDR;
        wb_trap_code := EXCEPTION_StoreMisalign;
    elsif i_ex_data_store_fault = '1' or r.hold_data_store_fault = '1' then
        w_trap_valid := '1';
        v.hold_data_store_fault := '0';
        if i_trap_ready = '0' then
            v.hold_data_store_fault := '1';
        end if;
        wb_trap_pc := CFG_NMI_STORE_FAULT_ADDR;
        if i_ex_data_store_fault = '1' then
            wb_mbadaddr := i_ex_data_store_fault_addr;     -- miss-access write data address
            v.hold_mbadaddr := i_ex_data_store_fault_addr;
        else
            wb_mbadaddr := r.hold_mbadaddr;
        end if;
        wb_trap_code := EXCEPTION_StoreFault;
    elsif i_ex_ecall = '1' then
        w_trap_valid := '1';
        if r.mode = PRV_M then
            wb_trap_pc := CFG_NMI_CALL_FROM_MMODE_ADDR;
            wb_trap_code := EXCEPTION_CallFromMmode;
        else
            wb_trap_pc := CFG_NMI_CALL_FROM_UMODE_ADDR;
            wb_trap_code := EXCEPTION_CallFromUmode;
        end if;
    elsif r.mstackovr_ena = '1' and w_mstackovr = '1' then
        w_trap_valid := '1';
        wb_trap_pc := CFG_NMI_STACK_OVERFLOW_ADDR;
        wb_trap_code := EXCEPTION_StackOverflow;
        if i_trap_ready = '1' then
            v.mstackovr := (others => '0');
            v.mstackovr_ena := '0';
        end if;
    elsif r.mstackund_ena = '1' and w_mstackund = '1' then
        w_trap_valid := '1';
        wb_trap_pc := CFG_NMI_STACK_UNDERFLOW_ADDR;
        wb_trap_code := EXCEPTION_StackUnderflow;
        if i_trap_ready = '1' then
            v.mstackund := (others => '0');
            v.mstackund_ena := '0';
        end if;
    elsif w_ext_irq = '1' and r.ext_irq = '0' then
        w_trap_valid := '1';
        wb_trap_pc := r.mtvec(CFG_CPU_ADDR_BITS-1 downto 0);
        wb_trap_code := INTERRUPT_MExternal;
        w_trap_irq := '1';
    end if;

    if (not w_exception_xret and (i_mret or i_uret)) = '1' then
        -- Switch to previous mode
        v.mie := r.mpie;
        v.mpie := '1';
        v.mode := r.mpp;
        v.mpp := PRV_U;
    end if;

    -- Behaviour on EBREAK instruction defined by 'i_break_mode':
    --     0 = halt;
    --     1 = generate trap
    if (w_trap_valid and i_trap_ready and (r.break_mode or not i_ex_breakpoint)) = '1' then
        v.mie := '0';
        v.mpp := r.mode;
        v.mepc := i_ex_npc;
        v.mbadaddr := wb_mbadaddr;
        v.trap_code := wb_trap_code;
        v.trap_irq := w_trap_irq;
        v.mode := PRV_M;
        case r.mode is
        when PRV_U =>
            v.mpie := r.uie;
        when PRV_M =>
            v.mpie := r.mie;
        when others =>
        end case;
    end if;

    if r.halt = '0' or i_e_next_ready = '1' then
        v.cycle_cnt := r.cycle_cnt + 1;
    end if;
    if i_e_next_ready = '1' then
        v.executed_cnt := r.executed_cnt + 1;
    end if;
    v.timer := r.timer + 1;

    if i_e_next_ready = '1' then
        if r.progbuf_ena = '1' then
            v.progbuf_data_out := r.progbuf_data(tnpc + 31 downto tnpc);
            v.progbuf_data_pc := r.progbuf_data_npc;
            if r.progbuf_data(tnpc + 1 downto tnpc) = "11" then
                v.progbuf_data_npc := r.progbuf_data_npc + 2;
            else 
                v.progbuf_data_npc := r.progbuf_data_npc + 1;
            end if;
            if and_reduce(r.progbuf_data_pc(4 downto 1)) = '1' then
                -- use end of buffer as a watchdog
                v.progbuf_ena := '0';
                v.halt        := '1';
            end if;
        elsif or_reduce(r.stepping_mode_cnt) = '1' then
            v.stepping_mode_cnt := r.stepping_mode_cnt - 1;
            if or_reduce(r.stepping_mode_cnt(RISCV_ARCH-1 downto 1)) = '0' then
                v.halt := '1';
                v_cur_halt := '1';
                v.stepping_mode := '0';
                v.halt_cause := HALT_CAUSE_STEP;
            end if;
        end if;
    end if;

    if r.break_event = '1' then
        if r.progbuf_ena = '1' then
            v.halt := '1';  -- do not modify halt cause in debug mode
            v.progbuf_ena := '0';
        else
            if r.break_mode = '0' then
                v.halt := '1';
                v.halt_cause := HALT_CAUSE_EBREAK;
            end if;
        end if;
    elsif v_req_halt = '1' and r.halt = '0' then
        if r.progbuf_ena = '0' and r.stepping_mode = '0' then
            v.halt := '1';
            v.halt_cause := HALT_CAUSE_HALTREQ;
        end if;
    elsif v_req_progbuf = '1' then
        v.progbuf_ena := '1';
        v.progbuf_data_out := r.progbuf_data(31 downto 0);
        v.progbuf_data_pc := (others => '0');
        if r.progbuf_data(1 downto 0) = "11" then
            v.progbuf_data_npc := "00010";
        else
            v.progbuf_data_npc := "00001";
        end if;
        v.halt := '0';
    elsif v_req_resume = '1' and r.halt = '1' then
        v.halt := '0';
    end if;

    if v_clear_progbuferr = '1' then
        v.progbuf_err := PROGBUF_ERR_NONE;
    elsif r.progbuf_ena = '1' then
        if i_ex_data_load_fault = '1'
            or i_ex_data_store_fault = '1' then
            v.progbuf_err := PROGBUF_ERR_EXCEPTION;
        elsif i_ex_unalign_store = '1'
                or i_ex_unalign_load = '1'
                or i_ex_mpu_store = '1'
                or i_ex_mpu_load = '1' then
            v.progbuf_err := PROGBUF_ERR_BUS;
        end if;
    end if;


    if not async_reset and i_nrst = '0' then
        v := R_RESET;
    end if;

    o_executed_cnt <= r.executed_cnt;
    o_trap_valid <= w_trap_valid;
    o_trap_pc <= wb_trap_pc;
    o_dbg_pc_write <= v_dbg_pc_write;
    o_dbg_pc <= vb_dbg_pc;
    o_rdata <= vb_rdata;
    o_mepc <= r.mepc;
    o_uepc <= r.uepc;
    o_dport_valid <= v_dport_valid;
    o_dport_rdata <= vb_rdata;
    o_mpu_region_we <= r.mpu_we;
    o_mpu_region_idx <= r.mpu_idx;
    o_mpu_region_addr <= r.mpu_addr;
    o_mpu_region_mask <= r.mpu_mask;
    o_mpu_region_flags <= r.mpu_flags;
    o_progbuf_ena <= r.progbuf_ena;
    o_progbuf_pc <= X"000000" & "00" & r.progbuf_data_pc & '0';
    o_progbuf_data <= r.progbuf_data_out;
    o_flushi_ena <= r.flushi_ena;
    o_flushi_addr <= r.flushi_addr;
    o_halt <= r.halt or v_cur_halt;
    
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
