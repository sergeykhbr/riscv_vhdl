///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                 32-Deep by 8-bit Wide Multi Port RAM 
// /___/   /\     Filename : RAM32M.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    03/21/06 - Initial version.
//    08/30/13 - PR683925 - add invertible pin support.
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale 1 ps/1 ps

`celldefine

module RAM32M (DOA, DOB, DOC, DOD, ADDRA, ADDRB, ADDRC, ADDRD, DIA, DIB, DIC, DID, WCLK, WE);

  parameter  [63:0] INIT_A = 64'h0000000000000000;
  parameter  [63:0] INIT_B = 64'h0000000000000000;
  parameter  [63:0] INIT_C = 64'h0000000000000000;
  parameter  [63:0] INIT_D = 64'h0000000000000000;
  parameter [0:0] IS_WCLK_INVERTED = 1'b0;

`ifdef XIL_TIMING //Simprim
 
  parameter LOC = "UNPLACED";

`endif

  output [1:0] DOA;
  output [1:0] DOB;
  output [1:0] DOC;
  output [1:0] DOD;
  input [4:0] ADDRA;
  input [4:0] ADDRB;
  input [4:0] ADDRC;
  input [4:0] ADDRD;
  input [1:0] DIA;
  input [1:0] DIB;
  input [1:0] DIC;
  input [1:0] DID;
  input WCLK;
  input WE;

  reg [63:0] mem_a, mem_b, mem_c, mem_d;
  reg notifier;
  wire WCLK_in;

  assign WCLK_in = IS_WCLK_INVERTED ^ WCLK;

  initial begin
    mem_a = INIT_A;
    mem_b = INIT_B;
    mem_c = INIT_C;
    mem_d = INIT_D;
  end

  always @(posedge WCLK_in)
    if (WE) begin
      mem_a[2*ADDRD] <= #100 DIA[0];
      mem_a[2*ADDRD + 1] <= #100 DIA[1];
      mem_b[2*ADDRD] <= #100 DIB[0];
      mem_b[2*ADDRD + 1] <= #100 DIB[1];
      mem_c[2*ADDRD] <= #100 DIC[0];
      mem_c[2*ADDRD + 1] <= #100 DIC[1];
      mem_d[2*ADDRD] <= #100 DID[0];
      mem_d[2*ADDRD + 1] <= #100 DID[1];
  end

   assign  DOA[0] = mem_a[2*ADDRA];
   assign  DOA[1] = mem_a[2*ADDRA + 1];
   assign  DOB[0] = mem_b[2*ADDRB];
   assign  DOB[1] = mem_b[2*ADDRB + 1];
   assign  DOC[0] = mem_c[2*ADDRC];
   assign  DOC[1] = mem_c[2*ADDRC + 1];
   assign  DOD[0] = mem_d[2*ADDRD];
   assign  DOD[1] = mem_d[2*ADDRD + 1];

//   always @(notifier) begin
//       mem_a[2*ADDRD] <= 1'bx;
//       mem_a[2*ADDRD + 1] <= 1'bx;
//       mem_b[2*ADDRD] <= 1'bx;
//       mem_b[2*ADDRD + 1] <= 1'bx;
//       mem_c[2*ADDRD] <= 1'bx;
//       mem_c[2*ADDRD + 1] <= 1'bx;
//       mem_d[2*ADDRD] <= 1'bx;
//       mem_d[2*ADDRD + 1] <= 1'bx;
//   end

endmodule

`endcelldefine
