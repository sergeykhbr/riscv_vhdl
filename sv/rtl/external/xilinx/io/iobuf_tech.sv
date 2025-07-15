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

    IOBUF #(.DRIVE(12), .IOSTANDARD("DEFAULT"), .SLEW("SLOW")) io_inst (.O(o), .IO(io), .I(i), .T(t));
    
endmodule
