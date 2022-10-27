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
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_cache.all;

entity mpu  is generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_iaddr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_daddr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_region_we : in std_logic;
    i_region_idx : in std_logic_vector(CFG_MPU_TBL_WIDTH-1 downto 0);
    i_region_addr : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_region_mask : in std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
    i_region_flags : in std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);  -- {ena, cachable, r, w, x}
    o_iflags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    o_dflags : out std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0)
  );
end; 
 
architecture arch_mpu of mpu is

  type MpuTableItemType is record
      addr : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      mask : std_logic_vector(CFG_CPU_ADDR_BITS-1 downto 0);
      flags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
  end record;

  type mpu_tbl_type is array (0 to CFG_MPU_TBL_SIZE-1) of MpuTableItemType;

  signal tbl : mpu_tbl_type;
  signal rin_tbl : mpu_tbl_type;

begin

  comb : process(i_nrst, i_iaddr, i_daddr, tbl,
                 i_region_we, i_region_idx, i_region_addr, i_region_mask, i_region_flags)
    variable v_item : MpuTableItemType;
    variable v_tbl : mpu_tbl_type;
    variable v_iflags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);
    variable v_dflags : std_logic_vector(CFG_MPU_FL_TOTAL-1 downto 0);

  begin

    v_tbl := tbl;
    v_iflags := (others => '1');
    v_dflags := (others => '1');

    v_item.flags := i_region_flags;
    if i_region_flags(CFG_MPU_FL_ENA) = '1' then
        v_item.addr := i_region_addr;
        v_item.mask := i_region_mask;
    else
        v_item.addr := (others => '1');
        v_item.mask := (others => '1');
    end if;


    for i in 0 to CFG_MPU_TBL_SIZE-1 loop
        if tbl(i).addr = (i_iaddr and tbl(i).mask) then
            if tbl(i).flags(CFG_MPU_FL_ENA) = '1' then
                v_iflags := tbl(i).flags;
            end if;
        end if;

        if tbl(i).addr = (i_daddr and tbl(i).mask) then
            if tbl(i).flags(CFG_MPU_FL_ENA) = '1' then
                v_dflags := tbl(i).flags;
            end if;
        end if;
    end loop;


    if i_region_we = '1' then
        v_tbl(conv_integer(i_region_idx)) := v_item;
    end if;


    if not async_reset and i_nrst = '0' then
        for i in 0 to CFG_MPU_TBL_SIZE-1 loop
            v_tbl(i).flags := (others => '0');
            v_tbl(i).addr := (others => '0');
            v_tbl(i).mask := (others => '1');
        end loop;

        -- All address above 0x80000000 are uncached (IO devices)
        v_tbl(0).addr(31 downto 0) := X"80000000";
        v_tbl(0).mask(31 downto 0) := X"80000000";
        v_tbl(0).flags(CFG_MPU_FL_ENA) := '1';
        v_tbl(0).flags(CFG_MPU_FL_CACHABLE) := '0';
        v_tbl(0).flags(CFG_MPU_FL_EXEC) := '1';
        v_tbl(0).flags(CFG_MPU_FL_RD) := '1';
        v_tbl(0).flags(CFG_MPU_FL_WR) := '1';


        -- (debug) Make first 128 Byte uncachable to test MPU
        v_tbl(1).addr(31 downto 0) := X"00000000";
        v_tbl(1).mask(31 downto 0) := X"FFFFFF80";
        v_tbl(1).flags(CFG_MPU_FL_ENA) := '1';
        v_tbl(1).flags(CFG_MPU_FL_CACHABLE) := '0';
        v_tbl(1).flags(CFG_MPU_FL_EXEC) := '1';
        v_tbl(1).flags(CFG_MPU_FL_RD) := '1';
        v_tbl(1).flags(CFG_MPU_FL_WR) := '1';
    end if;

    rin_tbl <= v_tbl;

    o_iflags <= v_iflags;
    o_dflags <= v_dflags;
  end process;


  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
        for i in 0 to CFG_MPU_TBL_SIZE-1 loop
            tbl(i).flags <= (others => '0');
            tbl(i).addr <= (others => '0');
            tbl(i).mask <= (others => '1');
        end loop;

        -- All address above 0x80000000 are uncached (IO devices)
        tbl(0).addr(31 downto 0) <= X"80000000";
        tbl(0).mask(31 downto 0) <= X"80000000";
        tbl(0).flags(CFG_MPU_FL_ENA) <= '1';
        tbl(0).flags(CFG_MPU_FL_CACHABLE) <= '0';
        tbl(0).flags(CFG_MPU_FL_EXEC) <= '1';
        tbl(0).flags(CFG_MPU_FL_RD) <= '1';
        tbl(0).flags(CFG_MPU_FL_WR) <= '1';

     elsif rising_edge(i_clk) then 
        tbl <= rin_tbl;
     end if; 
  end process;

end;
