//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual Gigabits buffer with the differential signals.
//--------------------------------------------------------------------------

module igdsbuf_tech (
    input gclk_p,
    input gclk_n,
    output logic o_clk
);
 

    assign o_clk = gclk_p;
  
endmodule
