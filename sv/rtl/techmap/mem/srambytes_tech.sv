//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Internal SRAM implementation with the byte access.
//----------------------------------------------------------------------------

module srambytes_tech #(
    parameter integer abits = 16,
    parameter integer log2_dbytes = 3  // 2^log2_dbytes = number of bytes on data bus
)
(
    input clk,
    input logic [abits-1 : 0] addr,
    output logic [8*(2**log2_dbytes)-1 : 0] rdata,
    input we,
    input [(2**log2_dbytes)-1 : 0] wstrb,
    input [8*(2**log2_dbytes)-1 : 0] wdata
);

import config_target_pkg::*;

//! reduced name of configuration constant:

localparam integer dbytes = (2**log2_dbytes);
localparam integer dbits = 8*dbytes;

logic [dbytes-1 : 0] wr_ena;


generate
  
   for (genvar n = 0; n <= dbytes-1; n++) begin : rx
      assign wr_ena[n] = we & wstrb[n];
                  
      ram_tech #(
          .abits(abits-log2_dbytes),
          .dbits(8)
      ) x0 (
          .i_clk(clk), 
          .i_addr(addr[abits-1:log2_dbytes]),
          .o_rdata(rdata[8*n+:8]),
          .i_wena(wr_ena[n]),
          .i_wdata(wdata[8*n+:8])
      );
   end : rx

endgenerate

endmodule

