-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     FIFO on 4096 samples. Each sample is 4-bits width.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;

entity FifoSection is
generic (
   sys : integer := GEN_SYSTEM_GPSCA
);
port (
    i_nrst : in std_logic;
    i_clk  : in std_logic;
    i_ena  : in std_logic;
    i_I    : in std_logic_vector(1 downto 0);
    i_Q    : in std_logic_vector(1 downto 0);
    o_I    : out std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
    o_Q    : out std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
    o_rdyI : out std_logic;
    o_rdyQ : out std_logic
);
end;

architecture rtl of FifoSection is


type regtype is record
  I       : std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
  Q       : std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
  rdyI    : std_logic;
  rdyQ    : std_logic;
end record;

signal r, rin : regtype;

begin

  comb : process (r, i_nrst, i_ena, i_I, i_Q)
  variable v : regtype;
  begin

    v := r;
    if i_ena = '1' then
      v.I := i_I & r.I(2*fse_section_num(sys)*1024-1 downto 2);
      v.Q := i_Q & r.Q(2*fse_section_num(sys)*1024-1 downto 2);
    end if;

    v.rdyI := i_ena;
    v.rdyQ := r.rdyI;


    -- Reset all
    if i_nrst = '0' then
      v.rdyI := '0';
      v.rdyQ := '0';
      v.I := (others =>'0');
      v.Q := (others =>'0');
    end if;
    
    rin <= v;
  end process; 


  o_I <= r.I;
  o_Q <= r.Q;
  o_rdyI <= r.rdyI;
  o_rdyQ <= r.rdyQ;

  reg : process(i_clk)
  begin 
    if rising_edge(i_clk) then 
      r <= rin;
    end if;
  end process;

end; 


