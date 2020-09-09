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
--! @brief     Access to debug port of CPUs through the DMI registers.
-----------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;  -- or_reduce()
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
library ambalib;
use ambalib.types_amba4.all;
library riverlib;
use riverlib.river_cfg.all;
use riverlib.types_river.all;

entity dmi_regs is
  generic (
    async_reset : boolean := false;
    cpu_available : integer := 1
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    -- port[0] connected to JTAG TAP has access to AXI master interface (SBA registers)
    i_dmi_jtag_req_valid : in std_logic;
    o_dmi_jtag_req_ready : out std_logic;
    i_dmi_jtag_write : in std_logic;
    i_dmi_jtag_addr : in std_logic_vector(6 downto 0);
    i_dmi_jtag_wdata : in std_logic_vector(31 downto 0);
    o_dmi_jtag_resp_valid : out std_logic;
    i_dmi_jtag_resp_ready : in std_logic;
    o_dmi_jtag_rdata : out std_logic_vector(31 downto 0);
    -- port[1] connected to DSU doesn't have access to AXI master interface
    i_dmi_dsu_req_valid : in std_logic;
    o_dmi_dsu_req_ready : out std_logic;
    i_dmi_dsu_write : in std_logic;
    i_dmi_dsu_addr : in std_logic_vector(6 downto 0);
    i_dmi_dsu_wdata : in std_logic_vector(31 downto 0);
    o_dmi_dsu_resp_valid : out std_logic;
    i_dmi_dsu_resp_ready : in std_logic;
    o_dmi_dsu_rdata : out std_logic_vector(31 downto 0);
    -- Common signals
    o_hartsel : out std_logic_vector(CFG_LOG2_CPU_MAX-1 downto 0);
    o_dmstat : out std_logic_vector(1 downto 0);
    o_ndmreset : out std_logic;        -- non-debug module reset

    o_cfg  : out axi4_master_config_type;
    i_xmsti  : in axi4_master_in_type;
    o_xmsto  : out axi4_master_out_type;
    o_dporti : out dport_in_vector;
    i_dporto : in dport_out_vector
  );
end;

