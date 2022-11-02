//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual simple output buffer.
//--------------------------------------------------------------------------

module obuf_tech (
    output logic o,
    input i
); 
 
import config_target_pkg::*;

`ifdef TARGET_INFERRED

    assign o = i;

`elsif TARGET_KC705
   
    OBUF x0(.O(o), .I(i));
       
`else
   
    $error("OBUF_INSTANCE macro is undefined, check technology-dependent buffers.");
       
`endif
   
endmodule
