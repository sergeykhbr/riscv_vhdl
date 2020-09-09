--!
--! Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
--!
--! Licensed under the Apache License, Version 2.0 (the "License");
--! you may not use this file except in compliance with the License.
--! You may obtain a copy of the License at
--!
--!     http://www.apache.org/licenses/LICENSE-2.0
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

entity dcom_jtag is
  generic (
    id : std_logic_vector(31 downto 0) := X"01040093");
  port (
    rst         : in std_ulogic;
    tck         : in std_ulogic;
    tms         : in std_ulogic;
    tdi         : in std_ulogic;
    tdo         : out std_ulogic;
    tapi_tdo    : in std_ulogic;
    tapo_tck    : out std_ulogic;
    tapo_tdi    : out std_ulogic;
    tapo_inst   : out std_logic_vector(4 downto 0);
    tapo_rst    : out std_ulogic;
    tapo_capt   : out std_ulogic;
    tapo_shft   : out std_ulogic;
    tapo_upd    : out std_ulogic;
    tapo_xsel1  : out std_ulogic;
    tapo_xsel2  : out std_ulogic
    );
end;


architecture rtl of dcom_jtag is

  type ltap_out_type is record                           
    tck   : std_ulogic;
    tdi   : std_ulogic;
    inst  : std_logic_vector(4 downto 0);
    asel  : std_ulogic;
    dsel  : std_ulogic;
    reset : std_ulogic;
    capt  : std_ulogic;
    shift : std_ulogic;
    upd   : std_ulogic;     
  end record;

  constant IDCODE : std_logic_vector(4 downto 0)     := "00001";
  constant DTMCS : std_logic_vector(4 downto 0)      := "10000";
  constant DMI_ACCESS : std_logic_vector(4 downto 0) := "10001";
 
  type state_type is (test_rst, run_idle, select_dr, capture_dr, shift_dr, exit1_dr,
                      pause_dr, exit2_dr, update_dr, select_ir, capture_ir, shift_ir,
                      exit1_ir, pause_ir, exit2_ir, update_ir); 

  type reg_type is record
   state  : state_type;
   inst   : std_logic_vector(4 downto 0);
   shft   : std_logic_vector(31 downto 0);
   tdo    : std_ulogic;
   sel_user1 : std_logic;
   sel_user2 : std_logic;
  end record;

  signal r, rin : reg_type;
 
begin 

  comb : process(tck, tms, tdi, tapi_tdo, r)   
    variable v : reg_type;
    variable vtapo : ltap_out_type;
    variable vtdo : std_ulogic;
  begin

    v := r; 
    vtapo.tck := tck;
    vtapo.reset := '0'; 
    vtapo.tdi := tdi;
    vtapo.inst := (others => '0'); 
    vtapo.inst(4 downto 0) := r.inst;
    vtapo.capt := '0'; 
    vtapo.upd := '0'; 
    vtapo.shift := '0'; 
    vtapo.asel := '0'; 
    vtapo.dsel := '0';
    if r.inst /= DMI_ACCESS then
        v.tdo := r.shft(0);
    else
        v.tdo := tapi_tdo;
    end if;   

    --if (r.inst = IDCODE_I) or (r.inst = BYPASS) then v.tdo := r.shft(0);
    --else                                             v.tdo := tapi_tdo; end if;   
	                                                     
	    case r.state is
	      when test_rst   => if tms = '0' then v.state := run_idle; end if;
	      when run_idle   => if tms = '1' then v.state := select_dr; end if;
	      when select_dr  => if tms = '0' then v.state := capture_dr; else v.state := select_ir; end if;
	      when capture_dr => if tms = '0' then v.state := shift_dr; else v.state := exit1_dr; end if;
	      when shift_dr   => if tms = '1' then v.state := exit1_dr; end if;
	      when exit1_dr   => if tms = '0' then v.state := pause_dr; else v.state := update_dr; end if;
	      when pause_dr   => if tms = '1' then v.state := exit2_dr; end if;
	      when exit2_dr   => if tms = '0' then v.state := shift_dr; else v.state := update_dr; end if;
	      when update_dr  => if tms = '0' then v.state := run_idle; else v.state := select_dr; end if;
	      when select_ir  => if tms = '0' then v.state := capture_ir; else v.state := test_rst; end if;
	      when capture_ir => if tms = '0' then v.state := shift_ir; else v.state := exit1_ir; end if;
	      when shift_ir   => if tms = '1' then v.state := exit1_ir; end if;
	      when exit1_ir   => if tms = '0' then v.state := pause_ir; else v.state := update_ir; end if;
	      when pause_ir   => if tms = '1' then v.state := exit2_ir; end if;
	      when exit2_ir   => if tms = '0' then v.state := shift_ir; else v.state := update_ir; end if;
	      when update_ir  => if tms = '0' then v.state := run_idle; else v.state := select_dr; end if;
	    end case;
	     
    case r.state is
    when test_rst =>
        vtapo.reset := '1';
        v.inst := IDCODE;
    when capture_dr =>
        vtapo.capt := '1';
        if r.inst = IDCODE then 
            v.shft := id;
        elsif r.inst = DTMCS then 
            v.shft := (others => '0');
            v.shft(14 downto 12) := "001"; -- idle: 1=Enter Run-Test/Idle and leave it immediately
            v.shft(11 downto 10) := "00";  -- dmstat: TODO
            v.shft(9 downto 4) := conv_std_logic_vector(7, 6);  -- abits: 7 bits dmi address width
            v.shft(3 downto 0) := X"1";   -- version: 1=spec 0.13
        elsif r.inst = DMI_ACCESS then
            v.sel_user1 := '1';
        else
            v.shft(0) := '0';  -- BYPASS
        end if;
