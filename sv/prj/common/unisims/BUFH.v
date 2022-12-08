///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2004 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 10.1
//  \   \         Description : Xilinx Functional Simulation Library Component
//  /   /                  H Clock Buffer
// /___/   /\     Filename : BUFH.v
// \   \  /  \    Timestamp :
//  \___\/\___\
//
// Revision:
//    04/08/08 - Initial version.
//    09//9/08 - Change to use BUFHCE according to yaml.
//    11/11/08 - Change to not use BUFHCE.
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
// End Revision

`timescale  1 ps / 1 ps


`celldefine

module BUFH (O, I);


`ifdef XIL_TIMING

    parameter LOC = " UNPLACED";
    reg notifier;

`endif


    output O;
    input  I;

    buf B1 (O, I);

`ifdef XIL_TIMING
    
    specify
        (I => O) = (0:0:0, 0:0:0);
	$period (posedge I, 0:0:0, notifier);
        specparam PATHPULSE$ = 0;
    endspecify
    
`endif
    
endmodule

`endcelldefine

