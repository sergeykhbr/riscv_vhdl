module axi4_rom #(
    parameter integer memtech = 0,
    parameter integer abits = 12,
    parameter bit async_reset = 0,
    parameter filename = ""  // without '.hex' extension
)
(
    input clk,
    input nrst,
    input types_amba_pkg::mapinfo_type i_mapinfo,
    output types_amba_pkg::dev_config_type cfg,
    input types_amba_pkg::axi4_slave_in_type i,
    output types_amba_pkg::axi4_slave_out_type o
);
 
import types_amba_pkg::*;
 
// To avoid warning 'literal negative value' use -1048576 instead of 16#fff00000#
//parameter integer size_4kbytes = ((~(xmask << 12)) + 1) >> 12; 
//parameter integer abits = 12 + log2[size_4kbytes];

logic w_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_req_addr;
logic [7:0] wb_req_size;
logic w_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_req_wstrb;
logic w_req_last;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_rdata;

axi_slv #(
   .async_reset(async_reset),
   .vid(VENDOR_OPTIMITECH),
   .did(OPTIMITECH_ROM)
) axi0 (
    .i_clk(clk),
    .i_nrst(nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(cfg),
    .i_xslvi(i),
    .o_xslvo(o),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_size(wb_req_size),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .o_req_wstrb(wb_req_wstrb),
    .o_req_last(w_req_last),
    .i_req_ready(1'b1),
    .i_resp_valid(1'b1),
    .i_resp_rdata(wb_rdata),
    .i_resp_err(1'd0)
);

rom_tech #(
  .abits(abits),
  .log2_dbytes(CFG_LOG2_SYSBUS_DATA_BYTES),
  .filename(filename)
) tech0 (
  .clk(clk),
  .address(wb_req_addr[abits-1: 0]),
  .data(wb_rdata)
);

endmodule
