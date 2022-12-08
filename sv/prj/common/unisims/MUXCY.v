///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  2 to 1 Multiplexer for Carry Logic
// /___/   /\     Filename : MUXCY.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    04/01/08 - Initial version.
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module MUXCY (O, CI, DI, S);

`ifdef XIL_TIMING //Simprim
 
  parameter LOC = "UNPLACED";

`endif

    output O;
    reg    o_out;

    input  CI, DI, S;

    buf B1 (O, o_out);

  always @(CI or DI or S) begin
      if (S)
    o_out <= CI;
      else
    o_out <= DI;
  end

endmodule

`endcelldefine
