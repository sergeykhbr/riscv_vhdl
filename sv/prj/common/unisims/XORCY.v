///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2004 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 10.1
//  \   \         Description : Xilinx Functional Simulation Library Component
//  /   /                  XOR for Carry Logic with General Output
// /___/   /\     Filename : XORCY.v
// \   \  /  \    Timestamp : Thu Mar 25 16:43:42 PST 2004
//  \___\/\___\
//
// Revision:
//    03/23/04 - Initial version.
//    05/23/07 - Changed timescale to 1 ps / 1 ps.

`timescale  1 ps / 1 ps


`celldefine

module XORCY (O, CI, LI);


`ifdef XIL_TIMING

    parameter LOC = "UNPLACED";

`endif

    
    output O;

    input  CI, LI;

	xor X1 (O, CI, LI);

`ifdef XIL_TIMING

    specify
        
        (CI => O) = (0:0:0, 0:0:0);
        (LI => O) = (0:0:0, 0:0:0);
        specparam PATHPULSE$ = 0;
        
    endspecify

`endif
    
endmodule

`endcelldefine