architecture arch_dmi_regs of dmi_regs is

  constant xconfig : axi4_master_config_type := (
     descrtype => PNP_CFG_TYPE_MASTER,
     descrsize => PNP_CFG_MASTER_DESCR_BYTES,
     vid => VENDOR_GNSSSENSOR,
     did => RISCV_RIVER_DMI
  );

  constant HARTSELLEN : integer := CFG_LOG2_CPU_MAX;
  constant HART_AVAILABLE_MASK : std_logic_vector(HARTSELLEN-1 downto 0) :=
          conv_std_logic_vector(2**log2(cpu_available) - 1, HARTSELLEN);

  type state_type is (
      Idle,
      DmiRequest,
      AbstractCommand,
      DportRequest,
      DportResponse,
      DportPostexec,
      DportBroadbandRequest,
      DportBroadbandResponse,
      Dma_AR,
      Dma_R,
      Dma_AW,
      Dma_W,
      Dma_B,
      DmiResponse
   );
  

  type registers is record
    state : state_type;
    dmstat : std_logic_vector(1 downto 0);
    hartsel : std_logic_vector(HARTSELLEN-1 downto 0);
    ndmreset : std_logic;   -- non-debug module reset
    resumeack : std_logic;
    halt_after_reset : std_logic;
    
    addr : std_logic_vector(CFG_DPORT_ADDR_BITS-1 downto 0);
    rdata : std_logic_vector(63 downto 0);
    wdata : std_logic_vector(63 downto 0);
    wstrb : std_logic_vector(7 downto 0);
    memdata : std_logic_vector(63 downto 0);
    arg0 : std_logic_vector(63 downto 0);
    command : std_logic_vector(31 downto 0);
    autoexecdata : std_logic_vector(CFG_DATA_REG_TOTAL-1 downto 0);
    autoexecprogbuf : std_logic_vector(CFG_PROGBUF_REG_TOTAL-1 downto 0);
    transfer : std_logic;
    write : std_logic;
    postexec : std_logic;
    jtag_dsu : std_logic;
    broadband_req : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);

    sberror : std_logic_vector(2 downto 0);
    sbreadonaddr : std_logic;
    sbaccess : std_logic_vector(2 downto 0);
    sbautoincrement : std_logic;
    sbreadondata : std_logic;
    sbaddress : std_logic_vector(63 downto 0);
  end record;

  constant R_RESET : registers := (
     Idle, -- state
     "00", -- dmstat
     (others => '0'), -- hartsel
     '0', -- ndmreset
     '0', -- resumeack
     '0', -- halt_after_reset
     (others => '0'), -- addr
     (others => '0'), -- rdata
     (others => '0'), -- wdata
     (others => '0'), -- wstrb
     (others => '0'), -- memdata
     (others => '0'),  -- arg0
     (others => '0'),  -- command
     (others => '0'),  -- autoexecdata
     (others => '0'),  -- autoexecprogbuf
     '0', -- transfer
     '0', -- write
     '0',  -- postexec
     '0',  -- jtag_dsu
     (others => '0'),  -- broadband_req
     (others => '0'),  -- sberror
     '0',  -- sbreadonaddr
     (others => '0'),  -- sbaccess
     '0',  -- sbautoincrement
     '0',  -- sbreadondata
     (others => '0')  -- sbaddress
  );

  constant h004_DATA0 : std_logic_vector(11 downto 0) := X"004";
  constant h005_DATA1 : std_logic_vector(11 downto 0) := X"005";
  constant h010_DMCONTROL : std_logic_vector(11 downto 0) := X"010";
  constant h011_DMSTATUS : std_logic_vector(11 downto 0) := X"011";
  constant h016_ABSTRACTCS : std_logic_vector(11 downto 0) := X"016";
  constant h017_COMMAND : std_logic_vector(11 downto 0) := X"017";
  constant h018_ABSTRACTAUTO : std_logic_vector(11 downto 0) := X"018";
  constant h02n_PROGBUFn : std_logic_vector(11 downto 0) := X"020";
  constant h038_SBCS : std_logic_vector(11 downto 0) := X"038";
  constant h039_SBADDRESS0 : std_logic_vector(11 downto 0) := X"039";
  constant h03A_SBADDRESS1 : std_logic_vector(11 downto 0) := X"03A";
  constant h03C_SBDATA0 : std_logic_vector(11 downto 0) := X"03C";
  constant h03D_SBDATA1 : std_logic_vector(11 downto 0) := X"03D";
  constant h040_HALTSUM0 : std_logic_vector(11 downto 0) := X"040";

  signal r, rin: registers;

