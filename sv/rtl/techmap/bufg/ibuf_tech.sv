//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual simple input buffer.
//----------------------------------------------------------------------------

module ibuf_tech(
    output logic o,
    input i
);

import config_target_pkg::*;

`ifdef TARGET_INFERRED

   assign o = i;

`elsif TARGET_KC705

    BUF x0(.O(o), .I(i));
       
`else
   
   $error("IBUF_INSTANCE macro is undefined, check technology-dependent buffers.");

`endif
   
endmodule
