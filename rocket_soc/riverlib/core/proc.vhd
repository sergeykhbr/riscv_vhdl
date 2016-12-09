-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     CPU pipeline implementation.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;


entity Processor is
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
    o_resp_ctrl_ready : out std_logic;
    -- Data path:
    i_req_data_ready : in std_logic;                                  -- DCache is ready to accept request
    o_req_data_valid : out std_logic;                                 -- Request to DCache is valid
    o_req_data_write : out std_logic;                                 -- Read/Write transaction
    o_req_data_size : out std_logic_vector(1 downto 0);               -- Size [Bytes]: 0=1B; 1=2B; 2=4B; 3=8B
    o_req_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Requesting address to DCache
    o_req_data_data : out std_logic_vector(RISCV_ARCH-1 downto 0);    -- Writing value
    i_resp_data_valid : in std_logic;                                 -- DCache response is valid
    i_resp_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- DCache response address must be equal to the latest request address
    i_resp_data_data : in std_logic_vector(RISCV_ARCH-1 downto 0);    -- Read value
    o_resp_data_ready : out std_logic;
    -- External interrupt pin
    i_ext_irq : in std_logic;                                         -- PLIC interrupt accordingly with spec
    o_step_cnt : out std_logic_vector(63 downto 0)                    -- Number of valid executed instructions
  );
end; 
 
architecture arch_Processor of Processor is

    type FetchType is record
        req_fire : std_logic;
        valid : std_logic;
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        instr : std_logic_vector(31 downto 0);
        imem_req_valid : std_logic;
        imem_req_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        predict_miss : std_logic;
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
        unsigned_op : std_logic;                             -- Unsigned operands
        isa_type : std_logic_vector(ISA_Total-1 downto 0);
        instr_vec : std_logic_vector(Instr_Total-1 downto 0);
        exception : std_logic;
    end record;

    type ExecuteType is record
        valid : std_logic;
        instr : std_logic_vector(31 downto 0);
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        npc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);

        radr1 : std_logic_vector(4 downto 0);
        radr2 : std_logic_vector(4 downto 0);
        res_addr : std_logic_vector(4 downto 0);
        res_data : std_logic_vector(RISCV_ARCH-1 downto 0);
        trap_ena : std_logic;                                 -- Trap pulse
        trap_code : std_logic_vector(4 downto 0);             -- bit[4] : 1=interrupt; 0=exception; bits[3:0]=code
        trap_pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- trap on pc
        xret : std_logic;
        csr_addr : std_logic_vector(11 downto 0);
        csr_wena : std_logic;
        csr_wdata : std_logic_vector(RISCV_ARCH-1 downto 0);

        memop_sign_ext : std_logic;
        memop_load : std_logic;
        memop_store : std_logic;
        memop_size : std_logic_vector(1 downto 0);
        memop_addr : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        pipeline_hold : std_logic;                            -- Hold pipeline from Execution stage
    end record;

    type MemoryType is record
        valid : std_logic;
        instr : std_logic_vector(31 downto 0);
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        step_cnt : std_logic_vector(63 downto 0);
        pipeline_hold : std_logic;
    end record;

    type WriteBackType is record
        pc : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
        wena : std_logic;
        waddr : std_logic_vector(4 downto 0);
        wdata : std_logic_vector(RISCV_ARCH-1 downto 0);
    end record;

    type IntRegsType is record
        rdata1 : std_logic_vector(RISCV_ARCH-1 downto 0);
        rdata2 : std_logic_vector(RISCV_ARCH-1 downto 0);
        ra : std_logic_vector(RISCV_ARCH-1 downto 0);       -- Return address
    end record;

    type CsrType is record
        rdata : std_logic_vector(RISCV_ARCH-1 downto 0);
        ie : std_logic;                                     -- Interrupt enable bit
        mtvec : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);-- Interrupt descriptor table
        mode : std_logic_vector(1 downto 0);                -- Current processor mode
    end record;

    --! 5-stages CPU pipeline
    type PipelineType is record
        f : FetchType;                            -- Fetch instruction stage
        d : InstructionDecodeType;                -- Decode instruction stage
        e : ExecuteType;                          -- Execute instruction
        m : MemoryType;                           -- Memory load/store
        w : WriteBackType;                        -- Write back registers value
    end record;

    type RegistersType is record
      clk_cnt : std_logic_vector(63 downto 0);
    end record;

    signal ireg : IntRegsType;
    signal csr : CsrType;
    signal w : PipelineType;
    signal r, rin : RegistersType;

    signal wb_npc_predict : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    
    signal w_fetch_pipeline_hold : std_logic;
    signal w_any_pipeline_hold : std_logic;
    signal w_exec_pipeline_hold : std_logic;

