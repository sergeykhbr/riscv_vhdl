//--------------------------------------------------------------------------
//! @author     Sergey Khabarov
//! @brief      Virtual IO buffer.
//--------------------------------------------------------------------------

module iobuf_tech (

    output logic o,
    inout logic io,
    input i,
    input t
    
);

import config_target_pkg::*;

`ifdef TARGET_INFERRED

   assign o = t ? io : 1'b0;
   assign io = t ? 1'bz : i;

`elsif TARGET_KC705

    IOBUF #(.DRIVE(12), .IOSTANDARD("DEFAULT"), .SLEW("SLOW")) io_inst (.O(o), .IO(io), .I(i), .T(t));
    
`else
   
    $error("IOBUF_INSTANCE macro is undefined, check technology-dependent buffers.");
   
`endif

endmodule
