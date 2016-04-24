-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Implementation of nasti_dsu (Debug Support Unit).
--! @details   DSU provides access to the internal CPU registers (CSRs) via
--!            'Rocket-chip' specific bus HostIO.
-----------------------------------------------------------------------------
--! 
--! @page dsu_link Debug Support Unit (DSU)
--! 
--! @par Overview
--! Debug Support Unit (DSU) was developed to simplify debugging on target
--! hardware and provide access to the "Rocket-chip" specific HostIO bus
--! interface. This bus provides access to the internal CPU control registers
--! (CSRs) that store information about Core configuration, current operational
--! mode (Machine, Hypervisor, Supervisor or User) and allows to change
--! processor run-time behaviour by injecting interrupts for an example.
--! General CSR registers are described in RISC-V privileged ISA
--! specification. Take into account that CPU can have any number of platform
--! specific CSRs that usually not entirely documented.
--! 
--! @par Operation
--! DSU acts like a slave AMBA AXI4 device that is directly mapped into 
--! physical memory. Default address location for our implementation 
--! is 0x80020000. DSU directly transforms device offset address
--! into CSR index by removing last 4 bits of address.
--! All CSR values is always 64-bits width.
--!
--! @par Example:
--!     Bus transaction at address <em>0x80027820</em>
--!     will be redirected to HostIO bus with CSR index <em>0x782</em>.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
--! RISCV specific funcionality.
library rocketlib;
use rocketlib.types_rocket.all;

entity nasti_dsu is
  generic (
    xindex   : integer := 0;
    xaddr    : integer := 0;
    xmask    : integer := 16#fffff#;
    htif_index  : integer := 0
  );
  port 
  (
    clk    : in std_logic;
    nrst   : in std_logic;
    o_cfg  : out nasti_slave_config_type;
    i_axi  : in nasti_slave_in_type;
    o_axi  : out nasti_slave_out_type;
    i_host : in host_in_type;
    o_host : out host_out_type;
    o_soft_reset : out std_logic
  );
end;

architecture arch_nasti_dsu of nasti_dsu is

  constant xconfig : nasti_slave_config_type := (
     xindex => xindex,
     xaddr => conv_std_logic_vector(xaddr, CFG_NASTI_CFG_ADDR_BITS),
     xmask => conv_std_logic_vector(xmask, CFG_NASTI_CFG_ADDR_BITS),
     vid => VENDOR_GNSSSENSOR,
     did => GNSSSENSOR_DSU,
     descrtype => PNP_CFG_TYPE_SLAVE,
     descrsize => PNP_CFG_SLAVE_DESCR_BYTES
  );
  constant CSR_MRESET : std_logic_vector(11 downto 0) := X"782";

type state_type is (wait_grant, writting, wait_resp, skip1);

type registers is record
  bank_axi : nasti_slave_bank_type;
  --! Message multiplexer to form 128 request message of writting into CSR
  state         : state_type;
  addr16_sel : std_logic;
  waddr : std_logic_vector(11 downto 0);
  wdata : std_logic_vector(63 downto 0);
  rdata : std_logic_vector(63 downto 0);
  -- Soft reset via CSR 0x782 MRESET doesn't work
  -- so here I implement special register that will reset CPU via 'rst' input.
  -- Otherwise I cannot load elf-file while CPU is not halted.
  soft_reset : std_logic;
end record;

