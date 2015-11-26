----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Memory components declaration for the various technologies.
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
library rocketlib;
use rocketlib.types_nasti.all;

package allmem is

  component BootRom_tech is
  generic (
    memtech : integer := 0;
    sim_hexfile : string
  );
  port (
    clk       : in std_logic;
    address   : in std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto CFG_NASTI_ADDR_OFFSET);
    data      : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
  end component;

  component BootRom_inferred is
  generic (
    hex_filename : string
  );
  port (
    clk     : in  std_ulogic;
    address : in std_logic_vector(CFG_NASTI_ADDR_BITS-1 downto CFG_NASTI_ADDR_OFFSET);
    data    : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
  end component;

  component RomImage_tech is
  generic (
    memtech : integer := 0;
    sim_hexfile : string    --! for simulation and FPGA inferred ROM
  );
  port (
    clk       : in std_logic;
    address   : in global_addr_array_type;
    data      : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
  end component;

  component RomImage_inferred is
  generic (
    hex_filename : string
  );
  port (
    clk     : in  std_ulogic;
    address : in  global_addr_array_type;
    data    : out std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0)
  );
  end component;


  component srambytes_tech is
  generic (
    memtech : integer := 0;
    abits   : integer := 16
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

  component sram8_inferred is
  generic (
     abits : integer := 12;
     byte_idx : integer := 0
  );
  port (
    clk     : in  std_ulogic;
    address : in  std_logic_vector(abits-1 downto 0);
    rdata   : out std_logic_vector(7 downto 0);
    we      : in  std_logic;
    wdata   : in  std_logic_vector(7 downto 0)
  );
  end component;

end;
