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


entity Processor is
  generic (
    hartid : integer;
    async_reset : boolean;
    fpu_ena : boolean;
    tracer_ena : boolean
  );
  port (
    i_clk : in std_logic;                                             -- CPU clock
    i_nrst : in std_logic;                                            -- Reset. Active LOW.
    -- Control path:
    i_req_ctrl_ready : in std_logic;                                  -- ICache is ready to accept request
    o_req_ctrl_valid : out std_logic;                                 -- Request to ICache is valid
    o_req_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Requesting address to ICache
    i_resp_ctrl_valid : in std_logic;                                 -- ICache response is valid
    i_resp_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Response address must be equal to the latest request address
    i_resp_ctrl_data : in std_logic_vector(31 downto 0);              -- Read value
    i_resp_ctrl_load_fault : in std_logic;                            -- bus response with error
    i_resp_ctrl_executable : in std_logic;
    o_resp_ctrl_ready : out std_logic;
    -- Data path:
    i_req_data_ready : in std_logic;                                  -- DCache is ready to accept request
    o_req_data_valid : out std_logic;                                 -- Request to DCache is valid
    o_req_data_write : out std_logic;                                 -- Read/Write transaction
    o_req_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Requesting address to DCache
    o_req_data_wdata : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0); -- Writing value
    o_req_data_wstrb : out std_logic_vector(BUS_DATA_BYTES-1 downto 0); -- 8-bytes aligned strobs
    i_resp_data_valid : in std_logic;                                 -- DCache response is valid
    i_resp_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- DCache response address must be equal to the latest request address
    i_resp_data_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);    -- Read value
    i_resp_data_store_fault_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_data_load_fault : in std_logic;                            -- Bus response with SLVERR or DECERR on read
    i_resp_data_store_fault : in std_logic;                           -- Bus response with SLVERR or DECERR on write
    i_resp_data_er_mpu_load : in std_logic;
    i_resp_data_er_mpu_store : in std_logic;
    o_resp_data_ready : out std_logic;
    -- External interrupt pin
    i_ext_irq : in std_logic;                                         -- PLIC interrupt accordingly with spec
    o_time : out std_logic_vector(63 downto 0);                       -- Timer in clock except halt state
    o_exec_cnt : out std_logic_vector(63 downto 0);
    -- MPU interface
    o_mpu_region_we : out std_logic;
    o_mpu_region_idx : out std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    o_mpu_region_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_mpu_region_mask : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_mpu_region_flags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);  -- {ena, cachable, r, w, x}
    -- Debug interface:
    i_dport_valid : in std_logic;                                     -- Debug access from DSU is valid
    i_dport_write : in std_logic;                                     -- Write command flag
    i_dport_region : in std_logic_vector(1 downto 0);                 -- Registers region ID: 0=CSR; 1=IREGS; 2=Control
    i_dport_addr : in std_logic_vector(11 downto 0);                  -- Register idx
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);       -- Write value
    o_dport_ready : out std_logic;                                    -- Response is ready
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);      -- Response value
    o_halted : out std_logic;
    -- Debug signals:
    o_flush_address : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Address of instruction to remove from ICache
    o_flush_valid : out std_logic;                                    -- Remove address from ICache is valid
    o_data_flush_address : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);    -- Address of instruction to remove from D$
    o_data_flush_valid : out std_logic;                               -- Remove address from D$ is valid
    i_istate : in std_logic_vector(3 downto 0);                       -- ICache state machine value
    i_dstate : in std_logic_vector(3 downto 0);                       -- DCache state machine value
    i_cstate : in std_logic_vector(1 downto 0)                        -- CacheTop state machine value
  );
end; 
 
