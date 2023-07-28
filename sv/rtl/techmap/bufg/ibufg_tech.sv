//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual clock buffered output.
//----------------------------------------------------------------------------

module ibufg_tech (
    output logic o,
    input i
);

`ifdef TARGET_INFERRED

    assign o = i;

`elsif TARGET_KC705
   
    BUFG x0(.O(o), .I(i));
       
`else
    $error("IBUFG_INSTANCE macro is undefined, check technology-dependent buffers.");
       
`endif
   

endmodule
