-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     TileLink-to-AXI4 bridge implementation.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
--! AMBA system bus specific library.
library ambalib;
--! AXI4 configuration constants.
use ambalib.types_amba4.all;
library rocketlib;
use rocketlib.types_rocket.all;

entity Tile2Axi is
  port (
    clk   : in  std_logic;
    nrst  : in  std_logic;

    --! Tile-to-AXI direction
    tloi : in tile_out_type;
    msto : out nasti_master_out_type;
    --! AXI-to-Tile direction
    msti : in nasti_master_in_type;
    tlio : out tile_in_type
  );
end; 
 
architecture arch_Tile2Axi of Tile2Axi is

component TLToAXI4
port (
    clock : in  std_logic;
    reset : in  std_logic;
    io_in_0_a_ready : out  std_logic;
    io_in_0_a_valid : in  std_logic;
    io_in_0_a_bits_opcode : in std_logic_vector(2 downto 0);
    io_in_0_a_bits_param : in std_logic_vector(2 downto 0);
    io_in_0_a_bits_size : in std_logic_vector(3 downto 0);
    io_in_0_a_bits_source : in std_logic_vector(5 downto 0);
    io_in_0_a_bits_address : in std_logic_vector(31 downto 0);
    io_in_0_a_bits_mask : in std_logic_vector(7 downto 0);
    io_in_0_a_bits_data : in std_logic_vector(63 downto 0);
    io_in_0_b_ready : in  std_logic;
    io_in_0_b_valid : out  std_logic;
    io_in_0_b_bits_opcode : out std_logic_vector(2 downto 0);
    io_in_0_b_bits_param : out std_logic_vector(1 downto 0);
    io_in_0_b_bits_size : out std_logic_vector(3 downto 0);
    io_in_0_b_bits_source : out std_logic_vector(5 downto 0);
    io_in_0_b_bits_address : out std_logic_vector(31 downto 0);
    io_in_0_b_bits_mask : out std_logic_vector(7 downto 0);
    io_in_0_b_bits_data : out std_logic_vector(63 downto 0);
    io_in_0_c_ready : out  std_logic;
    io_in_0_c_valid : in  std_logic;
    io_in_0_c_bits_opcode : in std_logic_vector(2 downto 0);
    io_in_0_c_bits_param : in std_logic_vector(2 downto 0);
    io_in_0_c_bits_size : in std_logic_vector(3 downto 0);
    io_in_0_c_bits_source : in std_logic_vector(5 downto 0);
    io_in_0_c_bits_address : in std_logic_vector(31 downto 0);
    io_in_0_c_bits_data : in std_logic_vector(63 downto 0);
    io_in_0_c_bits_error : in  std_logic;
    io_in_0_d_ready : in  std_logic;
    io_in_0_d_valid : out  std_logic;
    io_in_0_d_bits_opcode : out std_logic_vector(2 downto 0);
    io_in_0_d_bits_param : out std_logic_vector(1 downto 0);
    io_in_0_d_bits_size : out std_logic_vector(3 downto 0);
    io_in_0_d_bits_source : out std_logic_vector(5 downto 0);
    io_in_0_d_bits_sink : out  std_logic;
    io_in_0_d_bits_addr_lo : out std_logic_vector(2 downto 0);
    io_in_0_d_bits_data : out std_logic_vector(63 downto 0);
    io_in_0_d_bits_error : out  std_logic;
    io_in_0_e_ready : out  std_logic;
    io_in_0_e_valid : in  std_logic;
    io_in_0_e_bits_sink : in  std_logic;
    io_out_0_aw_ready : in  std_logic;
    io_out_0_aw_valid : out  std_logic;
    io_out_0_aw_bits_id : out std_logic_vector(3 downto 0);
    io_out_0_aw_bits_addr : out std_logic_vector(31 downto 0);
    io_out_0_aw_bits_len : out std_logic_vector(7 downto 0);
    io_out_0_aw_bits_size : out std_logic_vector(2 downto 0);
    io_out_0_aw_bits_burst : out std_logic_vector(1 downto 0);
    io_out_0_aw_bits_lock : out  std_logic;
    io_out_0_aw_bits_cache : out std_logic_vector(3 downto 0);
    io_out_0_aw_bits_prot : out std_logic_vector(2 downto 0);
    io_out_0_aw_bits_qos : out std_logic_vector(3 downto 0);
    io_out_0_w_ready : in  std_logic;
    io_out_0_w_valid : out  std_logic;
    io_out_0_w_bits_data : out std_logic_vector(63 downto 0);
    io_out_0_w_bits_strb : out std_logic_vector(7 downto 0);
    io_out_0_w_bits_last : out  std_logic;
    io_out_0_b_ready : out  std_logic;
    io_out_0_b_valid : in  std_logic;
    io_out_0_b_bits_id : in std_logic_vector(3 downto 0);
    io_out_0_b_bits_resp : in std_logic_vector(1 downto 0);
    io_out_0_ar_ready : in  std_logic;
    io_out_0_ar_valid : out  std_logic;
    io_out_0_ar_bits_id : out std_logic_vector(3 downto 0);
    io_out_0_ar_bits_addr : out std_logic_vector(31 downto 0);
    io_out_0_ar_bits_len : out std_logic_vector(7 downto 0);
    io_out_0_ar_bits_size : out std_logic_vector(2 downto 0);
    io_out_0_ar_bits_burst : out std_logic_vector(1 downto 0);
    io_out_0_ar_bits_lock : out std_logic;
    io_out_0_ar_bits_cache : out std_logic_vector(3 downto 0);
    io_out_0_ar_bits_prot : out std_logic_vector(2 downto 0);
    io_out_0_ar_bits_qos : out std_logic_vector(3 downto 0);
    io_out_0_r_ready : out  std_logic;
    io_out_0_r_valid : in  std_logic;
    io_out_0_r_bits_id : in std_logic_vector(3 downto 0);
    io_out_0_r_bits_data : in std_logic_vector(63 downto 0);
    io_out_0_r_bits_resp : in std_logic_vector(1 downto 0);
    io_out_0_r_bits_last : in  std_logic
);
end component;

  signal reset : std_logic;
  signal wb_a_source : std_logic_vector(5 downto 0);
  signal wb_b_source : std_logic_vector(5 downto 0);
  signal wb_c_source : std_logic_vector(5 downto 0);
  signal wb_d_source : std_logic_vector(5 downto 0);
  signal wb_aw_bits_addr : std_logic_vector(31 downto 0);
  signal wb_ar_bits_addr : std_logic_vector(31 downto 0);