signal r, rin: registers;
begin

  comblogic : process(i_axi, i_host, r)
    variable v : registers;
    variable rdata : std_logic_vector(CFG_NASTI_DATA_BITS-1 downto 0);

    variable vhost : host_out_type;
  begin
    v := r;
    vhost := host_out_none;

    procedureAxi4(i_axi, xconfig, r.bank_axi, v.bank_axi);
    --! redefine value 'always ready' inserting waiting states.
    v.bank_axi.rwaitready := '0';

    if r.bank_axi.wstate = wtrans then
      -- 32-bits burst transaction
      v.addr16_sel := r.bank_axi.waddr(0)(16);
      v.waddr := r.bank_axi.waddr(0)(15 downto 4);
      if r.bank_axi.wburst = NASTI_BURST_INCR and r.bank_axi.wsize = 4 then
         if r.bank_axi.waddr(0)(2) = '1' then
             v.state := writting;
             v.wdata(63 downto 32) := i_axi.w_data(31 downto 0);
         else
             v.wdata(31 downto 0) := i_axi.w_data(31 downto 0);
         end if;
      else
         -- Write data on next clock.
         if i_axi.w_strb(7 downto 0) /= X"00" then
            v.wdata := i_axi.w_data(63 downto 0);
         else
            v.wdata := i_axi.w_data(127 downto 64);
         end if;
         v.state := writting;
      end if;
    end if;

    case r.state is
      when wait_grant =>
           vhost.csr_req_bits_addr := r.bank_axi.raddr(0)(15 downto 4);
           if r.bank_axi.rstate = rtrans then
               if r.bank_axi.raddr(0)(16) = '1' then
                  -- Control registers (not implemented)
                  v.bank_axi.rwaitready := '1';
                  v.rdata := (others => '0');
                  v.state := skip1;
               elsif r.bank_axi.raddr(0)(15 downto 4) = CSR_MRESET then
                  v.bank_axi.rwaitready := '1';
                  v.rdata(0) := r.soft_reset;
                  v.rdata(63 downto 1) := (others => '0');
                  v.state := skip1;
               else
                  vhost.csr_req_valid     := '1';
                  if (i_host.grant(htif_index) and i_host.csr_req_ready) = '1' then
                    v.state := wait_resp;
                  end if;
               end if;
           end if;
      when writting =>
           if r.addr16_sel = '1' then
              -- Bank with control register (not implemented by CPU)
              v.bank_axi.rwaitready := '1';
              v.state := skip1;
           elsif r.waddr = CSR_MRESET then
              -- Soft Reset
              v.bank_axi.rwaitready := '1';
              v.soft_reset := r.wdata(0);
              v.state := skip1;
           else
              vhost.csr_req_valid     := '1';
              vhost.csr_req_bits_rw   := '1';
              vhost.csr_req_bits_addr := r.waddr;
              vhost.csr_req_bits_data := r.wdata;
              if (i_host.grant(htif_index) and i_host.csr_req_ready) = '1' then
                 v.state := wait_resp;
              end if;
           end if;
      when wait_resp =>
           vhost.csr_resp_ready := '1';
           if i_host.csr_resp_valid = '1' then
               v.state := skip1;
               v.rdata := i_host.csr_resp_bits;
               v.bank_axi.rwaitready := '1';
           end if;
      when skip1 =>
           v.state := wait_grant;
      when others =>
    end case;

    if r.bank_axi.raddr(0)(2) = '0' then
       rdata(31 downto 0) := r.rdata(31 downto 0);
    else
       -- 32-bits aligned access (can be generated by MAC)
       rdata(31 downto 0) := r.rdata(63 downto 32);
    end if;
    rdata(63 downto 32) := r.rdata(63 downto 32);

    if CFG_NASTI_DATA_BITS = 128 then
      rdata(95 downto 64) := rdata(31 downto 0);
      rdata(127 downto 96) := rdata(63 downto 32);
    end if;

    o_axi <= functionAxi4Output(r.bank_axi, rdata);
    o_host <= vhost;

    rin <= v;
  end process;

  o_cfg  <= xconfig;
  o_soft_reset <= r.soft_reset;


  -- registers:
  regs : process(clk, nrst)
  begin 
    if nrst = '0' then 
       r.bank_axi <= NASTI_SLAVE_BANK_RESET;
       r.state <= wait_grant;
       r.addr16_sel <= '0';
       r.waddr <= (others => '0');
       r.wdata <= (others => '0');
       r.rdata <= (others => '0');
       r.soft_reset <= '0';
    elsif rising_edge(clk) then 
       r <= rin; 
    end if; 
  end process;
end;
