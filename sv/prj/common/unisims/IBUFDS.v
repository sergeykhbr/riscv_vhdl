///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2004 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 10.1
//  \   \         Description : Xilinx Unified Simulation Library Component
//  /   /                  Differential Signaling Input Buffer
// /___/   /\     Filename : IBUFDS.v
// \   \  /  \
//  \___\/\___\
//
// Revision:
//    03/23/04 - Initial version.
//    03/11/05 - Add LOC paramter;
//    07/21/05 - CR 212974 -- matched unisim parameters as requested by other tools
//    07/19/06 - Add else to handle x case for o_out (CR 234718).
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
//    07/13/12 - 669215 - add parameter DQS_BIAS
//    08/29/12 - 675511 - add DQS_BIAS functionality
//    09/11/12 - 677753 - remove X glitch on O
//    10/22/14 - Added #1 to $finish (CR 808642).
// End Revision

`timescale 1 ps / 1 ps

`celldefine


module IBUFDS (O, I, IB);

`ifdef XIL_TIMING
  parameter LOC = "UNPLACED";
`endif
  parameter CAPACITANCE = "DONT_CARE";
  parameter DIFF_TERM = "FALSE";
  parameter DQS_BIAS = "FALSE";
  parameter IBUF_DELAY_VALUE = "0";
  parameter IBUF_LOW_PWR = "TRUE";
  parameter IFD_DELAY_VALUE = "AUTO";
  parameter IOSTANDARD = "DEFAULT";

  localparam MODULE_NAME = "IBUFDS";

    output O;
    input  I, IB;

    wire i_in, ib_in;
    reg o_out;
    reg DQS_BIAS_BINARY = 1'b0;

    assign O = o_out;

    assign i_in = I;
    assign ib_in = IB;

    initial begin
   
        case (DQS_BIAS)

            "TRUE"  : DQS_BIAS_BINARY <= #1 1'b1;
            "FALSE" : DQS_BIAS_BINARY <= #1 1'b0;
            default : begin
                          $display("Attribute Syntax Error : The attribute DQS_BIAS on %s instance %m is set to %s.  Legal values for this attribute are TRUE or FALSE.", MODULE_NAME, DQS_BIAS);
                          #1 $finish;
                      end

        endcase

        case (CAPACITANCE)

            "LOW", "NORMAL", "DONT_CARE" : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute CAPACITANCE on %s instance %m is set to %s.  Legal values for this attribute are DONT_CARE, LOW or NORMAL.", MODULE_NAME, CAPACITANCE);
                          #1 $finish;
                      end

        endcase

   case (DIFF_TERM)

            "TRUE", "FALSE" : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute DIFF_TERM on %s instance %m is set to %s.  Legal values for this attribute are TRUE or FALSE.", MODULE_NAME, DIFF_TERM);
                          #1 $finish;
                      end

   endcase // case(DIFF_TERM)


   case (IBUF_DELAY_VALUE)

            "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute IBUF_DELAY_VALUE on %s instance %m is set to %s.  Legal values for this attribute are 0, 1, 2, ... or 16.", MODULE_NAME, IBUF_DELAY_VALUE);
                          #1 $finish;
                      end

        endcase

        case (IBUF_LOW_PWR)

            "FALSE", "TRUE" : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute IBUF_LOW_PWR on %s instance %m is set to %s.  Legal values for this attribute are TRUE or FALSE.", MODULE_NAME, IBUF_LOW_PWR);
                          #1 $finish;
                      end

        endcase

   case (IFD_DELAY_VALUE)

            "AUTO", "0", "1", "2", "3", "4", "5", "6", "7", "8" : ;
            default : begin
                          $display("Attribute Syntax Error : The attribute IFD_DELAY_VALUE on %s instance %m is set to %s.  Legal values for this attribute are AUTO, 0, 1, 2, ... or 8.", MODULE_NAME, IFD_DELAY_VALUE);
                          #1 $finish;
                      end

   endcase

end

    always @(i_in or ib_in or DQS_BIAS_BINARY) begin
        if (i_in == 1'b1 && ib_in == 1'b0)
          o_out <= 1'b1;
        else if (i_in == 1'b0 && ib_in == 1'b1)
          o_out <= 1'b0;
        else if ((i_in === 1'bz || i_in == 1'b0) && (ib_in === 1'bz || ib_in == 1'b1))
          if (DQS_BIAS_BINARY == 1'b1)
            o_out <= 1'b0;
          else
            o_out <= 1'bx;
        else if ((i_in === 1'bx) || (ib_in === 1'bx))
          o_out <= 1'bx;
        end
    
`ifdef XIL_TIMING
  specify
    (I => O) = (0:0:0, 0:0:0);
    (IB => O) = (0:0:0, 0:0:0);
    specparam PATHPULSE$ = 0;
  endspecify
`endif
endmodule

`endcelldefine
