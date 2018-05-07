----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Internal SRAM implementation with the byte access.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

library commonlib;
use commonlib.types_common.all;

library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;

--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;

entity srambytes_tech is
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
end;

architecture rtl of srambytes_tech is

--! reduced name of configuration constant:
constant dw : integer := CFG_NASTI_ADDR_OFFSET;

type local_addr_type is array (0 to CFG_NASTI_DATA_BYTES-1) of
   std_logic_vector(abits-dw-1 downto 0);

signal address : local_addr_type;
signal wr_ena : std_logic_vector(CFG_NASTI_DATA_BYTES-1 downto 0);

  --! @brief   Declaration of the one-byte SRAM element.
  --! @details This component is used for the FPGA implementation.
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

  --! @brief   Declaration of the one-byte SRAM element with init function.
  --! @details This component is used for the RTL simulation.
  component sram8_inferred_init is
  generic (
     abits     : integer := 12;
     byte_idx  : integer := 0;
     init_file : string
  );
  port (
    clk     : in  std_ulogic;
    address : in  std_logic_vector(abits-1 downto 0);
    rdata   : out std_logic_vector(7 downto 0);
    we      : in  std_logic;
    wdata   : in  std_logic_vector(7 downto 0)
  );
  end component;

begin

     
  --! Instantiate component for RTL simulation
  rtlsim0 : if memtech = inferred generate
    rx : for n in 0 to CFG_NASTI_DATA_BYTES-1 generate

      wr_ena(n) <= we and wstrb(n);
      address(n) <= waddr(n / CFG_ALIGN_BYTES)(abits-1 downto dw) when we = '1'
                else raddr(n / CFG_ALIGN_BYTES)(abits-1 downto dw);
                  
      x0 : sram8_inferred_init generic map 
      (
          abits => abits-dw,
          byte_idx => n,
          init_file => init_file
      ) port map (
          clk, 
          address => address(n),
          rdata => rdata(8*(n+1)-1 downto 8*n),
          we => wr_ena(n), 
          wdata => wdata(8*(n+1)-1 downto 8*n)
      );
    end generate; -- cycle
  end generate; -- tech=inferred


  --! Instantiate component for FPGA (checked with Xilinx)
  fpgasim0 : if memtech /= inferred and is_fpga(memtech) /= 0 generate
    rx : for n in 0 to CFG_NASTI_DATA_BYTES-1 generate

      wr_ena(n) <= we and wstrb(n);
      address(n) <= waddr(n / CFG_ALIGN_BYTES)(abits-1 downto dw) when we = '1'
                else raddr(n / CFG_ALIGN_BYTES)(abits-1 downto dw);

      x0 : sram8_inferred generic map 
      (
          abits => abits-dw,
          byte_idx => n
      ) port map (
          clk, 
          address => address(n),
          rdata => rdata(8*(n+1)-1 downto 8*n),
          we => wr_ena(n), 
          wdata => wdata(8*(n+1)-1 downto 8*n)
      );
    end generate; -- cycle
  end generate; -- tech=inferred
  
end; 


