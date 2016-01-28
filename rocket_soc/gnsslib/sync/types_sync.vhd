------------------------------------------------------------------------------
--  INFORMATION:  http://www.GNSS-sensor.com
--  PROPERTY:     GNSS Sensor Ltd
--  E-MAIL:       chief@gnss-sensor.com
--  DESCRIPTION:  Clock domain transition modules description
------------------------------------------------------------------------------
--  WARNING:      
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package types_sync is

  ------------------------------------------------------------------------------
  -- Reclocking from ADC to FSE clock domain
  type reclk_in_type is record
    nrst             : std_logic;
    clk_fse          : std_logic;
    clk_adc          : std_logic;
    I                : std_logic_vector(1 downto 0);
    Q                : std_logic_vector(1 downto 0);
    ms_pulse         : std_logic;
    pps              : std_logic;
  end record;

  type reclk_out_type is record
    I                : std_logic_vector(1 downto 0);
    Q                : std_logic_vector(1 downto 0);
    ms_pulse         : std_logic;
    pps              : std_logic;
    adc_valid        : std_logic;
  end record;

  component Reclk
  port (
      i : in reclk_in_type;
      o : out reclk_out_type
  );
  end component;


  ------------------------------------------------------------------------------
  -- Asynchronous clock transition fifo/pipe
  component Pipe is
  generic (
      generic_bitsz    : integer := 32;
      generic_depth    : integer := 4
  );
  port (
      i_nrst  : in std_logic;
      i_rclk  : in std_logic;
      i_wclk  : in std_logic;
      i_wena  : in std_logic;
      i_wdata : in std_logic_vector(generic_bitsz-1 downto 0);
      o_rdata : out std_logic_vector(generic_bitsz-1 downto 0);
      o_rdy   : out std_logic
  );
  end component;


  ------------------------------------------------------------------------------
  component afifo is
    generic (
        abits : integer := 4;
        dbits : integer := 8
    );
    port (
        i_nrst      : in  std_logic;
        -- Reading port.
        i_rclk      : in  std_logic;
        i_rd_ena    : in  std_logic;
        o_data      : out std_logic_vector (dbits-1 downto 0);
        o_empty     : out std_logic;
        -- Writing port.
        i_wclk      : in  std_logic;
        i_wr_ena    : in  std_logic;
        i_data      : in  std_logic_vector (dbits-1 downto 0);
        o_full      : out std_logic
	 
    );
end component;


end;
