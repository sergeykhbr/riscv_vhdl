///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2004 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 10.1
//  \   \         Description : Xilinx Functional Simulation Library Component
//  /   /                  General Purpose Buffer
// /___/   /\     Filename : BUF.v
// \   \  /  \    Timestamp : Thu Mar 25 16:42:13 PST 2004
//  \___\/\___\
//
// Revision:
//    03/23/04 - Initial version.
//    05/23/07 - Changed timescale to 1 ps / 1 ps.
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
// End Revision

`timescale  1 ps / 1 ps


`celldefine

module BUF (O, I);


`ifdef XIL_TIMING

    parameter LOC = "UNPLACED";

`endif

    output O;
    input I;
    
    buf B1 (O, I);

`ifdef XIL_TIMING

    specify

	(I => O) = (0:0:0, 0:0:0);
	specparam PATHPULSE$ = 0;
	
    endspecify

`endif
    
endmodule

`endcelldefine

