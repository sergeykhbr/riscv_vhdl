///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                 System Monitor 
// /___/   /\     Filename  : XADC.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    12/09/09 - Initial version.
//    08/16/13 - Added invertible pins support (CR 715417).
//    10/31/14 - Added inverter functionality for IS_*_INVERTED parameter (CR 828995).
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale 1ps / 1ps

`celldefine

module XADC (
        ALM,
        BUSY,
        CHANNEL,
        DO,
        DRDY,
        EOC,
        EOS,
        JTAGBUSY,
        JTAGLOCKED,
        JTAGMODIFIED,
        MUXADDR,
        OT,
        CONVST,
        CONVSTCLK,
        DADDR,
        DCLK,
        DEN,
        DI,
        DWE,
        RESET,
        VAUXN,
        VAUXP,
        VN,
        VP

);

output BUSY;
output DRDY;
output EOC;
output EOS;
output JTAGBUSY;
output JTAGLOCKED;
output JTAGMODIFIED;
output OT;
output [15:0] DO;
output [7:0] ALM;
output [4:0] CHANNEL;
output [4:0] MUXADDR;

input CONVST;
input CONVSTCLK;
input DCLK;
input DEN;
input DWE;
input RESET;
input VN;
input VP;
input [15:0] DI;
input [15:0] VAUXN;
input [15:0] VAUXP;
input [6:0] DADDR;

    parameter  [15:0] INIT_40 = 16'h0;
    parameter  [15:0] INIT_41 = 16'h0;
    parameter  [15:0] INIT_42 = 16'h0800;
    parameter  [15:0] INIT_43 = 16'h0;
    parameter  [15:0] INIT_44 = 16'h0;
    parameter  [15:0] INIT_45 = 16'h0;
    parameter  [15:0] INIT_46 = 16'h0;
    parameter  [15:0] INIT_47 = 16'h0;
    parameter  [15:0] INIT_48 = 16'h0;
    parameter  [15:0] INIT_49 = 16'h0;
    parameter  [15:0] INIT_4A = 16'h0;
    parameter  [15:0] INIT_4B = 16'h0;
    parameter  [15:0] INIT_4C = 16'h0;
    parameter  [15:0] INIT_4D = 16'h0;
    parameter  [15:0] INIT_4E = 16'h0;
    parameter  [15:0] INIT_4F = 16'h0;
    parameter  [15:0] INIT_50 = 16'h0;
    parameter  [15:0] INIT_51 = 16'h0;
    parameter  [15:0] INIT_52 = 16'h0;
    parameter  [15:0] INIT_53 = 16'h0;
    parameter  [15:0] INIT_54 = 16'h0;
    parameter  [15:0] INIT_55 = 16'h0;
    parameter  [15:0] INIT_56 = 16'h0;
    parameter  [15:0] INIT_57 = 16'h0;
    parameter  [15:0] INIT_58 = 16'h0;
    parameter  [15:0] INIT_59 = 16'h0;
    parameter  [15:0] INIT_5A = 16'h0;
    parameter  [15:0] INIT_5B = 16'h0;
    parameter  [15:0] INIT_5C = 16'h0;
    parameter  [15:0] INIT_5D = 16'h0;
    parameter  [15:0] INIT_5E = 16'h0;
    parameter  [15:0] INIT_5F = 16'h0;
    parameter IS_CONVSTCLK_INVERTED = 1'b0;
    parameter IS_DCLK_INVERTED = 1'b0;
    parameter SIM_DEVICE = "7SERIES";
    parameter SIM_MONITOR_FILE = "design.txt";
    
`ifdef XIL_TIMING

  parameter LOC = "UNPLACED";

`endif
   
    wire CONVSTCLK_in;
    wire DCLK_in;
   
    assign CONVSTCLK_in = CONVSTCLK ^ IS_CONVSTCLK_INVERTED;
    assign DCLK_in = DCLK ^ IS_DCLK_INVERTED;
   
     XADC_bb inst_bb (
            .ALM(ALM),
            .BUSY(BUSY),
            .CHANNEL(CHANNEL),
            .DO(DO),
            .DRDY(DRDY),
            .EOC(EOC),
            .EOS(EOS),
            .JTAGBUSY(JTAGBUSY),
            .JTAGLOCKED(JTAGLOCKED),
            .JTAGMODIFIED(JTAGMODIFIED),
            .MUXADDR(MUXADDR),
            .OT(OT),
            .CONVST(CONVST),
            .CONVSTCLK(CONVSTCLK_in),
            .DADDR(DADDR),
            .DCLK(DCLK_in),
            .DEN(DEN),
            .DI(DI),
            .DWE(DWE),
            .RESET(RESET),
            .VAUXN(VAUXN),
            .VAUXP(VAUXP),
            .VN(VN),
            .VP(VP)
            );

endmodule

module XADC_bb (
  output BUSY,
  output DRDY,
  output EOC,
  output EOS,
  output JTAGBUSY,
  output JTAGLOCKED,
  output JTAGMODIFIED,
  output OT,
  output [15:0] DO,
  output [7:0] ALM,
  output [4:0] CHANNEL,
  output [4:0] MUXADDR,

  input CONVST,
  input CONVSTCLK,
  input DCLK,
  input DEN,
  input DWE,
  input RESET,
  input VN,
  input VP,
  input [15:0] DI,
  input [15:0] VAUXN,
  input [15:0] VAUXP,
  input [6:0] DADDR
);

endmodule

`endcelldefine 
