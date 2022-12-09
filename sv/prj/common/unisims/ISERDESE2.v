///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2010 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 13.1
//  \   \         Description : Xilinx Timing Simulation Library Component
//  /   /                  Source Synchronous Input Deserializer for Virtex7
// /___/   /\     Filename : ISERDESE2.v
// \   \  /  \    Timestamp : Tue Jan 19 16:29:39 PST 2010
//  \___\/\___\
//
// Revision:
//    01/19/10 - Initial version.
//    03/24/11 - Sync-up
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
//    10/22/14 - Added #1 to $finish (CR 808642).
// End Revision

`timescale 1 ps / 1 ps 

`celldefine

module ISERDESE2 (
  O,
  Q1,
  Q2,
  Q3,
  Q4,
  Q5,
  Q6,
  Q7,
  Q8,
  SHIFTOUT1,
  SHIFTOUT2,

  BITSLIP,
  CE1,
  CE2,
  CLK,
  CLKB,
  CLKDIV,
  CLKDIVP,
  D,
  DDLY,
  DYNCLKDIVSEL,
  DYNCLKSEL,
  OCLK,
  OCLKB,
  OFB,
  RST,
  SHIFTIN1,
  SHIFTIN2
);

  parameter DATA_RATE = "DDR";
  parameter integer DATA_WIDTH = 4;
  parameter DYN_CLKDIV_INV_EN = "FALSE";
  parameter DYN_CLK_INV_EN = "FALSE";
  parameter [0:0] INIT_Q1 = 1'b0;
  parameter [0:0] INIT_Q2 = 1'b0;
  parameter [0:0] INIT_Q3 = 1'b0;
  parameter [0:0] INIT_Q4 = 1'b0;
  parameter INTERFACE_TYPE = "MEMORY";
  parameter IOBDELAY = "NONE";
  parameter [0:0] IS_CLKB_INVERTED = 1'b0;
  parameter [0:0] IS_CLKDIVP_INVERTED = 1'b0;
  parameter [0:0] IS_CLKDIV_INVERTED = 1'b0;
  parameter [0:0] IS_CLK_INVERTED = 1'b0;
  parameter [0:0] IS_D_INVERTED = 1'b0;
  parameter [0:0] IS_OCLKB_INVERTED = 1'b0;
  parameter [0:0] IS_OCLK_INVERTED = 1'b0;

  `ifdef XIL_TIMING //Simprim
  parameter LOC = "UNPLACED";
  `endif
  parameter integer NUM_CE = 2;
  parameter OFB_USED = "FALSE";
  parameter SERDES_MODE = "MASTER";
  parameter [0:0] SRVAL_Q1 = 1'b0;
  parameter [0:0] SRVAL_Q2 = 1'b0;
  parameter [0:0] SRVAL_Q3 = 1'b0;
  parameter [0:0] SRVAL_Q4 = 1'b0;
  
  localparam in_delay = 0;
  localparam out_delay = 0;
  localparam INCLK_DELAY = 0;
  localparam OUTCLK_DELAY = 0;

  output O;
  output Q1;
  output Q2;
  output Q3;
  output Q4;
  output Q5;
  output Q6;
  output Q7;
  output Q8;
  output SHIFTOUT1;
  output SHIFTOUT2;

  input BITSLIP;
  input CE1;
  input CE2;
  input CLK;
  input CLKB;
  input CLKDIV;
  input CLKDIVP;
  input D;
  input DDLY;
  input DYNCLKDIVSEL;
  input DYNCLKSEL;
  input OCLK;
  input OCLKB;
  input OFB;
  input RST;
  input SHIFTIN1;
  input SHIFTIN2;

  tri0  GSR = glbl.GSR;

  reg INTERFACE_TYPE_BINARY;
  reg IOBDELAY_BINARY;
  reg [0:0] DATA_RATE_BINARY;
  reg [0:0] DYN_CLKDIV_INV_EN_BINARY;
  reg [0:0] DYN_CLK_INV_EN_BINARY;
  reg [0:0] INIT_Q1_BINARY;
  reg [0:0] INIT_Q2_BINARY;
  reg [0:0] INIT_Q3_BINARY;
  reg [0:0] INIT_Q4_BINARY;
  reg [0:0] NUM_CE_BINARY;
  reg [0:0] SERDES_MODE_BINARY;
  reg [1:0] OFB_USED_BINARY;
  reg [3:0] DATA_WIDTH_BINARY;

  reg data_in = 0;
  reg o_out_pre_fb = 0, o_delay_pre_fb = 0;
  reg o_out = 0;

  reg notifier;

  wire O_OUT;
  wire Q1_OUT;
  wire Q2_OUT;
  wire Q3_OUT;
  wire Q4_OUT;
  wire Q5_OUT;
  wire Q6_OUT;
  wire Q7_OUT;
  wire Q8_OUT;
  wire SHIFTOUT1_OUT;
  wire SHIFTOUT2_OUT;

  wire BITSLIP_IN;
  wire CE1_IN;
  wire CE2_IN;
  wire CLKB_IN;
  wire CLKDIVP_IN;
  wire CLKDIV_IN;
  wire CLK_IN;
  wire DDLY_IN;
  wire DYNCLKDIVSEL_IN;
  wire DYNCLKSEL_IN;
  wire D_IN;
  wire OCLKB_IN;
  wire OCLK_IN;
  wire OFB_IN;
  wire RST_IN;
  wire SHIFTIN1_IN;
  wire SHIFTIN2_IN;

  wire BITSLIP_INDELAY;
  wire CE1_INDELAY;
  wire CE2_INDELAY;
  wire CLKB_INDELAY;
  wire CLKDIVP_INDELAY;
  wire CLKDIV_INDELAY;
  wire CLK_INDELAY;
  wire DDLY_INDELAY;
  wire DYNCLKDIVSEL_INDELAY;
  wire DYNCLKSEL_INDELAY;
  wire D_INDELAY;
  wire OCLKB_INDELAY;
  wire OCLK_INDELAY;
  wire OFB_INDELAY;
  wire RST_INDELAY;
  wire SHIFTIN1_INDELAY;
  wire SHIFTIN2_INDELAY;

