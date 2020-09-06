--!
--! Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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
--! @brief     Group of "River" CPUs with L2-cache.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! RIVER CPU specific library.
library riverlib;
--! RIVER CPU configuration constants.
use riverlib.river_cfg.all;
--! River top level with AMBA interface module declaration
use riverlib.types_river.all;

entity river_workgroup is 
  generic (
    cpunum : integer;
    memtech : integer;
    async_reset : boolean;
    fpu_ena : boolean;
    coherence_ena : boolean;
    tracer_ena : boolean
  );
  port ( 
    i_nrst   : in std_logic;
    i_clk    : in std_logic;
    i_msti   : in axi4_master_in_type;
    o_msto   : out axi4_master_out_type;
    o_mstcfg : out axi4_master_config_type;
    i_dport  : in dport_in_vector;
    o_dport  : out dport_out_vector;
    i_ext_irq : in std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0)
);
end;
 
architecture arch_river_workgroup of river_workgroup is

  constant xconfig : axi4_master_config_type := (
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     descrtype => PNP_CFG_TYPE_MASTER,
     vid => VENDOR_GNSSSENSOR,
     did => RISCV_RIVER_WORKGROUP
  );

  signal corei : axi4_l1_in_vector;
  signal coreo : axi4_l1_out_vector;
  signal l2i : axi4_l2_in_type;
  signal l2o : axi4_l2_out_type;

begin

  o_mstcfg <= xconfig;

  --! @brief RISC-V Processor core River.
  cpuslotx : for n in 0 to CFG_TOTAL_CPU_MAX-1 generate
      cpux : if n < cpunum generate
          river0 : river_amba generic map (
            memtech => memtech,
            hartid => n,
            async_reset => async_reset,
            fpu_ena => fpu_ena,
            coherence_ena => coherence_ena,
            tracer_ena => tracer_ena
          ) port map ( 
            i_nrst => i_nrst,
            i_clk => i_clk,
            i_msti => corei(n),
            o_msto => coreo(n),
            i_dport => i_dport(n),
            o_dport => o_dport(n),
            i_ext_irq => i_ext_irq(n)
          );
      end generate;
      emptyx : if n >= cpunum generate
          cpudummy0 : river_dummycpu port map ( 
            o_msto => coreo(n),
            o_dport => o_dport(n),
            o_flush_l2 => open
          );
      end generate;
  end generate;

  l2_ena : if coherence_ena generate
      -- TODO: see Wasserfall implementation
  end generate;
  l2_dis : if not coherence_ena generate
    l2dummy0 : RiverL2Dummy generic map (
        async_reset => async_reset
      ) port map (
        i_clk => i_clk,
        i_nrst => i_nrst,
        i_l1o => coreo,
        o_l1i => corei,
        i_l2i => l2i,
        o_l2o => l2o,
        i_flush_valid => '0'
      );
  end generate;

  l2serdes0 : river_l2serdes generic map (
      async_reset => async_reset
  ) port map ( 
      i_nrst => i_nrst,
      i_clk => i_clk,
      i_l2o => l2o,
      o_l2i => l2i,
      i_msti => i_msti,
      o_msto => o_msto
  );

end;
