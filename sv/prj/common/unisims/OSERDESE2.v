///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2010 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 13.1
//  \   \         Description : Xilinx Timing Simulation Library Component
//  /   /                  Source Synchronous Output Serializer Virtex7
// /___/   /\     Filename : OSERDESE2.v
// \   \  /  \    Timestamp : Fri Jan 29 14:59:32 PST 2010
//  \___\/\___\
//
// Revision:
//    01/29/10 - Initial version.
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
//    10/22/14 - Added #1 to $finish (CR 808642).
// End Revision

`timescale 1 ps / 1 ps 

`celldefine

module OSERDESE2 (
  OFB,
  OQ,
  SHIFTOUT1,
  SHIFTOUT2,
  TBYTEOUT,
  TFB,
  TQ,

  CLK,
  CLKDIV,
  D1,
  D2,
  D3,
  D4,
  D5,
  D6,
  D7,
  D8,
  OCE,
  RST,
  SHIFTIN1,
  SHIFTIN2,
  T1,
  T2,
  T3,
  T4,
  TBYTEIN,
  TCE
);

  parameter DATA_RATE_OQ = "DDR";
  parameter DATA_RATE_TQ = "DDR";
  parameter integer DATA_WIDTH = 4;
  parameter [0:0] INIT_OQ = 1'b0;
  parameter [0:0] INIT_TQ = 1'b0;
  parameter [0:0] IS_CLKDIV_INVERTED = 1'b0;
  parameter [0:0] IS_CLK_INVERTED = 1'b0;
  parameter [0:0] IS_D1_INVERTED = 1'b0;
  parameter [0:0] IS_D2_INVERTED = 1'b0;
  parameter [0:0] IS_D3_INVERTED = 1'b0;
  parameter [0:0] IS_D4_INVERTED = 1'b0;
  parameter [0:0] IS_D5_INVERTED = 1'b0;
  parameter [0:0] IS_D6_INVERTED = 1'b0;
  parameter [0:0] IS_D7_INVERTED = 1'b0;
  parameter [0:0] IS_D8_INVERTED = 1'b0;
  parameter [0:0] IS_T1_INVERTED = 1'b0;
  parameter [0:0] IS_T2_INVERTED = 1'b0;
  parameter [0:0] IS_T3_INVERTED = 1'b0;
  parameter [0:0] IS_T4_INVERTED = 1'b0;


  `ifdef XIL_TIMING //Simprim
  parameter LOC = "UNPLACED";
  `endif
  parameter SERDES_MODE = "MASTER";
  parameter [0:0] SRVAL_OQ = 1'b0;
  parameter [0:0] SRVAL_TQ = 1'b0;
  parameter TBYTE_CTL = "FALSE";
  parameter TBYTE_SRC = "FALSE";
  parameter integer TRISTATE_WIDTH = 4;
  
  localparam in_delay = 0;
  localparam out_delay = 0;
  localparam INCLK_DELAY = 0;
  localparam OUTCLK_DELAY = 0;

  output OFB;
  output OQ;
  output SHIFTOUT1;
  output SHIFTOUT2;
  output TBYTEOUT;
  output TFB;
  output TQ;

  input CLK;
  input CLKDIV;
  input D1;
  input D2;
  input D3;
  input D4;
  input D5;
  input D6;
  input D7;
  input D8;
  input OCE;
  input RST;
  input SHIFTIN1;
  input SHIFTIN2;
  input T1;
  input T2;
  input T3;
  input T4;
  input TBYTEIN;
  input TCE;

  reg DATA_RATE_OQ_BINARY;
  reg DATA_WIDTH_BINARY;
  reg LOC_BINARY;
  reg [0:0] INIT_OQ_BINARY;
  reg [0:0] INIT_TQ_BINARY;
  reg [0:0] SERDES_MODE_BINARY;
  reg [0:0] SRVAL_OQ_BINARY;
  reg [0:0] SRVAL_TQ_BINARY;
  reg [0:0] TBYTE_CTL_BINARY;
  reg [0:0] TBYTE_SRC_BINARY;
  reg [0:0] TRISTATE_WIDTH_BINARY;
  reg [5:0] DATA_RATE_TQ_BINARY;

  tri0 GSR = glbl.GSR;
  reg notifier;

  wire OFB_OUT;
  wire OQ_OUT;
  wire SHIFTOUT1_OUT;
  wire SHIFTOUT2_OUT;
  wire TBYTEOUT_OUT;
  wire TFB_OUT;
  wire TQ_OUT;

  wire CLKDIV_IN;
  wire CLK_IN;
  wire D1_IN;
  wire D2_IN;
  wire D3_IN;
  wire D4_IN;
  wire D5_IN;
  wire D6_IN;
  wire D7_IN;
  wire D8_IN;
  wire OCE_IN;
  wire RST_IN;
  wire SHIFTIN1_IN;
  wire SHIFTIN2_IN;
  wire T1_IN;
  wire T2_IN;
  wire T3_IN;
  wire T4_IN;
  wire TBYTEIN_IN;
  wire TCE_IN;

  wire CLKDIV_INDELAY;
  wire CLK_INDELAY;
  wire D1_INDELAY;
  wire D2_INDELAY;
  wire D3_INDELAY;
  wire D4_INDELAY;
  wire D5_INDELAY;
  wire D6_INDELAY;
  wire D7_INDELAY;
  wire D8_INDELAY;
  wire OCE_INDELAY;
  wire RST_INDELAY;
  wire SHIFTIN1_INDELAY;
  wire SHIFTIN2_INDELAY;
  wire T1_INDELAY;
  wire T2_INDELAY;
  wire T3_INDELAY;
  wire T4_INDELAY;
  wire TBYTEIN_INDELAY;
  wire TCE_INDELAY;

  wire delay_OFB,OFB_out;
  wire delay_OQ,OQ_out;
  wire delay_SHIFTOUT1,SHIFTOUT1_out;
  wire delay_SHIFTOUT2,SHIFTOUT2_out;
  wire delay_TBYTEOUT,TBYTEOUT_out;
  wire delay_TFB,TFB_out;
  wire delay_TQ,TQ_out;

  wire delay_CLK,CLK_in;
  wire delay_CLKDIV,CLKDIV_in;
  wire delay_D1,D1_in;
  wire delay_D2,D2_in;
  wire delay_D3,D3_in;
  wire delay_D4,D4_in;
  wire delay_D5,D5_in;
  wire delay_D6,D6_in;
  wire delay_D7,D7_in;
  wire delay_D8,D8_in;
  wire delay_OCE,OCE_in;
  wire delay_RST,RST_in;
  wire delay_SHIFTIN1,SHIFTIN1_in;
  wire delay_SHIFTIN2,SHIFTIN2_in;
  wire delay_T1,T1_in;
  wire delay_T2,T2_in;
  wire delay_T3,T3_in;
  wire delay_T4,T4_in;
  wire delay_TBYTEIN,TBYTEIN_in;
  wire delay_TCE,TCE_in;


  assign #(out_delay) OFB = delay_OFB;
  assign #(out_delay) OQ = delay_OQ;
  assign #(out_delay) SHIFTOUT1 = delay_SHIFTOUT1;
  assign #(out_delay) SHIFTOUT2 = delay_SHIFTOUT2;
  assign #(out_delay) TBYTEOUT = delay_TBYTEOUT;
  assign #(out_delay) TFB = delay_TFB;
  assign #(out_delay) TQ = delay_TQ;
  
  assign delay_OFB = OFB_out;
  assign delay_OQ = OQ_out;
  assign  delay_SHIFTOUT1 = SHIFTOUT1_out;
  assign  delay_SHIFTOUT2 = SHIFTOUT2_out;
  assign delay_TBYTEOUT = TBYTEOUT_out;
  assign  delay_TFB = TFB_out;
  assign  delay_TQ = TQ_out;

  