--	        if r.inst = BYPASS then v.shft(0) := '0'; end if;       
--	        if r.inst = IDCODE_I then  v.shft := id; end if;
    when shift_dr   =>
        vtapo.shift := '1';
        if (r.inst = IDCODE) or (r.inst = DTMCS) then
            v.shft(31 downto 0) := tdi & r.shft(31 downto 1);
        else
            v.shft(0) := tdi;  -- BYPASS
        end if;
--	        if r.inst = BYPASS then v.shft(0) := tdi; end if;
--	        if r.inst = IDCODE_I then v.shft := tdi & r.shft(31 downto 1); end if;
    when update_dr  =>
        vtapo.upd := '1';
        v.sel_user1 := '0';
        v.sel_user2 := '0';
    when capture_ir => 
        v.shft(4 downto 2) := r.inst(4 downto 2); 
        v.shft(1 downto 0) := "01";
        v.sel_user1 := '0';
        v.sel_user2 := '0';
    when shift_ir   => 
        v.shft(4 downto 0) := tdi & r.shft(4 downto 1);
    when update_ir  => 
        v.inst := r.shft(4 downto 0);
    when others => 
    end case;
	
    rin <= v; 
	    
    tdo <= r.tdo;
    tapo_tck <= tck;
      --if (r.sel_user1 or r.sel_user2)='1' then tapo_tck <= tck;
      --else                                     tapo_tck <= '1'; end if;
    tapo_tdi <= tdi; 
    tapo_inst <= vtapo.inst; 
    tapo_rst <= vtapo.reset;
    tapo_capt <= vtapo.capt; 
    tapo_shft <= vtapo.shift; 
    tapo_upd <= vtapo.upd;   
    tapo_xsel1 <= r.sel_user1;
    tapo_xsel2 <= r.sel_user2;
  end process;
	 
  posreg : process(tck, rst) begin
     if rising_edge(tck) then
         r.state <= rin.state;
         r.shft  <= rin.shft;
     end if;
     if rst = '0' then 
         r.state <= test_rst; 
         r.shft  <= id;
     end if;
  end process;
	
  negreg : process(tck, rst) begin
      if falling_edge(tck) then
          r.inst <= rin.inst;
          r.tdo  <= rin.tdo;
          r.sel_user1 <= rin.sel_user1;
          r.sel_user2 <= rin.sel_user2;
      end if;
      if rst = '0' then 
          r.inst <= IDCODE; 
          r.sel_user1 <= '0';
          r.sel_user2 <= '0';
      end if;
  end process; 
	   
end;
