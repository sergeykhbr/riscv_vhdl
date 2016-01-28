------------------------------------------------------------------------------
--  INFORMATION:   http://www.GNSS-sensor.com
--  PROPERTY:      GNSS Sensor Ltd
--  Package:       fse_v2
--  Author:        Sergey Khabarov - sergeykhbr@gmail.com
------------------------------------------------------------------------------
--  Description:   Data structures and modules declaration for the 
--                 FSE-2 Top Level
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library techmap;
use techmap.gencomp.all;
library gnsslib;
use gnsslib.types_gnss.all;
library rocketlib;
use rocketlib.types_nasti.all;

package types_fse_v2 is

-- !! In REAL ASIC/FPGA DEVICE THIS VALUE SHOULD BE EQUAL/GRATER THAN 32
constant CFG_FSE_HW_ID            : integer := 16#0614#;

------------------------------------------------------------------------------
-- Bridge from AXI to FSE registers
type bridge_in_type is record
  nrst             : std_logic;
  clk_bus          : std_logic;
  clk_fse          : std_logic;
  axi              : nasti_slave_in_type;
  rdata            : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  rdata_rdy        : std_logic;
end record;

type bridge_out_type is record
  cfg              : nasti_slave_config_type;
  axi              : nasti_slave_out_type;
  wr_ena           : std_logic;
  rd_ena           : std_logic;
  addr             : std_logic_vector(8 downto 0);
  data             : std_logic_vector(31 downto 0);
end record;

component AxiFseBridge
  generic
  (
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    did      : std_logic_vector(15 downto 0) := GNSSSENSOR_FSE_V2
  );
port (
    i : in bridge_in_type;
    o : out bridge_out_type
);
end component;

------------------------------------------------------------------------------
-- FSE Control module
type fsectrl_in_type is record
  nrst             : std_logic;
  clk              : std_logic;
  wr_ena           : std_logic;
  rd_ena           : std_logic;
  addr             : std_logic_vector(8 downto 0);
  wdata            : std_logic_vector(31 downto 0);
  ms_pulse         : std_logic;
  pps              : std_logic;
  adc_valid        : std_logic;
  rec_rdy          : std_logic;
  latch_max        : std_logic_vector(31 downto 0);
  latch_ind        : std_logic_vector(11 downto 0);
  latch_noise      : std_logic_vector(31 downto 0);
  latch_dopler     : std_logic_vector(31 downto 0);
  latch_rdy        : std_logic;
end record;

type fsectrl_out_type is record
  rdata            : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);
  rdata_rdy        : std_logic;
  carrnco_th       : std_logic_vector(31 downto 0);
  codenco_th       : std_logic_vector(31 downto 0);
  carr_nco_if      : std_logic_vector(31 downto 0);
  dopler_nco       : std_logic_vector(31 downto 0);
  code_nco         : std_logic_vector(31 downto 0);
  adc_fmt          : std_logic;
  coh_ena          : std_logic;
  ms_total         : std_logic_vector(9 downto 0);
  new_cycle        : std_logic;  -- selecting new channel or selecting new doppler
  new_prn          : std_logic;  -- channel was selected, generate pulse
  rst_max          : std_logic;  -- idle or selecting new channel
  prn              : std_logic_vector(9 downto 0);
  rec_ena          : std_logic;
  play_ena         : std_logic;
end record;

component FseControl
generic (
    generic_sys    : integer := 0
);
port (
    i : in fsectrl_in_type;
    o : out fsectrl_out_type
);
end component;


------------------------------------------------------------------------------
-- Low-pass Filer
type lpf_in_type is record
  nrst             : std_logic;
  clk              : std_logic;
  ena              : std_logic;
  I                : std_logic_vector(15 downto 0);
  Q                : std_logic_vector(15 downto 0);
end record;

type lpf_out_type is record
  rdy              : std_logic;
  flt              : std_logic_vector(15 downto 0);
end record;

component LpFilter
generic (
    generic_rate    : integer := 12
);
port (
    i : in lpf_in_type;
    o : out lpf_out_type
);
end component;

------------------------------------------------------------------------------
-- Requant 16 bits value into sign/magn 2-bits value
type rqnt_in_type is record
  nrst             : std_logic;
  clk              : std_logic;
  ena              : std_logic;
  I                : std_logic_vector(15 downto 0);
  Q                : std_logic_vector(15 downto 0);
end record;

type rqnt_out_type is record
  I                : std_logic_vector(1 downto 0);
  Q                : std_logic_vector(1 downto 0);
  rdy              : std_logic;
end record;

component Requant
port (
    i : in rqnt_in_type;
    o : out rqnt_out_type
);
end component;

------------------------------------------------------------------------------
-- Decimator of the ADC samples into 4096 MHz
type decim_in_type is record
  nrst             : std_logic;
  clk              : std_logic;
  ena              : std_logic;
  code_nco_th      : std_logic_vector(31 downto 0);
  code_nco         : std_logic_vector(31 downto 0);
  I                : std_logic_vector(6 downto 0);
  Q                : std_logic_vector(6 downto 0);