`ifndef XIL_TIMING // unisim
  assign #(INCLK_DELAY) delay_CLKDIV = CLKDIV;
  assign #(INCLK_DELAY) delay_CLK =  CLK;

  assign #(in_delay) delay_D1 =  D1;
  assign #(in_delay) delay_D2 =  D2;
  assign #(in_delay) delay_D3 =  D3;
  assign #(in_delay) delay_D4 =  D4;
  assign #(in_delay) delay_D5 =  D5;
  assign #(in_delay) delay_D6 =  D6;
  assign #(in_delay) delay_D7 =  D7;
  assign #(in_delay) delay_D8 =  D8;
  assign #(in_delay) delay_OCE = OCE;
  assign #(in_delay) delay_RST = RST;
  assign #(in_delay) delay_SHIFTIN1 = SHIFTIN1;
  assign #(in_delay) delay_SHIFTIN2 = SHIFTIN2;
  assign #(in_delay) delay_T1 =  T1;
  assign #(in_delay) delay_T2 =  T2;
  assign #(in_delay) delay_T3 =  T3;
  assign #(in_delay) delay_T4 =  T4;
  assign #(in_delay) delay_TBYTEIN = TBYTEIN;
  assign #(in_delay) delay_TCE = TCE;
`endif //  `ifndef XIL_TIMING

