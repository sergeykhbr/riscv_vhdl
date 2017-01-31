-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Debug Support Unit (DSU) with AXI4 interface.
--! @details   DSU provides access to the internal CPU registers via
--!            'Debug port' bus interface available only on <b>RIVER</b> CPU.
--!            It is also implements a set of registers collecting bus
--!            utilization statistic and additional debug information.
-----------------------------------------------------------------------------

--! @defgroup dsu_link Debug Support Unit (DSU)
--! @ingroup peripheries_group
--! 
--! @section dsu_overview Overview
--! Debug Support Unit (DSU) was developed to interact with "RIVER" CPU
--! via its debug port interace. This bus provides access to all internal CPU
--! registers and states and may be additionally extended by request.
--! Run control functionality like 'run', 'halt', 'step' or 'breakpoints'
--! imlemented using proprietary algorithms and intend to simplify integration
--! with debugger application.
--!
--! Set of general registers and control registers (CSR) are described in 
--! RISC-V privileged ISA specification and also available for read and write
--! access via debug port.
--!
--! @note Take into account that CPU can have any number of
--! platform specific CSRs that usually not entirely documented.
--! 
--! @section dsu_regs DSU registers mapping
--! DSU acts like a slave AMBA AXI4 device that is directly mapped into 
--! physical memory. Default address location for our implementation 
--! is 0x80020000. DSU directly transforms device offset address
--! into one of regions of the debug port:
--! <ul>
--!    <li><b>0x00000..0x08000 (Region 1):</b> CSR registers.</li>
--!    <li><b>0x08000..0x10000 (Region 2):</b> General set of registers.</li>
--!    <li><b>0x10000..0x18000 (Region 3):</b> Run control and debug support registers.</li>
--!    <li><b>0x18000..0x20000 (Region 4):</b> Local DSU region that doesn't access CPU debug port.</li>
--! </ul>
--!
--! @par Example:
--!     Bus transaction at address <em>0x80023C10</em>
--!     will be redirected to Debug port with CSR index <em>0x782</em>.
--!     
--! @subsection dsu_csr CSR Region (32 KB)
--!
--! @par User Exception Program Counter (0x00208). ISA offset 0x041.
--!
--! |Bits|Type| Reset |  Name         | Definition 
--! |:--:|:--:|:-----:|:-------------:|---------------------------------------------|
--! | 64 | RO | 64h'0 | uepc | <b>User mode exception program counter</b>. Instruction URET is used to return from traps in User Mode into specified instruction pointer. URET is only provided if user-mode traps are supported.
--!
--! @par Machine Status Register (0x01800). ISA offset 0x300.
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 1  | RW | 1b'0  | SD       | 63    | Bit summarizes whether either the FS field or XS field signals the presence of some dirty state that will require saving extended user context to memory
--! | 22 | RW | 22h'0 | WPRI     | 62:20 | Reserved
--! | 5  | RW | 5h'0  | VM (WARL)| 28:24 | Virtual addressing enable
--! | 4  | RW | 4h'0  | WPRI     | 23:20 | Reserved
--! | 1  | RW | 1b'0  | MXR      | 19    | <b>Make eXecutable Readable</b>
--! | 1  | RW | 1b'0  | PUM      | 18    | <b>Protect User Memory</b> bit modifies the privilege with which loads access virtual memory
--! | 1  | RW | 1b'0  | MPRV     | 17    | Privilege level at which loads and stores execute
--! | 2  | RW | 2h'0  | XS       | 16:15 | Context switch reducing flags: 0=All Off; 1=None dirty or clean, some on; 2=None dirty, some clean; 3=Some dirty
--! | 2  | RW | 2h'0  | FS       | 14:13 | Context switch reducing flags: 0=Off; 1=Initial; 2=Clean; 3=Dirty
--! | 2  | RW | 2h'0  | MPP      | 12:11 | Priviledge mode on MRET
--! | 2  | RW | 2h'0  | HPP      | 10:9  | Priviledge mode on HRET
--! | 1  | RW | 1b'0  | SPP      | 8     | Priviledge mode on SRET
--! | 1  | RW | 1b'0  | MPIE     | 7     | MIE prior to the trap
--! | 1  | RW | 1b'0  | HPIE     | 6     | HIE prior to the trap
--! | 1  | RW | 1b'0  | SPIE     | 5     | SIE prior to the trap
--! | 1  | RW | 1b'0  | UPIE     | 4     | UIE prior to the trap
--! | 1  | RW | 1b'0  | MIE      | 3     | Machine interrupt enable bit
--! | 1  | RW | 1b'0  | HIE      | 2     | Hypervisor interrupt enable bit
--! | 1  | RW | 1b'0  | SIE      | 1     | Super-user interrupt enable bit
--! | 1  | RW | 1b'0  | UIE      | 0     | User interrupt enable bit
--!
--! @par Machine Trap-Vector Base-Address Register (0x01828). <br>ISA offset 0x305.
--!
--! |Bits|Type| Reset | Field Name        | Definition 
--! |:--:|:--:|:-----:|:-----------------:|:---------------------------------------------|
--! | 64 | RW | 64h'0 | mtvec | <b>Trap-vector Base Address</b>. The mtvec register is an XLEN-bit read/write register that holds the base address of the M-mode trap vector.
--!
--! @par Machine Exception Program Counter (0x01A08). <br>ISA offset 0x341.
--!
--! |Bits|Type| Reset | Field Name        | Definition 
--! |:--:|:--:|:-----:|:-----------------:|:---------------------------------------------|
--! | 64 | RW | 64h'0 | mepc | <b>Machine mode exception program counter</b>. Instruction MRET is used to return from traps in User Mode into specified instruction pointer. On implementations that do not support instruction-set extensions with 16-bit instruction alignment, the two low bits (mepc[1:0]) are always zero.
--!
--! @par Machine Cause Register (0x01A10). <br>ISA offset 0x342.
--!
--! |Bits|Type| Reset | Field Name   |Bits| Definition 
--! |:--:|:--:|:-----:|:-------------|:--:|:---------------------------------------------|
--! | 1  | RW | 1b'0  | Interrupt    | 63 | The Interrupt bit is set if the trap was caused by an interrupt.
--! | 63 | RW | 63h'0 | Exception Code | 62:0 | <b>Exception code</b>. The Exception Code field contains a code identifying the last exception. Table 3.6 lists the possible machine-level exception codes.
--!
--! @par Machine Cause Register (0x01A18). <br>ISA offset 0x343.
--!
--! |Bits|Type| Reset | Field Name   |Bits| Definition 
--! |:--:|:--:|:-----:|:-------------|:--:|:---------------------------------------------|
--! | 64 | RW | 64h'0 | mbadaddr     | 63:0 | <b>Exception address</b>. When a hardware breakpoint is triggered, or an instruction-fetch, load, or store address-misaligned or access exception occurs, mbadaddr is written with the faulting address. mbadaddr is not modified for other exceptions.
--!
--! @par Machine ISA Register (0x07880). ISA offset 0xf10.
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 2  | RO | 2h'2  | Base (WARL)| 63:62 | <b>Integer ISA width</b>: 1=32 bits; 2=64 bits; 3=128 bits.
--! | 34 | RO | 64h'0 | WIRI       | 61:28 | Reserved.
--! | 28 | RO | 28h'141181 | Extension (WARL) | 27:0 | <b>Supported ISA extensions</b>. See priviledge-isa datasheet.
--!
--! @par Machine Vendor ID (0x07888). ISA offset 0xf11.
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RO | 64h'0 | Vendor | 63:0 | <b>Vendor ID</b>. read-only register encoding the manufacturer of the part. This register must be readable in any implementation, but a value of 0 can be returned to indicate the field is not implemented or that this is a non-commercial implementation.
--!
--! @par Machine Architecture ID  Register (0x07890). ISA offset 0xf12.
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RO | 64h'0 | marchid | 63:0 |<b>Architecture ID</b>. Read-only register encoding the base microarchitecture of the hart. This register must be readable in any implementation, but a value of 0 can be returned to indicate the field is not implemented. The combination of mvendorid and marchid should uniquely identify the type of hart microarchitecture that is implemented.
--!
--! @par Machine implementation ID Register (0x07898). ISA offset 0xf13.
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RO | 64h'0 | mimplid | 63:0 | <b>Implementation ID</b>. CSR provides a unique encoding of the version of the processor implementation. This register must be readable in any implementation, but a value of 0 can be returned to indicate that the field is not implemented.
--!
--! @par Hart ID Register (0x078A0). ISA offset 0xf14.
--!
--! |Bits|Type| Reset |Field Name| Bits  | Description 
--! |:--:|:--:|:-----:|:---------|:-----:|:------------------------------------------------------------|
--! | 64 | RO | 64h'0 | mhartid | 63:0 | <b>Integer ID of hardware thread</b>.  Hart IDs mightnot necessarily be numbered contiguously in a multiprocessor system, but at least one hart musthave a hart ID of zero.
--!
--!
--! @subsection dsu_iregs General CPU Registers Region (32 KB)
--!
--! @par CPU integer registers (0x08000).
--!
--! |Offset |Bits|Type| Reset | Name | Definition 
--! |:------|:--:|:--:|:-----:|:----:|---------------------------------------------|
--! |0x08000| 64 | RW | 64h'0 | zero | <b>x0</b>. CPU General Integer Register hardware connected to zero.
--! |0x08008| 64 | RW | 64h'0 | ra   | <b>x1</b>. Return address.
--! |0x08010| 64 | RW | 64h'0 | sp   | <b>x2</b>. Stack pointer.
--! |0x08018| 64 | RW | 64h'0 | gp   | <b>x3</b>. Global pointer.
--! |0x08020| 64 | RW | 64h'0 | tp   | <b>x4</b>. Thread pointer.
--! |0x08028| 64 | RW | 64h'0 | t0   | <b>x5</b>. Temporaries 0.
--! |0x08030| 64 | RW | 64h'0 | t1   | <b>x6</b>. Temporaries 1.
--! |0x08038| 64 | RW | 64h'0 | t2   | <b>x7</b>. Temporaries 2.
--! |0x08040| 64 | RW | 64h'0 | s0/fp| <b>x8</b>. CPU General Integer Register 'Saved register 0/ Frame pointer'.
--! |0x08048| 64 | RW | 64h'0 | s1   | <b>x9</b>. Saved register 1.
--! |0x08050| 64 | RW | 64h'0 | a0   | <b>x10</b>. Function argument 0. It is also used to save return value.
--! |0x08058| 64 | RW | 64h'0 | a1   | <b>x11</b>. Function argument 1.
--! |0x08060| 64 | RW | 64h'0 | a2   | <b>x12</b>. Function argument 2.
--! |0x08068| 64 | RW | 64h'0 | a3   | <b>x13</b>. Function argument 3.
--! |0x08070| 64 | RW | 64h'0 | a4   | <b>x14</b>. Function argument 4.
--! |0x08078| 64 | RW | 64h'0 | a5   | <b>x15</b>. Function argument 5.
--! |0x08080| 64 | RW | 64h'0 | a6   | <b>x16</b>. Function argument 6.
--! |0x08088| 64 | RW | 64h'0 | a7   | <b>x17</b>. Function argument 7.
--! |0x08090| 64 | RW | 64h'0 | s2   | <b>x18</b>. Saved register 2.
--! |0x08098| 64 | RW | 64h'0 | s3   | <b>x19</b>. Saved register 3.
--! |0x080a0| 64 | RW | 64h'0 | s4   | <b>x20</b>. Saved register 4.
--! |0x080a8| 64 | RW | 64h'0 | s5   | <b>x21</b>. Saved register 5.
--! |0x080b0| 64 | RW | 64h'0 | s6   | <b>x22</b>. Saved register 6.
--! |0x080b8| 64 | RW | 64h'0 | s7   | <b>x23</b>. Saved register 7.
--! |0x080c0| 64 | RW | 64h'0 | s8   | <b>x24</b>. Saved register 8.
--! |0x080c8| 64 | RW | 64h'0 | s9   | <b>x25</b>. Saved register 9.
--! |0x080d0| 64 | RW | 64h'0 | s10  | <b>x26</b>. Saved register 10.
--! |0x080d8| 64 | RW | 64h'0 | s11  | <b>x27</b>. Saved register 11.
--! |0x080e0| 64 | RW | 64h'0 | t3   | <b>x28</b>. Temporaries 3.
--! |0x080e8| 64 | RW | 64h'0 | t4   | <b>x29</b>. Temporaries 4.
--! |0x080f0| 64 | RW | 64h'0 | t5   | <b>x30</b>. Temporaries 5.
--! |0x080f8| 64 | RW | 64h'0 | t6   | <b>x31</b>. Temporaries 6.
--! |0x08100| 64 | RO | 64h'0 | pc   | <b>Instruction pointer</b>. Cannot be modified because shows the latest executed instruction address
--! |0x08108| 64 | RW | 64h'0 | npc  | <b>Next Instruction Pointer</b>
--!
--!
--! @subsection dsu_control Run Control and Debug support Region (32 KB)
--!
--! @par Run control/status registers (0x10000).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 44 | RW | 61h'0 | Reserved   | 63:6  | Reserved.
--! | 16 | RO | 16h'0 | core_id    | 15:4  | <b>Core ID</b>.
--! | 1  | RW | 1b'0  | Reserved   | 3     | Reserved.
--! | 1  | RO | 1b'0  | breakpoint | 2     | <b>Breakpoint</b>. Status bit is set when CPU was halted due the EBREAK instruction.
--! | 1  | WO | 1b'0  | stepping_mode | 1  | <b>Stepping mode</b>. This bit enables stepping mode if the Register 'steps' is non zero.
--! | 1  | RW | 1b'0  | halt       | 0     | <b>Halt mode</b>. When this bit is set CPU pipeline is in the halted state. CPU can be halted at any time without impact on processing data.
--!
--! @par Stepping mode Steps registers (0x10008).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | steps   | 63:0  | <b>Step counter</b>. Total number of instructions that should execute CPU before halt. CPU is set into stepping using 'stepping mode' bit in Run Control register.
--!
--! @par Clock counter registers (0x10010).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | clock_cnt  | 63:0  | <b>Clock counter</b>. Clock counter is used for hardware computation of CPI rate. Clock counter isn't incrementing in Halt state.
--!
--! @par Step counter registers (0x10018).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | executed_cnt | 63:0  | <b>Step counter</b>. Total number of executed instructions. Step counter is used for hardware computation of CPI rate.
--!
--! @par Breakpoint Control registers (0x10020).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 63 | RW | 63h'0 | Reserved   | 63:1  | Reserved
--! | 1  | RW | 1b'0  | trap_on_break | 0     | <b>Trap On Break</b>. Generate exception 'Breakpoint' on EBRAK instruction if this bit is set or just Halt the pipeline otherwise.
--!
--! @par Add hardware breakpoint registers (0x10028).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | add_break   | 63:0  | <b>Add HW breakpoint address</b>. Add specified address into Hardware breakpoint stack. In case of matching Instruction Pointer (pc) and any HW breakpoint there's injected EBREAK instruction on hardware level.
--!
--! @par Remove hardware breakpoint registers (0x10030).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | rem_break   | 63:0  | <b>Remove HW breakpoint address</b>. Remove specified address from Hardware breakpoints stack.
--!
--! @par Breakpoint Address Fetch registers (0x10038).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | br_address_fetch | 63:0  | <b>Breakpoint fetch address</b>. Specify address that will be ignored by Fetch stage and used Breakpoint Fetch Instruction value instead. This logic is used to avoid re-writing EBREAK into memory.
--!
--! @par Breakpoint Instruction Fetch registers (0x10040).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RW | 64h'0 | br_instr_fetch | 63:0  | <b>Breakpoint fetch instruction</b>. Specify instruction that should executed instead of fetched from memory in a case of matching Breapoint Address Fetch register and Instruction pointer (pc).
--!
--!
--! @subsection dsu_local Local DSU Region (32 KB)
--!
--! @par Soft Reset registers (0x18000).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 63 | RW | 63h'0 | Reserved   | 63:1  | Reserved.
--! | 1  | RW | 1b'0  | soft_reset | 0     | <b>Soft Reset</b>. Status bit is set when CPU was halted due the EBREAK instruction.
--!
--! @par Miss Access counter registers (0x18008).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RO | 64h'0  | miss_access_cnt | 63:0 | <b>Miss Access counter</b>. This value as an additional debugging informantion provided by AXI Controller. It is possible to enable interrupt generation in Interrupt Controller on miss-access.
--!
--! @par Miss Access Address registers (0x18010).
--!
--! |Bits|Type| Reset | Field Name | Bits  | Description 
--! |:--:|:--:|:-----:|:----------:|:-----:|:------------------------------------------------------------|
--! | 64 | RO | 64h'0  | miss_access_addr | 63:0 | <b>Miss Access address</b>. Address of the latest miss-accessed transaction. This information comes from AXI Controller.
--!
--! @par Bus Utilization registers (0x18040 + n*2*sizeof(uint64_t)).
--!
--! |Offset |Bits|Type| Reset | Name  | Definition 
--! |:------|:--:|:--:|:-----:|:-----:|---------------------------------------------|
--! |0x18040| 64 | RO | 64h'0 | w_cnt | <b>Write transactions counter for master 0</b>. Master 0 is the RIVER CPU by default.
--! |0x18048| 64 | RO | 64h'0 | r_cnt | <b>Read transactions counter for master 0</b>.
--! |0x18050| 64 | RO | 64h'0 | w_cnt | <b>Write transactions counter for master 1</b>. Master 1 is unused in a case of configuration with RIVER CPU.
--! |0x18058| 64 | RO | 64h'0 | r_cnt | <b>Read transactions counter for master 1</b>.
--! |0x18060| 64 | RO | 64h'0 | w_cnt | <b>Write transactions counter for master 2</b>. Master 2 is the GRETH by default (Ethernet Controller with master interface). 
--! |0x18068| 64 | RO | 64h'0 | r_cnt | <b>Read transactions counter for master 2</b>.


