----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Declaration types_mem package components.
------------------------------------------------------------------------------
--! Standard library
library ieee;
use ieee.std_logic_1164.all;
--! Provide common generic log() function
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;

--! @brief      Declaration of 'virtual' Memory components.
package types_mem is

  --! @brief   Declaration of the "virtual" BootROM component.
  --! @details BootRom start address must implements address matching to the
  --!          CPU reset vector (0x200) and all processing after power-on is
  --!          using this memory block. BootRom size depends of the configuration
  --!          and size of the generated hex file. 
  --!          Component implements one-clock access to the
  --!          ROM without wait-staits. Datawidth depends of the AXI4 bus
  --!          configuration.
  --! @param[in] tech    Generic technology selector.
  --! @param[in] hex_filename     Generic argument defining hex-file location.
  --! @param[in] clk     System bus clock.
  --! @param[in] address Input address.
  --! @param[out] data   Output data value.
  component BootRom_tech is
  generic (
    memtech : integer := 0;
    sim_hexfile : string
  );
  port (
    clk       : in std_logic;
    address   : in global_addr_array_type;
    data      : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
  end component;


  --! @brief   Declaration of the "virtual" RomImage component.
  --! @details This module stores pre-built firmware image that is coping
  --!          into internal SRAM during Boot stage without any modificaiton.
  --!          RomImage size is limited by global configuration parameter and
  --!          it cannot be more than internal SRAM size.  Component implements
  --!          one-clock access to the ROM without wait-staits. 
  --!          Datawidth depends of the AXI4 bus configuration.
  --! @param[in] tech    Generic technology selector.
  --! @param[in] sim_hexfile     Generic argument defining hex-file location.
  --! @param[in] clk     System bus clock.
  --! @param[in] address Input address.
  --! @param[out] data   Output data value.
  component RomImage_tech is
  generic (
    memtech : integer := 0;
    sim_hexfile : string
  );
  port (
    clk       : in std_logic;
    address   : in global_addr_array_type;
    data      : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
  end component;

  ------------------------------------------------------------------------------
  --! @brief   Galileo PRN codes ROM storage:
  --! @details This ROM is used in FSE Engine to form reference E1 reference
  --!          signals. HEX-file isn't used for this ROM because 'inferred'
  --!          module was built using "case when" operators.
  component RomPrn_tech is
  generic (
    generic_tech : integer := 0
  );
  port (
    i_clk       : in std_logic;
    i_address   : in std_logic_vector(12 downto 0);
    o_data      : out std_logic_vector(31 downto 0)
  );
  end component;

  --! @brief   Declaration of the "virtual" SRAM component with unaligned access.
  --! @details This module implements internal SRAM and support unaligned access 
  --!          without wait-states. For example it allows to read 4 bytes from
  --!          address 0x3 for one clock.
  --!          Component implements one-clock access without wait-staits. 
  --!          Datawidth depends of the AXI4 bus configuration.
  --! @param[in] memtech Generic technology selector.
  --! @param[in] abits   Generic argument defining SRAM size as 2**abits.
  --! @param[in] clk     System bus clock.
  --! @param[in] raddr   Read address.
  --! @param[out] rdata  Output data value.
  --! @param[in] waddr   Write address.
  --! @param[in] we      Write enable.
  --! @param[in] wstrb   Byte selector to form write only for the specified bytes.
  --! @param[in] wdata   Write data.
  component srambytes_tech is
  generic (
    memtech : integer := 0;
    abits   : integer := 16;
    init_file : string := ""
  );
  port (
    clk       : in std_logic;
    raddr     : in global_addr_array_type;
    rdata     : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
    waddr     : in global_addr_array_type;
    we        : in std_logic;
    wstrb     : in std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);
    wdata     : in std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
  end component;


  --! @brief Virtual SRAM block with fixed 32-bits data width.
  --! @details This module doesn't support byte access and always implements
  --!          4-bytes alignment.
  component Ram32_tech
  generic (
    generic_tech   : integer := 0;
    generic_kWords : integer := 1
  );
  port (
    i_clk      : in std_logic;
    i_address  : in std_logic_vector(10+log2(generic_kWords)-1 downto 0);
    i_wr_ena   : in std_logic;
    i_data     : in std_logic_vector(31 downto 0);
    o_data     : out std_logic_vector(31 downto 0)
  );
  end component;

  --! @brief Virtual SRAM block with fixed 64-bits data width.
  --! @details This module doesn't support byte access and always implements
  --!          8-bytes alignment.
  component Ram64_tech
  generic (
    generic_tech   : integer := 0;
    generic_abits  : integer := 4
  );
  port (
    i_clk      : in std_logic;
    i_address  : in std_logic_vector(generic_abits-1 downto 0);
    i_wr_ena   : in std_logic;
    i_data     : in std_logic_vector(63 downto 0);
    o_data     : out std_logic_vector(63 downto 0)
  );
  end component;

  --! @brief dual-port RAM declaration.
  component syncram_2p_tech is
  generic (
    tech : integer := 0;
    abits : integer := 6;
    dbits : integer := 8;
    sepclk : integer := 0;
    wrfst : integer := 0;
    testen : integer := 0;
    words : integer := 0;
    custombits : integer := 1
  );
  port (
    rclk     : in std_ulogic;
    renable  : in std_ulogic;
    raddress : in std_logic_vector((abits -1) downto 0);
    dataout  : out std_logic_vector((dbits -1) downto 0);
    wclk     : in std_ulogic;
    write    : in std_ulogic;
    waddress : in std_logic_vector((abits -1) downto 0);
    datain   : in std_logic_vector((dbits -1) downto 0)
  );
  end component;

end;
