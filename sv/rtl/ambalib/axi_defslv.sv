`timescale 1ns/10ps

module axi_defslv
#(
    parameter async_reset = 0
)
(
    input i_clk,
    input i_nrst,
    input types_amba_pkg::axi4_slave_in_type i_xslvi,
    output types_amba_pkg::axi4_slave_out_type o_xslvo
);

import types_amba_pkg::*;

typedef enum logic [1:0] {
    DefSlave_Idle,
    DefSlave_R,
    DefSlave_W,
    DefSlave_B
} defslv_state_type;

defslv_state_type rin_state, r_state;
logic [7:0] rin_burst_cnt, r_burst_cnt;

always_comb
begin: main_proc
    types_amba_pkg::axi4_slave_out_type vslvo;
    defslv_state_type v_state;
    logic [7:0] v_burst_cnt;

    v_state = r_state;
    v_burst_cnt = r_burst_cnt;
    vslvo = types_amba_pkg::axi4_slave_out_none;
    vslvo.b_resp = AXI_RESP_DECERR;
    vslvo.r_resp = AXI_RESP_DECERR;

    case (r_state)
    DefSlave_Idle: begin
        vslvo.ar_ready = 1'b1;
        vslvo.aw_ready = 1'b1;
        if (i_xslvi.aw_valid == 1'b1) begin
            vslvo.ar_ready = 1'b0;
            v_state = DefSlave_W;
            v_burst_cnt = i_xslvi.aw_bits.len;
        end else if (i_xslvi.ar_valid == 1'b1) begin
            v_state = DefSlave_R;
            v_burst_cnt = i_xslvi.ar_bits.len;
        end
    end
    DefSlave_R: begin
        vslvo.r_valid = 1'b1;
        vslvo.r_data = '1;
        if (r_burst_cnt == 8'h00) begin
           v_state = DefSlave_Idle;
           vslvo.r_last = 1'b1;
        end else begin
           v_burst_cnt = r_burst_cnt - 1;
        end
    end
    DefSlave_W: begin
        vslvo.w_ready = 1'b1;
        if (r_burst_cnt == 8'h00) begin
           v_state = DefSlave_B;
        end else begin
           v_burst_cnt = r_burst_cnt - 1;
        end
    end
    DefSlave_B: begin
        vslvo.b_valid = 1'b1;
        v_state = DefSlave_Idle;
    end
    endcase


    if (!async_reset && (i_nrst == 1'b0)) begin
        v_state = DefSlave_Idle;
        v_burst_cnt = 8'd0;
    end

    o_xslvo <= vslvo;
    rin_state <= v_state;
    rin_burst_cnt <= v_burst_cnt;

end: main_proc

 // registers
 generate
     if (async_reset)
         begin: async_rst_gen
             always_ff @(posedge i_clk or negedge i_nrst) begin: rg_proc
                 if( i_nrst == 1'b0 ) begin
                      r_state <= DefSlave_Idle;
                      r_burst_cnt <= rin_burst_cnt;
                 end else begin
                      r_state <= rin_state;
                      r_burst_cnt <= rin_burst_cnt;
                 end
             end: rg_proc
         end: async_rst_gen
     else begin: no_rst_gen
         always_ff @(posedge i_clk) begin: rg_proc
              r_state <= rin_state;
              r_burst_cnt <= rin_burst_cnt;
         end: rg_proc
     end: no_rst_gen
 endgenerate

endmodule: axi_defslv