--! VHDL base library.
library ieee;
--! VHDL base types import
use ieee.std_logic_1164.all;
--! VHDL base numeric import
use ieee.numeric_std.all;
--! SoC common functionality library.
library commonlib;
--! SoC common types import
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;
--! River top level with AMBA interface module declaration
use riverlib.types_river.all;

entity axi_dsu is
  generic (
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    o_dporti : out dport_in_type;
    i_dporto : in dport_out_type;
    --! reset CPU and interrupt controller
    o_soft_rst : out std_logic;
    -- Platfrom run-time statistic
    i_miss_irq  : in std_logic;
    i_miss_addr : in std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
    i_bus_util_w : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0);
    i_bus_util_r : in std_logic_vector(CFG_NASTI_MASTER_TOTAL-1 downto 0)
  );
end;

architecture arch_axi_dsu of axi_dsu is

  constant xconfig : nasti_slave_config_type := (
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES,
     irq_idx => 0,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_DSU
  );

type state_type is (reading, writting, dport_response, ready);
  
type mst_utilization_type is array (0 to CFG_NASTI_MASTER_TOTAL-1) 
       of std_logic_vector(63 downto 0);

type mst_utilization_map_type is array (0 to 2*CFG_NASTI_MASTER_TOTAL-1) 
       of std_logic_vector(63 downto 0);

