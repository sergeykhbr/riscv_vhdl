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

library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
library commonlib;
use commonlib.types_common.all;
use commonlib.types_util.all;

entity ethphy_sim is 
  generic (
    OUTPUT_ENA : std_logic := '1'
  );
  port (
    rst : in std_logic;
    clk : in std_logic;
    o_rxd  : out std_logic_vector(3 downto 0);
    o_rxdv : out std_logic
  );
end;

architecture ethphy_sim_rtl of ethphy_sim is

  type crc32type is array (0 to 255) 
	of std_logic_vector(31 downto 0);
  constant crc32_table : crc32type := (
            X"00000000", X"77073096", X"ee0e612c", X"990951ba", X"076dc419", X"706af48f", X"e963a535", X"9e6495a3",
            X"0edb8832", X"79dcb8a4", X"e0d5e91e", X"97d2d988", X"09b64c2b", X"7eb17cbd", X"e7b82d07", X"90bf1d91",
            X"1db71064", X"6ab020f2", X"f3b97148", X"84be41de", X"1adad47d", X"6ddde4eb", X"f4d4b551", X"83d385c7",
            X"136c9856", X"646ba8c0", X"fd62f97a", X"8a65c9ec", X"14015c4f", X"63066cd9", X"fa0f3d63", X"8d080df5",
            X"3b6e20c8", X"4c69105e", X"d56041e4", X"a2677172", X"3c03e4d1", X"4b04d447", X"d20d85fd", X"a50ab56b",
            X"35b5a8fa", X"42b2986c", X"dbbbc9d6", X"acbcf940", X"32d86ce3", X"45df5c75", X"dcd60dcf", X"abd13d59",
            X"26d930ac", X"51de003a", X"c8d75180", X"bfd06116", X"21b4f4b5", X"56b3c423", X"cfba9599", X"b8bda50f",
            X"2802b89e", X"5f058808", X"c60cd9b2", X"b10be924", X"2f6f7c87", X"58684c11", X"c1611dab", X"b6662d3d",
            X"76dc4190", X"01db7106", X"98d220bc", X"efd5102a", X"71b18589", X"06b6b51f", X"9fbfe4a5", X"e8b8d433",
            X"7807c9a2", X"0f00f934", X"9609a88e", X"e10e9818", X"7f6a0dbb", X"086d3d2d", X"91646c97", X"e6635c01",
            X"6b6b51f4", X"1c6c6162", X"856530d8", X"f262004e", X"6c0695ed", X"1b01a57b", X"8208f4c1", X"f50fc457",
            X"65b0d9c6", X"12b7e950", X"8bbeb8ea", X"fcb9887c", X"62dd1ddf", X"15da2d49", X"8cd37cf3", X"fbd44c65",
            X"4db26158", X"3ab551ce", X"a3bc0074", X"d4bb30e2", X"4adfa541", X"3dd895d7", X"a4d1c46d", X"d3d6f4fb",
            X"4369e96a", X"346ed9fc", X"ad678846", X"da60b8d0", X"44042d73", X"33031de5", X"aa0a4c5f", X"dd0d7cc9",
            X"5005713c", X"270241aa", X"be0b1010", X"c90c2086", X"5768b525", X"206f85b3", X"b966d409", X"ce61e49f",
            X"5edef90e", X"29d9c998", X"b0d09822", X"c7d7a8b4", X"59b33d17", X"2eb40d81", X"b7bd5c3b", X"c0ba6cad",
            X"edb88320", X"9abfb3b6", X"03b6e20c", X"74b1d29a", X"ead54739", X"9dd277af", X"04db2615", X"73dc1683",
            X"e3630b12", X"94643b84", X"0d6d6a3e", X"7a6a5aa8", X"e40ecf0b", X"9309ff9d", X"0a00ae27", X"7d079eb1",
            X"f00f9344", X"8708a3d2", X"1e01f268", X"6906c2fe", X"f762575d", X"806567cb", X"196c3671", X"6e6b06e7",
            X"fed41b76", X"89d32be0", X"10da7a5a", X"67dd4acc", X"f9b9df6f", X"8ebeeff9", X"17b7be43", X"60b08ed5",
            X"d6d6a3e8", X"a1d1937e", X"38d8c2c4", X"4fdff252", X"d1bb67f1", X"a6bc5767", X"3fb506dd", X"48b2364b",
            X"d80d2bda", X"af0a1b4c", X"36034af6", X"41047a60", X"df60efc3", X"a867df55", X"316e8eef", X"4669be79",
            X"cb61b38c", X"bc66831a", X"256fd2a0", X"5268e236", X"cc0c7795", X"bb0b4703", X"220216b9", X"5505262f",
            X"c5ba3bbe", X"b2bd0b28", X"2bb45a92", X"5cb36a04", X"c2d7ffa7", X"b5d0cf31", X"2cd99e8b", X"5bdeae1d",
            X"9b64c2b0", X"ec63f226", X"756aa39c", X"026d930a", X"9c0906a9", X"eb0e363f", X"72076785", X"05005713",
            X"95bf4a82", X"e2b87a14", X"7bb12bae", X"0cb61b38", X"92d28e9b", X"e5d5be0d", X"7cdcefb7", X"0bdbdf21",
            X"86d3d2d4", X"f1d4e242", X"68ddb3f8", X"1fda836e", X"81be16cd", X"f6b9265b", X"6fb077e1", X"18b74777",
            X"88085ae6", X"ff0f6a70", X"66063bca", X"11010b5c", X"8f659eff", X"f862ae69", X"616bffd3", X"166ccf45",
            X"a00ae278", X"d70dd2ee", X"4e048354", X"3903b3c2", X"a7672661", X"d06016f7", X"4969474d", X"3e6e77db",
            X"aed16a4a", X"d9d65adc", X"40df0b66", X"37d83bf0", X"a9bcae53", X"debb9ec5", X"47b2cf7f", X"30b5ffe9",
            X"bdbdf21c", X"cabac28a", X"53b39330", X"24b4a3a6", X"bad03605", X"cdd70693", X"54de5729", X"23d967bf",
            X"b3667a2e", X"c4614ab8", X"5d681b02", X"2a6f2b94", X"b40bbe37", X"c30c8ea1", X"5a05df1b", X"2d02ef8d");

   --constant EDCL0 : std_logic_vector(455 downto 0) := 
   --X"5d207098001032badcacefebeb800000e100450000000029bd11000c8a00a00c8a00f07788556600a02266000000002300010020009ac7c2dc";

   -- edcl_idx=6, read 0x80090000, 1 word (4 bytes)
   constant EDCL0 : std_logic_vector(455 downto 0) := 
   X"5d207098001032badcacefebeb800000e100450000000029bd11000c8a00a00c8a00f07788556600a04e44000000812000089000002ea59ac7";

  function gen_msg(edcl_idx : in integer) return std_logic_vector is
    variable crc : std_logic_vector(31 downto 0);
    variable idx : integer;
    variable wb_edcl_idx : std_logic_vector(15 downto 0);
    variable checksum : std_logic_vector(31 downto 0);
    variable byte : std_logic_vector(7 downto 0);
    variable ret : std_logic_vector(455 downto 0);
  begin
    ret := EDCL0;
    wb_edcl_idx := conv_std_logic_vector(edcl_idx, 14) & "00";
    ret(95 downto 80) := wb_edcl_idx(11 downto 8) & wb_edcl_idx(15 downto 12) 
                       & wb_edcl_idx(3 downto 0) & wb_edcl_idx(7 downto 4);
    checksum(31 downto 16) := (others => '0');
    checksum(15 downto 0) := EDCL0(123 downto 120) & EDCL0(127 downto 124) 
              & EDCL0(115 downto 112) & EDCL0(119 downto 116);

    -- [6] checksum 0x2266 => 0x224e
    --     crc = 0xa542c092
    --     5d207098001032badcacefebeb800000e100450000000029bd11000c8a00a00c8a00f07788556600a022e400000081230001002000290c245a
    checksum := checksum - (conv_std_logic_vector(edcl_idx, 30) & "00");
    if checksum(31) = '1' then
        checksum(15 downto 0) := checksum(15 downto 0) + checksum(31 downto 16);
    end if;

    ret(127 downto 112) := checksum(11 downto 8) & checksum(15 downto 12)
                         & checksum(3 downto 0) & checksum(7 downto 4);

    crc := (others => '1');
    for i in 0 to 51 loop
       byte := ret(443-8*i downto 440-8*i) & ret(447-8*i downto 444-8*i);
       idx := conv_integer(crc(7 downto 0) xor byte);
       crc := (X"00" & crc(31 downto 8)) xor crc32_table(idx);
    end loop;
    crc := not crc;
    ret(7 downto 0) := crc(27 downto 24) & crc(31 downto 28);
    ret(15 downto 8) := crc(19 downto 16) & crc(23 downto 20);
    ret(23 downto 16) := crc(11 downto 8) & crc(15 downto 12);
    ret(31 downto 24) := crc(3 downto 0) & crc(7 downto 4);

    return ret;
  end function;


   -- Use '20150601_riscv' project to generate network nibbles in string format:
   --   Open grethaxi.cpp (line 855)
   --   Use methods sendEdclRead/sendEdclWrite and generate_tb_string.

   -- Message 1: Read npc register from debug port of RIVER CPU:
   --    edcl index = 0
   --    address = 0x80088000 | (33 << 3) = 0x80088108
   --    size = 2 x word32 = 8 bytes
   constant EDCL_MSG1_START : integer := 1000;
   constant EDCL_MSG1_LEN : integer := 114;
   constant EDCL_MSG1 : std_logic_vector(EDCL_MSG1_LEN*4-1 downto 0) := 
   X"5d207098001032badcacefebeb800000e100450000000029bd11000c8a00a00c8a00f07788556600a01655000000004000088018802a32c544";
   
   -- Message 2: Write CSR register mtimecmp to debug port of RIVER CPU:
   --    edcl index = 1
   --    address = 0x8008000 | (321 << 3)
   --    size = 8 bytes
   --    wdata = 0x8877665544332211
   constant EDCL_MSG2_START : integer := 2000;
   constant EDCL_MSG2_LEN : integer := 130;
   constant EDCL_MSG2 : std_logic_vector(EDCL_MSG2_LEN*4-1 downto 0) := 
   X"5d207098001032badcacefebeb80000062004500000000293e11000c8a00a00c8a00f07788556600217baf000000604000088091801122334455667788ec10d362";


   -- Message 3: Read 32 x words32 from ROM (romimage):
   --    edcl index = 2
   --    address = 0x00100000
   --    size = 32 x word32 = 256 bytes
   constant EDCL_MSG3_START : integer := 3000;
   constant EDCL_MSG3_LEN : integer := 114;
   constant EDCL_MSG3 : std_logic_vector(EDCL_MSG3_LEN*4-1 downto 0) := 
   X"5d207098001032badcacefebeb800000e100450000000029bd11000c8a00a00c8a00f07788556600a062e40000008004000001000048d5979f";
   
   -- Message 4: Read 20 x words32 from SRAM:
   --    edcl index = 3
   --    address = 0x10001450
   --    size = 20 x word32 = 80 bytes
   constant EDCL_MSG4_START : integer := 4000;
   constant EDCL_MSG4_LEN : integer := 114;
   constant EDCL_MSG4 : std_logic_vector(EDCL_MSG4_LEN*4-1 downto 0) := 
   X"5d207098001032badcacefebeb800000e100450000000029bd11000c8a00a00c8a00f07788556600a0a1a0000000c082000100410560799d79";
   
   -- Message 5: Read 10 x words32 from SRAM:
   --    edcl index = 4
   --    address = 0x10001D60
   --    size = 10 x word32 = 40 bytes
   constant EDCL_MSG5_START : integer := 5000-22;
   constant EDCL_MSG5_LEN : integer := 114;
   constant EDCL_MSG5 : std_logic_vector(EDCL_MSG5_LEN*4-1 downto 0) := 
   X"5d207098001032badcacefebeb800000e100450000000029bd11000c8a00a00c8a00f07788556600a0426f0000000141000100d106e3d35cdc";

   -- Message 6: Write 8 x 64-bits (64 bytes) into SRAM:
   --    edcl index = 5
   --    address = 0x10030d40
   --    uint64_t wdata6[8] = {0x2222222211111111ull", 0x4444444433333333ull", 0x6666666655555555ull", 0x8888888877777777ull,
   --                          0xaaaaaaaa99999999ull", 0xccccccccbbbbbbbbull", 0xffffffffeeeeeeeeull", 0xcafecafebeefbeefull};
   --    size = 64 bytes
   constant EDCL_MSG6_START : integer := 6000;
   constant EDCL_MSG6_LEN : integer := 242;
   constant EDCL_MSG6 : std_logic_vector(EDCL_MSG6_LEN*4-1 downto 0) := 
   X"5d207098001032badcacefebeb800000e500450000000039b111000c8a00a00c8a00f07788556600a4705b0000006102000130d004111111112222222233333333444444445555555566666666777777778888888899999999aaaaaaaabbbbbbbbcccccccceeeeeeeefffffffffeebfeebefacefacc3875a17";   

  type registers is record
      clk_cnt : integer;
      edcl_idx : integer;
      msg_len : integer;
      msg_cnt : integer;
      msg : std_logic_vector(455 downto 0);
  end record;
  
  signal r, rin : registers;
  
begin

  comblogic : process(rst, r)
    variable v : registers;
    variable rxd  : std_logic_vector(3 downto 0);
    variable rxdv : std_logic;
    variable ibit : integer;
    variable wb_clk_cnt : std_logic_vector(31 downto 0);
  begin
     v := r;

     rxd := "0000";
     rxdv := '0';
     v.clk_cnt := r.clk_cnt + 1;
     if r.clk_cnt >= 10000 then
       wb_clk_cnt := conv_std_logic_vector(r.clk_cnt, 32);
       if wb_clk_cnt(9 downto 0) = "0000000000" then
          v.msg := gen_msg(r.edcl_idx);
          v.edcl_idx := r.edcl_idx + 1;
          v.msg_len := 114;
          v.msg_cnt := 0;
       end if;
     end if;

     if r.clk_cnt >= EDCL_MSG1_START and r.clk_cnt < (EDCL_MSG1_START + EDCL_MSG1_LEN) then
         ibit := r.clk_cnt - EDCL_MSG1_START;
         rxd := EDCL_MSG1(4*(EDCL_MSG1_LEN - ibit)-1 downto 4*(EDCL_MSG1_LEN - ibit)-4);
         rxdv := OUTPUT_ENA;
     elsif r.clk_cnt >= EDCL_MSG2_START and r.clk_cnt < (EDCL_MSG2_START + EDCL_MSG2_LEN) then
         ibit := r.clk_cnt - EDCL_MSG2_START;
         rxd := EDCL_MSG2(4*(EDCL_MSG2_LEN - ibit)-1 downto 4*(EDCL_MSG2_LEN - ibit)-4);
         rxdv := OUTPUT_ENA;
     elsif r.clk_cnt >= EDCL_MSG3_START and r.clk_cnt < (EDCL_MSG3_START + EDCL_MSG3_LEN) then
         ibit := r.clk_cnt - EDCL_MSG3_START;
         rxd := EDCL_MSG3(4*(EDCL_MSG3_LEN - ibit)-1 downto 4*(EDCL_MSG3_LEN - ibit)-4);
         rxdv := OUTPUT_ENA;
     elsif r.clk_cnt >= EDCL_MSG4_START and r.clk_cnt < (EDCL_MSG4_START + EDCL_MSG4_LEN) then
         ibit := r.clk_cnt - EDCL_MSG4_START;
         rxd := EDCL_MSG4(4*(EDCL_MSG4_LEN - ibit)-1 downto 4*(EDCL_MSG4_LEN - ibit)-4);
         rxdv := OUTPUT_ENA;
     elsif r.clk_cnt >= EDCL_MSG5_START and r.clk_cnt < (EDCL_MSG5_START + EDCL_MSG5_LEN) then
         ibit := r.clk_cnt - EDCL_MSG5_START;
         rxd := EDCL_MSG5(4*(EDCL_MSG5_LEN - ibit)-1 downto 4*(EDCL_MSG5_LEN - ibit)-4);
         rxdv := OUTPUT_ENA;
     elsif r.clk_cnt >= EDCL_MSG6_START and r.clk_cnt < (EDCL_MSG6_START + EDCL_MSG6_LEN) then
         ibit := r.clk_cnt - EDCL_MSG6_START;
         rxd := EDCL_MSG6(4*(EDCL_MSG6_LEN - ibit)-1 downto 4*(EDCL_MSG6_LEN - ibit)-4);
         rxdv := OUTPUT_ENA;
     elsif r.msg_cnt /= r.msg_len then
         v.msg_cnt := r.msg_cnt + 1;
         rxd := r.msg(4*(r.msg_len - r.msg_cnt)-1 downto 4*(r.msg_len - r.msg_cnt)-4);
         rxdv := OUTPUT_ENA;
     end if;

     -- Reset
     if rst = '1' then
        v.clk_cnt := 0;
        v.edcl_idx := 6;
        v.msg_len := 0;
        v.msg_cnt := 0;
        v.msg := (others => '0');
     end if;
     rin <= v;

     o_rxd <= rxd;
     o_rxdv <= rxdv;

   end process;


  procCheck : process (clk)
  begin
    if rising_edge(clk) then
        r <= rin;
    end if;
  end process;
  
end;