//---------------------------------------
  buf B_O (O, O_OUT);
  buf B_Q1 (Q1, Q1_OUT);
  buf B_Q2 (Q2, Q2_OUT);
  buf B_Q3 (Q3, Q3_OUT);
  buf B_Q4 (Q4, Q4_OUT);
  buf B_Q5 (Q5, Q5_OUT);
  buf B_Q6 (Q6, Q6_OUT);
  buf B_Q7 (Q7, Q7_OUT);
  buf B_Q8 (Q8, Q8_OUT);
  buf B_SHIFTOUT1 (SHIFTOUT1, SHIFTOUT1_OUT);
  buf B_SHIFTOUT2 (SHIFTOUT2, SHIFTOUT2_OUT);

  buf B_BITSLIP (BITSLIP_IN, BITSLIP);
  buf B_CE1 (CE1_IN, CE1);
  buf B_CE2 (CE2_IN, CE2);
  buf B_CLK (CLK_IN, CLK);
  buf B_CLKB (CLKB_IN, CLKB);
  buf B_CLKDIV (CLKDIV_IN, CLKDIV);
  buf B_CLKDIVP (CLKDIVP_IN, CLKDIVP);
  buf B_D (D_IN, D);
  buf B_DDLY (DDLY_IN, DDLY);
  buf B_DYNCLKDIVSEL (DYNCLKDIVSEL_IN, DYNCLKDIVSEL);
  buf B_DYNCLKSEL (DYNCLKSEL_IN, DYNCLKSEL);
  buf B_OCLK (OCLK_IN, OCLK);
  buf B_OCLKB (OCLKB_IN, OCLKB);
  buf B_OFB (OFB_IN, OFB);
  buf B_RST (RST_IN, RST);
  buf B_SHIFTIN1 (SHIFTIN1_IN, SHIFTIN1);
  buf B_SHIFTIN2 (SHIFTIN2_IN, SHIFTIN2);

  wire delay_O;
  wire delay_Q1;
  wire delay_Q2;
  wire delay_Q3;
  wire delay_Q4;
  wire delay_Q5;
  wire delay_Q6;
  wire delay_Q7;
  wire delay_Q8;
  wire delay_SHIFTOUT1;
  wire delay_SHIFTOUT2;

  wire delay_BITSLIP,BITSLIP_in;
  wire delay_CE1,CE1_in;
  wire delay_CE2,CE2_in;
  wire delay_CLK,CLK_inv,CLK_in;
  wire delay_CLKB,CLKB_inv,CLKB_in;
  wire delay_CLKDIV,CLKDIV_inv,CLKDIV_in;
  wire delay_CLKDIVP,CLKDIVP_inv,CLKDIVP_in;
  wire delay_D,D_inv,D_in;
  wire delay_DDLY,DDLY_in;
  wire delay_DYNCLKDIVSEL,DYNCLKDIVSEL_in;
  wire delay_DYNCLKSEL,DYNCLKSEL_in;
  wire delay_OCLK,OCLK_inv,OCLK_in;
  wire delay_OCLKB,OCLKB_inv,OCLKB_in;
  wire delay_OFB,OFB_in;
  wire delay_RST,RST_in;
  wire delay_SHIFTIN1,SHIFTIN1_in;
  wire delay_SHIFTIN2,SHIFTIN2_in;

  assign #(out_delay) O_OUT = o_out;
  assign #(out_delay) Q1_OUT = delay_Q1;
  assign #(out_delay) Q2_OUT = delay_Q2;
  assign #(out_delay) Q3_OUT = delay_Q3;
  assign #(out_delay) Q4_OUT = delay_Q4;
  assign #(out_delay) Q5_OUT = delay_Q5;
  assign #(out_delay) Q6_OUT = delay_Q6;
  assign #(out_delay) Q7_OUT = delay_Q7;
  assign #(out_delay) Q8_OUT = delay_Q8;
  assign #(out_delay) SHIFTOUT1_OUT = delay_SHIFTOUT1;
  assign #(out_delay) SHIFTOUT2_OUT = delay_SHIFTOUT2;

