///////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 1995/2018 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2018.1
//  \   \         Description : Xilinx Unified Simulation Library Component
//  /   /                        Two input AND gate implemented in place of a CLB Latch
// /___/   /\     Filename : AND2B1L.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
//  Revision:
//    04/01/08 - Initial version.
//    04/14/09 - 517897 - Invert SRI not DI
//    12/13/11 - 524859 - Added `celldefine and `endcelldefine
//    04/16/13 - 683925 - add invertible pin support.
//  End Revision:
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module AND2B1L #(
  `ifdef XIL_TIMING
  parameter LOC = "UNPLACED",
  `endif
  parameter [0:0] IS_SRI_INVERTED = 1'b0
)(
  output O,
  
  input DI,
  input SRI
);
  
// define constants
  localparam MODULE_NAME = "AND2B1L";
  
  reg trig_attr;
// include dynamic registers - XILINX test only
`ifdef XIL_DR
  `include "AND2B1L_dr.v"
`else
  reg [0:0] IS_SRI_INVERTED_REG = IS_SRI_INVERTED;
`endif

`ifdef XIL_XECLIB
  tri0 glblGSR = 1'b0;
`else
  tri0 glblGSR = glbl.GSR;
`endif

  wire DI_in;
  wire SRI_in;

  assign DI_in = DI;
  assign SRI_in = SRI ^ IS_SRI_INVERTED_REG;

`ifdef XIL_TIMING
  reg notifier;
`endif

// begin behavioral model

    assign O = ~glblGSR && DI_in && ~SRI_in;

// end behavioral model

`ifndef XIL_XECLIB
`ifdef XIL_TIMING
  specify
  (DI => O) = (0:0:0, 0:0:0);
  (SRI => O) = (0:0:0, 0:0:0);
    $width (negedge SRI, 0:0:0, 0, notifier);
    $width (posedge SRI, 0:0:0, 0, notifier);
    specparam PATHPULSE$ = 0;
  endspecify
`endif
`endif
endmodule

`endcelldefine
