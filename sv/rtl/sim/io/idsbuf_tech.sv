//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual input buffer with the differential signals.
//--------------------------------------------------------------------------

module idsbuf_tech (
    input clk_p,
    input clk_n,
    output o_clk
);
 
    assign o_clk = clk_p;

endmodule