begin

  comblogic : process(nrst,
       i_dmi_jtag_req_valid, i_dmi_jtag_write, i_dmi_jtag_addr, i_dmi_jtag_wdata, i_dmi_jtag_resp_ready,
       i_dmi_dsu_req_valid, i_dmi_dsu_write, i_dmi_dsu_addr, i_dmi_dsu_wdata, i_dmi_dsu_resp_ready,
       i_xmsti, i_dporto, r)
    variable v : registers;
    variable v_dmi_jtag_req_ready : std_logic;
    variable v_dmi_dsu_req_ready : std_logic;
    variable v_dmi_jtag_resp_valid : std_logic;
    variable v_dmi_dsu_resp_valid : std_logic;
    variable vdporti : dport_in_vector;
    variable v_ar_valid : std_logic;
    variable v_aw_valid : std_logic;
    variable v_w_valid : std_logic;
    variable vxmsto : axi4_master_out_type;
    variable hsel : integer range 0 to CFG_TOTAL_CPU_MAX-1;
    variable v_axi_ready : std_logic;
    variable vb_haltsum : std_logic_vector(CFG_TOTAL_CPU_MAX-1 downto 0);
    variable sbaidx3 : integer range 0 to 7;
    variable sbaidx2 : integer range 0 to 3;
    variable sbaidx1 : integer range 0 to 1;
  begin
    v := r;

    v_dmi_jtag_req_ready := '0';
    v_dmi_dsu_req_ready := '0';
    v_dmi_jtag_resp_valid := '0';
    v_dmi_dsu_resp_valid := '0';
    vdporti := (others => dport_in_none);
    vxmsto := axi4_master_out_none;
    v_ar_valid := '0';
    v_aw_valid := '0';
    v_w_valid := '0';
    hsel := conv_integer(r.hartsel);
    sbaidx3 := conv_integer(r.sbaddress(2 downto 0));
    sbaidx2 := conv_integer(r.sbaddress(2 downto 1));
    sbaidx1 := conv_integer(r.sbaddress(2 downto 2));
    
    for n in 0 to CFG_TOTAL_CPU_MAX-1 loop
      vb_haltsum(n) := i_dporto(n).halted;
    end loop;

    case r.state is
    when Idle =>
        v.addr := (others => '0');
        v.wdata := (others => '0');
        v.rdata := (others => '0');
        v.transfer := '0';
        v.postexec := '0';
        if i_dmi_jtag_req_valid = '1' then
            v.jtag_dsu := '0';
            v_dmi_jtag_req_ready := '1';
            v.write := i_dmi_jtag_write;
            v.addr(6 downto 0) := i_dmi_jtag_addr;
            v.wdata(31 downto 0) := i_dmi_jtag_wdata;
            v.state := DmiRequest;
        elsif i_dmi_dsu_req_valid = '1' then
            v.jtag_dsu := '1';
            v_dmi_dsu_req_ready := '1';
            v.write := i_dmi_dsu_write;
            v.addr(6 downto 0) := i_dmi_dsu_addr;
            v.wdata(31 downto 0) := i_dmi_dsu_wdata;
            v.state := DmiRequest;
        end if;
          
    when DmiRequest =>
        v.state := DmiResponse;       -- default no dport transfer
        if r.addr(11 downto 0) = h004_DATA0 then
            v.rdata(31 downto 0) := r.arg0(31 downto 0);
            if r.write = '1' then
                v.arg0(31 downto 0) := r.wdata(31 downto 0);
            end if;
            if r.autoexecdata(0) = '1'then 
                v.state := AbstractCommand;
            end if;
        elsif r.addr(11 downto 0) = h005_DATA1 then
            v.rdata(31 downto 0) := r.arg0(63 downto 32);
            if r.write = '1' then
                v.arg0(63 downto 32) := r.wdata(31 downto 0);
            end if;
            if r.autoexecdata(1) = '1' then
                v.state := AbstractCommand;
            end if;
        elsif r.addr(11 downto 0) = h010_DMCONTROL then
            v.rdata(16+HARTSELLEN-1 downto 16) := r.hartsel;
            v.rdata(1) := r.ndmreset;
            v.rdata(0) := '1';                          -- dmactive: 1=module functional normally 
            if r.write = '1' then
                -- Access to CSR only on writing
                v.hartsel := r.wdata(16+HARTSELLEN-1 downto 16) and HART_AVAILABLE_MASK;  -- hartsello
                v.ndmreset := r.wdata(1);               -- ndmreset
                v.resumeack := not r.wdata(31) and r.wdata(30) and i_dporto(conv_integer(v.hartsel)).halted;
                if r.ndmreset = '1' and r.wdata(1) = '0' and r.halt_after_reset = '1' then
                    v.state := DportRequest;
                    v.addr(13 downto 0) := "00" & CSR_runcontrol;
                    v.wdata := (others => '0');
                    v.wdata(31) := '1';                  -- haltreq:
                elsif r.wdata(1) = '1' then              -- ndmreset
                    -- do not make DPort request the CPU will be resetted and cannot respond
                    v.halt_after_reset := r.wdata(31);   -- haltreq
                elsif (r.wdata(31) or r.wdata(30)) = '1' then
                    v.state := DportRequest;
                    v.addr(13 downto 0) := "00" & CSR_runcontrol;
                end if;
            end if;
        elsif r.addr(11 downto 0) = h011_DMSTATUS then
            v.rdata(17) := r.resumeack;                 -- allresumeack
            v.rdata(16) := r.resumeack;                 -- anyresumeack
            v.rdata(15) := not i_dporto(hsel).available; -- allnonexistent
            v.rdata(14) := not i_dporto(hsel).available; -- anynonexistent
            v.rdata(13) := not i_dporto(hsel).available; -- allunavail
            v.rdata(12) := not i_dporto(hsel).available; -- anyunavail
            v.rdata(11) := not i_dporto(hsel).halted and i_dporto(hsel).available; -- allrunning:
            v.rdata(10) := not i_dporto(hsel).halted and i_dporto(hsel).available; -- anyrunning:
            v.rdata(9) := i_dporto(hsel).halted and i_dporto(hsel).available;      -- allhalted:
            v.rdata(8) := i_dporto(hsel).halted and i_dporto(hsel).available;      -- anyhalted:
            v.rdata(7) := '1';                          -- authenticated:
            v.rdata(3 downto 0) := X"2";                -- version: dbg spec v0.13
        elsif r.addr(11 downto 0) = h016_ABSTRACTCS then
            v.state := DportRequest;
            v.addr(13 downto 0) := "00" & CSR_abstractcs;
        elsif r.addr(11 downto 0) = h017_COMMAND then
            if r.write = '1' then
                v.command := r.wdata(31 downto 0);          -- original value for auto repeat
                v.state := AbstractCommand;
            end if;
        elsif r.addr(11 downto 0) = h018_ABSTRACTAUTO then
            v.rdata(CFG_DATA_REG_TOTAL-1 downto 0) := r.autoexecdata;
            v.rdata(16+CFG_PROGBUF_REG_TOTAL-1 downto 16) := r.autoexecprogbuf;
            if r.write = '1' then
                v.autoexecdata := r.wdata(CFG_DATA_REG_TOTAL-1 downto 0);
                v.autoexecprogbuf := r.wdata(16+CFG_PROGBUF_REG_TOTAL-1 downto 16);
            end if;
        elsif r.addr(11 downto 4) = h02n_PROGBUFn(11 downto 4) then          -- PROGBUF0..PROGBUF15
            v.addr(13 downto 0) := "00" & CSR_progbuf;
            v.wdata(35 downto 32) :=  r.addr(3 downto 0);
            v.broadband_req := (others => '1');         -- to all Harts
            v.state := DportBroadbandRequest;
        elsif r.addr(11 downto 0) = h038_SBCS then
            v.rdata(31 downto 29) := "001";               -- sbversion: 1=current spec
            if (r.state = Dma_AR) or (r.state = Dma_R)
             or (r.state = Dma_AW) or (r.state = Dma_W) or (r.state = Dma_B) then
                v.rdata(21) := '1';                       -- sbbusy
            end if;
            v.rdata(20) := r.sbreadonaddr;                -- when 1 auto-read on write to sbaddress0
            v.rdata(19 downto 17) := r.sbaccess;          -- 2=32; 3=64 bits
            v.rdata(16) := r.sbautoincrement;             -- increment after each system access
            v.rdata(15) := r.sbreadondata;                -- when 1 every auto-read on read from sbdata0
            v.rdata(14 downto 12) := r.sberror;           -- 1=timeout; 2=bad address; 3=unalignment;4=wrong size
            v.rdata(11 downto 5) := conv_std_logic_vector(64,7); -- system bus width in bits
            v.rdata(3) := '1';                            -- sbaccess64 - supported 64-bit access
            v.rdata(2) := '1';                            -- sbaccess32 - supported 32-bit access
            v.rdata(1) := '1';                            -- sbaccess16 - supported 16-bit access
            v.rdata(0) := '1';                            -- sbaccess8  - supported 8-bit access
            if r.write = '1' then
                v.sbreadonaddr := r.wdata(20);
                v.sbaccess := r.wdata(19 downto 17);
                v.sbautoincrement := r.wdata(16);
                v.sbreadondata := r.wdata(15);
                if r.wdata(12) = '1' then
                    v.sberror := (others => '0');
                end if;
            end if;
        elsif r.addr(11 downto 0) = h039_SBADDRESS0 then
            v.rdata(31 downto 0) := r.sbaddress(31 downto 0);
            if r.write = '1' then
                v.sbaddress(31 downto 0) := r.wdata(31 downto 0);
                if r.sbreadonaddr = '1' then
                    v.state := Dma_AR;
                end if;
            end if;
        elsif r.addr(11 downto 0) = h03A_SBADDRESS1 then
            v.rdata(31 downto 0) := r.sbaddress(63 downto 32);
            if r.write = '1' then
                v.sbaddress(63 downto 32) := r.wdata(31 downto 0);
            end if;
        elsif r.addr(11 downto 0) = h03C_SBDATA0 then
            v.rdata(31 downto 0) := r.memdata(31 downto 0);
            if r.write = '0' then
                v.state := Dma_AR;
            else
                v.state := Dma_AW;
                v.memdata := (others => '0');
                v.wstrb := (others => '0');
                case r.sbaccess is
                when "000" =>   -- 8-bits access
                    v.memdata(8*sbaidx3+7 downto 8*sbaidx3) := r.wdata(7 downto 0);
                    v.wstrb(sbaidx3) := '1';
                when "001" =>   -- 16-bits access
                    v.memdata(16*sbaidx2+15 downto 16*sbaidx2) := r.wdata(15 downto 0);
                    v.wstrb(2*sbaidx2+1 downto 2*sbaidx2) := "11";
                when "010" =>   -- 32-bits access
                    v.memdata(32*sbaidx1+31 downto 32*sbaidx1) := r.wdata(31 downto 0);
                    v.wstrb(4*sbaidx1+3 downto 4*sbaidx1) := X"F";
                when others =>
                    v.memdata := r.wdata;
                    v.wstrb := X"FF";
                end case;
            end if;
        elsif r.addr(11 downto 0) = h03D_SBDATA1 then
            v.rdata(31 downto 0) := r.memdata(63 downto 32);
            if r.write = '0' then
                v.memdata(63 downto 32) := r.wdata(31 downto 0);
            end if;
        elsif r.addr(11 downto 0) = h040_HALTSUM0 then
            v.rdata(CFG_TOTAL_CPU_MAX-1 downto 0) := vb_haltsum;
        end if;

    when AbstractCommand =>
        v.state := DmiResponse;  -- no transfer or not implemented command type
        if r.command(31 downto 24) = X"00" then       -- cmdtype: 0=register access
            v.wdata := r.arg0;
            v.addr(13 downto 0) := r.command(13 downto 0); -- regno:
            v.write := r.command(16);                 -- write:
            v.transfer := r.command(17);              -- transfer
            v.postexec := r.command(18);              -- postexec:
            if r.command(19) = '1' then               -- aarpostincrement
                v.command(13 downto 0) := r.command(13 downto 0) + 1;
            end if;
            if r.command(16) = '0' or r.command(17) = '1' then
                -- read operation or write with transfer
                v.state := DportRequest;
            end if;
        end if;
          
    when DportRequest =>  
        vdporti(hsel).req_valid := '1';
        vdporti(hsel).addr := r.addr;
        vdporti(hsel).write := r.write;
        vdporti(hsel).wdata := r.wdata;
        if i_dporto(hsel).req_ready = '1' then
            v.state := DportResponse;
        end if;
    when DportResponse =>
        vdporti(hsel).resp_ready := '1';
        if i_dporto(hsel).resp_valid = '1' then
            v.state := DmiResponse;
            v.rdata := i_dporto(hsel).rdata;
            if r.write = '0' and r.transfer = '1' then
                v.arg0 := i_dporto(hsel).rdata;
            end if;
            if r.postexec = '1' then
                v.state := DportPostexec;
            end if;
        end if;
    when DportPostexec =>
        v.write := '1';
        v.postexec := '0';
        v.transfer := '0';
        v.addr(13 downto 0) := "00" & CSR_runcontrol;
        v.wdata := (others => '0');
        v.wdata(27) := '1';             -- req_progbuf: request to execute progbuf
        v.state := DportRequest;

    when DportBroadbandRequest =>
        for i in 0 to CFG_TOTAL_CPU_MAX-1 loop
            vdporti(i).req_valid := r.broadband_req(i);
            vdporti(i).wdata := r.wdata;
            vdporti(i).addr := r.addr;
            vdporti(i).write := r.write;
            if i_dporto(i).req_ready = '1' then
                v.broadband_req(i) := '0';
            end if;
        end loop;
        if or_reduce(r.broadband_req) = '0' then
            v.broadband_req := (others => '1');
            v.state := DportBroadbandResponse;
        end if;
    when DportBroadbandResponse =>
        for i in 0 to CFG_TOTAL_CPU_MAX-1 loop
             vdporti(i).resp_ready := r.broadband_req(i);
             if i_dporto(i).resp_valid = '1' then
                v.broadband_req(i) := '0';
             end if;
        end loop;
        if or_reduce(r.broadband_req) = '0' then
            if r.postexec = '1' then
                v.state := DportPostexec;
            else
                v.state := DmiResponse;
            end if;
        end if;

    when Dma_AR =>
        v_ar_valid := '1';
        if i_xmsti.ar_ready = '1' then
            v.state := Dma_R;
        end if;
    when Dma_R =>
        case r.sbaccess is
        when "000" =>   -- 8-bits access
            v.memdata(7 downto 0) := i_xmsti.r_data(8*sbaidx3+7 downto 8*sbaidx3);
        when "001" =>   -- 16-bits access
            v.memdata(15 downto 0) := i_xmsti.r_data(16*sbaidx2+15 downto 16*sbaidx2);
        when "010" =>   -- 32-bits access
            v.memdata(31 downto 0) := i_xmsti.r_data(32*sbaidx1+31 downto 32*sbaidx1);
        when others =>
            v.memdata := i_xmsti.r_data;
        end case;
        if i_xmsti.r_valid = '1' then
            v.state := DmiResponse;
            if i_xmsti.r_resp(1) = '1' then
                v.sberror := "010";   -- Bad address was accessed
            end if;
            if r.sbautoincrement = '1' then
                v.sbaddress := r.sbaddress + XSizeToBytes(sbaidx3);
            end if;
        end if;
    when Dma_AW =>
        v_aw_valid := '1';
        if i_xmsti.aw_ready = '1' then
            v.state := Dma_W;
        end if;
    when Dma_W =>
        v_w_valid := '1';
        if i_xmsti.w_ready = '1' then
            v.state := Dma_B;
        end if;
    when Dma_B =>
        if i_xmsti.b_valid = '1' then
            v.state := DmiResponse;
            if i_xmsti.b_resp(1) = '1' then
                v.sberror := "010";   -- Bad address was accessed
            end if;
            if r.sbautoincrement = '1' then
                v.sbaddress := r.sbaddress + XSizeToBytes(sbaidx3);
            end if;
        end if;

    when DmiResponse =>
        v_dmi_jtag_resp_valid := not r.jtag_dsu;
        v_dmi_dsu_resp_valid := r.jtag_dsu;
        if (not r.jtag_dsu and i_dmi_jtag_resp_ready) = '1' or
           (r.jtag_dsu and i_dmi_dsu_resp_ready) = '1' then
            v.state := Idle;
        end if;
    when others =>
    end case;

    if not async_reset and nrst = '0' then 
        v := R_RESET;
    end if;

    vxmsto.ar_valid := v_ar_valid; 
    vxmsto.ar_bits.addr := r.sbaddress(CFG_SYSBUS_ADDR_BITS-1 downto 0);
    vxmsto.ar_bits.size := r.sbaccess;
    vxmsto.r_ready := '1';
    
    vxmsto.aw_valid := v_aw_valid; 
    vxmsto.aw_bits.addr := r.sbaddress(CFG_SYSBUS_ADDR_BITS-1 downto 0);
    vxmsto.aw_bits.size := r.sbaccess;
    vxmsto.w_valid := v_w_valid; 
    vxmsto.w_data := r.memdata(CFG_SYSBUS_DATA_BITS-1 downto 0);
    vxmsto.w_strb := r.wstrb(CFG_SYSBUS_DATA_BYTES-1 downto 0);
    vxmsto.w_last := '1';
    vxmsto.b_ready := '1';

    rin <= v;

    o_dmi_jtag_req_ready <= v_dmi_jtag_req_ready;
    o_dmi_jtag_resp_valid <= v_dmi_jtag_resp_valid;
    o_dmi_jtag_rdata <= r.rdata(31 downto 0);

    o_dmi_dsu_req_ready <= v_dmi_dsu_req_ready;
    o_dmi_dsu_resp_valid <= v_dmi_dsu_resp_valid;
    o_dmi_dsu_rdata <= r.rdata(31 downto 0);

    o_dporti <= vdporti;
    o_xmsto <= vxmsto;
  end process;

  o_cfg  <= xconfig;
  o_hartsel <= r.hartsel;
  o_ndmreset <= r.ndmreset;
  o_dmstat <= r.dmstat;


  -- registers:
  regs : process(clk, nrst)
  begin 
      if async_reset and nrst = '0' then
          r <= R_RESET;
      elsif rising_edge(clk) then 
          r <= rin;
      end if; 
  end process;

end;
