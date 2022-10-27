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

entity axi4_slave is
  generic (
    async_reset : boolean
  );
  port (
    i_clk : in std_logic;
    i_nrst : in std_logic;
    i_xcfg : in axi4_slave_config_type;
    i_xslvi : in axi4_slave_in_type;
    o_xslvo : out axi4_slave_out_type;
    i_ready : in std_logic;
    i_rdata : in std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    o_re : out std_logic;
    o_r32 : out std_logic;
    o_radr : out global_addr_array_type;
    o_wadr : out global_addr_array_type;
    o_we : out std_logic;
    o_wstrb : out std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    o_wdata : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
  );
end; 
 
architecture arch_axi4_slave of axi4_slave is

  --! Slave device states during reading value operation.
  type axi_slave_rstatetype is (rwait, rhold, rtrans);
  --! Slave device states during writting data operation.
  type axi_slave_wstatetype is (wwait, wtrans);

  --! @brief Template bank of registers for any slave device.
  type axi_slave_bank_type is record
    rstate : axi_slave_rstatetype;
    wstate : axi_slave_wstatetype;

    rburst : std_logic_vector(1 downto 0);
    rsize  : integer;
    raddr  : global_addr_array_type;
    rlen   : integer;                       --! AXI4 supports 256 burst operation
    rid    : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
    rresp  : std_logic_vector(1 downto 0);  --! OK=0
    ruser  : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
    rswap : std_logic;
    rwaitready : std_logic;                 --! Reading wait state flag: 0=waiting. User's waitstates
    
    wburst : std_logic_vector(1 downto 0);  -- 0=INCREMENT
    wsize  : integer;                       -- code in range 0=1 Bytes upto 7=128 Bytes. 
    waddr  : global_addr_array_type;        --! 4 KB bank
    wlen   : integer;                       --! AXI4 supports 256 burst operation
    wid    : std_logic_vector(CFG_SYSBUS_ID_BITS-1 downto 0);
    wresp  : std_logic_vector(1 downto 0);  --! OK=0
    wuser  : std_logic_vector(CFG_SYSBUS_USER_BITS-1 downto 0);
    wswap : std_logic;
    b_valid : std_logic;
  end record;

  --! Reset value of the template bank of registers of a slave device.
  constant AXI_SLAVE_BANK_RESET : axi_slave_bank_type := (
    rwait, wwait,
    AXI_BURST_FIXED, 0, (others=>(others=>'0')), 0, (others=>'0'), AXI_RESP_OKAY, (others => '0'), '0', '1',
    AXI_BURST_FIXED, 0, (others=>(others=>'0')), 0, (others=>'0'), AXI_RESP_OKAY, (others => '0'), '0', '0'
  );

  signal rin, r : axi_slave_bank_type;