begin

    w_fetch_pipeline_hold <= w.e.pipeline_hold or w.m.pipeline_hold;
    w_any_pipeline_hold <= w.f.pipeline_hold or w.e.pipeline_hold or w.m.pipeline_hold;
    w_exec_pipeline_hold <= w.f.pipeline_hold or w.m.pipeline_hold;
    
    fetch0 : InstrFetch port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_pipeline_hold => w_fetch_pipeline_hold,
        i_mem_req_ready => i_req_ctrl_ready,
        o_mem_addr_valid => w.f.imem_req_valid,
        o_mem_addr => w.f.imem_req_addr,
        i_mem_data_valid => i_resp_ctrl_valid,
        i_mem_data_addr => i_resp_ctrl_addr,
        i_mem_data => i_resp_ctrl_data,
        o_mem_resp_ready => o_resp_ctrl_ready,
        i_e_npc_valid => w.e.valid,
        i_e_npc => w.e.npc,
        i_predict_npc => wb_npc_predict,
        o_predict_miss => w.f.predict_miss,
        o_mem_req_fire => w.f.req_fire,
        o_valid => w.f.valid,
        o_pc => w.f.pc,
        o_instr => w.f.instr,
        o_hold => w.f.pipeline_hold);
        
    dec0 : InstrDecoder port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_any_hold => w_any_pipeline_hold,
        i_f_valid => w.f.valid,
        i_f_pc => w.f.pc,
        i_f_instr => w.f.instr,
        o_valid => w.d.instr_valid,
        o_pc => w.d.pc,
        o_instr => w.d.instr,
        o_memop_store => w.d.memop_store,
        o_memop_load => w.d.memop_load,
        o_memop_sign_ext => w.d.memop_sign_ext,
        o_memop_size => w.d.memop_size,
        o_unsigned_op => w.d.unsigned_op,
        o_rv32 => w.d.rv32,
        o_isa_type => w.d.isa_type,
        o_instr_vec => w.d.instr_vec,
        o_exception => w.d.exception);

    exec0 : InstrExecute port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_pipeline_hold => w_exec_pipeline_hold,
        i_d_valid => w.d.instr_valid,
        i_d_pc => w.d.pc,
        i_d_instr => w.d.instr,
        i_wb_done => w.m.valid,
        i_memop_store => w.d.memop_store,
        i_memop_load => w.d.memop_load,
        i_memop_sign_ext => w.d.memop_sign_ext,
        i_memop_size => w.d.memop_size,
        i_unsigned_op => w.d.unsigned_op,
        i_rv32 => w.d.rv32,
        i_isa_type => w.d.isa_type,
        i_ivec => w.d.instr_vec,
        i_ie => csr.ie,
        i_mtvec => csr.mtvec,
        i_mode => csr.mode,
        i_unsup_exception => w.d.exception,
        i_ext_irq => i_ext_irq,
        o_radr1 => w.e.radr1,
        i_rdata1 => ireg.rdata1,
        o_radr2 => w.e.radr2,
        i_rdata2 => ireg.rdata2,
        o_res_addr => w.e.res_addr,
        o_res_data => w.e.res_data,
        o_pipeline_hold => w.e.pipeline_hold,
        o_xret => w.e.xret,
        o_csr_addr => w.e.csr_addr,
        o_csr_wena => w.e.csr_wena,
        i_csr_rdata => csr.rdata,
        o_csr_wdata => w.e.csr_wdata,
        o_trap_ena => w.e.trap_ena,
        o_trap_code => w.e.trap_code,
        o_trap_pc => w.e.trap_pc,
        o_memop_sign_ext => w.e.memop_sign_ext,
        o_memop_load => w.e.memop_load,
        o_memop_store => w.e.memop_store,
        o_memop_size => w.e.memop_size,
        o_memop_addr => w.e.memop_addr,
        o_valid => w.e.valid,
        o_pc => w.e.pc,
        o_npc => w.e.npc,
        o_instr => w.e.instr);

    mem0 : MemAccess port map (
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
        o_waddr => w.w.waddr,
        o_wena => w.w.wena,
        o_wdata => w.w.wdata,
        i_mem_req_ready => i_req_data_ready,
        o_mem_valid => o_req_data_valid,
        o_mem_write => o_req_data_write,
        o_mem_sz => o_req_data_size,
        o_mem_addr => o_req_data_addr,
        o_mem_data => o_req_data_data,
        i_mem_data_valid => i_resp_data_valid,
        i_mem_data_addr => i_resp_data_addr,
        i_mem_data => i_resp_data_data,
        o_mem_resp_ready => o_resp_data_ready,
        o_hold => w.m.pipeline_hold,
        o_valid => w.m.valid,
        o_pc => w.m.pc,
        o_instr => w.m.instr,
        o_step_cnt => w.m.step_cnt);

    predic0 : BranchPredictor port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_hold => w_any_pipeline_hold,
        i_req_mem_fire => w.f.req_fire,
        i_resp_mem_valid => i_resp_ctrl_valid,
        i_resp_mem_addr => i_resp_ctrl_addr,
        i_resp_mem_data => i_resp_ctrl_data,
        i_f_predic_miss => w.f.predict_miss,
        i_e_npc => w.e.npc,
        i_ra => ireg.ra,
        o_npc_predict => wb_npc_predict);


    iregs0 : RegIntBank port map ( 
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_radr1 => w.e.radr1,
        o_rdata1 => ireg.rdata1,
        i_radr2 => w.e.radr2,
        o_rdata2 => ireg.rdata2,
        i_waddr => w.w.waddr,
        i_wena => w.w.wena,
        i_wdata => w.w.wdata,
        o_ra => ireg.ra);   -- Return address

    csr0 : CsrRegs port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_xret => w.e.xret,
        i_addr => w.e.csr_addr,
        i_wena => w.e.csr_wena,
        i_wdata => w.e.csr_wdata,
        o_rdata => csr.rdata,
        i_trap_ena => w.e.trap_ena,
        i_trap_code => w.e.trap_code,
        i_trap_pc => w.e.trap_pc,
        o_ie => csr.ie,
        o_mode => csr.mode,
        o_mtvec => csr.mtvec);


    comb : process(i_nrst, r)
        variable v : RegistersType;
    begin
        v := r;
        v.clk_cnt := r.clk_cnt + 1;

        if i_nrst = '0' then
            v.clk_cnt := (others => '0');
        end if;
        rin <= v;
    end process;

    o_req_ctrl_valid <= w.f.imem_req_valid;
    o_req_ctrl_addr <= w.f.imem_req_addr;
    o_step_cnt <= w.m.step_cnt;

    -- registers:
    regs : process(i_clk)  begin 
       if rising_edge(i_clk) then 
          r <= rin;
       end if; 
    end process;

end;