`ifndef XIL_TIMING // unisim
  assign #(in_delay) delay_BITSLIP = BITSLIP;
  assign #(in_delay) delay_CE1 = CE1;
  assign #(in_delay) delay_CE2 = CE2;
  assign #(INCLK_DELAY) delay_CLK = CLK;
  assign #(INCLK_DELAY) delay_CLKB = CLKB;
  assign #(INCLK_DELAY) delay_CLKDIV = CLKDIV;
  assign #(INCLK_DELAY) delay_CLKDIVP = CLKDIVP;
  assign #(in_delay) delay_D = D;
  assign #(in_delay) delay_DDLY = DDLY;
  assign #(in_delay) delay_DYNCLKDIVSEL = DYNCLKDIVSEL;
  assign #(in_delay) delay_DYNCLKSEL = DYNCLKSEL;
  assign #(INCLK_DELAY) delay_OCLK = OCLK;
  assign #(INCLK_DELAY) delay_OCLKB = OCLKB;
  assign #(in_delay) delay_OFB = OFB;
  assign #(in_delay) delay_RST = RST;
  assign #(in_delay) delay_SHIFTIN1 = SHIFTIN1;
  assign #(in_delay) delay_SHIFTIN2 = SHIFTIN2;
`endif //  `ifndef XIL_TIMING

`ifdef XIL_TIMING //Simprim
  assign delay_DYNCLKDIVSEL = DYNCLKDIVSEL;
  assign delay_DYNCLKSEL = DYNCLKSEL;
  assign delay_OCLK = OCLK;
  assign delay_OCLKB = OCLKB;
  assign delay_SHIFTIN1 = SHIFTIN1;
  assign delay_SHIFTIN2 = SHIFTIN2;
`endif

//`ifdef XIL_TIMING //Simprim
  assign BITSLIP_in = delay_BITSLIP;
  assign CE1_in = delay_CE1;
  assign CE2_in = delay_CE2;
  assign DDLY_in = delay_DDLY;
  assign DYNCLKDIVSEL_in = delay_DYNCLKDIVSEL;
  assign DYNCLKSEL_in = delay_DYNCLKSEL;
  assign OFB_in = delay_OFB;
  assign RST_in = delay_RST;
  assign SHIFTIN1_in = delay_SHIFTIN1;
  assign SHIFTIN2_in = delay_SHIFTIN2;