architecture arch_Processor of Processor is

    type FetchType is record
        req_fire : std_logic;
        instr_load_fault : std_logic;
        instr_executable : std_logic;
        valid : std_logic;
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        instr : std_logic_vector(31 downto 0);
        imem_req_valid : std_logic;
        imem_req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        pipeline_hold : std_logic;
    end record;

    type InstructionDecodeType is record
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        instr : std_logic_vector(31 downto 0);
        instr_valid : std_logic;
        memop_store : std_logic;
        memop_load : std_logic;
        memop_sign_ext : std_logic;
        memop_size : std_logic_vector(1 downto 0);
        rv32 : std_logic;                                    -- 32-bits instruction
        compressed : std_logic;                              -- C-extension
        f64 : std_logic;                                     -- D-extension (FPU)
        unsigned_op : std_logic;                             -- Unsigned operands
        isa_type : std_logic_vector(ISA_Total-1 downto 0);
        instr_vec : std_logic_vector(Instr_Total-1 downto 0);
        exception : std_logic;
        instr_load_fault : std_logic;
        instr_executable : std_logic;
        radr1 : std_logic_vector(5 downto 0);
        radr2 : std_logic_vector(5 downto 0);
        waddr : std_logic_vector(5 downto 0);
        imm : std_logic_vector(RISCV_ARCH-1 downto 0);
    end record;

    type ExecuteType is record
        trap_ready : std_logic;
        valid : std_logic;
        instr : std_logic_vector(31 downto 0);
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        ex_npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);

        res_addr : std_logic_vector(5 downto 0);
        res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
        mret : std_logic;
        uret : std_logic;
        csr_addr : std_logic_vector(11 downto 0);
        csr_wena : std_logic;
        csr_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
        ex_instr_load_fault : std_logic;
        ex_instr_not_executable : std_logic;
        ex_illegal_instr : std_logic;
        ex_unalign_load : std_logic;
        ex_unalign_store : std_logic;
        ex_breakpoint : std_logic;
        ex_ecall : std_logic;
        ex_fpu_invalidop : std_logic;            -- FPU Exception: invalid operation
        ex_fpu_divbyzero : std_logic;            -- FPU Exception: divide by zero
        ex_fpu_overflow : std_logic;             -- FPU Exception: overflow
        ex_fpu_underflow : std_logic;            -- FPU Exception: underflow
        ex_fpu_inexact : std_logic;              -- FPU Exception: inexact
        fpu_valid : std_logic;

        memop_sign_ext : std_logic;
        memop_load : std_logic;
        memop_store : std_logic;
        memop_size : std_logic_vector(1 downto 0);
        memop_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        d_ready : std_logic;                     -- Hold pipeline from Execution stage
        fence : std_logic;                       -- instruction FENCE
        fencei : std_logic;                      -- instruction FENCE.I
        call : std_logic;
        ret : std_logic;
    end record;

    type MemoryType is record
        memop_ready : std_logic;
        valid : std_logic;
        instr : std_logic_vector(31 downto 0);
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        pipeline_hold : std_logic;
        wb_memop : std_logic;
    end record;

    type WriteBackType is record
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        wena : std_logic;
        waddr : std_logic_vector(5 downto 0);
        wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    end record;

    type IntRegsType is record
        rdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
        rdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);
        dport_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
        ra : std_logic_vector(RISCV_ARCH-1 downto 0);       -- Return address
        sp : std_logic_vector(RISCV_ARCH-1 downto 0);       -- Stack pointer
    end record;

    type FloatRegsType is record
        rdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
        rdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);
        dport_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    end record;

    type CsrType is record
        rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
        dport_rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
        trap_valid : std_logic;
        trap_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        break_event : std_logic;
    end record;

    --! 5-stages CPU pipeline
    type PipelineType is record
        f : FetchType;                            -- Fetch instruction stage
        d : InstructionDecodeType;                -- Decode instruction stage
        e : ExecuteType;                          -- Execute instruction
        m : MemoryType;                           -- Memory load/store
        w : WriteBackType;                        -- Write back registers value
    end record;

    type DebugType is record
        core_addr : std_logic_vector(11 downto 0);           -- Address of the sub-region register
        core_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);-- Write data
        csr_ena : std_logic;                                 -- Region 0: Access to CSR bank is enabled.
        csr_write : std_logic;                               -- Region 0: CSR write enable
        ireg_ena : std_logic;                                -- Region 1: Access to integer register bank is enabled
        ireg_write : std_logic;                              -- Region 1: Integer registers bank write pulse
        freg_ena : std_logic;                                -- Region 1: Access to float register bank is enabled
        freg_write : std_logic;                              -- Region 1: Float registers bank write pulse
        npc_write : std_logic;                               -- Region 1: npc write enable
        halt : std_logic;                                    -- Halt signal is equal to hold pipeline
        clock_cnt : std_logic_vector(63 downto 0);           -- Number of clocks excluding halt state
        executed_cnt : std_logic_vector(63 downto 0);        -- Number of executed instruction
        break_mode : std_logic;                              -- Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
        br_fetch_valid : std_logic;                          -- Fetch injection address/instr are valid
        br_address_fetch : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0); -- Fetch injection address to skip ebreak instruciton only once
        br_instr_fetch : std_logic_vector(31 downto 0);      -- Real instruction value that was replaced by ebreak
        flush_address : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Address of instruction to remove from ICache
        flush_valid : std_logic;                                    -- Remove address from ICache is valid
    end record;

    type BranchPredictorType is record
       npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    end record;

    signal ireg : IntRegsType;
    signal freg : FloatRegsType;
    signal csr : CsrType;
    signal w : PipelineType;
    signal dbg : DebugType;
    signal bp : BranchPredictorType;

    signal wb_ireg_dport_addr : std_logic_vector(4 downto 0);
    signal wb_freg_dport_addr : std_logic_vector(4 downto 0);
    signal wb_exec_dport_npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    
    signal w_fetch_pipeline_hold : std_logic;
    signal w_any_pipeline_hold : std_logic;

