//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual clock buffered output.
//----------------------------------------------------------------------------

module ibufg_tech (
    output logic o,
    input i
);

   
    BUFG x0(.O(o), .I(i));
       

endmodule