//`endif
  assign CLK_in = IS_CLK_INVERTED ^ delay_CLK;
  assign CLKB_in = IS_CLKB_INVERTED ^ delay_CLKB;
  assign CLKDIV_in = IS_CLKDIV_INVERTED ^ delay_CLKDIV;
  assign CLKDIVP_in = IS_CLKDIVP_INVERTED ^ delay_CLKDIVP;
  assign D_in = IS_D_INVERTED ^ delay_D;
  assign OCLK_in = IS_OCLK_INVERTED ^ delay_OCLK;
  assign OCLKB_in = IS_OCLKB_INVERTED ^ delay_OCLKB;

  assign #(INCLK_DELAY) CLKB_INDELAY = CLKB_IN;
  assign #(INCLK_DELAY) CLKDIVP_INDELAY = CLKDIVP_IN;
  assign #(INCLK_DELAY) CLKDIV_INDELAY = CLKDIV_IN;
  assign #(INCLK_DELAY) CLK_INDELAY = CLK_IN;
  assign #(INCLK_DELAY) OCLKB_INDELAY = OCLKB_IN;
  assign #(INCLK_DELAY) OCLK_INDELAY = OCLK_IN;

  assign #(in_delay) BITSLIP_INDELAY = BITSLIP_IN;
  assign #(in_delay) CE1_INDELAY = CE1_IN;
  assign #(in_delay) CE2_INDELAY = CE2_IN;
  assign #(in_delay) DDLY_INDELAY = DDLY_IN;
  assign #(in_delay) DYNCLKDIVSEL_INDELAY = DYNCLKDIVSEL_IN;
  assign #(in_delay) DYNCLKSEL_INDELAY = DYNCLKSEL_IN;
  assign #(in_delay) D_INDELAY = D_IN;
  assign #(in_delay) OFB_INDELAY = OFB_IN;
  assign #(in_delay) RST_INDELAY = RST_IN;
  assign #(in_delay) SHIFTIN1_INDELAY = SHIFTIN1_IN;
  assign #(in_delay) SHIFTIN2_INDELAY = SHIFTIN2_IN;
  assign delay_DYNCLKDIVSEL = DYNCLKDIVSEL_INDELAY;
  assign delay_DYNCLKSEL = DYNCLKSEL_INDELAY;
//  assign delay_OCLK = OCLK_INDELAY;
//  assign delay_OCLKB = OCLKB_INDELAY;
//  assign delay_RST = RST_INDELAY;
//  assign delay_SHIFTIN1 = SHIFTIN1_INDELAY;
//  assign delay_SHIFTIN2 = SHIFTIN2_INDELAY;

//----------------------------------------------------------
//------------------------- TASKS --------------------------
//----------------------------------------------------------
    task INTERFACE_TYPE_msg;
         begin
            $display("DRC  Warning : The combination of INTERFACE_TYPE, DATA_RATE and DATA_WIDTH values on instance %m is not recommended.\n");
            $display("The current settings are : INTERFACE_TYPE = %s, DATA_RATE = %s and DATA_WIDTH = %d\n", INTERFACE_TYPE, DATA_RATE, DATA_WIDTH);
            $display("The recommended combinations of values are :\n");
            $display("NETWORKING SDR 2, 3, 4, 5, 6, 7, 8\n");
            $display("NETWORKING DDR 4, 6, 8, 10, 14\n");
            $display("MEMORY DDR 4\n");
         end
    endtask // INTERFACE_TYPE_msg

    task OVERSAMPLE_DDR_SDR_msg;
         begin
            $display("DRC  Warning : The combination of INTERFACE_TYPE, DATA_RATE and DATA_WIDTH values on instance %m is not recommended.\n");
            $display("The current settings are : INTERFACE_TYPE = %s, DATA_RATE = %s and DATA_WIDTH = %d\n", INTERFACE_TYPE, DATA_RATE, DATA_WIDTH);
            $display("The recommended combinations of values are :\n");
            $display("OVERSAMPLE SDR 4\n");
            $display("OVERSAMPLE DDR 4\n");
         end
    endtask // OVERSAMPLE_DDR_SDR_msg

//----------------------------------------------------------
//------------------ Parameter Checks ----------------------
//----------------------------------------------------------
    initial begin
//-------------------------------------------------
//----- DATA_RATE check
//-------------------------------------------------
        case (DATA_RATE)
            "SDR", "DDR" :;
            default : begin
                          $display("Attribute Syntax Error : The attribute DATA_RATE on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are SDR or DDR", DATA_RATE);
                          #1 $finish;
                      end
        endcase // case(DATA_RATE)

//-------------------------------------------------
//----- DATA_WIDTH check
//-------------------------------------------------
        case (DATA_WIDTH)

            2, 3, 4, 5, 6, 7, 8, 10, 14 :;
            default : begin
                          $display("Attribute Syntax Error : The attribute DATA_WIDTH on ISERDESE2 instance %m is set to %d.  Legal values for this attribute are 2, 3, 4, 5, 6, 7, 8, 10 or 14", DATA_WIDTH);
                          #1 $finish;
                      end
        endcase // case(DATA_WIDTH)