`ifdef XIL_TIMING //Simprim
//  assign delay_RST = RST;
  assign delay_SHIFTIN1 = SHIFTIN1;
  assign delay_SHIFTIN2 = SHIFTIN2;
  assign delay_TBYTEIN = TBYTEIN;
`endif
 assign CLKDIV_in = IS_CLKDIV_INVERTED ^ delay_CLKDIV;
  assign CLK_in = IS_CLK_INVERTED ^ delay_CLK;

  assign D1_in = IS_D1_INVERTED ^ delay_D1;
  assign D2_in = IS_D2_INVERTED ^ delay_D2;
  assign D3_in = IS_D3_INVERTED ^ delay_D3;
  assign D4_in = IS_D4_INVERTED ^ delay_D4;
  assign D5_in = IS_D5_INVERTED ^ delay_D5;
  assign D6_in = IS_D6_INVERTED ^ delay_D6;
  assign D7_in = IS_D7_INVERTED ^ delay_D7;
  assign D8_in = IS_D8_INVERTED ^ delay_D8;
  assign OCE_in = delay_OCE;
  assign RST_in = delay_RST;
  assign SHIFTIN1_in = delay_SHIFTIN1;
  assign SHIFTIN2_in = delay_SHIFTIN2;
  assign T1_in = IS_T1_INVERTED ^ delay_T1;
  assign T2_in = IS_T2_INVERTED ^ delay_T2;
  assign T3_in = IS_T3_INVERTED ^ delay_T3;
  assign T4_in = IS_T4_INVERTED ^ delay_T4;
  assign TBYTEIN_in = delay_TBYTEIN;
  assign TCE_in = delay_TCE;

  assign SHIFTIN1_in = delay_SHIFTIN1;
  assign SHIFTIN2_in = delay_SHIFTIN2;
  assign TBYTEIN_in = delay_TBYTEIN;



   initial begin
//-------------------------------------------------
//----- DATA_RATE_OQ check
//-------------------------------------------------
        case (DATA_RATE_OQ)
            "SDR", "DDR" :;
            default : begin
                          $display("Attribute Syntax Error : The attribute DATA_RATE_OQ on OSERDESE2 instance %m is set to %s.  Legal values for this attribute are SDR or DDR", DATA_RATE_OQ);
                          #1 $finish;
                      end
        endcase // case(DATA_RATE_OQ)

//-------------------------------------------------
//----- DATA_RATE_TQ check
//-------------------------------------------------
        case (DATA_RATE_TQ)
            "BUF", "DDR", "SDR" :;
            default : begin
                          $display("Attribute Syntax Error : The attribute DATA_RATE_TQ on OSERDESE2 instance %m is set to %s.  Legal values for this attribute are BUF, SDR, or DDR", DATA_RATE_TQ);
                          #1 $finish;
                      end
        endcase // case(DATA_RATE_TQ)

//-------------------------------------------------
//----- DATA_WIDTH check
//-------------------------------------------------
        case (DATA_WIDTH)

            2, 3, 4, 5, 6, 7, 8, 10, 14 : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute DATA_WIDTH on OSERDESE2 instance %m is set to %d.  Legal values for this attribute are 2, 3, 4, 5, 6, 7, 8, 10 or 14", DATA_WIDTH);
                          #1 $finish;
                      end
        endcase // case(DATA_WIDTH)

//-------------------------------------------------
//----- SERDES_MODE check
//-------------------------------------------------
        case (SERDES_MODE) // {mem_slave}

            "MASTER", "SLAVE" :;
            default  : begin
                          $display("Attribute Syntax Error : The attribute SERDES_MODE on OSERDESE2 instance %m is set to %s.  Legal values for this attribute are MASTER or SLAVE", SERDES_MODE);
                          #1 $finish;
                      end

        endcase // case(SERDES_MODE)

