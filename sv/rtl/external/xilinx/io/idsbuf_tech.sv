//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual input buffer with the differential signals.
//--------------------------------------------------------------------------

module idsbuf_tech (
    input clk_p,
    input clk_n,
    output o_clk
);
 
  
    IBUFDS x1(.I(clk_p), .IB(clk_n), .O(o_clk));
       
endmodule
