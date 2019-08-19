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
    i_xcfg : in nasti_slave_config_type;
    i_xslvi : in nasti_slave_in_type;
    o_xslvo : out nasti_slave_out_type;
    i_ready : in std_logic;
    i_rdata : in std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    o_radr : out global_addr_array_type;
    o_wadr : out global_addr_array_type;
    o_wena : out std_logic;
    o_wstrb : out std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    o_wdata : out std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0)
  );
end; 
 
architecture arch_axi4_slave of axi4_slave is

  signal rin, r : nasti_slave_bank_type;

begin

  comblogic : process(i_nrst, i_xcfg, i_xslvi, i_ready, i_rdata, r)
    variable v : nasti_slave_bank_type;
    variable traddr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
    variable twaddr : std_logic_vector(CFG_SYSBUS_ADDR_BITS-1 downto 0);
    variable v_radr_mux : global_addr_array_type;
    variable v_wadr_mux : global_addr_array_type;
    variable v_radr : global_addr_array_type;
    variable v_wadr : global_addr_array_type;
    variable v_wena : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    variable v_wstrb : std_logic_vector(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    variable v_wdata : std_logic_vector(CFG_SYSBUS_DATA_BITS-1 downto 0);
    variable v_wreorder : std_logic;
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
        v_radr_mux(n) := traddr + n*CFG_ALIGN_BYTES;
        v_wadr_mux(n) := twaddr + n*CFG_ALIGN_BYTES;
    end loop;

    v_radr := r.raddr;
    v_wadr := r.waddr;
    v_wena := (others => '0');
    v_wreorder := r.wreorder;

    -- Reading state machine:
    case r.rstate is
    when rwait =>
        if i_xslvi.ar_valid = '1' and i_ready = '1'
           and i_xslvi.aw_valid = '0' and r.wstate = wwait then
            v.rstate := rtrans;

            if i_xslvi.ar_bits.addr(2) = '0' then
                v_radr := v_radr_mux;
            else
                v_radr(0) := v_radr_mux(1);
                v_radr(1) := v_radr_mux(0);
            end if;
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
                v.raddr(n) := v_radr(n) + XSizeToBytes(conv_integer(i_xslvi.ar_bits.size));
                if i_xslvi.ar_bits.burst = NASTI_BURST_WRAP then
                    -- i_bank.rsize = 8 and i_bank.rlen = 3
                    -- 8 x 4 = 32 bytes 
                    v.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5) 
                         := v_radr(n)(CFG_NASTI_ADDR_BITS-1 downto 5);
                end if;
            end loop;
            v.rreorder := i_xslvi.ar_bits.addr(2);
            v.rsize := XSizeToBytes(conv_integer(i_xslvi.ar_bits.size));
            v.rburst := i_xslvi.ar_bits.burst;
            v.rlen := conv_integer(i_xslvi.ar_bits.len);
            v.rid := i_xslvi.ar_id;
            v.rresp := NASTI_RESP_OKAY;
            v.ruser := i_xslvi.ar_user;
        end if;
    when rtrans =>
        if i_xslvi.r_ready = '1' and i_ready = '1' then
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
                v.raddr(n) := r.raddr(n) + r.rsize;
            end loop;
            if r.rburst = NASTI_BURST_WRAP then
                for n in 0 to CFG_WORDS_ON_BUS-1 loop
                    -- i_bank.rsize = 8 and i_bank.rlen = 3
                    -- 8 x 4 = 32 bytes 
                    v.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5) 
                         := r.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5);
                end loop;
            end if;
            -- End of transaction (or process another one):
            if r.rlen = 0 then
                if i_xslvi.ar_valid = '1' and i_xslvi.aw_valid = '0' then
                    if i_xslvi.ar_bits.addr(2) = '0' then
                        v_radr := v_radr_mux;
                    else
                        v_radr(0) := v_radr_mux(1);
                        v_radr(1) := v_radr_mux(0);
                    end if;
                    for n in 0 to CFG_WORDS_ON_BUS-1 loop
                        v.raddr(n) := v_radr(n) + XSizeToBytes(conv_integer(i_xslvi.ar_bits.size));
                        if i_xslvi.ar_bits.burst = NASTI_BURST_WRAP then
                            v.raddr(n)(CFG_NASTI_ADDR_BITS-1 downto 5) 
                                := v_radr(n)(CFG_NASTI_ADDR_BITS-1 downto 5);
                        end if;
                    end loop;
                    v.rreorder := i_xslvi.ar_bits.addr(2);
                    v.rsize := XSizeToBytes(conv_integer(i_xslvi.ar_bits.size));
                    v.rburst := i_xslvi.ar_bits.burst;
                    v.rlen := conv_integer(i_xslvi.ar_bits.len);
                    v.rid := i_xslvi.ar_id;
                    v.rresp := NASTI_RESP_OKAY;
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
        if i_xslvi.aw_valid = '1' and i_ready = '1' and r.rlen = 0 then
            if i_xslvi.w_valid = '0' then
                -- Full AXI bus protocol
                v.wstate := wtrans;
                v.wreorder := i_xslvi.aw_bits.addr(2);
            else
                -- AXI lite (no burst support)
                v_wena := (others => '1');
                v_wreorder := i_xslvi.aw_bits.addr(2);
            end if;

            if i_xslvi.aw_bits.addr(2) = '0' then
                v_wadr := v_wadr_mux;
            else
                v_wadr(0) := v_wadr_mux(1);
                v_wadr(1) := v_wadr_mux(0);
            end if;
            for n in 0 to CFG_WORDS_ON_BUS-1 loop
               v.waddr(n) := v_wadr(n);
            end loop;
            v.wsize := XSizeToBytes(conv_integer(i_xslvi.aw_bits.size));
            v.wburst := i_xslvi.aw_bits.burst;
            v.wlen := conv_integer(i_xslvi.aw_bits.len);
            v.wid := i_xslvi.aw_id;
            v.wresp := NASTI_RESP_OKAY;
            v.wuser := i_xslvi.aw_user;
        end if;
    when wtrans =>
        v_wena := (others => '1');
        if i_xslvi.w_valid = '1' and i_ready = '1' then
            if r.wburst = NASTI_BURST_INCR then
              for n in 0 to CFG_WORDS_ON_BUS-1 loop
                v.waddr(n) := r.waddr(n) + r.wsize;
              end loop;
            end if;
            -- End of transaction:
            if r.wlen = 0 then
                v.b_valid := '1';
                if i_xslvi.aw_valid = '0' then
                    v.wstate := wwait;
                else
                    -- Only Full AXI Bus protocol support here
                    if i_xslvi.aw_bits.addr(2) = '0' then
                        v.waddr := v_wadr_mux;
                    else
                        v.waddr(0) := v_wadr_mux(1);
                        v.waddr(1) := v_wadr_mux(0);
                    end if;
                    v.wreorder := i_xslvi.aw_bits.addr(2);
                    v.wsize := XSizeToBytes(conv_integer(i_xslvi.aw_bits.size));
                    v.wburst := i_xslvi.aw_bits.burst;
                    v.wlen := conv_integer(i_xslvi.aw_bits.len);
                    v.wid := i_xslvi.aw_id;
                    v.wresp := NASTI_RESP_OKAY;
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

    if v_wreorder = '0' then
        v_wdata := i_xslvi.w_data;
        v_wstrb := i_xslvi.w_strb and v_wena;
    else
        v_wdata(31 downto 0) := i_xslvi.w_data(63 downto 32);
        v_wdata(63 downto 32) := i_xslvi.w_data(31 downto 0);
        v_wstrb := (i_xslvi.w_strb(3 downto 0) & i_xslvi.w_strb(7 downto 4))
               and (v_wena(3 downto 0) & v_wena(7 downto 4));
    end if;

    o_radr <= v_radr;
    o_wadr <= v_wadr;
    o_wena <= v_wena(0);
    o_wdata <= v_wdata;
    o_wstrb <= v_wstrb;

    --! Device to AXI4 signals:

    v_aw_ready := i_ready;
    v_w_ready := '1';
    v_ar_ready := i_ready;

    if i_xslvi.aw_valid = '1' then
        v_ar_ready := '0';
    end if;

    v_r_last := '0';
    if r.rstate = rtrans then
        if r.rlen = 0 then
            v_r_last := '1';
        else
            v_aw_ready := '0';
            v_w_ready := '0';
            v_ar_ready := '0';
        end if;
    end if;

    if i_ready = '1' and r.rstate = rtrans then
        v_r_valid   := '1';
    else
        v_r_valid   := '0';
    end if;

    if r.rreorder = '0' then
        vb_r_data := i_rdata;
    else
        vb_r_data := i_rdata(31 downto 0) & i_rdata(63 downto 32);
    end if;

    -- Write transfer:
    if r.wstate = wtrans then
        v_ar_ready := '0';
        if r.wlen /= 0 then
            v_aw_ready := '0';
        end if;
    end if;


    if not async_reset and i_nrst = '0' then
        v := NASTI_SLAVE_BANK_RESET;
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
         r <= NASTI_SLAVE_BANK_RESET;
     elsif rising_edge(i_clk) then 
         r <= rin;
     end if; 
  end process;
end;