//-------------------------------------------------
//----- TRISTATE_WIDTH check
//-------------------------------------------------
        case (TRISTATE_WIDTH) // {mem_twidth4}

            1,2,4 : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute TRISTATE_WIDTH on OSERDESE2 instance %m is set to %d.  Legal values for this attribute are 1, 2 or 4", TRISTATE_WIDTH);
                          #1 $finish;
                      end

        endcase // case(TRISTATE_WIDTH)

//-------------------------------------------------
//----- DATA_RATE_OQ/DATA_WIDTH Combination
//-------------------------------------------------
        case (DATA_RATE_OQ) 
            "SDR" , "DDR" : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute DATA_RATE_OQ on OSERDESE2 instance %m is set to %s.  Legal values for this attribute are SDR or DDR", DATA_RATE_OQ);
                          #1 $finish;
                      end
        endcase // case(DATA_RATE_OQ/DATA_WIDTH)

//-------------------------------------------------
    end  // initial begin



  B_OSERDESE2 #(
    .DATA_RATE_OQ (DATA_RATE_OQ),
    .DATA_RATE_TQ (DATA_RATE_TQ),
    .DATA_WIDTH (DATA_WIDTH),
    .INIT_OQ (INIT_OQ),
    .INIT_TQ (INIT_TQ),
    .SERDES_MODE (SERDES_MODE),
    .SRVAL_OQ (SRVAL_OQ),
    .SRVAL_TQ (SRVAL_TQ),
    .TBYTE_CTL (TBYTE_CTL),
    .TBYTE_SRC (TBYTE_SRC),
    .TRISTATE_WIDTH (TRISTATE_WIDTH))

    B_OSERDESE2_INST (
    .OFB (OFB_out),
    .OQ (OQ_out),
    .SHIFTOUT1 (SHIFTOUT1_out),
    .SHIFTOUT2 (SHIFTOUT2_out),
    .TBYTEOUT (TBYTEOUT_out),
    .TFB (TFB_out),
    .TQ (TQ_out),
    .CLK (CLK_in),
    .CLKDIV (CLKDIV_in),
    .D1 (D1_in),
    .D2 (D2_in),
    .D3 (D3_in),
    .D4 (D4_in),
    .D5 (D5_in),
    .D6 (D6_in),
    .D7 (D7_in),
    .D8 (D8_in),
    .OCE (OCE_in),
    .RST (RST_in),
    .SHIFTIN1 (SHIFTIN1_in),
    .SHIFTIN2 (SHIFTIN2_in),
    .T1 (T1_in),
    .T2 (T2_in),
    .T3 (T3_in),
    .T4 (T4_in),
    .TBYTEIN (TBYTEIN_in),
    .TCE (TCE_in),
    .GSR (GSR)
  );

`ifdef XIL_TIMING
   wire clk_en_n;
   wire clk_en_p;
   wire clkdiv_en_p;
   wire clkdiv_en_n;
   assign clk_en_n = IS_CLK_INVERTED;
   assign clk_en_p = ~IS_CLK_INVERTED;
   assign clkdiv_en_n = IS_CLKDIV_INVERTED;
   assign clkdiv_en_p = ~IS_CLKDIV_INVERTED;

`endif

  specify
