-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2016 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     "River" CPU internal configuration parameters that don't 
--!            depend of external bus.
-----------------------------------------------------------------------------

--! Standard library.
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;

--! @brief   Library global parameters.
package river_cfg is

  --! Architecture size difinition.
  constant RISCV_ARCH : integer := 64;

  --! @name System bus parameters
  --! @brief Constants specify AXI bus global settigns
  --! @{

  --! @brief   Address bus bit-size.
  constant BUS_ADDR_WIDTH : integer := 32;
  --! @brief   Data bus bit-size.
  constant BUS_DATA_WIDTH : integer := 64;
  --! @brief   Num of data bytes per transaction.
  constant BUS_DATA_BYTES : integer := BUS_DATA_WIDTH / 8;
  --! @}

  --! @name   Encoded Memory operation size values
  --! @{

  constant MEMOP_8B : std_logic_vector(1 downto 0) := "11";
  constant MEMOP_4B : std_logic_vector(1 downto 0) := "10";
  constant MEMOP_2B : std_logic_vector(1 downto 0) := "01";
  constant MEMOP_1B : std_logic_vector(1 downto 0) := "00";
  --! @}
  
  constant RESET_VECTOR : std_logic_vector(BUS_ADDR_WIDTH-1 downto 0) := X"00001000";

  --! @name   Integer Registers specified by ISA
  --! @{
    constant Reg_Zero : integer := 0;
    constant Reg_ra : integer := 1;       -- [1] Return address
    constant Reg_sp : integer := 2;       -- [2] Stack pointer
    constant Reg_gp : integer := 3;       -- [3] Global pointer
    constant Reg_tp : integer := 4;       -- [4] Thread pointer
    constant Reg_t0 : integer := 5;       -- [5] Temporaries 0 s3
    constant Reg_t1 : integer := 6;       -- [6] Temporaries 1 s4
    constant Reg_t2 : integer := 7;       -- [7] Temporaries 2 s5
    constant Reg_s0 : integer := 8;       -- [8] s0/fp Saved register/frame pointer
    constant Reg_s1 : integer := 9;       -- [9] Saved register 1
    constant Reg_a0 : integer := 10;      -- [10] Function argumentes 0
    constant Reg_a1 : integer := 11;      -- [11] Function argumentes 1
    constant Reg_a2 : integer := 12;      -- [12] Function argumentes 2
    constant Reg_a3 : integer := 13;      -- [13] Function argumentes 3
    constant Reg_a4 : integer := 14;      -- [14] Function argumentes 4
    constant Reg_a5 : integer := 15;      -- [15] Function argumentes 5
    constant Reg_a6 : integer := 16;      -- [16] Function argumentes 6
    constant Reg_a7 : integer := 17;      -- [17] Function argumentes 7
    constant Reg_s2 : integer := 18;      -- [18] Saved register 2
    constant Reg_s3 : integer := 19;      -- [19] Saved register 3
    constant Reg_s4 : integer := 20;      -- [20] Saved register 4
    constant Reg_s5 : integer := 21;      -- [21] Saved register 5
    constant Reg_s6 : integer := 22;      -- [22] Saved register 6
    constant Reg_s7 : integer := 23;      -- [23] Saved register 7
    constant Reg_s8 : integer := 24;      -- [24] Saved register 8
    constant Reg_s9 : integer := 25;      -- [25] Saved register 9
    constant Reg_s10 : integer := 26;     -- [26] Saved register 10
    constant Reg_s11 : integer := 27;     -- [27] Saved register 11
    constant Reg_t3 : integer := 28;      -- [28] 
    constant Reg_t4 : integer := 29;      -- [29] 
    constant Reg_t5 : integer := 30;      -- [30] 
    constant Reg_t6 : integer := 31;      -- [31] 
    constant Reg_Total : integer := 32;
  --! @}

  --! @name   Instruction formats specified by ISA specification
  --! @{
  constant ISA_R_type : integer := 0;
  constant ISA_I_type : integer := 1;
  constant ISA_S_type : integer := 2;
  constant ISA_SB_type : integer := 3;
  constant ISA_U_type : integer := 4;
  constant ISA_UJ_type : integer := 5;
  constant ISA_Total : integer := 6;
  --! @}


  --! @name   Implemented instruction list and its indexes
  --! @{
  constant Instr_ADD : integer := 0;
  constant Instr_ADDI : integer := 1;
  constant Instr_ADDIW : integer := 2;
  constant Instr_ADDW : integer := 3;
  constant Instr_AND : integer := 4;
  constant Instr_ANDI : integer := 5;
  constant Instr_AUIPC : integer := 6;
  constant Instr_BEQ : integer := 7;
  constant Instr_BGE : integer := 8;
  constant Instr_BGEU : integer := 9;
  constant Instr_BLT : integer := 10;
  constant Instr_BLTU : integer := 11;
  constant Instr_BNE : integer := 12;
  constant Instr_JAL : integer := 13;
  constant Instr_JALR : integer := 14;
  constant Instr_LB : integer := 15;
  constant Instr_LH : integer := 16;
  constant Instr_LW : integer := 17;
  constant Instr_LD : integer := 18;
  constant Instr_LBU : integer := 19;
  constant Instr_LHU : integer := 20;
  constant Instr_LWU : integer := 21;
  constant Instr_LUI : integer := 22;
  constant Instr_OR : integer := 23;
  constant Instr_ORI : integer := 24;
  constant Instr_SLLI : integer := 25;
  constant Instr_SLT : integer := 26;
  constant Instr_SLTI : integer := 27;
  constant Instr_SLTU : integer := 28;
  constant Instr_SLTIU : integer := 29;
  constant Instr_SLL : integer := 30;
  constant Instr_SLLW : integer := 31;
  constant Instr_SLLIW : integer := 32;
  constant Instr_SRA : integer := 33;
  constant Instr_SRAW : integer := 34;
  constant Instr_SRAI : integer := 35;
  constant Instr_SRAIW : integer := 36;
  constant Instr_SRL : integer := 37;
  constant Instr_SRLI : integer := 38;
  constant Instr_SRLIW : integer := 39;
  constant Instr_SRLW : integer := 40;
  constant Instr_SB : integer := 41;
  constant Instr_SH : integer := 42;
  constant Instr_SW : integer := 43;
  constant Instr_SD : integer := 44;
  constant Instr_SUB : integer := 45;
  constant Instr_SUBW : integer := 46;
  constant Instr_XOR : integer := 47;
  constant Instr_XORI : integer := 48;
  constant Instr_CSRRW : integer := 49;
  constant Instr_CSRRS : integer := 50;
  constant Instr_CSRRC : integer := 51;
  constant Instr_CSRRWI : integer := 52;
  constant Instr_CSRRCI : integer := 53;
  constant Instr_CSRRSI : integer := 54;
  constant Instr_URET : integer := 55;
  constant Instr_SRET : integer := 56;
  constant Instr_HRET : integer := 57;
  constant Instr_MRET : integer := 58;
  constant Instr_FENCE : integer := 59;
  constant Instr_FENCE_I : integer := 60;
  constant Instr_DIV : integer := 61;
  constant Instr_DIVU : integer := 62;
  constant Instr_DIVW : integer := 63;
  constant Instr_DIVUW : integer := 64;
  constant Instr_MUL : integer := 65;
  constant Instr_MULW : integer := 66;
  constant Instr_REM : integer := 67;
  constant Instr_REMU : integer := 68;
  constant Instr_REMW : integer := 69;
  constant Instr_REMUW : integer := 70;
  constant Instr_ECALL : integer := 71;
  constant Instr_EBREAK : integer := 72;
  constant Instr_Total : integer := 73;
  --! @}

  --! @name PRV bits possible values:
  --!
  --! @{
  --! User-mode
  constant PRV_U : std_logic_vector(1 downto 0) := "00";
  --! super-visor mode
  constant PRV_S : std_logic_vector(1 downto 0) := "01";
  --! hyper-visor mode
  constant PRV_H : std_logic_vector(1 downto 0) := "10";
  --! machine mode
  constant PRV_M : std_logic_vector(1 downto 0) := "11";
  --! @}


  --! @name CSR registers.
  --!
  --! @{
  
  -- ISA and extensions supported.
  constant CSR_misa              : std_logic_vector(11 downto 0) := X"f10";
  -- Vendor ID.
  constant CSR_mvendorid         : std_logic_vector(11 downto 0) := X"f11";
  -- Architecture ID.
  constant CSR_marchid           : std_logic_vector(11 downto 0) := X"f12";
  -- Vendor ID.
  constant CSR_mimplementationid : std_logic_vector(11 downto 0) := X"f13";
  -- Thread id (the same as core).
  constant CSR_mhartid           : std_logic_vector(11 downto 0) := X"f14";
  -- Machine wall-clock time
  constant CSR_mtime         : std_logic_vector(11 downto 0) := X"701";
  -- Software reset.
  constant CSR_mreset        : std_logic_vector(11 downto 0) := X"782";

  -- machine mode status read/write register.
  constant CSR_mstatus       : std_logic_vector(11 downto 0) := X"300";
  -- Machine exception delegation
  constant CSR_medeleg       : std_logic_vector(11 downto 0) := X"302";
  -- Machine interrupt delegation
  constant CSR_mideleg       : std_logic_vector(11 downto 0) := X"303";
  -- Machine interrupt enable
  constant CSR_mie           : std_logic_vector(11 downto 0) := X"304";
  -- The base address of the M-mode trap vector.
  constant CSR_mtvec         : std_logic_vector(11 downto 0) := X"305";
  -- Machine wall-clock timer compare value.
  constant CSR_mtimecmp      : std_logic_vector(11 downto 0) := X"321";
  -- Scratch register for machine trap handlers.
  constant CSR_mscratch      : std_logic_vector(11 downto 0) := X"340";
  -- Exception program counters.
  constant CSR_uepc          : std_logic_vector(11 downto 0) := X"041";
  constant CSR_sepc          : std_logic_vector(11 downto 0) := X"141";
  constant CSR_hepc          : std_logic_vector(11 downto 0) := X"241";
  constant CSR_mepc          : std_logic_vector(11 downto 0) := X"341";
  -- Machine trap cause
  constant CSR_mcause        : std_logic_vector(11 downto 0) := X"342";
  -- Machine bad address.
  constant CSR_mbadaddr      : std_logic_vector(11 downto 0) := X"343";
  -- Machine interrupt pending
  constant CSR_mip           : std_logic_vector(11 downto 0) := X"344";
  --! @}

  --! @name   Exceptions
  --! @{
  -- Instruction address misaligned
  constant EXCEPTION_InstrMisalign   : std_logic_vector(3 downto 0) := X"0";
  -- Instruction access fault
  constant EXCEPTION_InstrFault      : std_logic_vector(3 downto 0) := X"1";
  -- Illegal instruction
  constant EXCEPTION_InstrIllegal    : std_logic_vector(3 downto 0) := X"2";
  -- Breakpoint
  constant EXCEPTION_Breakpoint      : std_logic_vector(3 downto 0) := X"3";
  -- Load address misaligned
  constant EXCEPTION_LoadMisalign    : std_logic_vector(3 downto 0) := X"4";
  -- Load access fault
  constant EXCEPTION_LoadFault       : std_logic_vector(3 downto 0) := X"5";
  -- Store/AMO address misaligned
  constant EXCEPTION_StoreMisalign   : std_logic_vector(3 downto 0) := X"6";
  -- Store/AMO access fault
  constant EXCEPTION_StoreFault      : std_logic_vector(3 downto 0) := X"7";
  -- Environment call from U-mode
  constant EXCEPTION_CallFromUmode   : std_logic_vector(3 downto 0) := X"8";
  -- Environment call from S-mode
  constant EXCEPTION_CallFromSmode   : std_logic_vector(3 downto 0) := X"9";
  -- Environment call from H-mode
  constant EXCEPTION_CallFromHmode   : std_logic_vector(3 downto 0) := X"A";
  -- Environment call from M-mode
  constant EXCEPTION_CallFromMmode   : std_logic_vector(3 downto 0) := X"B";
  --! @}

  --! @name   Interrupts
  --! @{
  -- User software interrupt
  constant INTERRUPT_USoftware       : std_logic_vector(3 downto 0) := X"0";
  -- Superuser software interrupt
  constant INTERRUPT_SSoftware       : std_logic_vector(3 downto 0) := X"1";
  -- Hypervisor software itnerrupt
  constant INTERRUPT_HSoftware       : std_logic_vector(3 downto 0) := X"2";
  -- Machine software interrupt
  constant INTERRUPT_MSoftware       : std_logic_vector(3 downto 0) := X"3";
  -- User timer interrupt
  constant INTERRUPT_UTimer          : std_logic_vector(3 downto 0) := X"4";
  -- Superuser timer interrupt
  constant INTERRUPT_STimer          : std_logic_vector(3 downto 0) := X"5";
  -- Hypervisor timer interrupt
  constant INTERRUPT_HTimer          : std_logic_vector(3 downto 0) := X"6";
  -- Machine timer interrupt
  constant INTERRUPT_MTimer          : std_logic_vector(3 downto 0) := X"7";
  -- User external interrupt
  constant INTERRUPT_UExternal       : std_logic_vector(3 downto 0) := X"8";
  -- Superuser external interrupt
  constant INTERRUPT_SExternal       : std_logic_vector(3 downto 0) := X"9";
  -- Hypervisor external interrupt
  constant INTERRUPT_HExternal       : std_logic_vector(3 downto 0) := X"A";
  -- Machine external interrupt (from PLIC)
  constant INTERRUPT_MExternal       : std_logic_vector(3 downto 0) := X"B";
  --! @}


  --! @param[in] i_clk             CPU clock
  --! @param[in] i_nrst            Reset. Active LOW.
  --! @param[in] i_req_mem_fire    Memory request was accepted
  --! @param[in] i_resp_mem_valid  Memory response from ICache is valid
  --! @param[in] i_resp_mem_addr   Memory response address
  --! @param[in] i_resp_mem_data   Memory response value
  --! @param[in] i_f_predic_miss   Fetch modul detects deviation between predicted and valid pc.
  --! @param[in] i_e_npc           Valid instruction value awaited by 'Executor'
  --! @param[in] i_ra              Return address register value
  --! @param[out] o_npc_predic     Predicted next instruction address
  component BranchPredictor is
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_mem_fire : in std_logic;
    i_resp_mem_valid : in std_logic;
    i_resp_mem_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_mem_data : in std_logic_vector(31 downto 0);
    i_f_predic_miss : in std_logic;
    i_e_npc : in std_logic_vector(31 downto 0);
    i_ra : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_npc_predict : out std_logic_vector(31 downto 0)
  );
  end component; 

  --! @param[in] i_clk          CPU clock
  --! @param[in] i_nrst         Reset. Active LOW.
  --! @param[in] i_xret         XRet instruction signals mode switching
  --! @param[in] i_addr         CSR address, if xret=1 switch mode accordingly
  --! @param[in] i_wena         Write enable
  --! @param[in] i_wdata        CSR writing value
  --! @param[out] o_rdata       CSR read value
  --! @param[in] i_break_mode   Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
  --! @param[in] i_breakpoint   Breakpoint (Trap or not depends of mode)
  --! @param[in] i_trap_ena     Trap pulse
  --! @param[in] i_trap_code    bit[4] : 1=interrupt; 0=exception; bits[3:0]=code
  --! @param[in] i_trap_pc      trap on pc
  --! @param[out] o_ie          Interrupt enable bit
  --! @param[out] o_mode        CPU mode
  --! @param[out] o_mtvec       Interrupt descriptors table
  --! @param[in] i_dport_ena    Debug port request is enabled
  --! @param[in] i_dport_write  Debug port Write enable
  --! @param[in] i_dport_addr   Debug port CSR address
  --! @param[in] i_dport_wdata  Debug port CSR writing value
  --! @param[out] o_dport_rdata Debug port CSR read value
  component CsrRegs is
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_xret : in std_logic;
    i_addr : in std_logic_vector(11 downto 0);
    i_wena : in std_logic;
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_break_mode : in std_logic;
    i_breakpoint : in std_logic;
    i_trap_ena : in std_logic;
    i_trap_code : in std_logic_vector(4 downto 0);
    i_trap_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_ie : out std_logic;
    o_mode : out std_logic_vector(1 downto 0);
    o_mtvec : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_dport_ena : in std_logic;
    i_dport_write : in std_logic;
    i_dport_addr : in std_logic_vector(11 downto 0);
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0)
  );
  end component; 

  --! @param[in] i_clk CPU clock
  --! @param[in] i_nrst Reset. Active LOW.
  --! @param[in] i_any_hold Hold pipeline by any reason
  --! @param[in] i_f_valid Fetch input valid
  --! @param[in] i_f_pc Fetched pc
  --! @param[in] i_f_instr Fetched instruction value
  --! @param[out] o_valid Current output values are valid
  --! @param[out] o_pc Current instruction pointer value
  --! @param[out] o_instr Current instruction value
  --! @param[out] o_memop_store Store to memory operation
  --! @param[out] o_memop_load Load from memoru operation
  --! @param[out] o_memop_sign_ext Load memory value with sign extending
  --! @param[out] o_memop_size Memory transaction size
  --! @param[out] o_rv32 32-bits instruction
  --! @param[out] o_insigned_op Unsigned operands
  --! @param[out] o_isa_type Instruction format accordingly with ISA
  --! @param[out] o_instr_vec One bit per decoded instruction bus
  --! @param[out] o_exception Unimplemented instruction
  component InstrDecoder is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_any_hold : in std_logic;
    i_f_valid : in std_logic;
    i_f_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_f_instr : in std_logic_vector(31 downto 0);
    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0);
    o_memop_store : out std_logic;
    o_memop_load : out std_logic;
    o_memop_sign_ext : out std_logic;
    o_memop_size : out std_logic_vector(1 downto 0);
    o_rv32 : out std_logic;
    o_unsigned_op : out std_logic;
    o_isa_type : out std_logic_vector(ISA_Total-1 downto 0);
    o_instr_vec : out std_logic_vector(Instr_Total-1 downto 0);
    o_exception : out std_logic
  );
  end component; 


  --! @param[in] i_clk  
  --! @param[in] i_nrst Reset active LOW
  --! @param[in] i_pipeline_hold Hold execution by any reason
  --! @param[in] i_d_valid Decoded instruction is valid
  --! @param[in] i_d_pc Instruction pointer on decoded instruction
  --! @param[in] i_d_instr Decoded instruction value
  --! @param[in] i_wb_done write back done (Used to clear hazardness)
  --! @param[in] i_memop_store Store to memory operation
  --! @param[in] i_memop_load Load from memoru operation
  --! @param[in] i_memop_sign_ext Load memory value with sign extending
  --! @param[in] i_memop_size Memory transaction size
  --! @param[in] i_unsigned_op Unsigned operands
  --! @param[in] i_rv32 32-bits instruction
  --! @param[in] i_isa_type Type of the instruction's structure (ISA spec.)
  --! @param[in] i_ivec One pulse per supported instruction.
  --! @param[in] i_ie Interrupt enable bit
  --! @param[in] i_mtvec Interrupt descriptor table
  --! @param[in] i_mode Current processor mode
  --! @param[in] i_break_mode        Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
  --! @param[in] i_unsup_exception   Unsupported instruction exception
  --! @param[in] i_ext_irq           External interrupt from PLIC (todo: timer & software interrupts)
  --! @param[in] i_dport_npc_write   Write npc value from debug port
  --! @param[in] i_dport_npc         Debug port npc value to write
  --! @param[out] o_radr1 Integer register index 1
  --! @param[in] i_rdata1 Integer register value 1
  --! @param[out] o_radr2 Integer register index 2
  --! @param[in] i_rdata2 Integer register value 2
  --! @param[out] o_res_addr Address to store result of the instruction (0=do not store)
  --! @param[out] o_res_data Value to store
  --! @param[out] o_pipeline_hold Hold pipeline while 'writeback' not done or multi-clock instruction.
  --! @param[out] o_xret XRET instruction: MRET, URET or other.
  --! @param[out] o_csr_addr CSR address. 0 if not a CSR instruction with xret signals mode switching
  --! @param[out] o_csr_wena Write new CSR value
  --! @param[in] i_csr_rdata CSR current value
  --! @param[out] o_csr_wdata CSR new value
  --! @param[out] o_trap_ena Trap occurs  pulse
  --! @param[out] o_trap_code bit[4] : 1=interrupt; 0=exception; bits[3:0]=code
  --! @param[out] o_trap_pc trap on pc
  --! @param[out] o_memop_sign_ext Load data with sign extending
  --! @param[out] o_memop_load Load data instruction
  --! @param[out] o_memop_store Store data instruction
  --! @param[out] o_memop_size 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
  --! @param[out] o_memop_addr  Memory access address
  --! @param[out] o_valid       Output is valid
  --! @param[out] o_pc          Valid instruction pointer
  --! @param[out] o_npc         Next instruction pointer. Next decoded pc must match to this value or will be ignored.
  --! @param[out] o_instr       Valid instruction value
  --! @param[out] o_breakpoint  ebreak instruction
  component InstrExecute is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_pipeline_hold : in std_logic;
    i_d_valid : in std_logic;
    i_d_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_d_instr : in std_logic_vector(31 downto 0);
    i_wb_done : in std_logic;
    i_memop_store : in std_logic;
    i_memop_load : in std_logic;
    i_memop_sign_ext : in std_logic;
    i_memop_size : in std_logic_vector(1 downto 0);
    i_unsigned_op : in std_logic;
    i_rv32 : in std_logic;
    i_isa_type : in std_logic_vector(ISA_Total-1 downto 0);
    i_ivec : in std_logic_vector(Instr_Total-1 downto 0);
    i_ie : in std_logic;
    i_mtvec : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mode : in std_logic_vector(1 downto 0);
    i_break_mode : in std_logic;
    i_unsup_exception : in std_logic;
    i_ext_irq : in std_logic;
    i_dport_npc_write : in std_logic;
    i_dport_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_radr1 : out std_logic_vector(4 downto 0);
    i_rdata1 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_radr2 : out std_logic_vector(4 downto 0);
    i_rdata2 : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_res_addr : out std_logic_vector(4 downto 0);
    o_res_data : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_pipeline_hold : out std_logic;
    o_xret : out std_logic;
    o_csr_addr : out std_logic_vector(11 downto 0);
    o_csr_wena : out std_logic;
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_csr_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_trap_ena : out std_logic;
    o_trap_code : out std_logic_vector(4 downto 0);
    o_trap_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_memop_sign_ext : out std_logic;
    o_memop_load : out std_logic;
    o_memop_store : out std_logic;
    o_memop_size : out std_logic_vector(1 downto 0);
    o_memop_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_npc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0);
    o_breakpoint : out std_logic
  );
  end component; 

  --! @param[in] i_clk
  --! @param[in] i_nrst
  --! @param[in] i_pipeline_hold
  --! @param[in] i_mem_ready
  --! @param[out] o_mem_addr_valid
  --! @param[out] o_mem_addr
  --! @param[in] i_mem_data_valid
  --! @param[in] i_mem_data_addr
  --! @param[in] i_mem_data
  --! @param[out] o_mem_ready
  --! @param[in] i_e_npc
  --! @param[in] i_predict_npc
  --! @param[out] o_predict_miss
  --! @param[out] o_mem_req_fire    Used by branch predictor to form new npc value
  --! @param[out] o_valid
  --! @param[out] o_pc
  --! @param[out] o_instr
  --! @param[out] o_hold            Hold due no response from icache yet
  --! @param[in] i_br_fetch_valid   Fetch injection address/instr are valid
  --! @param[in] i_br_address_fetch Fetch injection address to skip ebreak instruciton only once
  --! @param[in] i_br_instr_fetch   Real instruction value that was replaced by ebreak

  component InstrFetch is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_pipeline_hold : in std_logic;
    i_mem_req_ready : in std_logic;
    o_mem_addr_valid : out std_logic;
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data : in std_logic_vector(31 downto 0);
    o_mem_resp_ready : out std_logic;
    i_e_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_predict_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_predict_miss : out std_logic;

    o_mem_req_fire : out std_logic;
    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0);
    o_hold : out std_logic;
    i_br_fetch_valid : in std_logic;
    i_br_address_fetch : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_br_instr_fetch : in std_logic_vector(31 downto 0)
  );
  end component; 

  --! @param[in] i_clk
  --! @param[in] i_nrst
  --! @param[in] i_e_valid Execution stage outputs are valid
  --! @param[in] i_e_pc Execution stage instruction pointer
  --! @param[in] i_e_instr Execution stage instruction value
  --! @param[in] i_res_addr Register address to be written (0=no writing)
  --! @param[in] i_res_data Register value to be written
  --! @param[in] i_memop_sign_ext Load data with sign extending (if less than 8 Bytes)
  --! @param[in] i_memop_load Load data from memory and write to i_res_addr
  --! @param[in] i_memop_store Store i_res_data value into memory
  --! @param[in] i_memop_size Encoded memory transaction size in bytes:
  --!                         0=1B; 1=2B; 2=4B; 3=8B
  --! @param[in] i_memop_addr Memory access address
  --! @param[out] o_wena Write enable signal
  --! @param[out] o_waddr Output register address (0 = x0 = no write)
  --! @param[out] o_wdata Register value
  --! @param[in] i_mem_req_read Memory request is acceptable
  --! @param[out] o_mem_valid Memory request is valid
  --! @param[out] o_mem_write Memory write request
  --! @param[out] o_mem_sz Encoded data size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
  --! @param[out] o_mem_addr Data path requested address
  --! @param[out] o_mem_data Data path requested data (write transaction)
  --! @param[in] i_mem_data_valid Data path memory response is valid
  --! @param[in] i_mem_data_addr Data path memory response address
  --! @param[in] i_mem_data Data path memory response value
  --! @param[out] o_mem_resp_ready Data from DCache was accepted
  --! @param[out] o_hold Hold-on pipeline while memory operation not finished
  --! @param[out] o_valid Output is valid
  --! @param[out] o_pc Valid instruction pointer
  --! @param[out] o_instr Valid instruction value
  component MemAccess is
  port (
    i_clk  : in std_logic;
    i_nrst : in std_logic;
    i_e_valid : in std_logic;
    i_e_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_e_instr : in std_logic_vector(31 downto 0);
    i_res_addr : in std_logic_vector(4 downto 0);
    i_res_data : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_memop_sign_ext : in std_logic;
    i_memop_load : in std_logic;
    i_memop_store : in std_logic;
    i_memop_size : in std_logic_vector(1 downto 0);
    i_memop_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_wena : out std_logic;
    o_waddr : out std_logic_vector(4 downto 0);
    o_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_mem_req_ready : in std_logic;
    o_mem_valid : out std_logic;
    o_mem_write : out std_logic;
    o_mem_sz : out std_logic_vector(1 downto 0);
    o_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_mem_data_valid : in std_logic;
    i_mem_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    o_mem_resp_ready : out std_logic;
    o_hold : out std_logic;
    o_valid : out std_logic;
    o_pc : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_instr : out std_logic_vector(31 downto 0)
  );
  end component; 

  --! @param[in] i_clk CPU clock
  --! @param[in] i_nrst Reset. Active LOW.
  --! @param[in] i_radr1 Port 1 read address
  --! @param[out] o_rdata1 Port 1 read value
  --! @param[in] i_radr2 Port 2 read address
  --! @param[out] o_rdata2 Port 2 read value
  --! @param[in] i_waddr Writing value
  --! @param[in] i_wena Writing is enabled
  --! @param[in] i_wdata Writing value
  --! @param[in] i_dport_addr    Debug port address
  --! @param[in] i_dport_ena     Debug port is enabled
  --! @param[in] i_dport_write   Debug port write is enabled
  --! @param[in] i_dport_wdata   Debug port write value
  --! @param[out] o_dport_rdata  Debug port read value
  --! @param[out] o_ra           Return address for branch predictor
  component RegIntBank is
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_radr1 : in std_logic_vector(4 downto 0);
    o_rdata1 : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_radr2 : in std_logic_vector(4 downto 0);
    o_rdata2 : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_waddr : in std_logic_vector(4 downto 0);
    i_wena : in std_logic;
    i_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_dport_addr : in std_logic_vector(4 downto 0);
    i_dport_ena : in std_logic;
    i_dport_write : in std_logic;
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_ra : out std_logic_vector(RISCV_ARCH-1 downto 0)
  );
  end component; 

  --! @param[in] i_clk            CPU clock
  --! @param[in] i_nrst           Reset. Active LOW.
  --! @param[in] i_dport_valid    Debug access from DSU is valid
  --! @param[in] i_dport_write    Write command flag
  --! @param[in] i_dport_region   Registers region ID: 0=CSR; 1=IREGS; 2=Control
  --! @param[in] i_dport_addr     Register idx
  --! @param[in] i_dport_wdata    Write value
  --! @param[out] o_dport_ready   Response is ready
  --! @param[out] o_dport_rdata   Response value
  --! @param[out] o_core_addr     Address of the sub-region register
  --! @param[out] o_core_wdata    Write data
  --! @param[out] o_csr_ena       Region 0: Access to CSR bank is enabled.
  --! @param[out] o_csr_write     Region 0: CSR write enable
  --! @param[in] i_csr_rdata      Region 0: CSR read value
  --! @param[out] o_ireg_ena      Region 1: Access to integer register bank is enabled
  --! @param[out] o_ireg_write    Region 1: Integer registers bank write pulse
  --! @param[out] o_npc_write     Region 1: npc write enable
  --! @param[in] i_ireg_rdata     Region 1: Integer register read value
  --! @param[in] i_pc             Region 1: Instruction pointer
  --! @param[in] i_npc            Region 1: Next Instruction pointer
  --! @param[in] i_e_valid        Stepping control signal
  --! @param[in] i_m_valid        To compute number of valid executed instruction
  --! @param[out] o_clock_cnt     Number of clocks excluding halt state
  --! @param[out] o_executed_cnt  Number of executed instructions
  --! @param[out] o_halt          Halt signal is equal to hold pipeline
  --! @param[in] i_ebreak            ebreak instruction decoded
  --! @param[out] o_break_mode       Behaviour on EBREAK instruction: 0 = halt; 1 = generate trap
  --! @param[out] o_br_fetch_valid   Fetch injection address/instr are valid
  --! @param[out] o_br_address_fetch Fetch injection address to skip ebreak instruciton only once
  --! @param[out] o_br_instr_fetch   Real instruction value that was replaced by ebreak
  component DbgPort
  is port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_dport_valid : in std_logic;
    i_dport_write : in std_logic;
    i_dport_region : in std_logic_vector(1 downto 0);
    i_dport_addr : in std_logic_vector(11 downto 0);
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_ready : out std_logic;
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_core_addr : out std_logic_vector(11 downto 0);
    o_core_wdata : out std_logic_vector(RISCV_ARCH-1 downto 0);
    o_csr_ena : out std_logic;
    o_csr_write : out std_logic;
    i_csr_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_ireg_ena : out std_logic;
    o_ireg_write : out std_logic;
    o_npc_write : out std_logic;
    i_ireg_rdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    i_pc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_npc : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_e_valid : in std_logic;
    i_m_valid : in std_logic;
    o_clock_cnt : out std_logic_vector(63 downto 0);
    o_executed_cnt : out std_logic_vector(63 downto 0);
    o_halt : out std_logic;
    i_ebreak : in std_logic;
    o_break_mode : out std_logic;
    o_br_fetch_valid : out std_logic;
    o_br_address_fetch : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_br_instr_fetch : out std_logic_vector(31 downto 0)
  );
  end component; 

  --! @brief CPU 5-stages pipeline top-level
  --! @param[in] i_clk             CPU clock
  --! @param[in] i_nrst            Reset. Active LOW.
  --! @param[in] i_req_ctrl_ready  ICache is ready to accept request
  --! @param[out] o_req_ctrl_valid Request to ICache is valid
  --! @param[out] o_req_ctrl_addr  Requesting address to ICache
  --! @param[in] i_resp_ctrl_valid ICache response is valid
  --! @param[in] i_resp_ctrl_addr  Response address must be equal to the latest request address
  --! @param[in] i_resp_ctrl_data  Read value
  --! @param[out] o_resp_ctrl_ready Response from ICache is accepted
  --! @param[in] i_req_data_ready  DCache is ready to accept request
  --! @param[out] o_req_data_valid Request to DCache is valid
  --! @param[out] o_req_data_write Read/Write transaction
  --! @param[out] o_req_data_size  Size [Bytes]: 0=1B; 1=2B; 2=4B; 3=8B
  --! @param[out] o_req_data_addr  Requesting address to DCache
  --! @param[out] o_req_data_data  Writing value
  --! @param[in] i_resp_data_valid DCache response is valid
  --! @param[in] i_resp_data_addr  DCache response address must be equal to the latest request address
  --! @param[in] i_resp_data_data  Read value
  --! @param[out] o_resp_data_ready Response drom DCache is accepted
  --! @param[in] i_ext_irq         PLIC interrupt accordingly with spec
  --! @param[out] o_time           Timer in clock except halt state
  --! @param[in] i_dport_valid     Debug access from DSU is valid
  --! @param[in] i_dport_write     Write command flag
  --! @param[in] i_dport_region    Registers region ID: 0=CSR; 1=IREGS; 2=Control
  --! @param[in] i_dport_addr      Register idx
  --! @param[in] i_dport_wdata     Write value
  --! @param[out] o_dport_ready    Response is ready
  --! @param[out] o_dport_rdata    Response value
  component Processor is
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_ctrl_ready : in std_logic;
    o_req_ctrl_valid : out std_logic;
    o_req_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_ctrl_valid : in std_logic;
    i_resp_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_ctrl_data : in std_logic_vector(31 downto 0);
    o_resp_ctrl_ready : out std_logic;
    i_req_data_ready : in std_logic;
    o_req_data_valid : out std_logic;
    o_req_data_write : out std_logic;
    o_req_data_size : out std_logic_vector(1 downto 0);
    o_req_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_data_data : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_resp_data_valid : in std_logic;
    i_resp_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_resp_data_data : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_resp_data_ready : out std_logic;
    i_ext_irq : in std_logic;
    o_time : out std_logic_vector(63 downto 0);
    i_dport_valid : in std_logic;
    i_dport_write : in std_logic;
    i_dport_region : in std_logic_vector(1 downto 0);
    i_dport_addr : in std_logic_vector(11 downto 0);
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_ready : out std_logic;
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0)
  );
  end component; 

  --! @brief CPU cache top level
  --! @param[in] i_clk
  --! @param[in] i_nrst
  --! @param[in] i_req_ctrl_valid
  --! @param[in] i_req_ctrl_addr
  --! @param[out] o_req_ctrl_ready
  --! @param[out] o_resp_ctrl_valid
  --! @param[out] o_resp_ctrl_addr
  --! @param[out] o_resp_ctrl_data
  --! @param[in] i_resp_ctrl_ready
  --! @param[out] o_req_data_ready
  --! @param[in] i_req_data_valid
  --! @param[in] i_req_data_write
  --! @param[in] i_req_data_sz
  --! @param[in] i_req_data_addr
  --! @param[in] i_req_data_data
  --! @param[out] o_resp_data_valid
  --! @param[out] o_resp_data_addr
  --! @param[out] o_resp_data_data
  --! @param[in] i_resp_data_ready
  --! @param[in] i_req_mem_ready      AXI request was accepted
  --! @param[out] o_req_mem_valid
  --! @param[out] o_req_mem_write
  --! @param[out] o_req_mem_addr
  --! @param[out] o_req_mem_strob
  --! @param[out] o_req_mem_data
  --! @param[in] i_resp_mem_data_valid
  --! @param[in] i_resp_mem_data
  component CacheTop is
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_ctrl_valid : in std_logic;
    i_req_ctrl_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_ctrl_ready : out std_logic;
    o_resp_ctrl_valid : out std_logic;
    o_resp_ctrl_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_ctrl_data : out std_logic_vector(31 downto 0);
    i_resp_ctrl_ready : in std_logic;
    o_req_data_ready : out std_logic;
    i_req_data_valid : in std_logic;
    i_req_data_write : in std_logic;
    i_req_data_size : in std_logic_vector(1 downto 0);
    i_req_data_addr : in std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    i_req_data_data : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_resp_data_valid : out std_logic;
    o_resp_data_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_resp_data_data : out std_logic_vector(RISCV_ARCH-1 downto 0);
    i_resp_data_ready : in std_logic;
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0)
  );
  end component; 


  --! @brief "River" CPU Top level.
  --! @param[in] i_clk                 CPU clock
  --! @param[in] i_nrst                Reset. Active LOW.
  --! @param[in] i_req_mem_ready       AXI request was accepted
  --! @param[out] o_req_mem_valid      AXI memory request is valid
  --! @param[out] o_req_mem_write      AXI memory request is write type
  --! @param[out] o_req_mem_addr       AXI memory request address
  --! @param[out] o_req_mem_strob      Writing strob. 1 bit per Byte
  --! @param[out] o_req_mem_data       Writing data
  --! @param[in] i_resp_mem_data_valid AXI response is valid
  --! @param[in] i_resp_mem_data       Read data
  --! @param[in] i_ext_irq             Interrupt line from external interrupts controller (PLIC).
  --! @param[out] o_time               Timer. Clock counter except halt state.
  --! @param[in] i_dport_valid         Debug access from DSU is valid
  --! @param[in] i_dport_write         Write command flag
  --! @param[in] i_dport_region        Registers region ID: 0=CSR; 1=IREGS; 2=Control
  --! @param[in] i_dport_addr          Register idx
  --! @param[in] i_dport_wdata         Write value
  --! @param[out] o_dport_ready        Response is ready
  --! @param[out] o_dport_rdata        Response value
  component RiverTop is
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_req_mem_ready : in std_logic;
    o_req_mem_valid : out std_logic;
    o_req_mem_write : out std_logic;
    o_req_mem_addr : out std_logic_vector(BUS_ADDR_WIDTH-1 downto 0);
    o_req_mem_strob : out std_logic_vector(BUS_DATA_BYTES-1 downto 0);
    o_req_mem_data : out std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_resp_mem_data_valid : in std_logic;
    i_resp_mem_data : in std_logic_vector(BUS_DATA_WIDTH-1 downto 0);
    i_ext_irq : in std_logic;
    o_time : out std_logic_vector(63 downto 0);
    i_dport_valid : in std_logic;
    i_dport_write : in std_logic;
    i_dport_region : in std_logic_vector(1 downto 0);
    i_dport_addr : in std_logic_vector(11 downto 0);
    i_dport_wdata : in std_logic_vector(RISCV_ARCH-1 downto 0);
    o_dport_ready : out std_logic;
    o_dport_rdata : out std_logic_vector(RISCV_ARCH-1 downto 0)
  );
  end component; 

end; -- package body
