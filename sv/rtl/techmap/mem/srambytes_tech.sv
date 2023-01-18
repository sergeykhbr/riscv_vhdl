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

generate if (init_file == "") begin : i1
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
  end : i1
endgenerate

generate if (init_file != "") begin : i2
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
  end : i2
endgenerate

endmodule

