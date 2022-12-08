///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  32-Bit Shift Register Look-Up-Table with Carry and Clock Enable
// /___/   /\     Filename : SRLC32E.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    04/01/08 - Initial version.
//    08/30/13 - PR683925 - add invertible pin support.
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module SRLC32E (Q, Q31, A, CE, CLK, D);

    parameter [31:0] INIT = 32'h00000000;
    parameter [0:0] IS_CLK_INVERTED = 1'b0;

`ifdef XIL_TIMING //Simprim
 
  parameter LOC = "UNPLACED";

`endif

    output Q;
    output Q31;

    input  [4:0] A;
    input  CE, CLK, D;

    reg  [31:0] data;
    reg  [6:0]  count;
    wire [4:0]  addr;
    wire CLK_in;
    
    buf b_a4 (addr[4], A[4]);
    buf b_a3 (addr[3], A[3]);
    buf b_a2 (addr[2], A[2]);
    buf b_a1 (addr[1], A[1]);
    buf b_a0 (addr[0], A[0]);

    assign  Q = data[addr];
    assign  Q31 = data[31];
    assign CLK_in = IS_CLK_INVERTED ^ CLK;

    // synopsys translate_off
    initial begin
  for (count = 0; count < 32; count = count + 1)
      data[count] <= INIT[count];
    end
   // synopsys translate_on

    always @(posedge CLK_in)
  if (CE == 1'b1) 
      data <=  {data[30:0], D};


endmodule

`endcelldefine
