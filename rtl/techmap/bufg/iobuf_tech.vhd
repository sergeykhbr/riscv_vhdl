----------------------------------------------------------------------------
--! @file
--! @copyright  Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author     Sergey Khabarov
--! @brief      Virtual IO buffer.
----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;


entity iobuf_tech is
  generic
  (
    generic_tech : integer := 0
  );
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
end; 
 
architecture rtl of iobuf_tech is

component iobuf_inferred is
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
end component; 

component iobuf_virtex6 is
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
end component; 

component iobuf_micron180 is
  port (
    o  : out std_logic;
    io : inout std_logic;
    i  : in std_logic;
    t  : in std_logic
  );
end component; 

begin

  inf0 : if generic_tech = inferred generate 
    bufinf : iobuf_inferred port map
    (
      o => o,
      io => io,
      i => i,
      t => t
    );
  end generate;

  xv6 : if generic_tech = virtex6 or generic_tech = kintex7 or generic_tech = zynq7000 generate 
    bufv6 : iobuf_virtex6 port map
    (
      o => o,
      io => io,
      i => i,
      t => t
    );
  end generate;

  m180 : if generic_tech = mikron180 generate 
    bufm : iobuf_micron180 port map
    (
      o => o,
      io => io,
      i => i,
      t => t
    );
  end generate;

end;
