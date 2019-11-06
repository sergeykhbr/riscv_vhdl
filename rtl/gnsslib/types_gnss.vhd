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
library commonlib;
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;

package types_gnss is

  component gnss_ss is
    generic (
      async_reset : boolean := false;
      tech   : integer := 0;
      xaddr  : integer := 0;
      xmask  : integer := 16#FFFFF#;
      xirq   : integer := 0
    );
    port ( 
      i_nrst : in std_logic;
      i_clk_bus : in std_logic;
      i_clk_adc : in std_logic;                   -- GNSS ADC clock (4..40 MHz)
      -- ADC samples (2 complex channels)
      i_gps_I : in std_logic_vector(1 downto 0);  -- Channel 0 sampled I value
      i_gps_Q : in std_logic_vector(1 downto 0);  -- Channel 0 sampled Q value
      i_glo_I : in std_logic_vector(1 downto 0);  -- Channel 1 sampled I value
      i_glo_Q : in std_logic_vector(1 downto 0);  -- Channel 1 sampled I value
      o_pps : out std_logic;                      -- Pulse Per Second signal
      -- MAX2769 SPIs and antenna controls signals:
      i_gps_ld    : in std_logic;                 -- Channel 0 RF front-end Lock detect
      i_glo_ld    : in std_logic;                 -- Channel 1 RF front-end Lock detect
      o_max_sclk  : out std_logic;                -- RF synthesizer SPI clock
      o_max_sdata : out std_logic;                -- RF synthesizer SPI data
      o_max_ncs   : out std_logic_vector(1 downto 0); -- RF synthesizer channel 0/1 selector
      i_antext_stat   : in std_logic;             -- Antenna powered status
      i_antext_detect : in std_logic;             -- Antenna connected status
      o_antext_ena    : out std_logic;            -- Enabling/disabling antenna
      o_antint_contr  : out std_logic;            -- Antenna Internal/External selector
      -- AXI4 interface
      o_cfg : out axi4_slave_config_type;
      i_axi : in  axi4_slave_in_type;
      o_axi : out axi4_slave_out_type;
      o_irq : out std_logic
    );
  end component;
 
end;
