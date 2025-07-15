//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual simple output buffer.
//--------------------------------------------------------------------------

module obuf_tech (
    output logic o,
    input i
); 
 
  
    OBUF x0(.O(o), .I(i));
       
  
endmodule