//-------------------------------------------------
//----- DYN_CLKDIV_INV_EN check
//-------------------------------------------------
        case (DYN_CLKDIV_INV_EN)

            "TRUE", "FALSE" :;
            default : begin
                          $display("Attribute Syntax Error : The attribute DYN_CLKDIV_INV_EN on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE", DYN_CLKDIV_INV_EN);
                          #1 $finish;
                      end

        endcase // case(DYN_CLKDIV_INV_EN)

//-------------------------------------------------
//----- DYN_CLK_INV_EN check
//-------------------------------------------------
        case (DYN_CLK_INV_EN)

            "TRUE", "FALSE" :;
            default : begin
                          $display("Attribute Syntax Error : The attribute DYN_CLK_INV_EN on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE", DYN_CLK_INV_EN);
                          #1 $finish;
                      end

        endcase // case(DYN_CLK_INV_EN)

//-------------------------------------------------
//----- IOBDELAY check
//-------------------------------------------------
        case (IOBDELAY)
            "NONE", "IBUF", "IFD", "BOTH" :;
            default : begin
                          $display("Attribute Syntax Error : The attribute IOBDELAY on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are NONE, IBUF, IFD or BOTH", IOBDELAY);
                          #1 $finish;
                      end
        endcase // case(IOBDELAY)

//-------------------------------------------------
//----- OFB_USED check
//-------------------------------------------------
        case (OFB_USED)

            "TRUE", "FALSE" :;
            default : begin
                          $display("Attribute Syntax Error : The attribute OFB_USED on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are FALSE or TRUE", OFB_USED);
                          #1 $finish;
                      end

        endcase // case(OFB_USED)
//-------------------------------------------------
//----- NUM_CE check
//-------------------------------------------------
        case (NUM_CE)

            1, 2 :;
            default : begin
                          $display("Attribute Syntax Error : The attribute NUM_CE on ISERDESE2 instance %m is set to %d.  Legal values for this attribute are 1 or 2", NUM_CE);
                          #1 $finish;
                      end

        endcase // case(NUM_CE)


//-------------------------------------------------
//----- INTERFACE_TYPE check
//-------------------------------------------------
        case (INTERFACE_TYPE)
               "MEMORY" : begin
                        case(DATA_RATE)
                             "DDR" :
                                   case(DATA_WIDTH)
                                       4 : ;
                                       default :   INTERFACE_TYPE_msg;
                                   endcase // DATA_WIDTH
                             default :  INTERFACE_TYPE_msg;
                        endcase // DATA_RATE
               end
               "NETWORKING" : begin
                        case(DATA_RATE)
                             "SDR" :
                                   case(DATA_WIDTH)
                                       2, 3, 4, 5, 6, 7, 8 : ;
                                       default :  INTERFACE_TYPE_msg;
                                   endcase // DATA_WIDTH
                             "DDR" :
                                   case(DATA_WIDTH)
                                       4, 6, 8, 10, 14 : ;
                                       default :   INTERFACE_TYPE_msg;
                                   endcase // DATA_WIDTH
                             default :  ;
                        endcase // DATA_RATE
               end  
               "MEMORY_DDR3" :;
               "MEMORY_QDR" :;
               "OVERSAMPLE" : begin
                        case(DATA_RATE)
                             "SDR" : 
			           case(DATA_WIDTH)
                                       4 : ;
                                       default :   OVERSAMPLE_DDR_SDR_msg;
                                   endcase // DATA_WIDTH
                             "DDR" :
                                   case(DATA_WIDTH)
                                       4 : ;
                                       default :   OVERSAMPLE_DDR_SDR_msg;
                                   endcase // DATA_WIDTH
                             default :  ;
                        endcase // DATA_RATE
               end

               default : begin
                          $display("Attribute Syntax Error : The attribute INTERFACE_TYPE on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are MEMORY, NETWORKING, MEMORY_QDR, MEMORY_DDR3 or OVERSAMPLE", INTERFACE_TYPE);
                          #1 $finish;
                         end
        endcase // INTERFACE_TYPE

//-------------------------------------------------
//----- SERDES_MODE check
//-------------------------------------------------
        case (SERDES_MODE)
                "MASTER", "SLAVE" :;
                default  : begin
                          $display("Attribute Syntax Error : The attribute SERDES_MODE on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are MASTER or SLAVE", SERDES_MODE);
                          #1 $finish;
                          end
         endcase // case(SERDES_MODE)

    end  // initial begin