begin

    w_fetch_pipeline_hold <= not w.e.d_ready or w.m.pipeline_hold or dbg.halt;
    w_any_pipeline_hold <= w.f.pipeline_hold or not w.e.d_ready
                          or w.m.pipeline_hold or dbg.halt;

    wb_ireg_dport_addr <= dbg.core_addr(4 downto 0);
    wb_freg_dport_addr <= dbg.core_addr(4 downto 0);
    wb_exec_dport_npc <= dbg.core_wdata(BUS_ADDR_WIDTH-1 downto 0);

    o_req_ctrl_valid <= w.f.imem_req_valid;
    o_req_ctrl_addr <= w.f.imem_req_addr;
    o_time <= dbg.clock_cnt;
    o_exec_cnt <= dbg.executed_cnt;

    o_flush_valid <= w.e.fencei or dbg.flush_valid or csr.break_event;
    o_flush_address <= (others => '1') when w.e.fencei = '1'
                                       else w.e.npc when csr.break_event = '1'
                                       else dbg.flush_address;

    o_data_flush_address <= (others => '0');
    o_data_flush_valid <= '0';

    o_halted <= dbg.halt;

    
    fetch0 : InstrFetch generic map (
        async_reset => async_reset
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_pipeline_hold => w_fetch_pipeline_hold,
        i_mem_req_ready => i_req_ctrl_ready,
        o_mem_addr_valid => w.f.imem_req_valid,
        o_mem_addr => w.f.imem_req_addr,
        i_mem_data_valid => i_resp_ctrl_valid,
        i_mem_data_addr => i_resp_ctrl_addr,
        i_mem_data => i_resp_ctrl_data,
        i_mem_load_fault => i_resp_ctrl_load_fault,
        i_mem_executable => i_resp_ctrl_executable,
        o_mem_resp_ready => o_resp_ctrl_ready,
        i_e_fencei => w.e.fencei,
        i_predict_npc => bp.npc,
        o_mem_req_fire => w.f.req_fire,
        o_instr_load_fault => w.f.instr_load_fault,
        o_instr_executable => w.f.instr_executable,
        o_valid => w.f.valid,
        o_pc => w.f.pc,
        o_instr => w.f.instr,
        o_hold => w.f.pipeline_hold,
        i_br_fetch_valid => dbg.br_fetch_valid,
        i_br_address_fetch => dbg.br_address_fetch,
        i_br_instr_fetch => dbg.br_instr_fetch);
        
    dec0 : InstrDecoder generic map (
        async_reset => async_reset,
        fpu_ena => fpu_ena
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_any_hold => w_any_pipeline_hold,
        i_f_valid => w.f.valid,
        i_f_pc => w.f.pc,
        i_f_instr => w.f.instr,
        i_instr_load_fault => w.f.instr_load_fault,
        i_instr_executable => w.f.instr_executable,
        o_radr1 => w.d.radr1,
        o_radr2 => w.d.radr2,
        o_waddr => w.d.waddr,
        o_imm => w.d.imm,
        i_e_ready => w.e.d_ready,
        i_e_fencei => w.e.fencei,
        o_valid => w.d.instr_valid,
        o_pc => w.d.pc,
        o_instr => w.d.instr,
        o_memop_store => w.d.memop_store,
        o_memop_load => w.d.memop_load,
        o_memop_sign_ext => w.d.memop_sign_ext,
        o_memop_size => w.d.memop_size,
        o_unsigned_op => w.d.unsigned_op,
        o_rv32 => w.d.rv32,
        o_compressed => w.d.compressed,
        o_f64 => w.d.f64,
        o_isa_type => w.d.isa_type,
        o_instr_vec => w.d.instr_vec,
        o_exception => w.d.exception,
        o_instr_load_fault => w.d.instr_load_fault,
        o_instr_executable => w.d.instr_executable);

    exec0 : InstrExecute generic map (
        async_reset => async_reset,
        fpu_ena => fpu_ena
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_d_valid => w.d.instr_valid,
        i_d_pc => w.d.pc,
        i_d_instr => w.d.instr,
        i_d_radr1 => w.d.radr1,
        i_d_radr2 => w.d.radr2,
        i_d_waddr => w.d.waddr,
        i_d_imm => w.d.imm,
        i_wb_valid => w.m.valid,
        i_wb_waddr => w.w.waddr,
        i_memop_store => w.d.memop_store,
        i_memop_load => w.d.memop_load,
        i_memop_sign_ext => w.d.memop_sign_ext,
        i_memop_size => w.d.memop_size,
        i_unsigned_op => w.d.unsigned_op,
        i_rv32 => w.d.rv32,
        i_compressed => w.d.compressed,
        i_f64 => w.d.f64,
        i_isa_type => w.d.isa_type,
        i_ivec => w.d.instr_vec,
        i_unsup_exception => w.d.exception,
        i_instr_load_fault => w.d.instr_load_fault,
        i_instr_executable => w.d.instr_executable,
        i_dport_npc_write => dbg.npc_write,
        i_dport_npc => wb_exec_dport_npc,
        i_rdata1 => ireg.rdata1,
        i_rdata2 => ireg.rdata2,
        i_rfdata1 => freg.rdata1,
        i_rfdata2 => freg.rdata2,
        o_res_addr => w.e.res_addr,
        o_res_data => w.e.res_data,
        o_d_ready => w.e.d_ready,
        o_csr_addr => w.e.csr_addr,
        o_csr_wena => w.e.csr_wena,
        i_csr_rdata => csr.rdata,
        o_csr_wdata => w.e.csr_wdata,
        i_trap_valid => csr.trap_valid,
        i_trap_pc => csr.trap_pc,
        o_ex_npc => w.e.ex_npc,
        o_ex_instr_load_fault => w.e.ex_instr_load_fault,
        o_ex_instr_not_executable => w.e.ex_instr_not_executable,
        o_ex_illegal_instr => w.e.ex_illegal_instr,
        o_ex_unalign_store => w.e.ex_unalign_store,
        o_ex_unalign_load => w.e.ex_unalign_load,
        o_ex_breakpoint => w.e.ex_breakpoint,
        o_ex_ecall => w.e.ex_ecall,
        o_ex_fpu_invalidop => w.e.ex_fpu_invalidop,
        o_ex_fpu_divbyzero => w.e.ex_fpu_divbyzero,
        o_ex_fpu_overflow => w.e.ex_fpu_overflow,
        o_ex_fpu_underflow => w.e.ex_fpu_underflow,
        o_ex_fpu_inexact => w.e.ex_fpu_inexact,
        o_fpu_valid => w.e.fpu_valid,
        o_memop_sign_ext => w.e.memop_sign_ext,
        o_memop_load => w.e.memop_load,
        o_memop_store => w.e.memop_store,
        o_memop_size => w.e.memop_size,
        o_memop_addr => w.e.memop_addr,
        i_memop_ready => w.m.memop_ready,
        o_trap_ready => w.e.trap_ready,
        o_valid => w.e.valid,
        o_pc => w.e.pc,
        o_npc => w.e.npc,
        o_instr => w.e.instr,
        o_fence => w.e.fence,
        o_fencei => w.e.fencei,
        o_call => w.e.call,
        o_ret => w.e.ret,
        o_mret => w.e.mret,
        o_uret => w.e.uret);

    mem0 : MemAccess generic map (
        async_reset => async_reset
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_e_valid => w.e.valid,
        i_e_pc => w.e.pc,
        i_e_instr => w.e.instr,
        i_res_addr => w.e.res_addr,
        i_res_data => w.e.res_data,
        i_memop_sign_ext => w.e.memop_sign_ext,
        i_memop_load => w.e.memop_load,
        i_memop_store => w.e.memop_store,
        i_memop_size => w.e.memop_size,
        i_memop_addr => w.e.memop_addr,
        o_memop_ready => w.m.memop_ready,
        o_waddr => w.w.waddr,
        o_wena => w.w.wena,
        o_wdata => w.w.wdata,
        i_mem_req_ready => i_req_data_ready,
        o_mem_valid => o_req_data_valid,
        o_mem_write => o_req_data_write,
        o_mem_addr => o_req_data_addr,
        o_mem_wdata => o_req_data_wdata,
        o_mem_wstrb => o_req_data_wstrb,
        i_mem_data_valid => i_resp_data_valid,
        i_mem_data_addr => i_resp_data_addr,
        i_mem_data => i_resp_data_data,
        o_mem_resp_ready => o_resp_data_ready,
        o_hold => w.m.pipeline_hold,
        o_valid => w.m.valid,
        o_pc => w.m.pc,
        o_instr => w.m.instr,
        o_wb_memop => w.m.wb_memop);

    predic0 : BranchPredictor generic map (
        async_reset => async_reset
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_req_mem_fire => w.f.req_fire,
        i_resp_mem_valid => i_resp_ctrl_valid,
        i_resp_mem_addr => i_resp_ctrl_addr,
        i_resp_mem_data => i_resp_ctrl_data,
        i_e_npc => w.e.npc,
        i_ra => ireg.ra,
        o_npc_predict => bp.npc);


    iregs0 : RegIntBank generic map (
        async_reset => async_reset
      ) port map ( 
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_radr1 => w.d.radr1,
        o_rdata1 => ireg.rdata1,
        i_radr2 => w.d.radr2,
        o_rdata2 => ireg.rdata2,
        i_waddr => w.w.waddr,
        i_wena => w.w.wena,
        i_wdata => w.w.wdata,
        i_dport_addr => wb_ireg_dport_addr,
        i_dport_ena => dbg.ireg_ena,
        i_dport_write => dbg.ireg_write,
        i_dport_wdata => dbg.core_wdata,
        o_dport_rdata => ireg.dport_rdata,
        o_ra => ireg.ra,   -- Return address
        o_sp => ireg.sp);

    fpuena : if CFG_HW_FPU_ENABLE generate
      fregs0 : RegFloatBank generic map (
        async_reset => async_reset
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_radr1 => w.d.radr1,
        o_rdata1 => freg.rdata1,
        i_radr2 => w.d.radr2,
        o_rdata2 => freg.rdata2,
        i_waddr => w.w.waddr,
        i_wena => w.w.wena,
        i_wdata => w.w.wdata,
        i_dport_addr => wb_freg_dport_addr,
        i_dport_ena => dbg.freg_ena,
        i_dport_write => dbg.freg_write,
        i_dport_wdata => dbg.core_wdata,
        o_dport_rdata => freg.dport_rdata);
    end generate;

    fpudis : if not CFG_HW_FPU_ENABLE generate
        freg.rdata1 <= (others => '0');
        freg.rdata2 <= (others => '0');
        freg.dport_rdata <= (others => '0');
    end generate;

    csr0 : CsrRegs generic map (
        hartid => hartid,
        async_reset => async_reset
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_mret => w.e.mret,
        i_uret => w.e.uret,
        i_sp => ireg.sp,
        i_addr => w.e.csr_addr,
        i_wena => w.e.csr_wena,
        i_wdata => w.e.csr_wdata,
        o_rdata => csr.rdata,
        i_trap_ready => w.e.trap_ready,
        i_ex_pc => w.e.npc,
        i_ex_npc => w.e.ex_npc,
        i_ex_data_addr => i_resp_data_addr,
        i_ex_data_load_fault => i_resp_data_load_fault,
        i_ex_data_store_fault => i_resp_data_store_fault,
        i_ex_data_store_fault_addr => i_resp_data_store_fault_addr,
        i_ex_instr_load_fault => w.e.ex_instr_load_fault,
        i_ex_illegal_instr => w.e.ex_illegal_instr,
        i_ex_unalign_store => w.e.ex_unalign_store,
        i_ex_unalign_load => w.e.ex_unalign_load,
        i_ex_breakpoint => w.e.ex_breakpoint,
        i_ex_ecall => w.e.ex_ecall,
        i_ex_fpu_invalidop => w.e.ex_fpu_invalidop,
        i_ex_fpu_divbyzero => w.e.ex_fpu_divbyzero,
        i_ex_fpu_overflow => w.e.ex_fpu_overflow,
        i_ex_fpu_underflow => w.e.ex_fpu_underflow,
        i_ex_fpu_inexact => w.e.ex_fpu_inexact,
        i_fpu_valid => w.e.fpu_valid,
        i_irq_external => i_ext_irq,
        o_trap_valid => csr.trap_valid,
        o_trap_pc => csr.trap_pc,
        i_break_mode => dbg.break_mode,
        o_break_event => csr.break_event,
        o_mpu_region_we => o_mpu_region_we,
        o_mpu_region_idx => o_mpu_region_idx,
        o_mpu_region_addr => o_mpu_region_addr,
        o_mpu_region_mask => o_mpu_region_mask,
        o_mpu_region_flags => o_mpu_region_flags,
        i_dport_ena => dbg.csr_ena,
        i_dport_write => dbg.csr_write,
        i_dport_addr => dbg.core_addr,
        i_dport_wdata => dbg.core_wdata,
        o_dport_rdata => csr.dport_rdata);


    dbg0 : DbgPort generic map (
        async_reset => async_reset
    ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_dport_valid => i_dport_valid,
        i_dport_write => i_dport_write,
        i_dport_region => i_dport_region,
        i_dport_addr => i_dport_addr,
        i_dport_wdata => i_dport_wdata,
        o_dport_ready => o_dport_ready,
        o_dport_rdata => o_dport_rdata,
        o_core_addr => dbg.core_addr,
        o_core_wdata => dbg.core_wdata,
        o_csr_ena => dbg.csr_ena,
        o_csr_write => dbg.csr_write,
        i_csr_rdata => csr.dport_rdata,
        o_ireg_ena => dbg.ireg_ena,
        o_ireg_write => dbg.ireg_write,
        o_freg_ena => dbg.freg_ena,
        o_freg_write => dbg.freg_write,
        o_npc_write => dbg.npc_write,
        i_ireg_rdata => ireg.dport_rdata,
        i_freg_rdata => freg.dport_rdata,
        i_pc => w.e.pc,
        i_npc => w.e.npc,
        i_e_valid => w.e.valid,
        i_e_call => w.e.call,
        i_e_ret => w.e.ret,
        i_m_valid => w.m.valid,
        o_clock_cnt => dbg.clock_cnt,
        o_executed_cnt => dbg.executed_cnt,
        o_halt => dbg.halt,
        i_ebreak => csr.break_event,
        o_break_mode => dbg.break_mode,
        o_br_fetch_valid => dbg.br_fetch_valid,
        o_br_address_fetch => dbg.br_address_fetch,
        o_br_instr_fetch => dbg.br_instr_fetch,
        o_flush_address => dbg.flush_address,
        o_flush_valid => dbg.flush_valid,
        i_istate => i_istate,
        i_dstate => i_dstate,
        i_cstate => i_cstate);

end;
