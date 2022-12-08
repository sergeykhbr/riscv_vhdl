///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                       Latch used as 2-input OR Gate
// /___/   /\     Filename : OR2L.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    02/26/08 - Initial version.
//    04/01/08 - Add GSR
//    12/13/11 - 524859 - Added `celldefine and `endcelldefine
//    08/30/13 - 683925 - add invertible pin support XEC.
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module OR2L #(
  `ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
  `endif
  parameter [0:0] IS_SRI_INVERTED = 1'b0
)(
  output O,
  
  input DI,
  input SRI
);
  
    wire SRI_in;

    assign SRI_in = IS_SRI_INVERTED ^ SRI;

    assign O = DI || SRI_in;

endmodule
`endcelldefine