// CR 574021
//-------------------------------------------------
//   Input to ISERDES
//-------------------------------------------------

    always @(D_in or DDLY_in) begin

        case (IOBDELAY)

            "NONE" : begin
                         o_out_pre_fb   <= D_in;
                         o_delay_pre_fb <= D_in;

                     end
            "IBUF" : begin
                         o_out_pre_fb   <= DDLY_in;
                         o_delay_pre_fb <= D_in;
                     end
            "IFD"  : begin
                         o_out_pre_fb   <= D_in;
                         o_delay_pre_fb <= DDLY_in;
                     end
            "BOTH" : begin
                         o_out_pre_fb   <= DDLY_in;
                         o_delay_pre_fb <= DDLY_in;
                     end
            default : begin
                          $display("Attribute Syntax Error : The attribute IOBDELAY on ISERDESE2 instance %m is set to %s.  Legal values for this attribute are NONE, IBUF, IFD or BOTH", IOBDELAY);
                          $finish;
                      end

        endcase // case(IOBDELAY)

    end // always @ (D_in or DDLY_in)

    generate
      case (OFB_USED)
         "TRUE"  : always @(OFB_in)
                      begin
                         o_out   <= OFB_in;
                         data_in <= OFB_in;
                       end
         "FALSE" : begin
                      always @(o_out_pre_fb)    o_out   <= o_out_pre_fb;
                      always @(o_delay_pre_fb)  data_in <= o_delay_pre_fb;
                   end
      endcase
    endgenerate

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
  B_ISERDESE2 #(
    .DATA_RATE (DATA_RATE),
    .DATA_WIDTH (DATA_WIDTH),
    .DYN_CLKDIV_INV_EN (DYN_CLKDIV_INV_EN),
    .DYN_CLK_INV_EN (DYN_CLK_INV_EN),
    .INIT_Q1 (INIT_Q1),
    .INIT_Q2 (INIT_Q2),
    .INIT_Q3 (INIT_Q3),
    .INIT_Q4 (INIT_Q4),
    .INTERFACE_TYPE (INTERFACE_TYPE),
    .IOBDELAY (IOBDELAY),
    .NUM_CE (NUM_CE),
    .OFB_USED (OFB_USED),
    .SERDES_MODE (SERDES_MODE),
    .SRVAL_Q1 (SRVAL_Q1),
    .SRVAL_Q2 (SRVAL_Q2),
    .SRVAL_Q3 (SRVAL_Q3),
    .SRVAL_Q4 (SRVAL_Q4))

    B_ISERDESE2_INST (
    .O (delay_O),
    .Q1 (delay_Q1),
    .Q2 (delay_Q2),
    .Q3 (delay_Q3),
    .Q4 (delay_Q4),
    .Q5 (delay_Q5),
    .Q6 (delay_Q6),
    .Q7 (delay_Q7),
    .Q8 (delay_Q8),
    .SHIFTOUT1 (delay_SHIFTOUT1),
    .SHIFTOUT2 (delay_SHIFTOUT2),
    .BITSLIP (BITSLIP_in),
    .CE1 (CE1_in),
    .CE2 (CE2_in),
    .CLK (CLK_in),
    .CLKB (CLKB_in),
    .CLKDIV (CLKDIV_in),
    .CLKDIVP (CLKDIVP_in),
    .D (data_in),
    .DDLY (DDLY_in),
    .DYNCLKDIVSEL (DYNCLKDIVSEL_in),
    .DYNCLKSEL (DYNCLKSEL_in),
    .OCLK (OCLK_in),
    .OCLKB (OCLKB_in),
    .OFB (OFB_in),
    .RST (RST_in),
    .SHIFTIN1 (SHIFTIN1_in),
    .SHIFTIN2 (SHIFTIN2_in),
    .GSR(GSR)
  );

`ifdef XIL_TIMING
   wire clk_en_n;
   wire clk_en_p;
   wire clkb_en_n;
   wire clkb_en_p;
   wire clkdiv_en_p;
   wire clkdiv_en_n;
   wire clkdivp_en_n;
   wire clkdivp_en_p;
   assign clk_en_n = IS_CLK_INVERTED;
   assign clk_en_p = ~IS_CLK_INVERTED;
   assign clkb_en_n = IS_CLKB_INVERTED;
   assign clkb_en_p = ~IS_CLKB_INVERTED;
   assign clkdiv_en_n = IS_CLKDIV_INVERTED;
   assign clkdiv_en_p = ~IS_CLKDIV_INVERTED;
   assign clkdivp_en_n = IS_CLKDIVP_INVERTED;
   assign clkdivp_en_p = ~IS_CLKDIVP_INVERTED;

`endif
  specify
