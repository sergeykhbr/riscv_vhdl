//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual Gigabits buffer with the differential signals.
//--------------------------------------------------------------------------

module igdsbuf_tech (
    input i_clk_p,
    input i_clk_n,
    output logic o_clk
);
 
    IBUFDS_GTE2 x1(.I(i_clk_p), .IB(i_clk_n), .CEB(1'b0), .O(o_clk), .ODIV2());
 
endmodule
