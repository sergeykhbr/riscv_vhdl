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

entity uart_sim is 
  generic (
    clock_rate : integer := 10;
    binary_bytes_max : integer := 8;
    use_binary : boolean := false
  ); 
  port (
    rst : in std_logic;
    clk : in std_logic;
    wr_str : in std_logic;
    instr : in string;
    bin_data : in std_logic_vector(8*binary_bytes_max-1 downto 0);
    bin_bytes_sz : in integer;
    td  : in std_logic;
    rtsn : in std_logic;
    rd  : out std_logic;
    ctsn : out std_logic;
    busy : out std_logic
  );
end;

architecture uart_sim_rtl of uart_sim is

  constant STATE_idle : integer := 0;
  constant STATE_startbit : integer := 1;
  constant STATE_data : integer := 2;
  constant STATE_parity : integer := 3;
  constant STATE_stopbit : integer := 4;

  type registers is record
      txstate : integer range 0 to 4;
      tx_data : std_logic_vector(10 downto 0);
      txbitcnt : integer range 0 to 12;
      clk_rate_cnt : integer;
      msg_symb_cnt : integer range 0 to 256;
      msg_len : integer;
      msg : string(1 to 256);
      msg_bin : std_logic_vector(8*binary_bytes_max-1 downto 0);
      is_data : std_logic;
  end record;
  
  signal r, rin : registers;
  
begin

  comblogic : process(rst, r, wr_str, instr)
    variable v : registers;
    variable symbol : std_logic_vector(7 downto 0);
    variable RATE_EVENT : std_logic;
  begin
     v := r;
     RATE_EVENT := '0';
     if r.clk_rate_cnt = (clock_rate - 1) then
         RATE_EVENT := '1';
     end if;

     if wr_str = '1' then
        if use_binary = true then
            v.msg_bin := bin_data;
            v.msg_len := bin_bytes_sz;
        else
            v.msg := instr;
            v.msg_len := strlen(instr);
        end if;
        v.is_data := '1';
     end if;

     case r.txstate is
     when STATE_idle =>
        v.txbitcnt := 0;
        if r.is_data = '1' and RATE_EVENT = '1' then
           v.txstate := STATE_startbit;
           v.is_data := '0';
           if use_binary = true then
               symbol := r.msg_bin(8*binary_bytes_max-1 downto 8*binary_bytes_max-8);
               v.msg_bin := r.msg_bin(8*(binary_bytes_max-1)-1 downto 0) & X"00";
           else
               symbol := SymbolToSVector(r.msg, r.msg_symb_cnt);
           end if;
           v.tx_data := "01" & symbol & '0'; -- [stopbit=1, ?parity? (skiped), data, start_bit=0]
        elsif r.is_data = '1' then
          -- do nothing
        else
          v.tx_data := (others => '1');
          v.msg_symb_cnt := 0;     -- symbols in string start from 1 to len.
          v.clk_rate_cnt := 0;
        end if;

     when STATE_startbit =>
        if RATE_EVENT = '1' then
            v.txstate := STATE_data;
            v.tx_data := '0' & r.tx_data(10 downto 1);
        end if;

     when STATE_data =>
        if RATE_EVENT = '1' then
            v.tx_data := '0' & r.tx_data(10 downto 1);
            if r.txbitcnt = 8 then
                v.txstate := STATE_stopbit;
            end if;
        end if;

     when STATE_parity =>
         v.tx_data := '0' & r.tx_data(10 downto 1);

     when STATE_stopbit =>
        if RATE_EVENT = '1' then
            v.tx_data := (others => '1');
            if (r.msg_symb_cnt + 1) = r.msg_len then
                v.txstate := STATE_idle;
            else
                v.is_data := '1';
                v.txstate := STATE_idle;
                v.msg_symb_cnt := r.msg_symb_cnt + 1;
            end if;
        end if;
     when others =>
     end case;

     if r.txstate /= STATE_idle and RATE_EVENT = '1' then
         busy <= '1';
         v.txbitcnt := r.txbitcnt + 1;
     else
         busy <= '0';
     end if;      

     if RATE_EVENT = '1' then
       v.clk_rate_cnt := 0;
     else
       v.clk_rate_cnt := r.clk_rate_cnt + 1;
     end if;

     -- Reset
     if rst = '1' then
        v.txstate := STATE_idle;
        v.clk_rate_cnt := 0;
        v.is_data := '0';
     end if;

     ctsn <= rst;
     rd <= r.tx_data(0);
     rin <= v;
   end process;


  procCheck : process (clk)
  begin
    if rising_edge(clk) then
        r <= rin;
    end if;
  end process;
  
end;
