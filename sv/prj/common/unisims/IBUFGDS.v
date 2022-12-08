///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  Input Buffer
// /___/   /\     Filename : IBUFGDS.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    04/01/08 - Initial version.
//    02/05/14 - Removed X's assignment.
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module IBUFGDS (O, I, IB);

  parameter CAPACITANCE = "DONT_CARE";
  parameter DIFF_TERM = "FALSE";
  parameter IBUF_DELAY_VALUE = "0";
  parameter IBUF_LOW_PWR = "TRUE";
  parameter IOSTANDARD = "DEFAULT";

`ifdef XIL_TIMING //Simprim
 
  parameter LOC = "UNPLACED";

`endif

  output O;

  input  I, IB;

  reg o_out;

  buf b_0 (O, o_out);

  always @(I or IB) begin
    if (I == 1'b1 && IB == 1'b0)
      o_out <= I;
    else if (I == 1'b0 && IB == 1'b1)
      o_out <= I;
    end

endmodule

`endcelldefine
