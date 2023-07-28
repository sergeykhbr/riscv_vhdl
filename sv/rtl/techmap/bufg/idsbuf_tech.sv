//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual input buffer with the differential signals.
//--------------------------------------------------------------------------

module idsbuf_tech (
    input clk_p,
    input clk_n,
    output o_clk
);
 
`ifdef TARGET_INFERRED

    assign o_clk = clk_p;

`elsif TARGET_KC705
   
    IBUFDS x1(.I(clk_p), .IB(clk_n), .O(o_clk));
       
`else
   
    $error("IDSBUF_INSTANCE macro is undefined, check technology-dependent buffers.");
       
`endif

endmodule
