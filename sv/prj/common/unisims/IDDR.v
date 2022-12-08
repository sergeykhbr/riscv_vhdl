///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  Dual Data Rate Input D Flip-Flop
// /___/   /\     Filename : IDDR.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    03/23/04 - Initial version.
//    10/31/14 - Added inverter functionality for IS_*_INVERTED parameter (CR 828995).
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module IDDR (Q1, Q2, C, CE, D, R, S);

    parameter DDR_CLK_EDGE = "OPPOSITE_EDGE";    
    parameter INIT_Q1 = 1'b0;
    parameter INIT_Q2 = 1'b0;
    parameter [0:0] IS_C_INVERTED = 1'b0;
    parameter [0:0] IS_D_INVERTED = 1'b0;
    parameter SRTYPE = "SYNC";

`ifdef XIL_TIMING //Simprim
 
  parameter LOC = "UNPLACED";
  parameter MSGON = "TRUE";
  parameter XON = "TRUE";

`endif
    
    output Q1;
    output Q2;
    
    input C;
    input CE;
    input D;
    input R;
    input S;

   wire C_in;
   wire D_in;
   
   assign C_in = C ^ IS_C_INVERTED;
   assign D_in = D ^ IS_D_INVERTED;
   
   IDDR_bb inst_bb (
          .Q1(Q1),
          .Q2(Q2),
          .C(C_in),
          .CE(CE),
          .D(D_in),
          .R(R),
          .S(S)
          );

endmodule

module IDDR_bb (
    output Q1,
    output Q2,
    
    input C,
    input CE,
    input D,
    input R,
    input S
          );

endmodule

`endcelldefine
