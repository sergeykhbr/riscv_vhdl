module sim_uart_receiver
#(
    parameter p_ext_clk = 0
)
(
		input wire rx,
		output reg rdy,
		input wire rdy_clr,
		input wire clk_50m,
		input wire clken,
		output reg [7:0] data,
		input wire [31:0] scaler
);

initial begin
	rdy = 0;
	data = 8'b0;
end

parameter RX_STATE_START	= 2'b00;
parameter RX_STATE_DATA		= 2'b01;
parameter RX_STATE_STOP		= 2'b10;

reg [1:0] state = RX_STATE_START;
reg [31:0] sample = 0;
//reg [3:0] sample_max_default = 15;
wire [31:0] sample_max = (scaler << 1) - 1;
//reg [3:0] sample_mid_default = 8;
//reg [3:0] sample_mid = 5; // test
wire [31:0] sample_mid = scaler;
reg [3:0] bitpos = 0;
reg [7:0] scratch = 8'b0;

initial begin

end

always @(posedge clk_50m) begin
	if (rdy_clr)
		rdy <= 0;

	if (clken) begin
		case (state)
		RX_STATE_START: begin
			/*
			* Start counting from the first low sample, once we've
			* sampled a full bit, start collecting data bits.
			*/
			if (!rx || sample != 0)
				sample <= sample + 4'b1;

			if (sample == sample_max) begin
				state <= RX_STATE_DATA;
				bitpos <= 0;
				sample <= 0;
				scratch <= 0;
			end
		end
		RX_STATE_DATA: begin
			//sample <= sample + 4'b1;
         if (sample == sample_max) begin
            sample <= 0;
         end else begin
            sample <= sample + 1;
         end
			//if (sample == 4'h8) begin
			if (sample == sample_mid) begin
				scratch[bitpos[2:0]] <= rx;
				bitpos <= bitpos + 4'b1;
			end
			if (bitpos == 8 && sample == sample_max)
				state <= RX_STATE_STOP;
		end
		RX_STATE_STOP: begin
			/*
			 * Our baud clock may not be running at exactly the
			 * same rate as the transmitter.  If we thing that
			 * we're at least half way into the stop bit, allow
			 * transition into handling the next start bit.
			 */
			if (sample == sample_max || (sample >= sample_mid && !rx)) begin
				state <= RX_STATE_START;
				data <= scratch;
				rdy <= 1'b1;
				sample <= 0;
			end else begin
				sample <= sample + 4'b1;
			end
		end
		default: begin
			state <= RX_STATE_START;
		end
		endcase
	end
end

endmodule