`ifdef XIL_TIMING // Simprim
    $period (negedge CLK, 0:0:0, notifier);
    $period (negedge CLKDIV, 0:0:0, notifier);
    $period (posedge CLK, 0:0:0, notifier);
    $period (posedge CLKDIV, 0:0:0, notifier);
    $setuphold (posedge CLK, negedge OCE, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_OCE);
    $setuphold (posedge CLK, negedge T1, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_T1);
    $setuphold (posedge CLK, negedge TCE, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_TCE);
    $setuphold (posedge CLK, posedge OCE, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_OCE);
    $setuphold (posedge CLK, posedge T1, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_T1);
    $setuphold (posedge CLK, posedge TCE, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_TCE);
    $setuphold (posedge CLKDIV, negedge D1, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D1);
    $setuphold (posedge CLKDIV, negedge D2, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D2);
    $setuphold (posedge CLKDIV, negedge D3, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D3);
    $setuphold (posedge CLKDIV, negedge D4, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D4);
    $setuphold (posedge CLKDIV, negedge D5, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D5);
    $setuphold (posedge CLKDIV, negedge D6, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D6);
    $setuphold (posedge CLKDIV, negedge D7, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D7);
    $setuphold (posedge CLKDIV, negedge D8, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D8);
    $setuphold (posedge CLKDIV, negedge T1, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T1);
    $setuphold (posedge CLKDIV, negedge T2, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T2);
    $setuphold (posedge CLKDIV, negedge T3, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T3);
    $setuphold (posedge CLKDIV, negedge T4, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T4);
    $setuphold (posedge CLKDIV, posedge D1, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D1);
    $setuphold (posedge CLKDIV, posedge D2, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D2);
    $setuphold (posedge CLKDIV, posedge D3, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D3);
    $setuphold (posedge CLKDIV, posedge D4, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D4);
    $setuphold (posedge CLKDIV, posedge D5, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D5);
    $setuphold (posedge CLKDIV, posedge D6, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D6);
    $setuphold (posedge CLKDIV, posedge D7, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D7);
    $setuphold (posedge CLKDIV, posedge D8, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_D8);
    $setuphold (posedge CLKDIV, posedge T1, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T1);
    $setuphold (posedge CLKDIV, posedge T2, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T2);
    $setuphold (posedge CLKDIV, posedge T3, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T3);
    $setuphold (posedge CLKDIV, posedge T4, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_T4);

    $setuphold (posedge CLKDIV, negedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_RST);
    $setuphold (posedge CLKDIV, posedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_RST);

    $setuphold (negedge CLK, negedge OCE, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_OCE);
    $setuphold (negedge CLK, negedge T1, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_T1);
    $setuphold (negedge CLK, negedge TCE, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_TCE);
    $setuphold (negedge CLK, posedge OCE, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_OCE);
    $setuphold (negedge CLK, posedge T1, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_T1);
    $setuphold (negedge CLK, posedge TCE, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_TCE);
    $setuphold (negedge CLKDIV, negedge D1, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D1);
    $setuphold (negedge CLKDIV, negedge D2, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D2);
    $setuphold (negedge CLKDIV, negedge D3, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D3);
    $setuphold (negedge CLKDIV, negedge D4, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D4);
    $setuphold (negedge CLKDIV, negedge D5, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D5);
    $setuphold (negedge CLKDIV, negedge D6, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D6);
    $setuphold (negedge CLKDIV, negedge D7, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D7);
    $setuphold (negedge CLKDIV, negedge D8, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D8);
    $setuphold (negedge CLKDIV, negedge T1, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T1);
    $setuphold (negedge CLKDIV, negedge T2, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T2);
    $setuphold (negedge CLKDIV, negedge T3, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T3);
    $setuphold (negedge CLKDIV, negedge T4, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T4);
    $setuphold (negedge CLKDIV, posedge D1, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D1);
    $setuphold (negedge CLKDIV, posedge D2, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D2);
    $setuphold (negedge CLKDIV, posedge D3, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D3);
    $setuphold (negedge CLKDIV, posedge D4, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D4);
    $setuphold (negedge CLKDIV, posedge D5, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D5);
    $setuphold (negedge CLKDIV, posedge D6, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D6);
    $setuphold (negedge CLKDIV, posedge D7, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D7);
    $setuphold (negedge CLKDIV, posedge D8, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_D8);
    $setuphold (negedge CLKDIV, posedge T1, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T1);
    $setuphold (negedge CLKDIV, posedge T2, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T2);
    $setuphold (negedge CLKDIV, posedge T3, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T3);
    $setuphold (negedge CLKDIV, posedge T4, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_T4);

    $setuphold (negedge CLKDIV, negedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_RST);
    $setuphold (negedge CLKDIV, posedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_RST);

    
`endif
    ( CLK => OFB) = (100:100:100, 100:100:100);
    ( CLK => OQ) = (100:100:100, 100:100:100);
    ( CLK => TFB) = (100:100:100, 100:100:100);
    ( CLK => TQ) = (100:100:100, 100:100:100);
    ( T1 => TBYTEOUT) = (100:100:100, 100:100:100);
    ( T1 => TQ) = (100:100:100, 100:100:100);
    ( TBYTEIN => TQ) = (100:100:100, 100:100:100);

    specparam PATHPULSE$ = 0;
  endspecify
endmodule

`endcelldefine
