//!
//! Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
//!
//! Licensed under the Apache License, Version 2.0 (the "License");
//! you may not use this file except in compliance with the License.
//! You may obtain a copy of the License at
//!
//!     http://www.apache.org/licenses/LICENSE-2.0
//!
//! Unless required by applicable law or agreed to in writing, software
//! distributed under the License is distributed on an "AS IS" BASIS,
//! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//! See the License for the specific language governing permissions and
//! limitations under the License.
//!

module axi4_sram
#(
    parameter int async_reset = 0,
    parameter longint xaddr = 0,
    parameter longint xmask = 'hfffff,
    parameter int abits = 17,
    parameter init_file = ""
)
(
    input                                                   clk,
    input                                                   nrst,
    output    types_amba_pkg::axi4_slave_config_type              cfg,
    input     types_amba_pkg::axi4_slave_in_type                  i,
    output    types_amba_pkg::axi4_slave_out_type                 o
);

import types_amba_pkg::*;

const axi4_slave_config_type xconfig = '{
    descrtype : PNP_CFG_TYPE_SLAVE,
    descrsize : PNP_CFG_SLAVE_DESCR_BYTES,
    xaddr     : xaddr[CFG_SYSBUS_CFG_ADDR_BITS-1:0],
    xmask     : xmask[CFG_SYSBUS_CFG_ADDR_BITS-1:0],
    vid       : VENDOR_GNSSSENSOR,
    did       : GNSSSENSOR_SRAM
};

typedef struct {
  global_addr_array_type                raddr;
  logic                                 re;
  global_addr_array_type                waddr;
  logic                                 we;
  logic [CFG_SYSBUS_DATA_BYTES-1:0]     wstrb;
  logic [CFG_SYSBUS_DATA_BITS-1:0]      wdata;
} ram_in_type;

logic [CFG_SYSBUS_DATA_BITS-1:0]        rdata_mux;
ram_in_type                             rami;

assign cfg = xconfig;

axi_slv #(.async_reset(async_reset)) axi0 (
    .i_clk(clk),
    .i_nrst(nrst),
    .i_xcfg(xconfig), 
    .i_xslvi(i),
    .o_xslvo(o),
    .i_ready(1'b1),
    .i_rdata(rdata_mux),
    .o_re(rami.re),
    .o_r32(),
    .o_radr(rami.raddr),
    .o_wadr(rami.waddr),
    .o_we(rami.we),
    .o_wstrb(rami.wstrb),
    .o_wdata(rami.wdata)
);

srambytes_tech #(
    .abits(abits), 
    .init_file(init_file)
) tech0 (
    .clk(clk),
    .raddr(rami.raddr),
    .rdata(rdata_mux),
    .waddr(rami.waddr),
    .we(rami.we),
    .wstrb(rami.wstrb),
    .wdata(rami.wdata)
);

endmodule
