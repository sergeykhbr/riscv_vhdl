--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

library techmap;
use techmap.gencomp.all;

package config_target is
-- Technology and synthesis options
  constant CFG_FABTECH : integer := zynq7000;
  constant CFG_MEMTECH : integer := zynq7000;
  constant CFG_PADTECH : integer := zynq7000;
  constant CFG_JTAGTECH : integer := zynq7000;

  constant CFG_TOPDIR : string := "../../../";
  
  constant CFG_TARGET_ETHERNET_ENABLE : boolean := false;
end;
