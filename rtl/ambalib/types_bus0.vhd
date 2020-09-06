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

--! Standard library
library ieee;
--! Standard signal types import
use ieee.std_logic_1164.all;
--! Common constants and data conversion functions library
library commonlib;
--! Import SoC specific types common for all devices
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;

package types_bus0 is

--! @defgroup slave_id_group AMBA AXI slaves generic IDs.
--! @ingroup axi4_config_generic_group
--! @details Each module in a SoC has to be indexed by unique identificator.
--!          In current implementation it is used sequential indexing for it.
--!          Indexes are used to specify a device bus item in a vectors.
--! @{

--! @brief Configuration index of the Boot ROM module visible by the firmware.
constant CFG_BUS0_XSLV_BOOTROM  : integer := 0; 
--! Configuration index of the Firmware ROM Image module.
constant CFG_BUS0_XSLV_ROMIMAGE : integer := 1;
--! Configuration index of the SRAM module visible by the firmware.
constant CFG_BUS0_XSLV_SRAM     : integer := 2;
--! Configuration index of the UART module.
constant CFG_BUS0_XSLV_UART1    : integer := 3;
--! Configuration index of the GPIO (General Purpose In/Out) module.
constant CFG_BUS0_XSLV_GPIO     : integer := 4;
--! Configuration index of the Interrupt Controller module.
constant CFG_BUS0_XSLV_IRQCTRL  : integer := 5;
--! Configuration index of the GNSS Sub-System.
constant CFG_BUS0_XSLV_GNSS_SS  : integer := 6;
--! External Flash IC connected via SPI interface.
constant CFG_BUS0_XSLV_EXTFLASH : integer := 7;
--! Configuration index of the Ethernet MAC module.
constant CFG_BUS0_XSLV_ETHMAC   : integer := 8;
--! Configuration index of the Debug Support Unit module.
constant CFG_BUS0_XSLV_DSU      : integer := 9;
--! Configuration index of the Debug Support Unit module.
constant CFG_BUS0_XSLV_GPTIMERS : integer := 10;
--! Configuration index of the Mikron OTP 8 KB bank.
constant CFG_BUS0_XSLV_OTP      : integer := 11;
--! Configuration index of the Plug-n-Play module.
constant CFG_BUS0_XSLV_PNP      : integer := 12;
--! Total number of the slaves devices.
constant CFG_BUS0_XSLV_TOTAL    : integer := 13;
--! @}

--! @defgroup master_id_group AXI4 masters generic IDs.
--! @ingroup axi4_config_generic_group
--! @details Each master must be assigned to a specific ID that used
--!          as an index in the vector array of AXI master bus.
--! @{

--! RIVER workgroup.
constant CFG_BUS0_XMST_WORKGROUP: integer := 0;
--! Ethernet MAC master interface generic index.
constant CFG_BUS0_XMST_ETHMAC   : integer := 1;
--! Tap via UART (debug port) generic index.
constant CFG_BUS0_XMST_MSTUART  : integer := 2;
--! Tap via JTAG DMI generic index.
constant CFG_BUS0_XMST_DMI      : integer := 3;
--! Total Number of master devices on system bus.
constant CFG_BUS0_XMST_TOTAL    : integer := 4;
--! @}


type bus0_xslv_cfg_vector is array (0 to CFG_BUS0_XSLV_TOTAL-1) 
       of axi4_slave_config_type;

type bus0_xmst_cfg_vector is array (0 to CFG_BUS0_XMST_TOTAL-1) 
       of axi4_master_config_type;

type bus0_xmst_out_vector is array (0 to CFG_BUS0_XMST_TOTAL-1) 
       of axi4_master_out_type;

type bus0_xmst_in_vector is array (0 to CFG_BUS0_XMST_TOTAL-1) 
       of axi4_master_in_type;

type bus0_xslv_in_vector is array (0 to CFG_BUS0_XSLV_TOTAL-1) 
       of axi4_slave_in_type;

type bus0_xslv_out_vector is array (0 to CFG_BUS0_XSLV_TOTAL-1) 
       of axi4_slave_out_type;

--! @brief   AXI bus controller. 
--! @param [in] watchdog_memop 
--! @param [in] i_clk System bus clock.
--! @param [in] i_nrst Reset with active LOW level.
--! @param [in] i_slvcfg Slaves configuration vector.
--! @param [in] i_slvo Vector of slaves output signals.
--! @param [in] i_msto Vector of masters output signals.
--! @param [out] o_slvi Vector of slave inputs.
--! @param [out] o_msti Vector of master inputs.
--! @param [out] o_bus_util_w Write access bus utilization
--! @param [out] o_bus_util_r Read access bus utilization
--! @todo    Round-robin priority algorithm.
component axictrl_bus0 is
  generic (
    async_reset : boolean
  );
  port (
    i_clk    : in std_logic;
    i_nrst   : in std_logic;
    i_slvcfg : in  bus0_xslv_cfg_vector;
    i_slvo   : in  bus0_xslv_out_vector;
    i_msto   : in  bus0_xmst_out_vector;
    o_slvi   : out bus0_xslv_in_vector;
    o_msti   : out bus0_xmst_in_vector;
    o_bus_util_w : out std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0);
    o_bus_util_r : out std_logic_vector(CFG_BUS0_XMST_TOTAL-1 downto 0)
  );
end component;

end; -- package body
