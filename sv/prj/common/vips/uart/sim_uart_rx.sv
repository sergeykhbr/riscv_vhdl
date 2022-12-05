// UART simulation receiver
// Vladimir Domnin
// Aug 3 2018

module sim_uart_rx
#(
    parameter p_ext_clk = 0,
    parameter p_inst_num = 0,
    parameter p_uart_clk_half_period = 1.03ns
)
(
    input wire clk_in,
    input wire [31:0] scaler,
    input wire rx,
    input wire rst_n
);


/******************Localparams**************************/

    localparam PLUSARG_LOG = "UART_LOG_PATH";
    localparam DEFAULT_LOG_PATH = "uart";

/********************Variables**************************/

bit clk;
wire [7:0] rdata;
wire rdy;
bit rdy_clr;

string log_path;
string test_str;
int fd;
int fd_tmp;

string msg;
bit [7:0] chr;
bit skip;

/********************Processes**************************/
if (p_ext_clk == 0)
  always
  begin
    #p_uart_clk_half_period clk = ~clk;
  end
else
  assign clk = clk_in;

always @(posedge clk)
begin
    if(rdy_clr == 1'b0)
    begin
        if(rdy == 1'b1) rdy_clr <= 1'b1;
    end
    else
    begin
        rdy_clr <= 1'b0;
    end
end

initial
begin: main_proc
    test_str = {PLUSARG_LOG, "=%s"};
    assert($value$plusargs(test_str, log_path))
        else
        begin
            $display("UART log path is not set!\nUsing default log file:");
            log_path = $sformatf({DEFAULT_LOG_PATH, "_%1d.log"}, p_inst_num);
            $display("%s", log_path);
            $stop;
        end

    log_path = $sformatf({log_path, "_%1d.log"}, p_inst_num);
    fd       = $fopen(log_path, "wb");
    fd_tmp   = $fopen({log_path, ".tmp"}, "wb");

    assert(fd)
        else
        begin
            $display("Can't open UART log file!");
            $stop;
        end

    assert(fd_tmp)
        else
        begin
            $display("Can't open UART tmp log file!");
            $stop;
        end

    forever
    begin: main_loop
        @(posedge clk iff rdy === 1'b1);
        skip = 1'b0;
        if (chr == 8'h0A && rdata == 8'h0D) begin
            skip = 1'b0;
        end
        chr = rdata;
        msg = {msg, chr};
        if (skip == 1'b1) begin
        end else if(chr != 8'h0A) begin
            $fwrite(fd_tmp, "%s", {8'h0D, msg});
        end else begin
            $fwrite(fd_tmp, "%s\n", {8'h0D, msg});
            $fwrite(fd, "%s\n", msg);
            msg = "";
        end
        $fflush(fd_tmp);
        $fflush(fd);
        @(posedge clk iff rdy === 1'b0);
    end: main_loop
end: main_proc

/********************Instances**************************/
sim_uart_receiver
#(
    .p_ext_clk(p_ext_clk)
)
UART_RX
(
    .rx(rx),
    .rdy(rdy),
    .rdy_clr(rdy_clr),
    .clk_50m(clk),
    .clken(rst_n),
    .data(rdata),
    .scaler(scaler)
);

endmodule: sim_uart_rx