begin

  reset <= not nrst;
  wb_a_source <= "000" & tloi.a_source;
  tlio.b_source <= wb_b_source(2 downto 0);
  wb_c_source <= "000" & tloi.c_source;
  tlio.d_source <= wb_d_source(2 downto 0);
  tlio.d_sink(3 downto 1) <= "000";

  ver0 : TLToAXI4 port map (
    clock => clk,
    reset => reset,
    io_in_0_a_ready => tlio.a_ready,
    io_in_0_a_valid => tloi.a_valid,
    io_in_0_a_bits_opcode => tloi.a_opcode,
    io_in_0_a_bits_param => tloi.a_param,
    io_in_0_a_bits_size => tloi.a_size,
    io_in_0_a_bits_source => wb_a_source,
    io_in_0_a_bits_address => tloi.a_address,
    io_in_0_a_bits_mask => tloi.a_mask,
    io_in_0_a_bits_data => tloi.a_data,
    io_in_0_b_ready => tloi.b_ready,
    io_in_0_b_valid => tlio.b_valid,
    io_in_0_b_bits_opcode => tlio.b_opcode,
    io_in_0_b_bits_param => tlio.b_param,
    io_in_0_b_bits_size => tlio.b_size,
    io_in_0_b_bits_source => wb_b_source,
    io_in_0_b_bits_address => tlio.b_address,
    io_in_0_b_bits_mask => tlio.b_mask,
    io_in_0_b_bits_data => tlio.b_data,
    io_in_0_c_ready => tlio.c_ready,
    io_in_0_c_valid => tloi.c_valid,
    io_in_0_c_bits_opcode => tloi.c_opcode,
    io_in_0_c_bits_param => tloi.c_param,
    io_in_0_c_bits_size => tloi.c_size,
    io_in_0_c_bits_source => wb_c_source,
    io_in_0_c_bits_address => tloi.c_address,
    io_in_0_c_bits_data => tloi.c_data,
    io_in_0_c_bits_error => tloi.c_error,
    io_in_0_d_ready => tloi.d_ready,
    io_in_0_d_valid => tlio.d_valid,
    io_in_0_d_bits_opcode => tlio.d_opcode,
    io_in_0_d_bits_param => tlio.d_param,
    io_in_0_d_bits_size => tlio.d_size,
    io_in_0_d_bits_source => wb_d_source,
    io_in_0_d_bits_sink => tlio.d_sink(0),
    io_in_0_d_bits_addr_lo => tlio.d_addr_lo,
    io_in_0_d_bits_data => tlio.d_data,
    io_in_0_d_bits_error => tlio.d_error,
    io_in_0_e_ready => tlio.e_ready,
    io_in_0_e_valid => tloi.e_valid,
    io_in_0_e_bits_sink => tloi.e_sink(0),
    io_out_0_aw_ready => msti.aw_ready,
    io_out_0_aw_valid => msto.aw_valid,
    io_out_0_aw_bits_id => msto.aw_id(3 downto 0),
    io_out_0_aw_bits_addr => wb_aw_bits_addr,
    io_out_0_aw_bits_len => msto.aw_bits.len,
    io_out_0_aw_bits_size => msto.aw_bits.size,
    io_out_0_aw_bits_burst => msto.aw_bits.burst,
    io_out_0_aw_bits_lock => msto.aw_bits.lock,
    io_out_0_aw_bits_cache => msto.aw_bits.cache,
    io_out_0_aw_bits_prot => msto.aw_bits.prot,
    io_out_0_aw_bits_qos => msto.aw_bits.qos,
    io_out_0_w_ready => msti.w_ready,
    io_out_0_w_valid => msto.w_valid,
    io_out_0_w_bits_data => msto.w_data,
    io_out_0_w_bits_strb => msto.w_strb,
    io_out_0_w_bits_last => msto.w_last,
    io_out_0_b_ready => msto.b_ready,
    io_out_0_b_valid => msti.b_valid,
    io_out_0_b_bits_id => msti.b_id(3 downto 0),
    io_out_0_b_bits_resp => msti.b_resp,
    io_out_0_ar_ready => msti.ar_ready,
    io_out_0_ar_valid => msto.ar_valid,
    io_out_0_ar_bits_id => msto.ar_id(3 downto 0),
    io_out_0_ar_bits_addr => wb_ar_bits_addr,
    io_out_0_ar_bits_len => msto.ar_bits.len,
    io_out_0_ar_bits_size => msto.ar_bits.size,
    io_out_0_ar_bits_burst => msto.ar_bits.burst,
    io_out_0_ar_bits_lock => msto.ar_bits.lock,
    io_out_0_ar_bits_cache => msto.ar_bits.cache,
    io_out_0_ar_bits_prot => msto.ar_bits.prot,
    io_out_0_ar_bits_qos => msto.ar_bits.qos,
    io_out_0_r_ready => msto.r_ready,
    io_out_0_r_valid => msti.r_valid,
    io_out_0_r_bits_id => msti.r_id(3 downto 0),
    io_out_0_r_bits_data => msti.r_data,
    io_out_0_r_bits_resp => msti.r_resp,
    io_out_0_r_bits_last => msti.r_last
  );
  
  msto.aw_bits.addr <= wb_aw_bits_addr(31 downto 3) & "000";
  msto.ar_bits.addr <= wb_ar_bits_addr(31 downto 3) & "000";

end;
