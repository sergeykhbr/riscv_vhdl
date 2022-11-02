//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual Gigabits buffer with the differential signals.
//--------------------------------------------------------------------------

module igdsbuf_tech (
    input gclk_p,
    input gclk_n,
    output logic o_clk
);
 
import config_target_pkg::*;

`ifdef TARGET_INFERRED

    assign o_clk = gclk_p;

`elsif TARGET_KC705
   
    IBUFDS_GTE2 x1(.I(gclk_p), .IB(gclk_n), .CEB(1'b0), .O(o_clk), .ODIV2());
       
`else
   
    $error("IGDSBUF_INSTANCE macro is undefined, check technology-dependent buffers.");
       
`endif
  
endmodule
