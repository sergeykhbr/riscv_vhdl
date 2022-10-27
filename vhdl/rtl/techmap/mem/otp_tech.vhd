--!
--! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
--!
--! Unless required by applicable law or agreed to in writing, software
--! distributed under the License is distributed on an "AS IS" BASIS,
--! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--! See the License for the specific language governing permissions and
--! limitations under the License.
--!

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;
use techmap.types_mem.all;
library commonlib;
use commonlib.types_common.all;

entity otp_tech is
generic (
    memtech : integer := 0
);
port (
    clk      : in std_logic;  -- only for FPGA
    i_we     : in  std_ulogic;
    i_re     : in  std_ulogic;
    i_addr   : in std_logic_vector(11 downto 0);
    i_wdata  : in std_logic_vector(15 downto 0);
    o_rdata  : out std_logic_vector(15 downto 0);
    io_gnd   : inout std_logic;
    io_vdd   : inout std_logic;
    io_vdd18 : inout std_logic;
    io_upp   : inout std_logic
);
end;

architecture rtl of otp_tech is

  component otp_clocked is
  port (
    clk     : in  std_ulogic;
    we      : in  std_ulogic;
    re      : in  std_ulogic;
    address : in std_logic_vector(11 downto 0);
    wdata   : in std_logic_vector(15 downto 0);
    rdata   : out std_logic_vector(15 downto 0)
  );
  end component;

  component OTP_MEM_BLOCK_CORE_AUG18_v1rev1 is 
  port (
    D_I : in std_logic_vector(15 downto 0);
    D_A : in std_logic_vector(11 downto 0);
    WE_I : in std_logic;
    RE_I : in std_logic;
    D_O : out std_logic_vector(15 downto 0);
    GND : inout std_logic;
    VDD : inout std_logic;
    VDD18 : inout std_logic;
    UPP : inout std_logic
  );
  end component;

begin

  genotp0 : if memtech = inferred or is_fpga(memtech) /= 0 generate
    inf0 : otp_clocked port map (
        clk     => clk,   -- FPGA only
        we      => i_we,
        re      => i_re,
        address => i_addr,
        wdata   => i_wdata,
        rdata   => o_rdata
      );
  end generate;

  genotp1 : if memtech = mikron180 generate
      mik180 : OTP_MEM_BLOCK_CORE_AUG18_v1rev1 port map (
        D_I => i_wdata,
        D_A => i_addr,
        WE_I => i_we,
        RE_I => i_re,
        D_O => o_rdata,
        GND => io_gnd,
        VDD => io_vdd,
        VDD18 => io_vdd18,
        UPP => io_upp
      );
  end generate;
end; 


