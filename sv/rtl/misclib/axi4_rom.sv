module axi4_rom #(
    parameter integer memtech = 0,
    parameter bit async_reset = 0,
    parameter longint xaddr = 0,
    parameter longint unsigned xmask = 'hfffff,
    parameter sim_hexfile = ""
)
(
    input clk,
    input nrst,
    output types_amba_pkg::axi4_slave_config_type cfg,
    input types_amba_pkg::axi4_slave_in_type i,
    output types_amba_pkg::axi4_slave_out_type o
);
 
import types_amba_pkg::*;
 
// To avoid warning 'literal negative value' use -1048576 instead of 16#fff00000#
localparam integer size_4kbytes = ((~(xmask << 12)) + 1) >> 12; 
localparam integer abits = 12 + $clog2(size_4kbytes);

const axi4_slave_config_type xconfig = '{
   PNP_CFG_TYPE_SLAVE, //descrtype
   PNP_CFG_SLAVE_DESCR_BYTES, //descrsize
   xaddr, //xaddr
   xmask, //xmask
   VENDOR_GNSSSENSOR, //vid
   OPTIMITECH_ROM //did
};

global_addr_array_type raddr;
logic [CFG_SYSBUS_DATA_BITS-1 : 0] rdata;

assign cfg = xconfig;

axi_slv #(
  .async_reset(async_reset)
) axi0 (
  .i_clk(clk),
  .i_nrst(nrst),
  .i_xcfg(xconfig), 
  .i_xslvi(i),
  .o_xslvo(o),
  .i_ready(1'b1),
  .i_rdata(rdata),
  .o_re(),
  .o_r32(),
  .o_radr(raddr),
  .o_wadr(),
  .o_we(),
  .o_wstrb(),
  .o_wdata()
);

rom_tech #(
  .abits(abits),
  .sim_hexfile(sim_hexfile)
) tech0 (
  .clk(clk),
  .address(raddr),
  .data(rdata)
);

endmodule
