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

   assign o = t ? io : 1'b0;
   assign io = t ? 1'bz : i;

endmodule
