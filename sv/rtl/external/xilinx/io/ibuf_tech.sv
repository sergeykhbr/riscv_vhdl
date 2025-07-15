//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual simple input buffer.
//----------------------------------------------------------------------------

module ibuf_tech(
    output logic o,
    input i
);

    BUF x0(.O(o), .I(i));
       
  
endmodule
