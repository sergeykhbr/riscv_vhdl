module reset_global (

   input i_clk,
   input i_pwrreset,   // external button reset
   input i_dmreset,    // reset from DMI (debug) interface.
   output logic o_sys_nrst,   // reset whole system
   output logic o_dbg_nrst   // reset dmi interface
);

logic [7 : 0] r_delay_syscnt;
logic [7 : 0] rin_delay_syscnt;

logic [7 : 0] r_delay_dbgcnt;
logic [7 : 0] rin_delay_dbgcnt;
  
always_comb begin : main_proc
  logic [7 : 0] v_delay_syscnt;
  logic [7 : 0] v_delay_dbgcnt;

  v_delay_syscnt = r_delay_syscnt;
  if (r_delay_syscnt[7] == 1'b0) begin
    v_delay_syscnt = r_delay_syscnt + 1;
  end

  v_delay_dbgcnt = r_delay_dbgcnt;
  if (r_delay_dbgcnt[7] == 1'b0) begin
    v_delay_dbgcnt = r_delay_dbgcnt + 1;
  end
  
  rin_delay_syscnt = v_delay_syscnt;
  rin_delay_dbgcnt = v_delay_dbgcnt;
 end : main_proc
 
assign o_sys_nrst = r_delay_syscnt[7];
assign o_dbg_nrst = r_delay_dbgcnt[7];

always_ff@(posedge i_clk or posedge i_pwrreset) begin
    if ((i_pwrreset | i_dmreset) == 1'b1) begin
        r_delay_syscnt <= '0;
    end else begin
        r_delay_syscnt <= rin_delay_syscnt;
    end

    if (i_pwrreset == 1'b1) begin
        r_delay_dbgcnt <= '0;
    end else begin
        r_delay_dbgcnt <= rin_delay_dbgcnt;
    end
end

endmodule