end record;

type decim_out_type is record
  I                : std_logic_vector(15 downto 0);
  Q                : std_logic_vector(15 downto 0);
  rdy              : std_logic;
end record;

component Decimator
port (
    i : in decim_in_type;
    o : out decim_out_type
);
end component;


------------------------------------------------------------------------------
-- Integer diver for the Carrier NCO module
type dphs_in_type is record
  carr_th   : std_logic_vector(31 downto 0);
  carr_acc  : std_logic_vector(37 downto 0);
end record;

type dphs_out_type is record
  phase_cnt   : std_logic_vector(5 downto 0);
end record;

component Divphase
port (
    i       : in dphs_in_type;
    o       : out dphs_out_type
);
end component;

------------------------------------------------------------------------------
-- Carrier Nco
type carnco_in_type is record
  nrst     : std_logic;
  clk      : std_logic;
  ena      : std_logic;
  I        : std_logic_vector(2 downto 0);
  Q        : std_logic_vector(2 downto 0);
  thresh   : std_logic_vector(31 downto 0);
  nco      : std_logic_vector(31 downto 0);
end record;

type carnco_out_type is record
  I     : std_logic_vector(6 downto 0);
  Q     : std_logic_vector(6 downto 0);
  rdy   : std_logic;
end record;

component CarrierNco
port (
    i : in carnco_in_type;
    o : out carnco_out_type
);
end component;

------------------------------------------------------------------------------
-- PRN generator
component PrnGeneratorFse
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
end component;

------------------------------------------------------------------------------
-- Samples recorder 2+2 bits width per sample
type rec_in_type is record
  nrst          : std_logic;
  clk           : std_logic;
  I             : std_logic_vector(1 downto 0);
  Q             : std_logic_vector(1 downto 0);
  adc_valid     : std_logic;
  rec_ena       : std_logic;
  play_ena      : std_logic;
  test_mode     : std_logic;
end record;

type rec_out_type is record
  I             : std_logic_vector(1 downto 0);
  Q             : std_logic_vector(1 downto 0);
  rdy           : std_logic;
  rec_end       : std_logic;
end record;

component Recorder
generic
(
    generic_tech   : integer := 0;
    generic_sys    : integer := GEN_SYSTEM_GPSCA
);
port (
    i : in rec_in_type;
    o : out rec_out_type
);
end component;

------------------------------------------------------------------------------
-- FIFO on complex samples
component FifoSection
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
end component;

------------------------------------------------------------------------------
-- Matrix correlator
component MatrixCorrelator
generic (
    sys : integer := GEN_SYSTEM_GPSCA
);
port (
    i_nrst          : in std_logic;
    i_clk           : in std_logic;
    i_ena           : in std_logic;
    i_re_im         : in std_logic;
    i_prn           : in std_logic_vector(fse_section_num(sys)*1024-1 downto 0);
    i_I             : in std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
    i_Q             : in std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
    o_I             : out std_logic_vector(14 downto 0);
    o_Q             : out std_logic_vector(14 downto 0);
    o_rdy           : out std_logic
);
end component;

------------------------------------------------------------------------------
-- Coherent/Non-coherent accumulator
type accm_in_type is record
  nrst         : std_logic;
  clk          : std_logic;
  ms_total     : std_logic_vector(9 downto 0);
  dopler       : std_logic_vector(31 downto 0);
  coh_ena      : std_logic;
  new_cycle    : std_logic;
  rst_max      : std_logic;
  ena          : std_logic;
  I            : std_logic_vector(15 downto 0);
  Q            : std_logic_vector(15 downto 0);
end record;

type accm_out_type is record
  max          : std_logic_vector(31 downto 0);
  maxind       : std_logic_vector(11 downto 0);
  noise        : std_logic_vector(31 downto 0);
  dopler       : std_logic_vector(31 downto 0);
  rdy          : std_logic;
end record;

component Accumulator
generic
(
    generic_tech   : integer := 0;
    generic_sys    : integer := GEN_SYSTEM_GPSCA
);
port (
    i : in accm_in_type;
    o : out accm_out_type
);
end component;


------------------------------------------------------------------------------
-- Convert 2-bits ADC samples into Carrier Nco input signal (signed 3 bits):
function adc2amp ( tp  : in  std_logic;
                   val : in  std_logic_vector(1 downto 0)) 
                   return std_logic_vector;

end;

------------------------------------------------------------------------------
-- Common functions:
package body types_fse_v2 is

function adc2amp ( tp  : in  std_logic;
                   val : in  std_logic_vector(1 downto 0)) 
                   return std_logic_vector is
  variable res : std_logic_vector (2 downto 0);
begin

  if tp = '0' then
    -- binary/offset:
    case val is
      when "00" => res := "101";
      when "01" => res := "111";
      when "10" => res := "001";
      when others => res:= "011";
    end case;
  else
    -- sign/miagnitude
    case val is
      when "00" => res := "001";
      when "01" => res := "011";
      when "10" => res := "111";
      when others => res:= "101";
    end case;
  end if;
  return(res);
end;

end;


