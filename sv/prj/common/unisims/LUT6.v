///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  6 Inputs LUT
// /___/   /\     Filename : LUT6.v
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

module LUT6 (O, I0, I1, I2, I3, I4, I5);

    parameter [63:0] INIT = 64'h0000000000000000;

`ifdef XIL_TIMING //Simprim
 
  parameter LOC = "UNPLACED";

`endif

    input I0, I1, I2, I3, I4, I5;

    output O;


        assign O = INIT[{I5, I4, I3, I2, I1, I0}];


endmodule

`endcelldefine