`ifdef XIL_TIMING // Simprim
    $period (negedge CLK, 0:0:0, notifier);
    $period (negedge CLKB, 0:0:0, notifier);
    $period (negedge CLKDIV, 0:0:0, notifier);
    $period (negedge CLKDIVP, 0:0:0, notifier);
    $period (posedge CLK, 0:0:0, notifier);
    $period (posedge CLKB, 0:0:0, notifier);
    $period (posedge CLKDIV, 0:0:0, notifier);
    $period (posedge CLKDIVP, 0:0:0, notifier);
    
    $setuphold (posedge CLK, negedge CE1, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_CE1);
    $setuphold (posedge CLK, negedge D, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_D);
    $setuphold (posedge CLK, negedge DDLY, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_DDLY);
    $setuphold (posedge CLK, negedge OFB, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_OFB);
    $setuphold (posedge CLK, posedge CE1, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_CE1);
    $setuphold (posedge CLK, posedge D, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_D);
    $setuphold (posedge CLK, posedge DDLY, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_DDLY);
    $setuphold (posedge CLK, posedge OFB, 0:0:0, 0:0:0, notifier, clk_en_p, clk_en_p, delay_CLK, delay_OFB);
    $setuphold (posedge CLKB, negedge CE1, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_CE1);
    $setuphold (posedge CLKB, negedge D, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_D);
    $setuphold (posedge CLKB, negedge DDLY, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_DDLY);
    $setuphold (posedge CLKB, negedge OFB, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_OFB);
    $setuphold (posedge CLKB, posedge CE1, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_CE1);
    $setuphold (posedge CLKB, posedge D, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_D);
    $setuphold (posedge CLKB, posedge DDLY, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_DDLY);
    $setuphold (posedge CLKB, posedge OFB, 0:0:0, 0:0:0, notifier, clkb_en_p, clkb_en_p, delay_CLKB, delay_OFB);
    $setuphold (posedge CLKDIV, negedge BITSLIP, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_BITSLIP);
    $setuphold (posedge CLKDIV, negedge CE1, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_CE1);
    $setuphold (posedge CLKDIV, negedge CE2, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_CE2);
    $setuphold (posedge CLKDIV, posedge BITSLIP, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_BITSLIP);
    $setuphold (posedge CLKDIV, posedge CE1, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_CE1);
    $setuphold (posedge CLKDIV, posedge CE2, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_CE2);
    $setuphold (posedge CLKDIVP, negedge CE1, 0:0:0, 0:0:0, notifier, clkdivp_en_p, clkdivp_en_p, delay_CLKDIVP, delay_CE1);
    $setuphold (posedge CLKDIVP, negedge CE2, 0:0:0, 0:0:0, notifier, clkdivp_en_p, clkdivp_en_p, delay_CLKDIVP, delay_CE2);
    $setuphold (posedge CLKDIVP, posedge CE1, 0:0:0, 0:0:0, notifier, clkdivp_en_p, clkdivp_en_p, delay_CLKDIVP, delay_CE1);
    $setuphold (posedge CLKDIVP, posedge CE2, 0:0:0, 0:0:0, notifier, clkdivp_en_p, clkdivp_en_p, delay_CLKDIVP, delay_CE2);
    $setuphold (posedge CLKDIV, negedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_RST);
    $setuphold (posedge CLKDIV, posedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_p, clkdiv_en_p, delay_CLKDIV, delay_RST);
    $setuphold (posedge CLKDIVP, negedge RST, 0:0:0, 0:0:0, notifier, clkdivp_en_p, clkdivp_en_p, delay_CLKDIVP, delay_RST);
    $setuphold (posedge CLKDIVP, posedge RST, 0:0:0, 0:0:0, notifier, clkdivp_en_p, clkdivp_en_p, delay_CLKDIVP, delay_RST);

    $setuphold (negedge CLK, negedge CE1, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_CE1);
    $setuphold (negedge CLK, negedge D, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_D);
    $setuphold (negedge CLK, negedge DDLY, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_DDLY);
    $setuphold (negedge CLK, negedge OFB, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_OFB);
    $setuphold (negedge CLK, posedge CE1, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_CE1);
    $setuphold (negedge CLK, posedge D, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_D);
    $setuphold (negedge CLK, posedge DDLY, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_DDLY);
    $setuphold (negedge CLK, posedge OFB, 0:0:0, 0:0:0, notifier, clk_en_n, clk_en_n, delay_CLK, delay_OFB);
    $setuphold (negedge CLKB, negedge CE1, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_CE1);
    $setuphold (negedge CLKB, negedge D, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_D);
    $setuphold (negedge CLKB, negedge DDLY, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_DDLY);
    $setuphold (negedge CLKB, negedge OFB, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_OFB);
    $setuphold (negedge CLKB, posedge CE1, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_CE1);
    $setuphold (negedge CLKB, posedge D, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_D);
    $setuphold (negedge CLKB, posedge DDLY, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_DDLY);
    $setuphold (negedge CLKB, posedge OFB, 0:0:0, 0:0:0, notifier, clkb_en_n, clkb_en_n, delay_CLKB, delay_OFB);
    $setuphold (negedge CLKDIV, negedge BITSLIP, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_BITSLIP);
    $setuphold (negedge CLKDIV, negedge CE1, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_CE1);
    $setuphold (negedge CLKDIV, negedge CE2, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_CE2);
    $setuphold (negedge CLKDIV, posedge BITSLIP, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_BITSLIP);
    $setuphold (negedge CLKDIV, posedge CE1, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_CE1);
    $setuphold (negedge CLKDIV, posedge CE2, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_CE2);
    $setuphold (negedge CLKDIVP, negedge CE1, 0:0:0, 0:0:0, notifier, clkdivp_en_n, clkdivp_en_n, delay_CLKDIVP, delay_CE1);
    $setuphold (negedge CLKDIVP, negedge CE2, 0:0:0, 0:0:0, notifier, clkdivp_en_n, clkdivp_en_n, delay_CLKDIVP, delay_CE2);
    $setuphold (negedge CLKDIVP, posedge CE1, 0:0:0, 0:0:0, notifier, clkdivp_en_n, clkdivp_en_n, delay_CLKDIVP, delay_CE1);
    $setuphold (negedge CLKDIVP, posedge CE2, 0:0:0, 0:0:0, notifier, clkdivp_en_n, clkdivp_en_n, delay_CLKDIVP, delay_CE2);
    $setuphold (negedge CLKDIV, negedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_RST);
    $setuphold (negedge CLKDIV, posedge RST, 0:0:0, 0:0:0, notifier, clkdiv_en_n, clkdiv_en_n, delay_CLKDIV, delay_RST);
    $setuphold (negedge CLKDIVP, negedge RST, 0:0:0, 0:0:0, notifier, clkdivp_en_n, clkdivp_en_n, delay_CLKDIVP, delay_RST);
    $setuphold (negedge CLKDIVP, posedge RST, 0:0:0, 0:0:0, notifier, clkdivp_en_n, clkdivp_en_n, delay_CLKDIVP, delay_RST);


    
`endif
    ( CLK => Q1) = (100:100:100, 100:100:100);
    ( CLK => Q2) = (100:100:100, 100:100:100);
    ( CLK => Q3) = (100:100:100, 100:100:100);
    ( CLK => Q4) = (100:100:100, 100:100:100);
    ( CLK => Q5) = (100:100:100, 100:100:100);
    ( CLK => Q6) = (100:100:100, 100:100:100);
    ( CLK => Q7) = (100:100:100, 100:100:100);
    ( CLK => Q8) = (100:100:100, 100:100:100);
    ( CLKDIV => Q1) = (100:100:100, 100:100:100);
    ( CLKDIV => Q2) = (100:100:100, 100:100:100);
    ( CLKDIV => Q3) = (100:100:100, 100:100:100);
    ( CLKDIV => Q4) = (100:100:100, 100:100:100);
    ( CLKDIV => Q5) = (100:100:100, 100:100:100);
    ( CLKDIV => Q6) = (100:100:100, 100:100:100);
    ( CLKDIV => Q7) = (100:100:100, 100:100:100);
    ( CLKDIV => Q8) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q1) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q2) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q3) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q4) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q5) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q6) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q7) = (100:100:100, 100:100:100);
    ( CLKDIVP => Q8) = (100:100:100, 100:100:100);
    ( D => O) = (100:100:100, 100:100:100);
    ( DDLY => O) = (100:100:100, 100:100:100);
    ( OFB => O) = (100:100:100, 100:100:100);

    specparam PATHPULSE$ = 0;
  endspecify

endmodule

`endcelldefine