begin

  comblogic : process(i_nrst, i_xcfg, i_xslvi, i_ready, i_rdata, r)
    variable v : axi_slave_bank_type;
    variable traddr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
    variable twaddr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
    variable v_raddr_bus : global_addr_array_type;
    variable v_raddr_bus_swp : global_addr_array_type;
    variable v_raddr_bus_nxt : global_addr_array_type;
    variable v_raddr_bus_nxt_swp : global_addr_array_type;
    variable v_raddr_burst_nxt_swp : global_addr_array_type;
    variable v_wadr_bus : global_addr_array_type;
    variable v_wadr_bus_swp : global_addr_array_type;
    variable v_waddr_burst_nxt_swp : global_addr_array_type;
    variable v_re : std_logic;
    variable v_r32 : std_logic;
    variable v_radr : global_addr_array_type;
    variable v_we : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    variable v_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    variable v_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable v_aw_ready : std_logic;
    variable v_w_ready : std_logic;
    variable v_ar_ready : std_logic;
    variable v_r_valid : std_logic;
    variable v_r_last : std_logic;
    variable vb_r_data : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);

  begin

    v := r;

    traddr := (i_xslvi.ar_bits.addr(CFG_SYSBUS_ADDR_BITS-1 downto 12) and (not i_xcfg.xmask))
             & i_xslvi.ar_bits.addr(11 downto 0);

    twaddr := (i_xslvi.aw_bits.addr(CFG_SYSBUS_ADDR_BITS-1 downto 12) and (not i_xcfg.xmask))
             & i_xslvi.aw_bits.addr(11 downto 0);

    for n in 0 to CFG_WORDS_ON_BUS-1 loop
        v_raddr_bus(n) := traddr + n*CFG_ALIGN_BYTES;
        v_raddr_bus_nxt(n) := v_raddr_bus(n) + XSizeToBytes(conv_integer(i_xslvi.ar_bits.size));
        if i_xslvi.ar_bits.burst = AXI_BURST_WRAP then
            v_raddr_bus_nxt(n)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := v_raddr_bus(n)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
        end if;

        v_wadr_bus(n) := twaddr + n*CFG_ALIGN_BYTES;
    end loop;

    v_re := '0';
    v_r32 := '0';
    v_radr(0) := (others => '0');
    v_radr(1) := (others => '0');

    -- Next hold read address while write transaction not finished
    if i_xslvi.ar_bits.addr(2) = '0' then
        v_raddr_bus_swp := v_raddr_bus;
    else
        v_raddr_bus_swp(0) := v_raddr_bus(1);
        v_raddr_bus_swp(1) := v_raddr_bus(0);
    end if;

    -- Next read accepted address if no write request
    if (i_xslvi.ar_bits.addr(2) = '0' and i_xslvi.ar_bits.size = "011") or
       (i_xslvi.ar_bits.addr(2) = '1' and i_xslvi.ar_bits.size = "010") then
        v_raddr_bus_nxt_swp := v_raddr_bus_nxt;
    else
        v_raddr_bus_nxt_swp(0) := v_raddr_bus_nxt(1);
        v_raddr_bus_nxt_swp(1) := v_raddr_bus_nxt(0);
    end if;

    -- Next burst read address
    if r.rsize = 4 then
        v_raddr_burst_nxt_swp(0) := r.raddr(1) + r.rsize;
        v_raddr_burst_nxt_swp(1) := r.raddr(0) + r.rsize;
        if r.rburst = AXI_BURST_WRAP then
            v_raddr_burst_nxt_swp(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.raddr(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
            v_raddr_burst_nxt_swp(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.raddr(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
        end if;
    else
        v_raddr_burst_nxt_swp(0) := r.raddr(0) + r.rsize;
        v_raddr_burst_nxt_swp(1) := r.raddr(1) + r.rsize;
        if r.rburst = AXI_BURST_WRAP then
            v_raddr_burst_nxt_swp(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.raddr(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
            v_raddr_burst_nxt_swp(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.raddr(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
        end if;
    end if;


    -- Write swapped address
    if i_xslvi.aw_bits.addr(2) = '0' then
        v_wadr_bus_swp := v_wadr_bus;
    else
        v_wadr_bus_swp(0) := v_wadr_bus(1);
        v_wadr_bus_swp(1) := v_wadr_bus(0);
    end if;

    -- Next burst write address
    if r.wsize = 4 then
        v_waddr_burst_nxt_swp(0) := r.waddr(1) + r.wsize;
        v_waddr_burst_nxt_swp(1) := r.waddr(0) + r.wsize;
        if r.wburst = AXI_BURST_WRAP then
            v_waddr_burst_nxt_swp(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.waddr(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
            v_waddr_burst_nxt_swp(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.waddr(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
        end if;
    else
        v_waddr_burst_nxt_swp(0) := r.waddr(0) + r.wsize;
        v_waddr_burst_nxt_swp(1) := r.waddr(1) + r.wsize;
        if r.wburst = AXI_BURST_WRAP then
            v_waddr_burst_nxt_swp(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.waddr(0)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
            v_waddr_burst_nxt_swp(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5) 
                 := r.waddr(1)(CFG_SYSBUS_ADDR_BITS-1 downto 5);
        end if;
    end if;


    v_we := (others => '0');

    v_ar_ready := '0';
    v_r_valid := '0';
    v_r_last := '0';
    v_aw_ready := '0';
    v_w_ready := '0';

    -- Reading state machine:
    case r.rstate is
    when rwait =>
        v_ar_ready := '1';
        v_radr := v_raddr_bus_swp;

        if i_xslvi.ar_valid = '1' then
            if i_xslvi.aw_valid = '0' and r.wstate = wwait then
                v_re := '1';
                v.rstate := rtrans;
                v.raddr := v_raddr_bus_nxt_swp;
            else
                v.rstate := rhold;
                v.raddr := v_raddr_bus_swp;
            end if;

            if i_xslvi.ar_bits.size = "010" then
                v_r32 := '1';
            end if;
            v.rswap := i_xslvi.ar_bits.addr(2);
            v.rsize := XSizeToBytes(conv_integer(i_xslvi.ar_bits.size));
            v.rburst := i_xslvi.ar_bits.burst;
            v.rlen := conv_integer(i_xslvi.ar_bits.len);
            v.rid := i_xslvi.ar_id;
            v.rresp := AXI_RESP_OKAY;
            v.ruser := i_xslvi.ar_user;
        end if;
    when rhold =>
        v_radr := r.raddr;
        if r.rsize = 4 then
            v_r32 := '1';
        end if;
        if i_xslvi.aw_valid = '0' and r.wstate = wwait then
            v_re := '1';
            v.rstate := rtrans;
            v.raddr := v_raddr_burst_nxt_swp;
        end if;
    when rtrans =>
        v_r_valid := i_ready;
        v_radr := r.raddr;
        if r.rlen /= 0 then
            v_re := '1';   -- request next burst read address even if no ready data
        end if;
        if r.rsize = 4 then
            v_r32 := '1';
        end if;
        if i_xslvi.r_ready = '1' and i_ready = '1' then
            if r.rsize = 4 then
                v.rswap := not r.rswap;
            end if;
            v.raddr := v_raddr_burst_nxt_swp;
            -- End of transaction (or process another one):
            if r.rlen = 0 then
                v_r_last := '1';
                v_ar_ready := '1';
                v_radr := v_raddr_bus_swp;
                if i_xslvi.ar_valid = '1' then
                    if i_xslvi.aw_valid = '0' and r.wstate = wwait then
                        v_re := '1';
                        v.rstate := rtrans;
                        v.raddr := v_raddr_bus_nxt_swp;
                    else
                        v.rstate := rhold;
                        v.raddr := v_raddr_bus_swp;
                    end if;

                    if i_xslvi.ar_bits.size = "010" then
                        v_r32 := '1';
                    end if;
                    v.rswap := i_xslvi.ar_bits.addr(2);
                    v.rsize := XSizeToBytes(conv_integer(i_xslvi.ar_bits.size));
                    v.rburst := i_xslvi.ar_bits.burst;
                    v.rlen := conv_integer(i_xslvi.ar_bits.len);
                    v.rid := i_xslvi.ar_id;
                    v.rresp := AXI_RESP_OKAY;
                    v.ruser := i_xslvi.ar_user;
                else
                    v.rstate := rwait;
                end if;
            else
                v.rlen := r.rlen - 1;
            end if;
        end if;
    end case;

    -- Writing state machine:
    case r.wstate is
    when wwait =>
        if r.rlen = 0 or r.rstate = rhold then
            v_aw_ready := '1';
        end if;
        if i_xslvi.aw_valid = '1' and (r.rlen = 0 or r.rstate = rhold) then
            v.wstate := wtrans;
            v.waddr := v_wadr_bus_swp;
            v.wswap := i_xslvi.aw_bits.addr(2);
            v.wsize := XSizeToBytes(conv_integer(i_xslvi.aw_bits.size));
            v.wburst := i_xslvi.aw_bits.burst;
            v.wlen := conv_integer(i_xslvi.aw_bits.len);
            v.wid := i_xslvi.aw_id;
            v.wresp := AXI_RESP_OKAY;
            v.wuser := i_xslvi.aw_user;
        end if;
    when wtrans =>
        v_we := (others => '1');
        v_w_ready := i_ready;
        if i_xslvi.w_valid = '1' and i_ready = '1' then
            if r.wsize = 4 then
                v.wswap := not r.wswap;
            end if;
            v.waddr := v_waddr_burst_nxt_swp;
            -- End of transaction:
            if r.wlen = 0 then
                v.b_valid := '1';
                v_aw_ready := '1';
                if i_xslvi.aw_valid = '0' then
                    v.wstate := wwait;
                else
                    v.waddr := v_wadr_bus_swp;
                    v.wswap := i_xslvi.aw_bits.addr(2);
                    v.wsize := XSizeToBytes(conv_integer(i_xslvi.aw_bits.size));
                    v.wburst := i_xslvi.aw_bits.burst;
                    v.wlen := conv_integer(i_xslvi.aw_bits.len);
                    v.wid := i_xslvi.aw_id;
                    v.wresp := AXI_RESP_OKAY;
                    v.wuser := i_xslvi.aw_user;
                end if;
            else
                v.wlen := r.wlen - 1;
            end if;
        end if;
    end case;

    if i_xslvi.b_ready = '1' and r.b_valid = '1' then
        if r.wstate = wtrans and i_xslvi.w_valid = '1' and r.wlen = 0 then
            v.b_valid := '1';
        else
            v.b_valid := '0';
        end if;
    end if;

    -- AXI Lite must be 8-byte aligned in this implementation
    if r.wswap = '0' then
        v_wdata := i_xslvi.w_data;
        v_wstrb := i_xslvi.w_strb and v_we;
    else
        v_wdata(31 downto 0) := i_xslvi.w_data(63 downto 32);
        v_wdata(63 downto 32) := i_xslvi.w_data(31 downto 0);
        v_wstrb := (i_xslvi.w_strb(3 downto 0) & i_xslvi.w_strb(7 downto 4))
               and (v_we(3 downto 0) & v_we(7 downto 4));
    end if;

    o_re <= v_re;
    o_radr <= v_radr;
    o_r32 <= v_r32;
    o_wadr <= r.waddr;
    o_we <= v_we(0);
    o_wdata <= v_wdata;
    o_wstrb <= v_wstrb;

    if r.rswap = '0' then
        vb_r_data := i_rdata;
    else
        vb_r_data := i_rdata(31 downto 0) & i_rdata(63 downto 32);
    end if;


    if not async_reset and i_nrst = '0' then
        v := AXI_SLAVE_BANK_RESET;
    end if;
 
    rin <= v;

    o_xslvo.aw_ready <= v_aw_ready;
    o_xslvo.w_ready <= v_w_ready;
    o_xslvo.ar_ready <= v_ar_ready;
    o_xslvo.r_valid <= v_r_valid;
    o_xslvo.r_last <= v_r_last;
    o_xslvo.r_data <= vb_r_data;
    o_xslvo.r_id <= r.rid;
    o_xslvo.r_resp <= r.rresp;
    o_xslvo.r_user <= r.ruser;

    -- Write Handshaking:
    o_xslvo.b_id <= r.wid;
    o_xslvo.b_resp <= r.wresp;
    o_xslvo.b_user <= r.wuser;
    o_xslvo.b_valid <= r.b_valid;

  end process;

  -- registers
  regs : process(i_clk, i_nrst)
  begin 
     if async_reset and i_nrst = '0' then
         r <= AXI_SLAVE_BANK_RESET;
     elsif rising_edge(i_clk) then 
         r <= rin;
     end if; 
  end process;
end;
