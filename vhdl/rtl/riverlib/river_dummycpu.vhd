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
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_river.all;

entity river_dummycpu is 
  port ( 
    o_msto   : out axi4_l1_out_type;
    o_dport  : out dport_out_type;
    o_flush_l2 : out std_logic
);
end;
 
architecture arch_river_dummycpu of river_dummycpu is

begin
    o_msto <= axi4_l1_out_none;
    o_dport <= dport_out_none;
    o_flush_l2 <= '0';
end;