type registers is record
  bank_axi : nasti_slave_bank_type;
  --! Message multiplexer to form 32->64 request message
  state         : state_type;
  waddr : std_logic_vector(13 downto 0);
  wdata : std_logic_vector(63 downto 0);
  rdata : std_logic_vector(63 downto 0);
  soft_rst : std_logic;
  -- Platform statistic:
  clk_cnt : std_logic_vector(63 downto 0);
  miss_access_cnt : std_logic_vector(63 downto 0);
  miss_access_addr : std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto 0);
  util_w_cnt : mst_utilization_type;
  util_r_cnt : mst_utilization_type;
end record;

signal r, rin: registers;
begin

  comblogic : process(i_axi, i_dporto, i_miss_irq, i_miss_addr,
                      i_bus_util_w, i_bus_util_r, r)
    variable v : registers;
    variable mux_rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    variable vdporti : dport_in_type;
    variable iraddr : integer;
    variable wb_bus_util_map : mst_utilization_map_type;
  begin
    v := r;
    v.rdata := (others => '0');
    vdporti.valid := '0';
    vdporti.write := '0';
    vdporti.region := (others => '0');
    vdporti.addr := (others => '0');
    vdporti.wdata := (others => '0');
    
    -- Update statistic:
    v.clk_cnt := r.clk_cnt + 1;
    if i_miss_irq = '1' then
      v.miss_access_addr := i_miss_addr;
      v.miss_access_cnt := r.miss_access_cnt + 1;
    end if;

    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
       if i_bus_util_w(n) = '1' then
         v.util_w_cnt(n) := r.util_w_cnt(n) + 1;
       end if;
       if i_bus_util_r(n) = '1' then
         v.util_r_cnt(n) := r.util_r_cnt(n) + 1;
       end if;
    end loop;

    for n in 0 to CFG_NASTI_MASTER_TOTAL-1 loop
      wb_bus_util_map(2*n) := r.util_w_cnt(n);
      wb_bus_util_map(2*n+1) := r.util_r_cnt(n);
    end loop;

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);
    --! redefine value 'always ready' inserting waiting states.
    v.bank_axi.rwaitready := '0';

    if r.bank_axi.wstate = wtrans then
      -- 32-bits burst transaction
      v.waddr := r.bank_axi.waddr(0)(16 downto 3);
      if r.bank_axi.wburst = NASTI_BURST_INCR and r.bank_axi.wsize = 4 then
         if r.bank_axi.waddr(0)(2) = '1' then
             v.state := writting;
             v.wdata(63 downto 32) := i_axi.w_data(31 downto 0);
         else
             v.wdata(31 downto 0) := i_axi.w_data(31 downto 0);
         end if;
      else
         -- Write data on next clock.
         if i_axi.w_strb /= X"00" then
            v.wdata := i_axi.w_data;
         end if;
         v.state := writting;
      end if;
    end if;

    case r.state is
      when reading =>
           if r.bank_axi.rstate = rtrans then
              if r.bank_axi.raddr(0)(16 downto 15) = "11" then
                --! local region
                v.bank_axi.rwaitready := '1';
                iraddr := conv_integer(r.bank_axi.raddr(0)(14 downto 3));
                case iraddr is
                when 0 =>
                  v.rdata(0) := r.soft_rst;
                when 1 =>
                  v.rdata := r.miss_access_cnt;
                when 2 =>
                  v.rdata(CFG_NASTI_ADDR_BITS-1 downto 0) := r.miss_access_addr;
                when others =>
                  if (iraddr >= 8) and (iraddr < (8 + 2*CFG_NASTI_MASTER_TOTAL)) then
                      v.rdata := wb_bus_util_map(iraddr - 8);
                  end if;
                end case;
                v.state := ready;
              else
                --! debug port regions: 0 to 2
                vdporti.valid := '1';
                vdporti.write := '0';
                vdporti.region := r.bank_axi.raddr(0)(16 downto 15);
                vdporti.addr := r.bank_axi.raddr(0)(14 downto 3);
                vdporti.wdata := (others => '0');
                v.state := dport_response;
              end if;
           end if;
      when writting =>
           v.state := reading;
           if r.waddr(13 downto 12) = "11" then
             --! local region
             case conv_integer(r.waddr(11 downto 0)) is
             when 0 =>
               v.soft_rst := r.wdata(0);
             when others =>
             end case;
           else
             --! debug port regions: 0 to 2
             vdporti.valid := '1';
             vdporti.write := '1';
             vdporti.region := r.waddr(13 downto 12);
             vdporti.addr := r.waddr(11 downto 0);
             vdporti.wdata := r.wdata;
           end if;
      when dport_response =>
             v.state := ready;
             v.bank_axi.rwaitready := '1';
             v.rdata := i_dporto.rdata;
      when ready =>
             v.state := reading;
      when others =>
    end case;

    if r.bank_axi.raddr(0)(2) = '0' then
       mux_rdata(31 downto 0) := r.rdata(31 downto 0);
    else
       -- 32-bits aligned access (can be generated by MAC)
       mux_rdata(31 downto 0) := r.rdata(63 downto 32);
    end if;
    mux_rdata(63 downto 32) := r.rdata(63 downto 32);

    o_axi <= functionAxi4Output(r.bank_axi, mux_rdata);

    rin <= v;

    o_dporti <= vdporti;
  end process;

  o_cfg  <= xconfig;
  o_soft_rst <= r.soft_rst;


  -- registers:
  regs : process(clk, nrst)
  begin 
    if nrst = '0' then 
       r.bank_axi <= NASTI_SLAVE_BANK_RESET;
       r.state <= reading;
       r.waddr <= (others => '0');
       r.wdata <= (others => '0');
       r.rdata <= (others => '0');
       r.soft_rst <= '0';
       r.clk_cnt <= (others => '0');
       r.miss_access_cnt <= (others => '0');
       r.miss_access_addr <= (others => '0');
       r.util_w_cnt <= (others => (others => '0'));
       r.util_r_cnt <= (others => (others => '0'));
    elsif rising_edge(clk) then 
       r <= rin; 
    end if; 
  end process;
end;
