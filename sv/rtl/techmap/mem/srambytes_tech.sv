//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Internal SRAM implementation with the byte access.
//----------------------------------------------------------------------------

module srambytes_tech #(
    parameter integer abits = 16,
    parameter integer log2_dbytes = 3,  // 2^log2_dbytes = number of bytes on data bus
    parameter init_file = ""
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

//! Instantiate component for RTL simulation

`ifdef TARGET_INFERRED

generate
  
   for (genvar n = 0; n <= dbytes-1; n++) begin : rx
      assign wr_ena[n] = we & wstrb[n];
                  
      sram8_inferred_init #(
          .abits(abits-log2_dbytes),
          .byte_idx(n),
          .init_file(init_file)
      ) x0 (
          .clk(clk), 
          .address(addr[abits-1:log2_dbytes]),
          .rdata(rdata[8*n+:8]),
          .we(wr_ena[n]),
          .wdata(wdata[8*n+:8])
      );
   end : rx

endgenerate

`elsif TARGET_KC705

   for (genvar n = 0; n <= dbytes-1; n++) begin : rx
      assign wr_ena[n] = we & wstrb[n];
                  
      sram8_inferred_init #(
          .abits(abits-log2_dbytes),
          .byte_idx(n),
          .init_file(init_file)
      ) x0 (
          .clk(clk), 
          .address(addr[abits-1:log2_dbytes]),
          .rdata(rdata[8*n+:8]),
          .we(wr_ena[n]),
          .wdata(wdata[8*n+:8])
      );
   end : rx

`else

    initial $error("srambytes_tech target is not defined, check technology-dependent memories.");

`endif

endmodule

