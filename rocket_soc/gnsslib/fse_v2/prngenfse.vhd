-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Code generator
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity PrnGeneratorFse is
generic (
    tech  : integer := 0;
    sys   : integer := GEN_SYSTEM_GPSCA
);
port (
    i_nrst     : in std_logic;
    i_clk      : in std_logic;
    i_ena      : in std_logic;
    i_init_g2  : in std_logic_vector(9 downto 0);
    o_prn      : out std_logic_vector(1024*fse_section_num(sys)-1 downto 0)
);
end;

architecture rtl of PrnGeneratorFse is

component PrnGeneratorFseGal is generic (
    tech  : integer := 0
);
port (
    i_nrst     : in std_logic;
    i_clk      : in std_logic;
    i_ena      : in std_logic;
    i_init_adr : in std_logic_vector(9 downto 0);
    o_prn      : out std_logic_vector(1024*fse_section_num(GEN_SYSTEM_GALE1)-1 downto 0)
);
end component;

component PrnGeneratorFseGlo is
port (
    i_nrst     : in std_logic;
    i_clk      : in std_logic;
    i_ena      : in std_logic;
    o_prn : out std_logic_vector(1024*fse_section_num(GEN_SYSTEM_GLOCA)-1 downto 0)
);
end component;

component PrnGeneratorFseGps is
port (
    i_nrst     : in std_logic;
    i_clk      : in std_logic;
    i_ena      : in std_logic;
    i_init_g2  : in std_logic_vector(9 downto 0);
    o_prn : out std_logic_vector(1024*fse_section_num(GEN_SYSTEM_GPSCA)-1 downto 0)
);
end component;

begin

  sys0: if sys = GEN_SYSTEM_GPSCA generate
     clGps : PrnGeneratorFseGps port map
     (
         i_nrst, i_clk, i_ena, i_init_g2, o_prn
     );
  end generate;

  sys1: if sys = GEN_SYSTEM_GLOCA generate
     clGlo : PrnGeneratorFseGlo port map
     (
         i_nrst, i_clk, i_ena, o_prn
     );
  end generate;

  sys3: if sys = GEN_SYSTEM_GALE1 generate
     clGal : PrnGeneratorFseGal 
     generic map (tech) 
     port map
     (
         i_nrst, i_clk, i_ena, i_init_g2, o_prn
     );
  end generate;

end; 


