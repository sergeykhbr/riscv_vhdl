///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.4
//  \   \         Description : Xilinx Formal Library Component
//  /   /                  3-State Diffential Signaling I/O Buffer
// /___/   /\     Filename : IOBUFDS_DIFF_OUT_DCIEN.v
// \   \  /  \
//  \___\/\___\
//
///////////////////////////////////////////////////////////////////////////////
// Revision:
//    04/29/10 - Initial version.
//    07/24/14 - Added missing parameter (CR 806964).
//    10/20/14 - Removed b'x support (CR 817718).
// End Revision
///////////////////////////////////////////////////////////////////////////////

`timescale  1 ps / 1 ps

`celldefine

module IOBUFDS_DIFF_OUT_DCIEN (O, OB, IO, IOB, DCITERMDISABLE, I, IBUFDISABLE, TM, TS);

    parameter DIFF_TERM = "FALSE";
    parameter DQS_BIAS = "FALSE";
    parameter IBUF_LOW_PWR = "TRUE";
    parameter IOSTANDARD = "DEFAULT";
    parameter SIM_DEVICE = "7SERIES";
    parameter USE_IBUFDISABLE = "TRUE";
`ifdef XIL_TIMING
    parameter LOC = "UNPLACED";
`endif // `ifdef XIL_TIMING

    output O;
    output OB;
    inout  IO;
    inout  IOB;
    input  DCITERMDISABLE;
    input  I;
    input  IBUFDISABLE;
    input  TM;
    input  TS;

    wire t1, t2;
    wire T_OR_IBUFDISABLE_1;
    wire T_OR_IBUFDISABLE_2;


    or O1 (t1, TM);
    bufif0 B1 (IO, I, t1);

    or O2 (t2, TS);
    notif0 N2 (IOB, I, t2);

    reg O_int, OB_int;

    always @(IO or IOB) begin
        if (IO == 1'b1 && IOB == 1'b0) begin
            O_int  = IO;
            OB_int = ~IO;
        end
        else if (IO == 1'b0 && IOB == 1'b1) begin
            O_int  = IO;
            OB_int = ~IO;
        end
        else begin
            O_int  = 1'b0;
            OB_int = 1'b0;
        end
    end

    generate
       case (USE_IBUFDISABLE)
          "TRUE" :  begin
                       assign T_OR_IBUFDISABLE_1 = ~TM || IBUFDISABLE;
                       assign T_OR_IBUFDISABLE_2 = ~TS || IBUFDISABLE;
                       assign O = (T_OR_IBUFDISABLE_1 == 1'b1) ? 1'b1 : (T_OR_IBUFDISABLE_1 == 1'b0) ? O_int : 1'b0;
                       assign OB = (T_OR_IBUFDISABLE_2 == 1'b1) ? 1'b1 : (T_OR_IBUFDISABLE_2 == 1'b0) ? OB_int : 1'b0;
                    end
          "FALSE" : begin
                        assign O = O_int;
                        assign OB = OB_int;
                    end
       endcase
    endgenerate

endmodule

`endcelldefine
